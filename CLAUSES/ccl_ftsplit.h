/*-----------------------------------------------------------------------

  File  : ccl_ftsplit.h

  Author: Stephan Schulz (schulz@eprover.org)

  Contents

  Split rules for the feature trees (ccl_ftindexing, ccl_fsindexing):
  when a new clause reaches a leaf with a different feature
  vector/set, the leaf is replaced by a decision node on one of the
  features in which the two differ. The rule selects which one. Some
  rules need the number of indexed clauses with a non-zero value for
  each feature, which is maintained here. The bucket size (only for
  the feature set tree) is the number of different feature sets a
  leaf may hold before it is split.

  Copyright 2026 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: Thu Nov 12 2026

  -----------------------------------------------------------------------*/

#ifndef CCL_FTSPLIT

#define CCL_FTSPLIT

#include <clb_pdarrays.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* Which differing feature a new decision node tests:
   - FTSplitFirst: the smallest feature number
   - FTSplitLast: the largest feature number
   - FTSplitFrequent: the feature with a non-zero value in the most
     indexed clauses
   - FTSplitBalanced: the feature with a non-zero value in the number
     of indexed clauses closest to half of them
   Ties are broken by the smallest feature number. When a leaf with
   several feature sets is split, the candidates are the features on
   which not all of them agree. */

typedef enum
{
   FTSplitFirst,
   FTSplitLast,
   FTSplitFrequent,
   FTSplitBalanced
}FTSplitRule;

typedef struct ft_split_cell
{
   FTSplitRule rule;
   long        bucket_size;
   long        clauses;   /* Indexed clauses (only if FTSplitUsesFreq) */
   PDArray_p   freq;      /* Indexed clauses with a non-zero value, by
                             feature (only if FTSplitUsesFreq) */
}FTSplitCell, *FTSplit_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define FTSplitCellAlloc()    (FTSplitCell*)SizeMalloc(sizeof(FTSplitCell))
#define FTSplitCellFree(junk) SizeFree(junk, sizeof(FTSplitCell))

#define FTSplitUsesFreq(split) \
   (((split)->rule == FTSplitFrequent) || ((split)->rule == FTSplitBalanced))

/* Account for a clause entering (delta 1) or leaving (delta -1) the
   index, and for each of its features with a non-zero value. */
#define FTSplitCountClause(split, delta) ((split)->clauses += (delta))
#define FTSplitCountFeature(split, feature, delta)                   \
   PDArrayAssignInt((split)->freq, (feature),                        \
                    PDArrayElementInt((split)->freq, (feature))+(delta))

FTSplit_p   FTSplitAlloc(FTSplitRule rule, long bucket_size);
void        FTSplitFree(FTSplit_p junk);
FTSplitRule FTSplitRuleParse(const char* name);
long        FTSplitScore(FTSplit_p split, long feature);
void        FTSplitConsider(FTSplit_p split, long feature,
                            long *best, long *best_score);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
