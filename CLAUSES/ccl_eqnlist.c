/*-----------------------------------------------------------------------

File  : ccl_eqnlist.c

Author: Stephan Schulz

Contents

   Functions for dealing with (singly linked) lists of equations as
   used in clauses

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Fri Apr 10 21:11:18 MET DST 1998
    New

-----------------------------------------------------------------------*/

#include "ccl_eqnlist.h"
#include "cte_typecheck.h"



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
// Function: eqn_list_find_last()
//
//   Find the last EqnRef in *list (may be list itself).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

EqnRef eqn_list_find_last(EqnRef list)
{
   while(*list)
   {
      list = &((*list)->next);
   }
   return list;
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: EqnListFree()
//
//   Deallocate the list.
//
// Global Variables: -
//
// Side Effects    : Memory Management
//
/----------------------------------------------------------------------*/

void EqnListFree(Eqn_p list)
{
   Eqn_p handle;

   while(list)
   {
      handle = list;
      list = list->next;
      EqnFree(handle);
   }
}


/*-----------------------------------------------------------------------
//
// Function: EqnListGCMarkTerms()
//
//   Mark all terms in the eqnlist for the Garbage Collection.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void EqnListGCMarkTerms(Eqn_p list)
{
   while(list)
   {
      EqnGCMarkTerms(list);
      list = list->next;
   }
}


/*-----------------------------------------------------------------------
//
// Function: EqnListSetProp()
//
//   Set the properties prop in all literals from list. Return the
//   lenght of the list.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int EqnListSetProp(Eqn_p list, EqnProperties prop)
{
   int res = 0;

   while(list)
   {
      EqnSetProp(list, prop);
      list = list->next;
      res++;
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: EqnListDelProp()
//
//   Delete the properties prop in all literals from list. Return
//   lenght of the list.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int EqnListDelProp(Eqn_p list, EqnProperties prop)
{
   int res = 0;

   while(list)
   {
      EqnDelProp(list, prop);
      list = list->next;
      res++;
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: EqnListFlipProp()
//
//   Delete the properties prop in all literals from list. Return
//   lenght of the list.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int EqnListFlipProp(Eqn_p list, EqnProperties prop)
{
   int res = 0;

   while(list)
   {
      EqnFlipProp(list, prop);
      list = list->next;
      res++;
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: EqnListQueryPropNumber()
//
//   Return number of equations with props set.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int EqnListQueryPropNumber(Eqn_p list, EqnProperties prop)
{
   int res = 0;

   while(list)
   {
      if(EqnQueryProp(list, prop))
      {
    res++;
      }
      list = list->next;
   }
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: EqnListLength()
//
//   Return number of equations in the list.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int EqnListLength(Eqn_p list)
{
   int i=0;

   while(list)
   {
      i++;
      list = list->next;
   }
   return i;
}

/*-----------------------------------------------------------------------
//
// Function: EqnListFromArray()
//
//   Convert an array of Eqn_p's into a list.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Eqn_p EqnListFromArray(Eqn_p* array, int lenght)
{
   int i;
   Eqn_p *handle, res;

   handle = &(res);
   for(i=0; i<lenght; i++)
   {
      *handle = array[i];
      handle = &((*handle)->next);
   }
   *handle = NULL;

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: EqnListToStack()
//
//   Push the literals onto a newly created stack and return it. Does
//   not copy anything! The caller has to free the stack.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

PStack_p EqnListToStack(Eqn_p list)
{
   PStack_p stack = PStackAlloc();

   while(list)
   {
      PStackPushP(stack, list);
      list = list->next;
   }
   return stack;
}


/*-----------------------------------------------------------------------
//
// Function: EqnListExtractElement()
//
//   Take the given element out of the list and return a pointer to
//   it.
//
// Global Variables: -
//
// Side Effects    : Changes the list
//
/----------------------------------------------------------------------*/

Eqn_p EqnListExtractElement(EqnRef element)
{
   Eqn_p handle = *element;
   assert(handle);

   *element = handle->next;

   handle->next = NULL;
   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: EqnListExtractByProp()
//
//   Extract all equations with properties props (not) set (depending
//   on negate).
//
// Global Variables: -
//
// Side Effects    : Changes list
//
/----------------------------------------------------------------------*/

Eqn_p EqnListExtractByProps(EqnRef list, EqnProperties props, bool
             negate)
{
   Eqn_p res = NULL, tmp;

   while(*list)
   {
      if(XOR(EqnQueryProp(*list, props),negate))
      {
    tmp = EqnListExtractFirst(list);
    EqnListInsertFirst(&res, tmp);
      }
      else
      {
    list = &((*list)->next);
      }
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: EqnListDeleteElement()
//
//   Delete the given element from the list.
//
// Global Variables: -
//
// Side Effects    : Via EqnListExtractElement() and EqnFree()
//
/----------------------------------------------------------------------*/

void EqnListDeleteElement(EqnRef element)
{
   Eqn_p handle;

   handle = EqnListExtractElement(element);

   EqnFree(handle);
}


/*-----------------------------------------------------------------------
//
// Function: EqnListInsertElement()
//
//   Insert the element at the position defined by pos.
//
// Global Variables: -
//
// Side Effects    : Changes the list
//
/----------------------------------------------------------------------*/

void EqnListInsertElement(EqnRef pos, Eqn_p element)
{
   element->next = *pos;
   *pos = element;
}


/*-----------------------------------------------------------------------
//
// Function: EqnListAppend()
//
//   Append newpart at the end of *list.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Eqn_p EqnListAppend(EqnRef list, Eqn_p newpart)
{
   EqnRef result = list;

   list = eqn_list_find_last(list);
   assert(!(*list));
   *list = newpart;

   return *result;
}



/*-----------------------------------------------------------------------
//
// Function: EqnListFlatCopy()
//
//   Return a flat copy of the given list, reusing the existing terms.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Eqn_p EqnListFlatCopy(Eqn_p list)
{
   Eqn_p  newlist = NULL;
   EqnRef insert = &newlist;

   while(list)
   {
      *insert = EqnFlatCopy(list);
      insert = &((*insert)->next);
      list = list->next;
   }
   *insert = NULL;

   return newlist;
}


/*-----------------------------------------------------------------------
//
// Function: EqnListCopy()
//
//   Return a copy of the given list, with new terms from the term
//   bank. Instantiated terms are copied as instantiations.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Eqn_p EqnListCopy(Eqn_p list, TB_p bank)
{
   Eqn_p  newlist = NULL;
   EqnRef insert = &newlist;

   while(list)
   {
      *insert = EqnCopy(list, bank);
      insert = &((*insert)->next);
      list = list->next;
   }
   *insert = NULL;

   return newlist;
}


/*-----------------------------------------------------------------------
//
// Function: EqnListCopyExcept()
//
//   Return a copy of the given list, except for the equation given in
//   except, with new terms from the term bank. Instantiated terms are
//   copied as instantiations.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Eqn_p EqnListCopyExcept(Eqn_p list, Eqn_p except, TB_p bank)
{
   Eqn_p  newlist = NULL;
   EqnRef insert = &newlist;

   while(list)
   {
      if(list != except)
      {
    *insert = EqnCopy(list, bank);
    insert = &((*insert)->next);
      }
      list = list->next;
   }
   *insert = NULL;

   return newlist;
}


/*-----------------------------------------------------------------------
//
// Function: EqnListCopyOpt()
//
//   Copy an Eqnlist with the optimizations possible if all terms
//   (source and target) are from the same term bank.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Eqn_p EqnListCopyOpt(Eqn_p list)
{
   Eqn_p  newlist = NULL;
   EqnRef insert = &newlist;

   while(list)
   {
      *insert = EqnCopyOpt(list);
      insert = &((*insert)->next);
      list = list->next;
   }
   *insert = NULL;

   return newlist;
}


/*-----------------------------------------------------------------------
//
// Function: EqnListCopyOptExcept()
//
//   Copy an Eqnlist with one exception using the optimizations
//   possible if all terms (source and target) are from the same term
//   bank.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Eqn_p EqnListCopyOptExcept(Eqn_p list, Eqn_p except)
{
   Eqn_p  newlist = NULL;
   EqnRef insert = &newlist;

   while(list)
   {
      if(list != except)
      {
    *insert = EqnCopyOpt(list);
    insert = &((*insert)->next);
      }
      list = list->next;
   }
   *insert = NULL;

   return newlist;
}


/*-----------------------------------------------------------------------
//
// Function: EqnListCopyDisjoint()
//
//   Create a copy of list with disjoint variables (using the even/odd
//   convention).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Eqn_p EqnListCopyDisjoint(Eqn_p list)
{
   Eqn_p  newlist = NULL;
   EqnRef insert = &newlist;

   while(list)
   {
      *insert = EqnCopyDisjoint(list);
      insert = &((*insert)->next);
      list = list->next;
   }
   *insert = NULL;

   return newlist;
}



/*-----------------------------------------------------------------------
//
// Function: EqnListCopyRepl()
//
//   Return a copy of the list with terms from bank, except that
//   all occurances of "old" are replaced with repl (which has to be
//   in bank).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Eqn_p EqnListCopyRepl(Eqn_p list, TB_p bank, Term_p old, Term_p repl)
{
   Eqn_p  newlist = NULL;
   EqnRef insert = &newlist;

   while(list)
   {
      *insert = EqnCopyRepl(list, bank, old, repl);
      insert = &((*insert)->next);
      list = list->next;
   }
   *insert = NULL;

   return newlist;
}




/*-----------------------------------------------------------------------
//
// Function: EqnListNegateEqns()
//
//   Negate all signs in the list.
//
// Global Variables: -
//
// Side Effects    : Changes signs in the list
//
/----------------------------------------------------------------------*/

Eqn_p EqnListNegateEqns(Eqn_p list)
{
   Eqn_p handle = list;

   while(handle)
   {
      EqnFlipProp(handle, EPIsPositive);
      handle = handle->next;
   }
   return list;
}



/*-----------------------------------------------------------------------
//
// Function: EqnListRemoveDuplicates()
//
//   Remove all but one copy of identical (modulo commutativity)
//   elements from the list. Return number of removed literals.
//
// Global Variables: -
//
// Side Effects    : Changes the list
//
/----------------------------------------------------------------------*/

int EqnListRemoveDuplicates(Eqn_p list)
{
   EqnRef handle;
   int    removed = 0;

   while(list)
   {
      handle = &(list->next);
      while(*handle)
      {
    if(LiteralEqual(*handle, list))
    {
       EqnListDeleteElement(handle);
       removed++;
    }
    else
    {
       handle = &((*handle)->next);
    }
      }
      list = list->next;
   }
   return removed;
}


/*-----------------------------------------------------------------------
//
// Function:  EqnListRemoveResolved()
//
//   Remove trivially false equations.
//
// Global Variables: -
//
// Side Effects    : Changes the list.
//
/----------------------------------------------------------------------*/

int EqnListRemoveResolved(EqnRef list)
{
   int removed = 0;

   while(*list)
   {
      if(EqnIsFalse(*list))
      {
         EqnListDeleteElement(list);
         removed++;
      }
      else
      {
         list = &((*list)->next);
      }
   }
   return removed;
}


/*-----------------------------------------------------------------------
//
// Function:  EqnListRemoveACResolved()
//
//   Remove negative equations implied by the current AC theory.
//
// Global Variables: -
//
// Side Effects    : Changes the list.
//
/----------------------------------------------------------------------*/

int EqnListRemoveACResolved(EqnRef list)
{
   int removed = 0;

   while(*list)
   {
      if(!EqnIsPositive(*list) && EqnIsACTrivial(*list))
      {
    EqnListDeleteElement(list);
    removed++;
      }
      else
      {
    list = &((*list)->next);
      }
   }
   return removed;
}



/*-----------------------------------------------------------------------
//
// Function:  EqnListRemoveSimpleAnswers()
//
//   Remove all simple answer literals from the list
//
// Global Variables: -
//
// Side Effects    : Changes the list.
//
/----------------------------------------------------------------------*/

int EqnListRemoveSimpleAnswers(EqnRef list)
{
   int removed = 0;

   while(*list)
   {
      if(EqnIsSimpleAnswer(*list))
      {
    EqnListDeleteElement(list);
    removed++;
      }
      else
      {
    list = &((*list)->next);
      }
   }
   return removed;
}



/*-----------------------------------------------------------------------
//
// Function: EqnListFindNegPureVarLit()
//
//   Return a pointer to the first negative literal of the form X!=Y
//   (or NULL if no such literal exists).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

 Eqn_p EqnListFindNegPureVarLit(Eqn_p list)
{
   while(list)
   {
      if(EqnIsNegative(list)&&EqnIsPureVar(list))
      {
    break;
      }
      list = list->next;
   }
   return list;
}


/*-----------------------------------------------------------------------
//
// Function: EqnListFindTrue()
//
//   Return the first "always true" literal, if any. Return false
//   otherwise.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

Eqn_p EqnListFindTrue(Eqn_p list)
{
   while(list)
   {
      if(EqnIsTrue(list))
      {
    break;
      }
      list = list->next;
   }
   return list;
}


/*-----------------------------------------------------------------------
//
// Function: EqnListIsTrivial()
//
//   Return true if the list contains two equal literals with
//   opposing signs or a literal that always evaluates to true.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool EqnListIsTrivial(Eqn_p list)
{
   Eqn_p handle;

   while(list)
   {
      if(EqnIsTrue(list))
      {
    return true;
      }
      for(handle = list->next; handle; handle = handle->next)
      {
    if(!PropsAreEquiv(handle, list, EPIsPositive))
    {
       if(EqnEqual(handle, list))
       {
          return true;
       }
    }
      }
      list = list->next;
   }
   return false;
}


/*-----------------------------------------------------------------------
//
// Function: EqnListIsACTrivial()
//
//   Return true if the list contains a positive AC-trivial
//   equation.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool EqnListIsACTrivial(Eqn_p list)
{
   while(list)
   {
      if(EqnIsPositive(list) && EqnIsACTrivial(list))
      {
    return true;
      }
      list = list->next;
   }
   return false;
}


/*-----------------------------------------------------------------------
//
// Function: EqnListIsGround()
//
//   Return true if all equations in list are true, false otherwise.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool EqnListIsGround(Eqn_p list)
{
   bool res = true;

   while(list)
   {
      if(!EqnIsGround(list))
      {
    res = false;
    break;
      }
      list = list->next;
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: EqnListIsEquational()
//
//   Return true if any literal in the list is a true equations.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool EqnListIsEquational(Eqn_p list)
{
   while(list)
   {
      if(EqnIsEquLit(list))
      {
    return true;
      }
      list = list->next;
   }
   return false;
}


/*-----------------------------------------------------------------------
//
// Function: EqnListIsPureEquational()
//
//   Return true if all literals in the list are true equations.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool EqnListIsPureEquational(Eqn_p list)
{
   while(list)
   {
      if(!EqnIsEquLit(list))
      {
    return false;
      }
      list = list->next;
   }
   return true;
}


/*-----------------------------------------------------------------------
//
// Function: EqnListOrient()
//
//   Orient all the equations in list. Equations already oriented are
//   not reoriented! Return number of swapped equations.
//
// Global Variables: -
//
// Side Effects    : Orients equations
//
/----------------------------------------------------------------------*/

int EqnListOrient(OCB_p ocb, Eqn_p list)
{
   int res = 0;
   Eqn_p handle;

   for(handle = list; handle; handle = handle->next)
   {
      if(EqnOrient(ocb, handle))
      {
    res++;
      }

   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: EqnListMaximalLiterals()
//
//   Determine for each literal wether it is maximal or not. Returns
//   number of maximal literals. Also determines strictly maximal
//   literals.
//
// Global Variables: -
//
// Side Effects    : Sets the maximal flags of literals.
//
/----------------------------------------------------------------------*/

int EqnListMaximalLiterals(OCB_p ocb, Eqn_p list)
{
   Eqn_p handle, stepper;
   CompareResult cmp;
   int res = 0;

   res = EqnListSetProp(list, EPIsMaximal|EPIsStrictlyMaximal);
   for(handle = list; handle; handle = handle->next)
   {
      for(stepper = handle->next; stepper; stepper = stepper->next)
      {
    if(EqnIsMaximal(stepper) && EqnIsMaximal(handle))
    {
       cmp = LiteralCompare(ocb, handle, stepper);
       switch(cmp)
       {
       case to_greater:
        EqnDelProp(stepper, EPIsMaximal);
        EqnDelProp(stepper, EPIsStrictlyMaximal);
        res--;
        break;
       case to_lesser:
        EqnDelProp(handle, EPIsMaximal);
        EqnDelProp(handle, EPIsStrictlyMaximal);
        res--;
        break;
       case to_equal:
        EqnDelProp(stepper, EPIsStrictlyMaximal);
        EqnDelProp(handle, EPIsStrictlyMaximal);
        break;
       default:
        break;
       }
    }
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: EqnListEqnIsMaximal()
//
//   Return true if eqn is maximal with respect to list (i.e. if there
//   are no equations that dominate it), false otherwise. As above,
//   details of this may need change if the calculus changes.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool EqnListEqnIsMaximal(OCB_p ocb, Eqn_p list, Eqn_p eqn)
{
   Eqn_p handle;
   bool res = true;
   CompareResult cmp;

   for(handle=list; handle; handle = handle->next)
   {
      if(handle!=eqn && EqnIsMaximal(handle))
      {
    cmp = LiteralCompare(ocb, handle, eqn);

    if(cmp == to_greater)
    {
       res = false;
       break;
    }
      }
   }
/*   printf("\n");
   EqnPrintOriginal(stdout, eqn); printf(" is max in ");
   EqnListPrint(stdout, list, ";", normal, true);
   printf(" ResNormal: %d\n", res); */
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: EqnListEqnIsStrictlyMaximal()
//
//   Return true if eqn is strictly maximal with respect to list
//   (i.e. if there are no equations that dominate it), false
//   otherwise. As above, details of this may need change if the
//   calculus changes.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool EqnListEqnIsStrictlyMaximal(OCB_p ocb, Eqn_p list, Eqn_p eqn)
{
   Eqn_p handle;
   bool res = true;

   for(handle=list; handle&&res; handle = handle->next)
   {
      if(handle!=eqn && EqnIsMaximal(handle))
      {
    switch(LiteralCompare(ocb, handle, eqn))
    {
    case to_equal:
    case to_greater:
          res = false;
          break;
    default:
          break;
    }
      }
   }
   /* printf("\n");
   EqnPrintOriginal(stdout, eqn); printf(" is max in ");
   EqnListPrint(stdout, list, ";", normal, true);
   printf("ResStrict: %d\n", res); */
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: EqnListDeleteTermProperties()
//
//   Delete the given properties for all term occurences in the
//   eqnlist.
//
// Global Variables: -
//
// Side Effects    : Changes term bank
//
/----------------------------------------------------------------------*/

void EqnListDeleteTermProperties(Eqn_p list, TermProperties props)
{
   while(list)
   {
      TBRefDelProp(list->bank, &(list->lterm), props);
      TBRefDelProp(list->bank, &(list->rterm), props);
      list = list->next;
   }
}


/*-----------------------------------------------------------------------
//
// Function: EqnListPrint()
//
//   Print the list. Separate elements with the given separator
//   (usually "," oder ";"). If negated is true, negate equations
//   before printing (to allow for easy printing of clauses in
//   implicational form).
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void EqnListPrint(FILE* out, Eqn_p list, char* sep,
        bool negated, bool fullterms)
{
   Eqn_p handle = list;

   if(handle)
   {
      EqnPrint(out, handle, negated, fullterms);

      while(handle->next)
      {
    handle = handle->next;
    fputs(sep, out);
    EqnPrint(out, handle, negated, fullterms);
      }
   }
}

/*-----------------------------------------------------------------------
//
// Function: EqnListTSTPPrint()
//
//   Same as above, but without negation and uses TSTP literal format.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void EqnListTSTPPrint(FILE* out, Eqn_p list, char* sep, bool fullterms)
{
   Eqn_p handle = list;

   if(handle)
   {
      EqnTSTPPrint(out, handle, fullterms);

      while(handle->next)
      {
    handle = handle->next;
    fputs(sep, out);
    EqnTSTPPrint(out, handle, fullterms);
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: EqnListParse()
//
//   Parse a list of equations, separated by Tokens of type sep.
//
// Global Variables: -
//
// Side Effects    : Input, memory operations, by EqnParse()
//
/----------------------------------------------------------------------*/

Eqn_p EqnListParse(Scanner_p in, TB_p bank, TokenType sep)
{
   Eqn_p handle = NULL,
         list = NULL;

   if(((ScannerGetFormat(in) == TPTPFormat) &&
       TestInpTok(in, Plus|Hyphen))
      ||
      ((ScannerGetFormat(in) == LOPFormat) &&
       TestInpTok(in, TermStartToken|TildeSign))
      ||
      ((ScannerGetFormat(in) == TSTPFormat) &&
       TestInpTok(in, TermStartToken|TildeSign)))
   {
      list = EqnParse(in, bank);
      handle = list;
      while(TestInpTok(in,sep))
      {
    NextToken(in);
    handle->next = EqnParse(in, bank);
    handle = handle->next;
      }
   }
   return list;
}


/*-----------------------------------------------------------------------
//
// Function: NormSubstEqnListExcept()
//
//   Instantiate all variables in eqnlist (except for terms from
//   except)  with fresh variables from vars. Returns the current
//   position in subst.
//
// Global Variables: -
//
// Side Effects    : Builds substitution, instantiates variables
//
/----------------------------------------------------------------------*/

FunCode NormSubstEqnListExcept(Eqn_p list, Eqn_p except, Subst_p
                subst, VarBank_p vars)
{
   Eqn_p   handle;
   PStackPointer res = PStackGetSP(subst);

   for(handle = list; handle; handle = handle->next)
   {
      if(handle!= except)
      {
    SubstNormEqn(handle, subst, vars);
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: EqnListDepth()
//
//   Return the depth of an eqn-list (i.e. the maximal depth of a
//   term).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long EqnListDepth(Eqn_p list)
{
   long maxdepth = 0, eqndepth;

   while(list)
   {
      eqndepth = EqnDepth(list);
      maxdepth = MAX(maxdepth,eqndepth);
      list = list->next;
   }
   return maxdepth;
}



/*-----------------------------------------------------------------------
//
// Function: EqnListAddSymbolDistribution()
//
//   Count the number of occurences of function symbols in list and
//   add them to dist_array, which has to be a pointer to an array of
//   long that is sufficiently long (and preferably adequatly
//   initialized).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void EqnListAddSymbolDistribution(Eqn_p list, long *dist_array)
{
   while(list)
   {
      EqnAddSymbolDistribution(list, dist_array);
      list = list->next;
   }
}


/*-----------------------------------------------------------------------
//
// Function: EqnListAddSymbolDistribExist()
//
//   Count the number of occurences of function symbols in list and
//   add them to dist_array, which has to be a pointer to an array of
//   long that is sufficiently long (and preferably adequatly
//   initialized). Push occuring symbols onto exists (once).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void EqnListAddSymbolDistExist(Eqn_p list, long *dist_array, PStack_p exist)
{
   while(list)
   {
      EqnAddSymbolDistExist(list, dist_array, exist);
      list = list->next;
   }
}


/*-----------------------------------------------------------------------
//
// Function: EqnListAddSymbolFeatures()
//
//   Update features in feature_array with all equation in list.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void EqnListAddSymbolFeatures(Eqn_p list, PStack_p mod_stack, long *feature_array)
{
   while(list)
   {
      EqnAddSymbolFeatures(list, mod_stack, feature_array);
      list = list->next;
   }
}



/*-----------------------------------------------------------------------
//
// Function: EqnListComputeFunctionRanks()
//
//   Compute the occurance rank for all function symbols in list.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void EqnListComputeFunctionRanks(Eqn_p list, long *rank_array,
             long* count)
{
   while(list)
   {
      EqnComputeFunctionRanks(list, rank_array, count);
      list = list->next;
   }
}

/*-----------------------------------------------------------------------
//
// Function: EqnListCollectVariables()
//
//   Add all variables in list to tree. Return number of distinct
//   variables.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

long EqnListCollectVariables(Eqn_p list, PTree_p *tree)
{
   long res = 0;

   while(list)
   {
      res+=EqnCollectVariables(list, tree);
      list = list->next;
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: EqnListAddFunOccs()
//
//   For each symbol in literals that is not already marked in
//   f_occur, push it onto res_stack and mark its entry. Return
//   number of symbols found.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

long EqnListAddFunOccs(Eqn_p list,
                       PDArray_p f_occur,
                       PStack_p res_stack)
{
   long res = 0;

   while(list)
   {
      res+=EqnAddFunOccs(list, f_occur, res_stack);
      list = list->next;
   }
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: EqnListTermSetProp()
//
//   Set prop in all terms in list.
//
// Global Variables: -
//
// Side Effects    : None beyond the purpose
//
/----------------------------------------------------------------------*/

void EqnListTermSetProp(Eqn_p list, TermProperties props)
{
   while(list)
   {
      EqnTermSetProp(list, props);
      list = list->next;
   }
}


/*-----------------------------------------------------------------------
//
// Function: EqnListTBTermDelPropCount()
//
//   Delete prop in all terms in list, return number of termcells in
//   which prop was set.
//
// Global Variables: -
//
// Side Effects    : As above
//
/----------------------------------------------------------------------*/

long EqnListTBTermDelPropCount(Eqn_p list, TermProperties props)
{
   long count = 0;

    while(list)
   {
      count += EqnTBTermDelPropCount(list, props);
      list = list->next;
   }
    return count;
}


/*-----------------------------------------------------------------------
//
// Function: EqnListTermDelProp()
//
//   Delete prop in all terms in list, return number of termcells in
//   which prop was set.
//
// Global Variables: -
//
// Side Effects    : As above
//
/----------------------------------------------------------------------*/

long EqnListTermDelProp(Eqn_p list, TermProperties props)
{
   long count = 0;

    while(list)
   {
      EqnTermDelProp(list, props);
      list = list->next;
   }
    return count;
}


/*-----------------------------------------------------------------------
//
// Function: EqnListCollectSubterms()
//
//   Collect all subterms of list onto collector. Assumes that
//   TPOpFlag is set if and only if the term is already in the
//   collection. Returns the number of new terms found.
//
// Global Variables: -
//
// Side Effects    : Sets the OpFlag of newly collected terms.
//
/----------------------------------------------------------------------*/

long EqnListCollectSubterms(Eqn_p list, PStack_p collector)
{
   long res = 0;

   while(list)
   {
      res+= EqnCollectSubterms(list, collector);
      list = list->next;
   }
   return res;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
