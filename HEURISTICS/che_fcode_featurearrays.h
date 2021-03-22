/*-----------------------------------------------------------------------

  File  : che_fcode_featurearrays.h

  Author: Stephan Schulz

  Contents

  Sortable arrays associating a function symbol with a number of
  integer feature values (that define the order). Used by precedence
  generating functions, now also for weights.

  Copyright 1998-2020 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Thu Feb 20 21:39:34 CET 2003

-----------------------------------------------------------------------*/

#ifndef CHE_F_CODE_FEATUREARRAYS

#define CHE_F_CODE_FEATUREARRAYS

#include <clb_simple_stuff.h>
#include <che_clausesetfeatures.h>
#include <che_to_params.h>
#include <stdlib.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct fcode_feature_sort_cell
{
   int     key0;
   int     key1;
   int     key2;
   int     key3;
   int     freq;
   int     conjfreq;
   int     axiomfreq;
   int     pos_rank;
   FunCode symbol;
}FCodeFeatureSortCell, *FCodeFeatureSort_p;


typedef struct fcode_feature_array_cell
{
   long size;
   FCodeFeatureSort_p array;
}FCodeFeatureArrayCell, *FCodeFeatureArray_p;



/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


#define FCodeFeatureArrayCellAlloc() \
        (FCodeFeatureArrayCell*)SizeMalloc(sizeof(FCodeFeatureArrayCell))
#define FCodeFeatureArrayCellFree(junk) \
        SizeFree(junk, sizeof(FCodeFeatureArrayCell))

FCodeFeatureArray_p FCodeFeatureArrayAlloc(Sig_p sig, ClauseSet_p axioms);

void FCodeFeatureArrayUpdateOccKey(FCodeFeatureArray_p array, OrderParms_p oparms);
void FCodeFeatureArrayUpdateSymbKey(FCodeFeatureArray_p array, Sig_p sig, OrderParms_p oparms);
void FCodeFeatureArrayFree(FCodeFeatureArray_p junk);
void FCodeFeatureArraySort(FCodeFeatureArray_p array);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
