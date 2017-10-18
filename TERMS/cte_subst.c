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

PStackPointer SubstNormTerm(Term_p term, Subst_p subst, VarBank_p vars)
{
   PStackPointer ret = PStackGetSP(subst);
   PLocalStackInit(stack);
   PLocalStackPush(stack, term);

   while(!PLocalStackEmpty(stack))
   {
      term = TermDerefAlways(PLocalStackPop(stack));
      if(TermIsVar(term))
      {
         if(!TermCellQueryProp(term, TPSpecialFlag))
         {
            Term_p newvar = VarBankGetFreshVar(vars, term->sort);
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

bool SubstBindingPrint(FILE* out, Term_p var, Sig_p sig, DerefType
             deref)
{
   TermPrint(out, var, sig, DEREF_NEVER);
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
      assert(TermIsVar(var));
      assert(var->binding);
      deref=DEREF_ONCE;
      inst = TermDeref(var, &deref);

      if(!TermIsVar(inst))
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

   if(TermIsVar(term))
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
//   to default_term.
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

   if(TermIsVar(term))
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

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/




