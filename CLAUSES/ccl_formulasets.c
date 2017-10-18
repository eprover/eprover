/*-----------------------------------------------------------------------

File  : ccl_formulasets.c

Author: Stephan Schulz

Contents

  Wrapped formula set code.

Copyright 1998-2011 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Thu Jun 11 16:26:23 CEST 2009
    New (factored out)

-----------------------------------------------------------------------*/

#include "ccl_formulasets.h"



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
// Function: FormulaSetAlloc()
//
//   Allocate and initialize a formula set.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

FormulaSet_p FormulaSetAlloc(void)
{
   FormulaSet_p set = FormulaSetCellAlloc();

   set->members = 0;
   set->anchor  = WFormulaCellAlloc();
   set->anchor->succ = set->anchor;
   set->anchor->pred = set->anchor;
   set->identifier = DStrAlloc();

   return set;
}

/*-----------------------------------------------------------------------
//
// Function: FormulaSetFreeFormulas(set)
//
//   Free all formulas in set.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void FormulaSetFreeFormulas(FormulaSet_p set)
{
   assert(set);

   while(!FormulaSetEmpty(set))
   {
      FormulaSetDeleteEntry(set->anchor->succ);
   }
}

/*-----------------------------------------------------------------------
//
// Function: FormulaSetFree(set)
//
//   Free a formula set (and all its formulas).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void FormulaSetFree(FormulaSet_p set)
{
   assert(set);

   FormulaSetFreeFormulas(set);
   WFormulaCellFree(set->anchor);
   DStrFree(set->identifier);
   FormulaSetCellFree(set);
}


/*-----------------------------------------------------------------------
//
// Function: FormulaSetStackCardinality()
//
//   Assume stack is a stack of formulasets. Return the number of
//   formulas in all the sets.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long FormulaSetStackCardinality(PStack_p stack)
{
   FormulaSet_p handle;
   PStackPointer i;
   long res = 0;

   for(i=0; i<PStackGetSP(stack); i++)
   {
      handle = PStackElementP(stack, i);
      res += FormulaSetCardinality(handle);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: FormulaSetGCMarkCells()
//
//   For all tformulas in set, mark their cells as being in use (for
//   garbage collection).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void FormulaSetGCMarkCells(FormulaSet_p set)
{
   WFormula_p handle;

   handle = set->anchor->succ;

   while(handle!=set->anchor)
   {
      WFormulaGCMarkCells(handle);
      handle = handle->succ;
   }
}


/*-----------------------------------------------------------------------
//
// Function: FormulaSetMarkPolarity()
//
//   Mark the polarity of all subformulas in set.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
void FormulaSetMarkPolarity(FormulaSet_p set)
{
   WFormula_p handle;

   handle = set->anchor->succ;

   while(handle!=set->anchor)
   {
      WFormulaMarkPolarity(handle);
      handle = handle->succ;
   }
}


/*-----------------------------------------------------------------------
//
// Function: FormulaSetInsert()
//
//   Insert newnode into set.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void FormulaSetInsert(FormulaSet_p set, WFormula_p newform)
{
   assert(set);
   assert(newform);
   assert(!newform->set);

   newform->succ = set->anchor;
   newform->pred = set->anchor->pred;
   set->anchor->pred->succ = newform;
   set->anchor->pred = newform;
   newform->set = set;
   set->members++;
}


/*-----------------------------------------------------------------------
//
// Function: FormulaSetInsertSet()
//
//   Move all formulas from from into set (leaving from empty, but not
//   deleted).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long FormulaSetInsertSet(FormulaSet_p set, FormulaSet_p from)
{
   WFormula_p handle;
   long res = 0;

   while(!FormulaSetEmpty(from))
   {
      handle = FormulaSetExtractFirst(from);
      FormulaSetInsert(set, handle);
      res++;
   }
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: FormulaSetExtractEntry()
//
//   Extract a given formula from a formula set and return it.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

WFormula_p FormulaSetExtractEntry(WFormula_p form)
{
   assert(form);
   assert(form->set);

   form->pred->succ = form->succ;
   form->succ->pred = form->pred;
   form->set->members--;
   form->set = NULL;
   form->succ = NULL;
   form->pred = NULL;

   return form;
}

/*-----------------------------------------------------------------------
//
// Function: FormulaSetExtractFirst()
//
//   Extract and return the first formula from set, if any, otherwise
//   return NULL.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

WFormula_p FormulaSetExtractFirst(FormulaSet_p set)
{
   assert(set);

   if(FormulaSetEmpty(set))
   {
      return NULL;
   }
   return FormulaSetExtractEntry(set->anchor->succ);
}


/*-----------------------------------------------------------------------
//
// Function: FormulaSetDeleteEntry()
//
//   Delete an element of a formulaset.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void FormulaSetDeleteEntry(WFormula_p form)
{
   assert(form);

   FormulaSetExtractEntry(form);
   WFormulaFree(form);
}



/*-----------------------------------------------------------------------
//
// Function: FormulaSetPrint()
//
//   Print a set of formulae.
//
// Global Variables: OutputFormat
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void FormulaSetPrint(FILE* out, FormulaSet_p set, bool fullterms)
{
   WFormula_p handle;

   handle = set->anchor->succ;

   while(handle!=set->anchor)
   {
      WFormulaPrint(out, handle, fullterms);
      fputc('\n', out);
      handle = handle->succ;
   }
}

/*-----------------------------------------------------------------------
//
// Function: FormulaSetHasInterpretedSymbol()
//
//   Return true if any formula from set has a symbol from an
//   interpreted sort.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool FormulaSetHasInterpretedSymbol(FormulaSet_p set)
{
   WFormula_p handle;

   handle = set->anchor->succ;

   while(handle!=set->anchor)
   {
      if(WFormHasInterpretedSymbol(handle))
      {
         return true;
      }
      handle = handle->succ;
   }
   return false;
}



/*-----------------------------------------------------------------------
//
// Function: FormulaSetSplitConjectures()
//
//   Find all (real or negated) conjectures in set and sort them into
//   conjectures. Collect the rest in rest. Return number of
//   conjectures found.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

long FormulaSetSplitConjectures(FormulaSet_p set,
                                PList_p conjectures, PList_p rest)
{
   WFormula_p handle;
   long     res = 0;

   for(handle = set->anchor->succ;
       handle!=set->anchor;
       handle = handle->succ)
   {
      if(FormulaIsConjecture(handle))
      {
         PListStoreP(conjectures, handle);
         res++;
      }
      else
      {
         PListStoreP(rest, handle);
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: FormulaSetStandardWeight()
//
//   Return the sum of the standardweight of all clauses in set.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long long FormulaSetStandardWeight(FormulaSet_p set)
{
   WFormula_p  handle;
   long long res = 0;

   for(handle = set->anchor->succ;
       handle!=set->anchor;
       handle = handle->succ)
   {
      res += WFormulaStandardWeight(handle);
   }
   return res;

}

/*-----------------------------------------------------------------------
//
// Function: FormulaSetCountConjectures()
//
//   Count and return number of conjectures (and negated_conjectures)
//   in set. Also find number of hypotheses,  and add it to *hypos.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long FormulaSetCountConjectures(FormulaSet_p set, long* hypos)
{
   long ret = 0;
   WFormula_p handle;

   for(handle = set->anchor->succ;
       handle != set->anchor;
       handle = handle->succ)
   {
      if(FormulaIsConjecture(handle))
      {
         ret++;
      }
      if(FormulaIsHypothesis(handle))
      {
         (*hypos)++;
      }
   }
   return ret;
}

/*-----------------------------------------------------------------------
//
// Function: FormulaStackCondSetType()
//
//   Set the type of all formulas on stack to type if that does not
//   change the semantics of the formula.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void FormulaStackCondSetType(PStack_p stack, FormulaProperties type)
{
   PStackPointer i;
   WFormula_p handle;

   for(i=0; i<PStackGetSP(stack); i++)
   {
      handle = PStackElementP(stack,i);
      if(FormulaQueryType(handle)!=CPTypeConjecture || type==CPTypeConjecture)
      {
         FormulaSetType(handle, type);
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: FormulaSetCollectFCode()
//
//   Push all formulas that contain f_code onto result. Return number
//   of formulas found.
//
// Global Variables: -
//
// Side Effects    : Only via PStackPushP()
//
/----------------------------------------------------------------------*/

long FormulaSetCollectFCode(FormulaSet_p set, FunCode f_code,
                            PStack_p result)
{
   long ret = 0;
   WFormula_p handle;

   for(handle = set->anchor->succ;
       handle != set->anchor;
       handle = handle->succ)
   {
      if(TermHasFCode(handle->tformula, f_code))
      {
         PStackPushP(result, handle);
         ret++;
      }
   }
   return ret;
}




/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
