/*-----------------------------------------------------------------------

  File  : cte_subst.c

  Author: Stephan Schulz

  Contents

  General functions for substitutions.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  <1> Thu Mar  5 00:22:28 MET 1998
  New

  -----------------------------------------------------------------------*/

#include "cte_subst.h"
#include "clb_plocalstacks.h"
#include "cte_lambda.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function:  SubstBacktrackSingle()
//
//   Backtrack a single binding and remove it from the substitution
///  (if possible). Return true if successful, false if the
//   substitutuion is empty.
//
// Global Variables: -
//
// Side Effects    : As SubstAddBinding()
//
/----------------------------------------------------------------------*/

bool SubstBacktrackSingle(Subst_p subst)
{
   Term_p handle;

   assert(subst);

   if(PStackEmpty(subst))
   {
      return false;
   }
   handle = PStackPopP(subst);

   assert(TermIsFreeVar(handle));
   handle->binding = NULL;

   return true;
}

/*-----------------------------------------------------------------------
//
// Function: SubstBacktrackToPos()
//
//   Backtrack variable bindings up to (down to?) a given stack
//   pointer position.
//
// Global Variables: -
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

int SubstBacktrackToPos(Subst_p subst, PStackPointer pos)
{
   int ret = 0;

   while(PStackGetSP(subst) > pos)
   {
      SubstBacktrackSingle(subst);
      ret++;
   }
   return ret;
}

/*-----------------------------------------------------------------------
//
// Function: SubstBacktrack()
//
//   Undo all stored variable binding in subst.
//
// Global Variables: -
//
// Side Effects    : As noted...
//
/----------------------------------------------------------------------*/

int SubstBacktrack(Subst_p subst)
{
   int ret = 0;

   while(SubstBacktrackSingle(subst))
   {
      ret++;
   }
   return ret;
}


/*-----------------------------------------------------------------------
//
// Function: SubstNormTerm()
//
//   Instatiate all variables in term with fresh variables from the
//   VarBank. Return the current position of the substitution stack, so that
//   SubstBacktrackToPos() can be used to backtrack the
//   instantiations term by term. New variables are marked by
//   TPSpecialFlag, if other variables are marked thus the effect is
//   unpredictable.
//
//   Warning: As variables may be shared, other terms may be affected!
//   Take care...your best bet is to norm all terms you need with a
//   single substitution. If you need independendly normed terms, you
//   need to work with copy/backtrack operations (it's still better
//   than working with unshared terms).
//
// Global Variables: -
//
// Side Effects    : Instantiates variables
//
/----------------------------------------------------------------------*/

PStackPointer SubstNormTerm(Term_p term, Subst_p subst, VarBank_p vars, Sig_p sig)
{
   PStackPointer ret = PStackGetSP(subst);
   PLocalStackInit(stack);
   PLocalStackPush(stack, term);
   // assert(TermGetBank(term));

   Term_p (*deref)(Term_p) =
      problemType == PROBLEM_HO ? WHNF_deref : TermDerefAlways;

   while(!PLocalStackEmpty(stack))
   {
      term = deref(PLocalStackPop(stack));
      if(TermIsFreeVar(term))
      {
         if(!TermCellQueryProp(term, TPSpecialFlag))
         {
            Term_p newvar = VarBankGetFreshVar(vars, term->type);
            TermCellSetProp(newvar, TPSpecialFlag);
            SubstAddBinding(subst, term, newvar);
         }
      }
      else
      {
         PLocalStackPushTermArgsReversed(stack, term);
      }
   }

   PLocalStackFree(stack);
   return ret;
}


/*-----------------------------------------------------------------------
//
// Function: SubstBindingPrint()
//
//   Print a variable and its binding as x<-binding. Return true if
//   variable is bound. See comments on SubstPrint()!
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

bool SubstBindingPrint(FILE* out, Term_p var, Sig_p sig, DerefType deref)
{
   TermPrint(out, var, sig, DEREF_NEVER);
   // DBG_PRINT(out, " : ", TypePrintTSTP(out, sig->type_bank, var->type), " ");
   fprintf(out, "<-");
   if(var->binding)
   {
      TermPrint(out, var->binding, sig, deref);
      return true;
   }
   TermPrint(out, var, sig, DEREF_NEVER);
   return false;
}


/*-----------------------------------------------------------------------
//
// Function: SubstPrint()
//
//   Print a substitution. Note: Due to the different interpretations
//   of terms (follow/ignore bindings) and share variable, printing
//   substitutions with deref=DEREF_ALWAYS may lead to
//   unpredictable behaviour (if e.g. the substitution was generated
//   by matching x onto f(x)). Returns number of variables in subst
//   (well, why not...).
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

long SubstPrint(FILE* out, Subst_p subst, Sig_p sig, DerefType deref)
{
   PStackPointer i, limit;

   limit = PStackGetSP(subst);
   fprintf(out, "{");
   if(limit)
   {
      SubstBindingPrint(out,  PStackElementP(subst,0), sig, deref);
      {
         for(i=1; i<limit;i++)
         {
            fprintf(out, ", ");
            SubstBindingPrint(out,  PStackElementP(subst,i), sig,
                              deref);
         }
      }
   }
   fprintf(out, "}");

   return (long)limit;
}


/*-----------------------------------------------------------------------
//
// Function: SubstIsRenaming()
//
//   Return true if subst is just a variable renaming, false
//   otherwise. A substitution is a renaming if all variables are
//   instantiated to different variables. Checks only for one level of
//   instantiaton.
//
// Global Variables: -
//
// Side Effects    : Changes the TPOpFlag of terms
//
/----------------------------------------------------------------------*/

