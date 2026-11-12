/*-----------------------------------------------------------------------

  File  : ccl_ftsplit.c

  Author: Stephan Schulz (schulz@eprover.org)

  Contents

  Split rules for the feature trees (see ccl_ftsplit.h).

  Copyright 2026 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: Thu Nov 12 2026

  -----------------------------------------------------------------------*/

#include "ccl_ftsplit.h"


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
// Function: FTSplitAlloc()
//
//   Allocate a split control for rule and bucket_size, with no
//   clauses counted.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

FTSplit_p FTSplitAlloc(FTSplitRule rule, long bucket_size)
{
   FTSplit_p handle = FTSplitCellAlloc();

   assert(bucket_size >= 1);

   handle->rule        = rule;
   handle->bucket_size = bucket_size;
   handle->clauses     = 0;
   handle->freq        = FTSplitUsesFreq(handle)? PDIntArrayAlloc(64, 0) : NULL;

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: FTSplitFree()
//
//   Free a split control.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void FTSplitFree(FTSplit_p junk)
{
   if(junk)
   {
      if(junk->freq)
      {
         PDArrayFree(junk->freq);
      }
      FTSplitCellFree(junk);
   }
}


/*-----------------------------------------------------------------------
//
// Function: FTSplitRuleParse()
//
//   Return the FTSplitRule named by name ("First", "Last",
//   "Frequent", "Balanced", "Even"), or -1 if there is none.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

FTSplitRule FTSplitRuleParse(const char* name)
{
   if(strcmp(name, "First")==0)
   {
      return FTSplitFirst;
   }
   if(strcmp(name, "Last")==0)
   {
      return FTSplitLast;
   }
   if(strcmp(name, "Frequent")==0)
   {
      return FTSplitFrequent;
   }
   if(strcmp(name, "Balanced")==0)
   {
      return FTSplitBalanced;
   }
   if(strcmp(name, "Even")==0)
   {
      return FTSplitEven;
   }
   return -1;
}


/*-----------------------------------------------------------------------
//
// Function: FTSplitScore()
//
//   Return how good feature is as the test of a new decision node
//   under the split rule (larger is better).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long FTSplitScore(FTSplit_p split, long feature)
{
   long freq;

   switch(split->rule)
   {
   case FTSplitFirst:
   case FTSplitEven:
         return -feature;
   case FTSplitLast:
         return feature;
   case FTSplitFrequent:
         return PDArrayElementInt(split->freq, feature);
   case FTSplitBalanced:
         freq = PDArrayElementInt(split->freq, feature);
         return -labs(2*freq - split->clauses);
   default:
         assert(false && "Unknown split rule");
         return 0;
   }
}


/*-----------------------------------------------------------------------
//
// Function: FTSplitConsider()
//
//   Make feature the best candidate so far (*best, with score
//   *best_score) if there is none yet (*best == -1) or if it is
//   better (with ties going to the smaller feature number).
//
// Global Variables: -
//
// Side Effects    : Changes *best and *best_score
//
/----------------------------------------------------------------------*/

void FTSplitConsider(FTSplit_p split, long feature,
                     long *best, long *best_score)
{
   long score = FTSplitScore(split, feature);

   if(*best == -1 || score > *best_score ||
      (score == *best_score && feature < *best))
   {
      *best       = feature;
      *best_score = score;
   }
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
