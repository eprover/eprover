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

/* While a set is built, every contribution of a symbol occurrence is
   collected as a pair in a buffer (on the stack for small clauses).
   The buffer is then sorted by feature and runs of equal features are
   combined: counts are added, depths are combined by maximum. */

#define FS_BUILDER_LOCAL 128

typedef struct fs_builder_cell
{
   FSPair_p   pairs;
   long       count;
   long       size;
   FSPairCell local[FS_BUILDER_LOCAL];
}FSBuilderCell, *FSBuilder_p;


/*-----------------------------------------------------------------------
//
// Function: builder_push()
//
//   Add a contribution (feature, value) to the builder (ignored if
//   value is 0).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void builder_push(FSBuilder_p b, long feature, long value)
{
   assert(value >= 0);
   assert(feature >= 0 && feature <= INT32_MAX);
   assert(value <= INT32_MAX);

   if(!value)
   {
      return;
   }
   if(b->count == b->size)
   {
      long     new_size = 2*b->size;
      FSPair_p new_pairs = SizeMalloc(new_size*sizeof(FSPairCell));

      memcpy(new_pairs, b->pairs, b->count*sizeof(FSPairCell));
      if(b->pairs != b->local)
      {
         SizeFree(b->pairs, b->size*sizeof(FSPairCell));
      }
      b->pairs = new_pairs;
      b->size  = new_size;
   }
   b->pairs[b->count].feature = feature;
   b->pairs[b->count].value   = value;
   b->count++;
}


/*-----------------------------------------------------------------------
//
// Function: feature_is_count()
//
//   Return true if the values of feature are added up (literal and
//   symbol counts), false if they are combined by maximum (depths).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static bool feature_is_count(long feature)
{
   long kind;

   if(feature < FS_LIT_FEATURES)
   {
      return true;
   }
   kind = (feature-FS_LIT_FEATURES)%FS_SYMBOL_FEATURES;
   return (kind == FSNegFreq) || (kind == FSPosFreq);
}


/*-----------------------------------------------------------------------
//
// Function: pair_cmp()
//
//   Compare two pairs by feature (for qsort()).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static int pair_cmp(const void* p1, const void* p2)
{
   const FSPairCell *a = p1, *b = p2;

   return (a->feature > b->feature) - (a->feature < b->feature);
}


/*-----------------------------------------------------------------------
//
// Function: term_add_features()
//
//   Add the symbol features of term (at the given depth, in a
//   literal of the given sign) to the builder, as selected by
//   type. Minimal depths are only recorded if min_depth is true (see
//   the header for why). For the features of FSAllFeatures, this
//   mirrors TermAddSymbolFeaturesLimited() without the symbol limit
//   (phony applications only contribute to the depths of f_code 0).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void term_add_features(FSBuilder_p b, Term_p term, long depth,
                              bool positive, bool min_depth,
                              FeatureSetType type)
{
   if(!TermIsAnyVar(term))
   {
      int  i;
      long f_code = TermIsPhonyApp(term)? 0 : term->f_code;

      if((type != FSPlainFeatures) && !TermIsPhonyApp(term))
      {
         builder_push(b, FSSymbolFeatureNo(f_code, positive?
                                           FSPosFreq:FSNegFreq), 1);
      }
      builder_push(b, FSSymbolFeatureNo(f_code, positive?
                                        FSPosDepth:FSNegDepth), depth);
      if(min_depth)
      {
         builder_push(b, FSSymbolFeatureNo(f_code, positive?
                                           FSPosMinDepth:FSNegMinDepth),
                      MAX(FS_MINDEPTH_BASE-depth, 1));
      }
      for(i=0; i<term->arity; i++)
      {
         term_add_features(b, term->args[i], depth+1, positive,
                           min_depth, type);
      }
   }
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

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

   if(junk->pairs)
   {
      SizeFree(junk->pairs, junk->size*sizeof(FSPairCell));
   }
   FeatureSetCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: FeatureSetGetVal()
//
//   Return the value of feature in set (0 if it is not in the set),
//   by binary search.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long FeatureSetGetVal(FeatureSet_p set, long feature)
{
   long lo = 0, hi = set->size-1, mid;

   while(lo <= hi)
   {
      mid = (lo+hi)/2;
      if(set->pairs[mid].feature < feature)
      {
         lo = mid+1;
      }
      else if(set->pairs[mid].feature > feature)
      {
         hi = mid-1;
      }
      else
      {
         return set->pairs[mid].value;
      }
   }
   return 0;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseFeatureSetCompute()
//
//   Compute and return the feature set of clause with the features
//   selected by type.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

FeatureSet_p ClauseFeatureSetCompute(Clause_p clause, FeatureSetType type)
{
   FSBuilderCell b;
   FeatureSet_p  set = FeatureSetCellAlloc();
   Eqn_p         handle;
   long          i, n;

   b.pairs = b.local;
   b.count = 0;
   b.size  = FS_BUILDER_LOCAL;

   if(type != FSPlainFeatures)
   {
      builder_push(&b, FS_POS_LITS, clause->pos_lit_no);
      builder_push(&b, FS_NEG_LITS, clause->neg_lit_no);
   }
   for(handle = clause->literals; handle; handle = handle->next)
   {
      bool positive  = EqnIsPositive(handle);
      bool min_depth = (type != FSAllFeatures) &&
         (!positive || !EqnIsEquLit(handle));

      term_add_features(&b, handle->lterm, 0, positive, min_depth, type);
      term_add_features(&b, handle->rterm, 0, positive, min_depth, type);
   }
   qsort(b.pairs, b.count, sizeof(FSPairCell), pair_cmp);

   /* Combine runs of equal features in place */
   n = 0;
   for(i=0; i<b.count; i++)
   {
      if(n && b.pairs[n-1].feature == b.pairs[i].feature)
      {
         if(feature_is_count(b.pairs[i].feature))
         {
            b.pairs[n-1].value += b.pairs[i].value;
         }
         else
         {
            b.pairs[n-1].value = MAX(b.pairs[n-1].value, b.pairs[i].value);
         }
      }
      else
      {
         b.pairs[n++] = b.pairs[i];
      }
   }
   set->size  = n;
   set->pairs = NULL;
   if(n)
   {
      set->pairs = SizeMalloc(n*sizeof(FSPairCell));
      memcpy(set->pairs, b.pairs, n*sizeof(FSPairCell));
   }
   if(b.pairs != b.local)
   {
      SizeFree(b.pairs, b.size*sizeof(FSPairCell));
   }
   return set;
}


