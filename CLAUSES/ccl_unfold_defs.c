/*-----------------------------------------------------------------------

File  : ccl_unfold_defs.c

Author: Stephan Schulz

Contents

  Functions for unfolding equational definitions.

Copyright 1998-2011 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed Aug 14 20:00:53 CEST 2002
    New

-----------------------------------------------------------------------*/

#include "ccl_unfold_defs.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: term_top_unfold_def()
//
//   If possible, return the term that results from applying the
//   demodulator at top position, otherwise return term.
//
// Global Variables: -
//
// Side Effects    : Changes term bank.
//
/----------------------------------------------------------------------*/

static Term_p term_top_unfold_def(TB_p bank, Term_p term, ClausePos_p demod)
{
   Term_p lside, rside, res;
   Subst_p subst;
   bool tmp;

   assert(bank&&term&&demod);

   lside = ClausePosGetSide(demod);
   assert(!TermIsVar(lside));
   if(lside->f_code != term->f_code)
   {
      return term;
   }
   subst = SubstAlloc();
   tmp = SubstComputeMatch(lside, term, subst);
   UNUSED(tmp); assert(tmp); /* Match must exist because demod is demod! */
   rside = ClausePosGetOtherSide(demod);
   res = TBInsertInstantiated(bank, rside);
   SubstDelete(subst);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: term_unfold_def()
//
//   Apply demod everywhere in term. One-traversal leftmost-innermost
//   is complete, because we know that the top symbol of the
//   demodulator cannot occur in demodulated terms. pos_stack is
//   intended to keep positions, at the moment it just counts
//   applications in a very expensive way ;-)
//
// Global Variables: -
//
// Side Effects    : Changes termbank.
//
/----------------------------------------------------------------------*/

static Term_p term_unfold_def(TB_p bank, Term_p term, PStack_p
               pos_stack, ClausePos_p demod)
{
   Term_p res, tmp;
   int i;
   bool changed = false;

   res = TermTopCopyWithoutArgs(term);

   for(i=0; i<res->arity; i++)
   {
      res->args[i] = term_unfold_def(bank, term->args[i], pos_stack,
                 demod);
      if(res->args[i] != term->args[i])
      {
    changed = true;
      }
   }
   if(changed)
   {
      tmp = TBTermTopInsert(bank, res);
   }
   else
   {
      TermTopFree(res);
      tmp = term;
   }
   res = term_top_unfold_def(bank, tmp, demod);
   if(res!=tmp)
   {
      PStackPushP(pos_stack, term);
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: eqn_unfold_def()
//
//   Apply demod everywhere in the literal. See above. Return true if
//   one term changes.
//
// Global Variables: -
//
// Side Effects    : Changes termbank.
//
/----------------------------------------------------------------------*/

static bool eqn_unfold_def(Eqn_p eqn, PStack_p pos_stack, ClausePos_p demod)
{
   bool res = false;
   Term_p tmp;

   tmp = term_unfold_def(eqn->bank, eqn->lterm, pos_stack, demod);
   if(tmp != eqn->lterm)
   {
      res = true;
      eqn->lterm = tmp;
   }
   tmp = term_unfold_def(eqn->bank, eqn->rterm, pos_stack, demod);
   if(tmp != eqn->rterm)
   {
      res = true;
      eqn->rterm = tmp;
   }
   return res;
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: ClauseUnfoldEqDef()
//
//   Apply demod to normalize clause. Print unfolding as (annotated)
//   rewrite steps. Return true if clause changed.
//
// Global Variables: -
//
// Side Effects    : Changes clause
//
/----------------------------------------------------------------------*/

bool ClauseUnfoldEqDef(Clause_p clause, ClausePos_p demod)
{
   bool res = false;
   PStack_p pos_stack = PStackAlloc();
   Eqn_p handle;

   for(handle = clause->literals; handle; handle = handle->next)
   {
      eqn_unfold_def(handle, pos_stack, demod);
   }

   if(!PStackEmpty(pos_stack))
   {
      res = true;

      if(ClauseQueryTPTPType(demod->clause) == CPTypeConjecture)
      {
    ClauseSetTPTPType(clause, CPTypeConjecture);
      }
      DocClauseEqUnfold(GlobalOut, OutputLevel, clause, demod,
         pos_stack);
      if(BuildProofObject)
      {
         PStackPointer i;
         for(i=0; i<PStackGetSP(pos_stack); i++)
         {
            ClausePushDerivation(clause, DCUnfold, demod->clause, NULL);
         }
      }
   }
   PStackFree(pos_stack);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetUnfoldEqDef()
//
//   Apply demod to all clauses in set.
//
// Global Variables: -
//
// Side Effects    : Potentially changes clauses.
//
/----------------------------------------------------------------------*/

bool ClauseSetUnfoldEqDef(ClauseSet_p set, ClausePos_p demod)
{
   Clause_p handle;
   bool res = false;

   for(handle = set->anchor->succ;
       handle!=set->anchor;
       handle = handle->succ)
   {
      if(ClauseUnfoldEqDef(handle, demod))
      {
    res = true;
    ClauseRemoveSuperfluousLiterals(handle);
      }
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: ClauseSetUnfoldAllEqDefs()
//
//   While there are equational definitions where the right hand side
//   is not to much (eqdef_incrlimit) bigger than the left hand side,
//   apply them and remove them. Returns number of removed clauses.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ClauseSetUnfoldAllEqDefs(ClauseSet_p set, ClauseSet_p passive,
                              ClauseSet_p archive,
               int min_arity, int eqdef_incrlimit)
{
   ClausePos_p demod;
   long res = false;
   Clause_p start = NULL;

   while((demod = ClauseSetFindEqDefinition(set, min_arity, start)))
   {
      start = demod->clause->succ;
      if((TermStandardWeight(ClausePosGetOtherSide(demod))-
          TermStandardWeight(ClausePosGetSide(demod)))<=eqdef_incrlimit)
      {
         ClauseSetExtractEntry(demod->clause);
         ClauseSetUnfoldEqDef(set, demod);
         if(passive)
         {
            ClauseSetUnfoldEqDef(passive, demod);
         }
         if(BuildProofObject)
         {
            ClauseSetInsert(archive, demod->clause);
         }
         else
         {
            ClauseFree(demod->clause);
         }
         res++;
      }
      ClausePosCellFree(demod);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetPreprocess()
//
//   Perform preprocessing on the clause set: Removing tautologies,
//   definition unfolding and canonization. Returns number of clauses
//   removed. If passive is true, potential unfolding is applied to
//   clauses in that set as well.
//
// Global Variables: -
//
// Side Effects    : Changes set and term bank.
//
/----------------------------------------------------------------------*/

long ClauseSetPreprocess(ClauseSet_p set, ClauseSet_p passive,
                         ClauseSet_p archive, TB_p tmp_terms,
                         int eqdef_incrlimit, long eqdef_maxclauses)
{
   long res, tmp;

   ClauseSetRemoveSuperfluousLiterals(set);
   res = ClauseSetFilterTautologies(set, tmp_terms);
   ClauseSetCanonize(set);
   if((eqdef_incrlimit==INT_MIN) || (set->members > eqdef_maxclauses))
   {
      return res;
   }
   if((tmp = ClauseSetUnfoldAllEqDefs(set, passive, archive, 1, eqdef_incrlimit)))
   {
      res += tmp;
      res += ClauseSetFilterTautologies(set, tmp_terms);
      ClauseSetCanonize(set);
   }
   /* No further ClauseSetCanonize() here - no changes since the one
      above. */
   return res;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/








