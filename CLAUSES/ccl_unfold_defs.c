/*-----------------------------------------------------------------------

  File  : ccl_unfold_defs.c

  Author: Stephan Schulz

  Contents

  Functions for unfolding equational definitions.

  Copyright 1998-2018 by the author.

  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: Wed Aug 14 20:00:53 CEST 2002

  -----------------------------------------------------------------------*/

#include "ccl_unfold_defs.h"
#include <cte_lambda.h>



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
// Function: term_top_unfold_def_fo()
//
//   If possible, return the term that results from applying the
//   demodulator at top position, otherwise return term.
//
// Global Variables: -
//
// Side Effects    : Changes term bank.
//
/----------------------------------------------------------------------*/

static Term_p term_top_unfold_def_fo(TB_p bank, Term_p term, Term_p lside, Term_p rside)
{
   Term_p res;
   Subst_p subst;
   bool tmp;

   assert(bank&&term&&lside&&rside);

   assert(!TermIsAnyVar(lside));
   if(lside->f_code != term->f_code)
   {
      return term;
   }
   subst = SubstAlloc();
   tmp = SubstMatchComplete(lside, term, subst);
   UNUSED(tmp); assert(tmp); /* Match must exist because demod is demod! */
   res = TBInsertInstantiated(bank, rside);
   SubstDelete(subst);
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: term_top_unfold_def_ho()
//
//   Like term_top_unfold_def_fo, but assumes that all definitions
//   have been transformed into symbol = lambda expression, so it does
//   not do matching, but lambda normalization.
//
// Global Variables: -
//
// Side Effects    : Changes term bank.
//
/----------------------------------------------------------------------*/

static Term_p term_top_unfold_def_ho(TB_p bank, Term_p term, Term_p lside, Term_p rside)
{
   assert(bank&&term&&lside&&rside);

   assert(!TermIsTopLevelAnyVar(lside));
   assert(!TermIsLambda(lside));

   if(lside->f_code != term->f_code)
   {
      return term;
   }
   assert(lside->type == rside->type);
   if(term->arity == 0)
   {
      assert(term->type == rside->type);
      return rside;
   }
   else
   {
      PStack_p args = PStackAlloc();
      for(int i=0; i<term->arity; i++)
      {
         PStackPushP(args, term->args[i]);
      }
      Term_p res = WHNF_step(bank, ApplyTerms(bank, rside, args));
      PStackFree(args);
      assert(res->type == term->type);
      return res;
   }
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
                              pos_stack, Term_p demod_l, Term_p demod_r)
{
   Term_p res, tmp = NULL;
   int i;
   bool changed = false;

   res = TermTopCopyWithoutArgs(term);

   for(i=0; i<res->arity; i++)
   {
      res->args[i] = term_unfold_def(bank, term->args[i], pos_stack,
                                     demod_l, demod_r);
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
   res = (problemType == PROBLEM_FO ? term_top_unfold_def_fo : term_top_unfold_def_ho)
            (bank, tmp, demod_l, demod_r);
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

static bool eqn_unfold_def(Eqn_p eqn, PStack_p pos_stack, Term_p lside, Term_p rside)
{
   bool res = false;
   Term_p tmp;

   tmp = term_unfold_def(eqn->bank, eqn->lterm, pos_stack, lside, rside);
   if(tmp != eqn->lterm)
   {
      res = true;
      eqn->lterm = tmp;
   }
   tmp = term_unfold_def(eqn->bank, eqn->rterm, pos_stack, lside, rside);
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
//   rewrite steps. Return true if clause changed. NB: In case of HO
//   unfolding lside and rside can be transformed into a lambda equation.
//   Thus, they might be different in shape from what is stored in demod.
//   Demod is still used to build proof object.
//
// Global Variables: -
//
// Side Effects    : Changes clause
//
/----------------------------------------------------------------------*/

bool ClauseUnfoldEqDef(Clause_p clause, ClausePos_p demod, Term_p lside, Term_p rside)
{
   bool res = false;
   PStack_p pos_stack = PStackAlloc();
   Eqn_p handle;
   PStackPointer i;

   for(handle = clause->literals; handle; handle = handle->next)
   {
      eqn_unfold_def(handle, pos_stack, lside, rside);
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
      for(i=0; i<PStackGetSP(pos_stack); i++)
      {
         ClausePushDerivation(clause, DCUnfold, demod->clause, NULL);
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
   bool
      res = false,
      demod_is_conj = ClauseIsConjecture(demod->clause);

   Term_p lside = ClausePosGetSide(demod);
   Term_p rside = ClausePosGetOtherSide(demod);

   if(problemType == PROBLEM_HO)
   {
      ClauseExtractHODefinition(demod->clause, demod->side, &lside, &rside);
   }

   for(handle = set->anchor->succ;
       handle!=set->anchor;
       handle = handle->succ)
   {
      if(ClauseUnfoldEqDef(handle, demod, lside, rside))
      {
         res = true;
         ClauseRemoveSuperfluousLiterals(handle);
         /* If a non-conjecture clause is rewritten here, we make the
            result a conjecture - it's the right thing to do for
            e.g. SoS. */
         if(demod_is_conj)
         {
            ClauseSetTPTPType(handle,CPTypeNegConjecture);
         }
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
                              int min_arity, long eqdef_incrlimit)
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
         ClauseSetInsert(archive, demod->clause);
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
                         ClauseSet_p archive, TB_p tmp_terms, TB_p terms,
                         bool replace_injectivity_defs,
                         int eqdef_incrlimit, long eqdef_maxclauses)
{
   long res;

   ClauseSetRemoveSuperfluousLiterals(set);
   res = ClauseSetFilterTautologies(set, tmp_terms);
   if (replace_injectivity_defs)
   {
      ClauseSetReplaceInjectivityDefs(set,archive,terms);
   }
   ClauseSetCanonize(set);
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: ClauseSetUnfoldEqDefNormalize()
//
//    Unfold definitions and renormalize clause set.
//
// Global Variables: -
//
// Side Effects    : Changes set and term bank.
//
/----------------------------------------------------------------------*/

long ClauseSetUnfoldEqDefNormalize(ClauseSet_p set, ClauseSet_p passive,
                                   ClauseSet_p archive, TB_p tmp_terms,
                                   long eqdef_incrlimit, long eqdef_maxclauses)
{
   long res = 0, tmp;

   if((eqdef_incrlimit==LONG_MIN) || (set->members > eqdef_maxclauses))
   {
      return res;
   }
   if((tmp = ClauseSetUnfoldAllEqDefs(set, passive, archive, 1, eqdef_incrlimit)))
   {
      res += tmp;
      res += ClauseSetFilterTautologies(set, tmp_terms);
      ClauseSetCanonize(set);
   }
   return res;
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