/*-----------------------------------------------------------------------
//
// Function: FeatureSetTypeParse()
//
//   Return the FeatureSetType named by name ("All", "AllMinDepth",
//   "Plain"), or -1 if there is none.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

FeatureSetType FeatureSetTypeParse(const char* name)
{
   if(strcmp(name, "All")==0)
   {
      return FSAllFeatures;
   }
   if(strcmp(name, "AllMinDepth")==0)
   {
      return FSAllMinDepthFeatures;
   }
   if(strcmp(name, "Plain")==0)
   {
      return FSPlainFeatures;
   }
   return -1;
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
   long i, j = 0;

   if(set1 == set2)
   {
      return true;
   }
   if(set1->size > set2->size)
   {  /* Some feature of set1 is 0 in set2 */
      return false;
   }
   for(i=0; i<set1->size; i++)
   {
      while(j < set2->size && set2->pairs[j].feature < set1->pairs[i].feature)
      {
         j++;
      }
      if(j == set2->size ||
         set2->pairs[j].feature != set1->pairs[i].feature ||
         set2->pairs[j].value < set1->pairs[i].value)
      {
         return false;
      }
      j++;
   }
   return true;
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
   long i = 0, j = 0;

   if(set1 == set2)
   {
      return -1;
   }
   while(i < set1->size && j < set2->size)
   {
      if(set1->pairs[i].feature != set2->pairs[j].feature)
      {
         return MIN(set1->pairs[i].feature, set2->pairs[j].feature);
      }
      if(set1->pairs[i].value != set2->pairs[j].value)
      {
         return set1->pairs[i].feature;
      }
      i++;
      j++;
   }
   if(i < set1->size)
   {
      return set1->pairs[i].feature;
   }
   if(j < set2->size)
   {
      return set2->pairs[j].feature;
   }
   return -1;
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
   long i;

   fprintf(out, "{");
   for(i=0; i<set->size; i++)
   {
      fprintf(out, " %d:%d", set->pairs[i].feature, set->pairs[i].value);
   }
   fprintf(out, " }\n");
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
