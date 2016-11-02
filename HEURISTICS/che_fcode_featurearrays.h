/*-----------------------------------------------------------------------

File  : che_fcode_featurearrays.h

Author: Stephan Schulz

Contents

  Sortable arrays associating a function symbol with a number of
  integer feature values (that define the order). Used by precedence
  generating functions, now also for weights.

Copyright 1998-2011 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Thu Feb 20 21:39:34 CET 2003
    New (partially taken from che_to_precgen.c)

-----------------------------------------------------------------------*/

#ifndef CHE_F_CODE_FEATUREARRAYS

#define CHE_F_CODE_FEATUREARRAYS

#include <clb_simple_stuff.h>
#include <che_clausesetfeatures.h>
#include <stdlib.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct fcode_feature_sort_cell
{
   int     key1;
   int     key2;
   int     key3;
   int     freq;
   int     conjfreq;
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
void FCodeFeatureArrayFree(FCodeFeatureArray_p junk);
void FCodeFeatureArraySort(FCodeFeatureArray_p array);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





