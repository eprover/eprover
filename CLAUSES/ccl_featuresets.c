/*-----------------------------------------------------------------------

  File  : ccl_featuresets.c

  Author: Stephan Schulz (schulz@eprover.org)

  Contents

  Sparse clause features for subsumption indexing (see
  ccl_featuresets.h).

  Copyright 2026 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: Thu Nov 12 2026

  -----------------------------------------------------------------------*/

#include "ccl_featuresets.h"


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
// Function: feature_cell()
//
//   Return the cell for feature in set, creating it (with value 0)
//   if necessary.
//
// Global Variables: -
//
// Side Effects    : Memory operations, changes set
//
/----------------------------------------------------------------------*/

static NumTree_p feature_cell(FeatureSet_p set, long feature)
{
   NumTree_p cell = NumTreeFind(&(set->features), feature);

   if(!cell)
   {
      cell = NumTreeCellAllocEmpty();
      cell->key = feature;
      cell->val1.i_val = 0;
      NumTreeInsert(&(set->features), cell);
      set->size++;
   }
   return cell;
}


/*-----------------------------------------------------------------------
//
// Function: term_add_features()
//
//   Add the symbol features of term (at the given depth) to set,
//   using freq and depth as the feature kinds. This mirrors
//   TermAddSymbolFeaturesLimited() without the symbol limit.
//
// Global Variables: -
//
// Side Effects    : Changes set
//
/----------------------------------------------------------------------*/

