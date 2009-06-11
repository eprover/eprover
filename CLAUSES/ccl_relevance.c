/*-----------------------------------------------------------------------

File  : ccl_relevance.c

Author: Stephan Schulz (schulz@eprover.org)

Contents
 
  Approximate relevance determination and filtering.

  Copyright 2009 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Wed Jun  3 00:07:17 CEST 2009
    New

-----------------------------------------------------------------------*/

#include "ccl_relevance.h"



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
// Function: ClauseSetSplitConjectures()
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

long ClauseSetSplitConjectures(ClauseSet_p set, 
                               PList_p conjectures, PList_p rest)
{
   Clause_p handle;
   long     res = 0;

   for(handle = set->anchor->succ;
       handle!=set->anchor;
       handle = handle->succ)
   {
      if(ClauseIsConjecture(handle))
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
      if(WFormulaIsConjecture(handle))
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
// Function: RelevanceAlloc()
//
//   Allocate a relevancy data structure - mostly used to be able to
//   clearly state invariants. After initialization:
//   - Core contains the newly found relevant clauses and formulas
//   - Rest contains the remainder of clauses and formulas
//   - new_codes is the set of newly found relevant function symbols.
//   - f_code_relevance contains for all f_codes the relevance level
//     (if found relevant already) or 0.
//
// Global Variables: - 
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Relevance_p RelevanceAlloc()
{
   Relevance_p handle = RelevanceCellAlloc();

   handle->clauses_core   = PListAlloc();
   handle->formulas_core  = PListAlloc();
   
    handle->clauses_rest  = PListAlloc();
    handle->formulas_rest = PListAlloc();

    handle->fcode_relevance = PDArrayAlloc(100, 0);
    handle->new_codes       = PStackAlloc();

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: RelevanceFree()
//
//   Free a RelevanceCell data structure.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void RelevanceFree(Relevance_p junk)
{
   PListFree(junk->clauses_core);
   PListFree(junk->formulas_core);
   PListFree(junk->clauses_rest);
   PListFree(junk->formulas_rest);
   PDArrayFree(junk->fcode_relevance);
   PStackFree(junk->new_codes);

   RelevanceCellFree(junk);
}




/*-----------------------------------------------------------------------
//
// Function: ClausePListPrint()
//
//   Print a plist of clauses.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void ClausePListPrint(FILE* out, PList_p list)
{
   PList_p  handle;
   Clause_p clause;

   for(handle=list->succ; 
       handle != list;
       handle = handle->succ)
   {
      clause = handle->key.p_val;
      ClausePrint(out, clause, true); 
      fputc('\n', out);
   }
}

/*-----------------------------------------------------------------------
//
// Function: FormulaPListPrint()
//
//   Print a plist of WFormulas.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void FormulaPListPrint(FILE* out, PList_p list)
{
   PList_p    handle;
   WFormula_p form;

   for(handle=list->succ; 
       handle != list;
       handle = handle->succ)
   {
      form = handle->key.p_val;
      WFormulaPrint(out, form, true); 
      fputc('\n', out);
   }
}




/*-----------------------------------------------------------------------
//
// Function: 
//
//   
//
// Global Variables: 
//
// Side Effects    : 
//
/----------------------------------------------------------------------*/

long RelevanceDataInit(ProofState_p state, Relevance_p data)
{
   long res = 0;

   res += ClauseSetSplitConjectures(state->axioms, 
                                    data->clauses_core, 
                                    data->clauses_rest);
   printf("Clauses done...\n");
   res += FormulaSetSplitConjectures(state->f_axioms, 
                                    data->formulas_core, 
                                    data->formulas_rest);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: 
//
//   
//
// Global Variables: 
//
// Side Effects    : 
//
/----------------------------------------------------------------------*/

Relevance_p RelevanceDataCompute(ProofState_p state)
{
   Relevance_p handle = RelevanceAlloc();
   long goals;

   goals = RelevanceDataInit(state, handle);

   printf("Conjectures:\n");
   ClausePListPrint(stdout, handle->clauses_core);
   FormulaPListPrint(stdout, handle->formulas_core);
   printf("Rest:\n");
   ClausePListPrint(stdout, handle->clauses_rest);
   FormulaPListPrint(stdout, handle->formulas_rest);
   
   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: ProofStatePreprocess()
//
//   Perform proof state preprocssing, in particular compute relevancy
//   data and perform relevancy pruning.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ProofStatePreprocess(ProofState_p state)
{
   Relevance_p reldata = RelevanceDataCompute(state);

   RelevanceFree(reldata);
   return 0;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


