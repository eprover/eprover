/*-----------------------------------------------------------------------

  File  : che_fcode_featurearrays.c

  Author: Stephan Schulz

  Contents

  Implementation of arrays associating FunCodes and numeric features.

  Copyright 1998-2020 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: Thu Feb 20 21:57:25 CET 2003

-----------------------------------------------------------------------*/

#include "che_fcode_featurearrays.h"


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
// Function: feature_compare_function()
//
//   Compare two featuresortcells and return <0, =0, >0 as for
//   strcmp().
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static int feature_compare_function(const void* e1,
                                    const void* e2)
{
   const FCodeFeatureSort_p entry1 = (const FCodeFeatureSort_p)e1;
   const FCodeFeatureSort_p entry2 = (const FCodeFeatureSort_p)e2;

   int res;

   if((res = entry1->key0-entry2->key0))
   {
      return res;
   }
   if((res = entry1->key1-entry2->key1))
   {
      return res;
   }
   if((res = entry1->key2-entry2->key2))
   {
      return res;
   }
   if((res = entry1->key3-entry2->key3))
   {
      return res;
   }
   /* if((res = entry1->freq-entry2->freq))
   {
      return res;
      }*/
   return entry1->pos_rank-entry2->pos_rank;
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: FCodeFeatureArrayAlloc()
//
//   Allocate an initialized FCodeFeature array.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

FCodeFeatureArray_p FCodeFeatureArrayAlloc(Sig_p sig, ClauseSet_p axioms)
{
   FCodeFeatureArray_p handle;
   FunCode i;
   long  array_size      = sizeof(long)*(sig->f_count+1);
   long *rank_array      = SizeMalloc(array_size);
   long *dist_array      = SizeMalloc(array_size);
   long *conjdist_array  = SizeMalloc(array_size);
   long *axiomdist_array = SizeMalloc(array_size);
   long  rank = 0;

   handle = FCodeFeatureArrayCellAlloc();
   handle->size = sig->f_count+1;
   handle->array = SizeMalloc(handle->size*sizeof(FCodeFeatureSortCell));

   for(i=1; i<= sig->f_count; i++)
   {
      rank_array[i]      = 0;
      dist_array[i]      = 0;
      conjdist_array[i]  = 0;
   }
   ClauseSetComputeFunctionRanks(axioms, rank_array, &rank);
   ClauseSetAddSymbolDistribution(axioms, dist_array);
   ClauseSetAddConjSymbolDistribution(axioms, conjdist_array);
   ClauseSetAddAxiomSymbolDistribution(axioms, axiomdist_array);
   for(i=1; i<= sig->f_count; i++)
   {
      handle->array[i].key0      = 0;
      handle->array[i].key1      = 0;
      handle->array[i].key2      = 0;
      handle->array[i].key3      = 0;
      handle->array[i].freq      = dist_array[i];
      handle->array[i].conjfreq  = conjdist_array[i];
      handle->array[i].axiomfreq = axiomdist_array[i];
      handle->array[i].pos_rank  = rank_array[i];
      handle->array[i].symbol    = i;
   }
   SizeFree(rank_array, array_size);
   SizeFree(dist_array, array_size);
   SizeFree(conjdist_array, array_size);
   SizeFree(axiomdist_array, array_size);

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: FCodeFeatureArrayUpdateOccKey()
//
//    Update key0 based on the occurrence of the symbols in axioms,
//    conjectures, or both.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void FCodeFeatureArrayUpdateOccKey(FCodeFeatureArray_p array, OrderParms_p oparms)
{
   FunCode i;

   for(i=1; i< array->size; i++)
   {
      if(array->array[i].conjfreq)
      {
         if(array->array[i].axiomfreq)
         {
            array->array[i].key0 += oparms->conj_axiom_mod;
         }
         else
         {
            array->array[i].key0 += oparms->conj_only_mod;
         }
      }
      else if(array->array[i].axiomfreq)
      {
         array->array[i].key0 += oparms->axiom_only_mod;
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: FCodeFeatureArrayUpdateSymbKey()
//
//    Update key0 based on the occurrence of the symbols in axioms,
//    conjectures, or both.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void FCodeFeatureArrayUpdateSymbKey(FCodeFeatureArray_p array, Sig_p sig,
                                    OrderParms_p oparms)
{
   FunCode i;

   for(i=1; i< array->size; i++)
   {
      if(SigQueryFuncProp(sig, i, FPSkolemSymbol))
      {
         array->array[i].key0 += oparms->skolem_mod;
      }
      if(SigQueryFuncProp(sig, i, FPDefPred))
      {
         array->array[i].key0 += oparms->defpred_mod;
      }
   }
}



/*-----------------------------------------------------------------------
//
// Function: FCodeFeatureArrayFree()
//
//   Free an FCodeFeatureArray.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

void FCodeFeatureArrayFree(FCodeFeatureArray_p junk)
{
   SizeFree(junk->array, junk->size * sizeof(FCodeFeatureSortCell));
   FCodeFeatureArrayCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: FCodeFeatureArraySort()
//
//   Sort an array according to feature_compare_function()
//
// Global Variables: -
//
// Side Effects    : Sorts the array
//
/----------------------------------------------------------------------*/

void FCodeFeatureArraySort(FCodeFeatureArray_p array)
{
   qsort(&(array->array[SIG_TRUE_CODE+1]),
    array->size-(SIG_TRUE_CODE+1),
    sizeof(FCodeFeatureSortCell),
    feature_compare_function);
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