static void term_add_features(FeatureSet_p set, Term_p term, long depth,
                              FSSymbolFeature freq, FSSymbolFeature dkind)
{
   if(!TermIsAnyVar(term))
   {
      int i;

      if(TermIsPhonyApp(term))
      {
         FeatureSetMaxVal(set, FSSymbolFeatureNo(0, dkind), depth);
      }
      else
      {
         FeatureSetAddVal(set, FSSymbolFeatureNo(term->f_code, freq), 1);
         FeatureSetMaxVal(set, FSSymbolFeatureNo(term->f_code, dkind),
                          depth);
      }
      for(i=0; i<term->arity; i++)
      {
         term_add_features(set, term->args[i], depth+1, freq, dkind);
      }
   }
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: FeatureSetAlloc()
//
//   Return an empty feature set.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

FeatureSet_p FeatureSetAlloc(void)
{
   FeatureSet_p handle = FeatureSetCellAlloc();

   handle->size     = 0;
   handle->features = NULL;

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: FeatureSetFree()
//
//   Free a feature set.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void FeatureSetFree(FeatureSet_p junk)
{
   assert(junk);

   NumTreeFree(junk->features);
   FeatureSetCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: FeatureSetGetVal()
//
//   Return the value of feature in set (0 if it is not in the set).
//
// Global Variables: -
//
// Side Effects    : Reorganizes the tree (splaying)
//
/----------------------------------------------------------------------*/

long FeatureSetGetVal(FeatureSet_p set, long feature)
{
   NumTree_p cell = NumTreeFind(&(set->features), feature);

   return cell? cell->val1.i_val : 0;
}


/*-----------------------------------------------------------------------
//
// Function: FeatureSetAddVal()
//
//   Add value to the value of feature in set.
//
// Global Variables: -
//
// Side Effects    : Memory operations, changes set
//
/----------------------------------------------------------------------*/

void FeatureSetAddVal(FeatureSet_p set, long feature, long value)
{
   assert(value >= 0);

   if(value)
   {
      feature_cell(set, feature)->val1.i_val += value;
   }
}


/*-----------------------------------------------------------------------
//
// Function: FeatureSetMaxVal()
//
//   Set the value of feature in set to the maximum of its current
//   value and value.
//
// Global Variables: -
//
// Side Effects    : Memory operations, changes set
//
/----------------------------------------------------------------------*/

void FeatureSetMaxVal(FeatureSet_p set, long feature, long value)
{
   NumTree_p cell;

   assert(value >= 0);

   if(value)
   {
      cell = feature_cell(set, feature);
      cell->val1.i_val = MAX(cell->val1.i_val, value);
   }
}


/*-----------------------------------------------------------------------
//
// Function: ClauseFeatureSetCompute()
//
//   Compute and return the feature set of clause.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

FeatureSet_p ClauseFeatureSetCompute(Clause_p clause)
{
   FeatureSet_p set = FeatureSetAlloc();
   Eqn_p        handle;

   FeatureSetAddVal(set, FS_POS_LITS, clause->pos_lit_no);
   FeatureSetAddVal(set, FS_NEG_LITS, clause->neg_lit_no);

   for(handle = clause->literals; handle; handle = handle->next)
   {
      FSSymbolFeature freq  = EqnIsPositive(handle)? FSPosFreq : FSNegFreq;
      FSSymbolFeature depth = EqnIsPositive(handle)? FSPosDepth : FSNegDepth;

      term_add_features(set, handle->lterm, 0, freq, depth);
      term_add_features(set, handle->rterm, 0, freq, depth);
   }
   return set;
}


/*-----------------------------------------------------------------------
//
// Function: FeatureSetLessEqual()
//
//   Return true if every feature value in set1 is less than or equal
//   to the corresponding value in set2.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool FeatureSetLessEqual(FeatureSet_p set1, FeatureSet_p set2)
{
   PStack_p  iter1, iter2;
   NumTree_p cell1, cell2;
   bool      res = true;

   if(set1 == set2)
   {
      return true;
   }
   if(set1->size > set2->size)
   {  /* Some feature of set1 is 0 in set2 */
      return false;
   }
   iter1 = NumTreeTraverseInit(set1->features);
   iter2 = NumTreeTraverseInit(set2->features);
   cell2 = NumTreeTraverseNext(iter2);
   while(res && (cell1 = NumTreeTraverseNext(iter1)))
   {
      while(cell2 && cell2->key < cell1->key)
      {
         cell2 = NumTreeTraverseNext(iter2);
      }
      res = cell2 && (cell2->key == cell1->key) &&
         (cell1->val1.i_val <= cell2->val1.i_val);
   }
   NumTreeTraverseExit(iter1);
   NumTreeTraverseExit(iter2);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: FeatureSetFirstDiff()
//
//   Return the smallest feature in which the two sets differ, or -1
//   if they are equal.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long FeatureSetFirstDiff(FeatureSet_p set1, FeatureSet_p set2)
{
   PStack_p  iter1, iter2;
   NumTree_p cell1, cell2;
   long      res = -1;

   if(set1 == set2)
   {
      return -1;
   }
   iter1 = NumTreeTraverseInit(set1->features);
   iter2 = NumTreeTraverseInit(set2->features);
   cell1 = NumTreeTraverseNext(iter1);
   cell2 = NumTreeTraverseNext(iter2);
   while(cell1 || cell2)
   {
      if(!cell2 || (cell1 && cell1->key < cell2->key))
      {
         res = cell1->key;
         break;
      }
      if(!cell1 || cell2->key < cell1->key)
      {
         res = cell2->key;
         break;
      }
      if(cell1->val1.i_val != cell2->val1.i_val)
      {
         res = cell1->key;
         break;
      }
      cell1 = NumTreeTraverseNext(iter1);
      cell2 = NumTreeTraverseNext(iter2);
   }
   NumTreeTraverseExit(iter1);
   NumTreeTraverseExit(iter2);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: FeatureSetPrint()
//
//   Print a feature set as a list of feature:value pairs.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void FeatureSetPrint(FILE* out, FeatureSet_p set)
{
   PStack_p  iter = NumTreeTraverseInit(set->features);
   NumTree_p cell;

   fprintf(out, "{");
   while((cell = NumTreeTraverseNext(iter)))
   {
      fprintf(out, " %ld:%ld", cell->key, cell->val1.i_val);
   }
   fprintf(out, " }\n");
   NumTreeTraverseExit(iter);
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