bool SubstIsRenaming(Subst_p subst)
{
   PStackPointer i, size;
   Term_p        var, inst;
   DerefType     deref;

   assert(subst);
   size = PStackGetSP(subst);

   /* Check that variables are instantiated with variables, reset
      TPOpFlag of all terms concerned */

   for(i=0; i< size; i++)
   {
      var = PStackElementP(subst,i);
      assert(TermIsFreeVar(var));
      assert(var->binding);
      deref=DEREF_ONCE;
      inst = TermDeref(var, &deref);

      if(!TermIsFreeVar(inst))
      {
         return false;
      }
      TermCellDelProp(inst, TPOpFlag);
   }

   /* For each unchecked variable, check wether another variable was
      already mapped to its instantiation */

   for(i=0; i< size; i++)
   {
      var = PStackElementP(subst,i);
      deref=DEREF_ONCE;
      inst = TermDeref(var, &deref);

      if(TermCellQueryProp(inst, TPOpFlag))
      {
         return false;
      }
      TermCellSetProp(inst, TPOpFlag);
   }
   return true;
}


/*-----------------------------------------------------------------------
//
// Function: SubstBacktrackSkolem()
//
//   Backtrack a skolem subst, freeing the skolem terms along the way.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void SubstBacktrackSkolem(Subst_p subst)
{
   Term_p handle;

   while(!PStackEmpty(subst))
   {
      handle = PStackPopP(subst);
      assert(handle);
      assert(handle->binding);
      TermFree(handle->binding);
      handle->binding = NULL;
   }
}

/*-----------------------------------------------------------------------
//
// Function: SubstSkolemize()
//
//   Instantiate all variables in term with new skolem symbols from
//   sig.
//
// Global Variables: -
//
// Side Effects    : Changes sig, creates skolem terms.
//
/----------------------------------------------------------------------*/

void SubstSkolemizeTerm(Term_p term, Subst_p subst, Sig_p sig)
{
   int i;

   assert(term && subst && sig);

   if(TermIsFreeVar(term))
   {
      if(!(term->binding))
      {
         PStackPushP(subst, term);
         term->binding =
            TermConstCellAlloc(SigGetNewSkolemCode(sig,0));
      }
   }
   else
   {
      for(i=0;i<term->arity;i++)
      {
         SubstSkolemizeTerm(term->args[i], subst, sig);
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: SubstCompleteInstance()
//
//   Add bindings for all free variables in term subst, binding them
//   to default_term. Note that this function is likely obsolete for
//   most cases with the introduction of many-sorted logic.
//
// Global Variables: -
//
// Side Effects    : Changes subst.
//
/----------------------------------------------------------------------*/

void SubstCompleteInstance(Subst_p subst, Term_p term,
                           Term_p default_binding)
{
   int i;

   if(TermIsFreeVar(term))
   {
      if(!(term->binding))
      {
         SubstAddBinding(subst, term, default_binding);
      }
   }
   else
   {
      for(i=0;i<term->arity;i++)
      {
         SubstCompleteInstance(subst, term->args[i], default_binding);
      }
   }
}

/*-----------------------------------------------------------------------
//
// Function: SubstBindAppVar()
//
//   Bind variable var to prefix of term to_bind with up_to arguments
//   eaten. Prefix will potentially be created (if it is a proper
//   non-variable prefix).
//
//    IMPORTANT: If prefix is created it will be shared.
//
// Global Variables: -
//
// Side Effects    : Changes subst.
//
/----------------------------------------------------------------------*/

PStackPointer SubstBindAppVar(Subst_p subst, Term_p var, Term_p to_bind, int up_to, TB_p bank)
{
   PStackPointer ret = PStackGetSP(subst);
   assert(var);
   assert(to_bind);
   assert(TermIsFreeVar(var));
   assert(!(var->binding));
   assert(problemType == PROBLEM_HO || !TermCellQueryProp(to_bind, TPPredPos));
   assert(var->type);
   assert(to_bind->type);

   Term_p to_bind_pref = TermCreatePrefix(to_bind, up_to);
   to_bind_pref->type = var->type;

   // if term is not shared it is prefix
   var->binding = TermIsShared(to_bind_pref) ?
                     to_bind_pref : TBTermTopInsert(bank, to_bind_pref);
   PStackPushP(subst, var);

   return ret;
}

/*-----------------------------------------------------------------------
//
// Function: SubstHasHOBinding()
//
//   Does the substitution have any bound variable that is of
//   functional type
//
// Global Variables: -
//
// Side Effects    : Changes subst.
//
/----------------------------------------------------------------------*/

bool SubstHasHOBinding(Subst_p subst)
{
   bool ans = false;
   if(problemType == PROBLEM_HO)
   {
      for(PStackPointer i = 0; !ans && i<PStackGetSP(subst); i++)
      {
         Term_p var = PStackElementP(subst, i);
         ans = TypeIsArrow(var->type);
      }
   }
   return ans;
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
