/*-----------------------------------------------------------------------

  File  : ccl_fbindexing.c

  Author: Stephan Schulz (schulz@eprover.org)

  Contents

  Binary feature set tree indexing for subsumption (see
  ccl_fbindexing.h).

  Copyright 2026 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: Thu Nov 12 2026

  -----------------------------------------------------------------------*/

#include "ccl_fbindexing.h"


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
// Function: print_lvl()
//
//   Print indentation for the given level.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void print_lvl(FILE* out, int level)
{
   for(int i=0; i<level; i++)
   {
      fprintf(out, "--");
   }
}


/*-----------------------------------------------------------------------
//
// Function: fb_index_print()
//
//   Print the index rooted at node.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void fb_index_print(FILE* out, FBIndex_p node, bool fullterms,
                           int level)
{
   if(!node)
   {
      print_lvl(out, level);
      fprintf(out, "(empty)\n");
   }
   else if(node->final)
   {
      FSEntry_p entry;
      OrdSet_p  handle;

      print_lvl(out, level);
      fprintf(out, "Leaf (%ld clauses, %ld sets)\n",
              node->clause_count, node->entry_count);
      for(entry = node->u1.entries; entry; entry = entry->next)
      {
         print_lvl(out, level+1);
         fprintf(out, "Set (%ld clauses): ", entry->clause_count);
         FeatureSetPrint(out, entry->fset);
         fprintf(out, "\n");
         for(handle = OrdSetFirst(entry->clauses); handle;
             handle = OrdSetNext(entry->clauses, handle))
         {
            print_lvl(out, level+2);
            ClausePrint(out, handle->key, fullterms);
            fprintf(out, "\n");
         }
      }
   }
   else
   {
      print_lvl(out, level);
      fprintf(out, "Test feature %ld <= %ld (%ld clauses)\n",
              node->feature, node->threshold, node->clause_count);
      fb_index_print(out, node->u1.succ.le, fullterms, level+1);
      print_lvl(out, level);
      fprintf(out, "Feature %ld > %ld:\n", node->feature, node->threshold);
      fb_index_print(out, node->u1.succ.gt, fullterms, level+1);
   }
}


/*-----------------------------------------------------------------------
//
// Function: leaf_alloc()
//
//   Allocate a new leaf without entries (and with invalid weight
//   bounds).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static FBIndex_p leaf_alloc(long *storage)
{
   FBIndex_p handle = FBIndexCellAlloc();

   handle->final         = true;
   handle->clause_count  = 0;
   handle->feature       = -1;
   handle->threshold     = 0;
   handle->entry_count   = 0;
   handle->max_weight    = FS_WEIGHT_INVALID;
   handle->min_weight    = FS_WEIGHT_INVALID;
   handle->u1.entries    = NULL;
   *storage += FBINDEX_MEM;

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: leaf_append()
//
//   Append entry to the entries of leaf (keeping insertion order) and
//   account for its clauses.
//
// Global Variables: -
//
// Side Effects    : Changes leaf
//
/----------------------------------------------------------------------*/

static void leaf_append(FBIndex_p leaf, FSEntry_p entry)
{
   FSEntry_p *ref = &(leaf->u1.entries);

   assert(leaf->final);

   while(*ref)
   {
      ref = &((*ref)->next);
   }
   *ref = entry;
   entry->next = NULL;
   leaf->entry_count++;
   leaf->clause_count += entry->clause_count;
}


/*-----------------------------------------------------------------------
//
// Function: node_free()
//
//   Free a single node (a leaf, which has to be without entries, or a
//   decision node, but not its successors), and account for the
//   memory.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void node_free(long *storage, FBIndex_p junk)
{
   assert(!junk->final || !junk->u1.entries);

   *storage -= FBINDEX_MEM;
   FBIndexCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: bounds_add()
//
//   Update the weight bounds of node for a new clause with the given
//   weight (invalid bounds stay invalid).
//
// Global Variables: -
//
// Side Effects    : Changes node
//
/----------------------------------------------------------------------*/

static void bounds_add(FBIndex_p node, long weight)
{
   if(node->max_weight != FS_WEIGHT_INVALID)
   {
      node->max_weight = MAX(node->max_weight, weight);
   }
   if(node->min_weight != FS_WEIGHT_INVALID)
   {
      node->min_weight = MIN(node->min_weight, weight);
   }
}


/*-----------------------------------------------------------------------
//
// Function: bounds_remove()
//
//   Update the weight bounds of node for the removal of a clause with
//   the given weight: a bound reached by the clause becomes invalid.
//
// Global Variables: -
//
// Side Effects    : Changes node
//
/----------------------------------------------------------------------*/

static void bounds_remove(FBIndex_p node, long weight)
{
   if(weight >= node->max_weight)
   {
      node->max_weight = FS_WEIGHT_INVALID;
   }
   if(weight <= node->min_weight)
   {
      node->min_weight = FS_WEIGHT_INVALID;
   }
}


/*-----------------------------------------------------------------------
//
// Function: long_cmp(), intorp_cmp()
//
//   Compare two longs (for qsort()), or the integer values of two
//   IntOrP cells (for PStackSort()).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static int long_cmp(const void* a, const void* b)
{
   long x = *(const long*)a, y = *(const long*)b;

   return (x > y) - (x < y);
}

static int intorp_cmp(const void* a, const void* b)
{
   long x = ((const IntOrP*)a)->i_val, y = ((const IntOrP*)b)->i_val;

   return (x > y) - (x < y);
}


/*-----------------------------------------------------------------------
//
// Function: even_threshold()
//
//   Given the values of feature for the n entries of leaf (in vals,
//   used as scratch space), return the threshold that splits them
//   most evenly into values up to and above it (ties go to the
//   smaller threshold), and the imbalance (the difference in size of
//   the two parts) in *imbalance. If all values are equal, return -1
//   (with imbalance n).
//
// Global Variables: -
//
// Side Effects    : Sorts vals
//
/----------------------------------------------------------------------*/

static long even_threshold(FBIndex_p leaf, long feature, long *vals,
                           long *imbalance)
{
   long      n = 0, i, imb, res = -1;
   FSEntry_p entry;

   for(entry = leaf->u1.entries; entry; entry = entry->next)
   {
      vals[n++] = FeatureSetGetVal(entry->fset, feature);
   }
   qsort(vals, n, sizeof(long), long_cmp);
   *imbalance = n;
   for(i=0; i<n-1; i++)
   {
      if(vals[i] != vals[i+1])
      {
         imb = labs(2*(i+1) - n);
         if(imb < *imbalance)
         {
            *imbalance = imb;
            res = vals[i];
         }
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: choose_split()
//
//   Select the feature and threshold of the decision node replacing
//   leaf (which has entries with at least two different feature
//   sets). With FTSplitEven, the feature is the one (among those on
//   which the entries do not all agree) with the most even split
//   (ties go to the smaller feature), otherwise the one selected by
//   the split rule. The threshold always splits the entries as
//   evenly as possible for the feature.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void choose_split(FBIndex_p leaf, FTSplit_p split,
                         long *feature, long *threshold)
{
   long      *vals, imbalance, score = 0, best_imb, t;
   FSEntry_p first = leaf->u1.entries, entry;

   assert(leaf->entry_count > 1);

   vals = SizeMalloc(leaf->entry_count*sizeof(long));
   *feature = -1;
   if(split->rule == FTSplitEven)
   {
      PStack_p      cands = PStackAlloc();
      PStackPointer i;
      long          f, last = -1;

      for(entry = first->next; entry; entry = entry->next)
      {
         FeatureSetPushDiffs(first->fset, entry->fset, cands);
      }
      PStackSort(cands, intorp_cmp);
      best_imb = LONG_MAX;
      for(i=0; i<PStackGetSP(cands); i++)
      {
         f = PStackElementInt(cands, i);
         if(f == last)
         {
            continue;
         }
         last = f;
         t = even_threshold(leaf, f, vals, &imbalance);
         if(t != -1 && imbalance < best_imb)
         {
            best_imb   = imbalance;
            *feature   = f;
            *threshold = t;
         }
      }
      PStackFree(cands);
   }
   else
   {
      for(entry = first->next; entry; entry = entry->next)
      {
         FeatureSetSplitConsider(first->fset, entry->fset,
                                 split, feature, &score);
      }
      *threshold = even_threshold(leaf, *feature, vals, &imbalance);
   }
   assert(*feature != -1);
   assert(*threshold != -1);
   SizeFree(vals, leaf->entry_count*sizeof(long));
}


/*-----------------------------------------------------------------------
//
// Function: leaf_split()
//
//   Replace the leaf at *ref (with more than one entry) by a decision
//   node (see choose_split()) with two new leaves for its entries as
//   successors. The entries keep their relative order.
//
// Global Variables: -
//
// Side Effects    : Changes index, memory operations
//
/----------------------------------------------------------------------*/

static void leaf_split(FBIndex_p *ref, long *storage, FTSplit_p split)
{
   FBIndex_p leaf = *ref, node;
   FSEntry_p entry, next;
   long      feature, threshold;

   choose_split(leaf, split, &feature, &threshold);

   node = FBIndexCellAlloc();
   *storage += FBINDEX_MEM;
   node->final        = false;
   node->clause_count = leaf->clause_count;
   node->feature      = feature;
   node->threshold    = threshold;
   node->entry_count  = 0;
   node->max_weight   = leaf->max_weight;
   node->min_weight   = leaf->min_weight;
   node->u1.succ.le   = leaf_alloc(storage);
   node->u1.succ.gt   = leaf_alloc(storage);

   for(entry = leaf->u1.entries; entry; entry = next)
   {
      next = entry->next;
      leaf_append((FeatureSetGetVal(entry->fset, feature) <= threshold)?
                  node->u1.succ.le : node->u1.succ.gt,
                  entry);
   }
   assert(node->u1.succ.le->entry_count && node->u1.succ.gt->entry_count);
   leaf->u1.entries = NULL;
   node_free(storage, leaf);
   *ref = node;
}



/*-----------------------------------------------------------------------
//
// Function: lists_add()
//
//   Add a new entry to the lists of all features of its set (if
//   lists is not NULL).
//
// Global Variables: -
//
// Side Effects    : Changes lists, memory operations
//
/----------------------------------------------------------------------*/

static void lists_add(FBLists_p lists, FSEntry_p entry)
{
   long     i, f;
   OrdSet_p *set;

   if(!lists)
   {
      return;
   }
   for(i=0; i<entry->fset->size; i++)
   {
      f = entry->fset->pairs[i].feature;
      set = (OrdSet_p*)&(PDArrayElementP(lists->entries, f));
      OrdSetStore(set, entry);
      PDArrayElementInt(lists->sizes, f)++;
      lists->cells++;
   }
}


/*-----------------------------------------------------------------------
//
// Function: lists_remove()
//
//   Remove an entry (about to be freed) from the lists of all
//   features of its set (if lists is not NULL).
//
// Global Variables: -
//
// Side Effects    : Changes lists, memory operations
//
/----------------------------------------------------------------------*/

static void lists_remove(FBLists_p lists, FSEntry_p entry)
{
   long     i, f;
   OrdSet_p *set;

   if(!lists)
   {
      return;
   }
   for(i=0; i<entry->fset->size; i++)
   {
      f = entry->fset->pairs[i].feature;
      set = (OrdSet_p*)&(PDArrayElementP(lists->entries, f));
      UNUSED(OrdSetDeleteEntry(set, entry));
      assert(PDArrayElementInt(lists->sizes, f) > 0);
      PDArrayElementInt(lists->sizes, f)--;
      lists->cells--;
   }
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: FBListsAlloc()
//
//   Allocate empty per-feature entry lists.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

FBLists_p FBListsAlloc(void)
{
   FBLists_p handle = FBListsCellAlloc();

   handle->entries = PDArrayAlloc(64, 0);
   handle->sizes   = PDIntArrayAlloc(64, 0);
   handle->cells   = 0;

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: FBListsFree()
//
//   Free per-feature entry lists (but not the entries).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void FBListsFree(FBLists_p junk)
{
   long i;

   if(junk)
   {
      for(i=0; i<junk->entries->size; i++)
      {
         OrdSetFree(PDArrayElementP(junk->entries, i));
      }
      PDArrayFree(junk->entries);
      PDArrayFree(junk->sizes);
      FBListsCellFree(junk);
   }
}


/*-----------------------------------------------------------------------
//
// Function: FBListsShortest()
//
//   Return the shortest list of entries among those of the features
//   of fset (NULL if it is empty, or if fset has no features, which
//   does not happen for clause feature sets).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

OrdSet_p FBListsShortest(FBLists_p lists, FeatureSet_p fset)
{
   long i, f, size, best = -1, best_size = LONG_MAX;

   for(i=0; i<fset->size; i++)
   {
      f = fset->pairs[i].feature;
      size = PDArrayElementInt(lists->sizes, f);
      if(size < best_size)
      {
         best_size = size;
         best = f;
         if(!size)
         {
            break;
         }
      }
   }
   return (best == -1)? NULL : PDArrayElementP(lists->entries, best);
}


/*-----------------------------------------------------------------------
//
// Function: FBIndexFree()
//
//   Free an index (subtree), including the feature sets of its
//   leaves. Clauses referencing these sets have to be gone already
//   (they go with their clause sets).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void FBIndexFree(FBIndex_p junk)
{
   long      dummy = 0;
   FSEntry_p entry, next;

   if(!junk)
   {
      return;
   }
   if(junk->final)
   {
      for(entry = junk->u1.entries; entry; entry = next)
      {
         next = entry->next;
         FSEntryFree(&dummy, entry);
      }
   }
   else
   {
      FBIndexFree(junk->u1.succ.le);
      FBIndexFree(junk->u1.succ.gt);
   }
   FBIndexCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: FBIndexInsert()
//
//   Insert clause (with its feature set in clause->feature_set) into
//   the binary feature tree at *root. The clause follows the tests of
//   the decision nodes to a leaf. If the leaf has an entry with the
//   same feature set, the clause is stored there and references the
//   entry's set (its own is freed). Otherwise, a new entry is added
//   to the leaf, and if the leaf then has more entries than the
//   bucket size, it is split (see leaf_split()). A new entry is added
//   to the per-feature lists (if lists is not NULL). *storage is
//   updated.
//
// Global Variables: -
//
// Side Effects    : Changes index, may change clause->feature_set
//
/----------------------------------------------------------------------*/

void FBIndexInsert(FBIndex_p *root, long *storage, FTSplit_p split,
                   FBLists_p lists, Clause_p clause)
{
   FBIndex_p    *ref, node, leaf;
   FSEntry_p    entry;
   FeatureSet_p fset = clause->feature_set;

   assert(fset);

   FeatureSetSplitCount(split, fset, 1);
   ref = root;
   while(*ref && !(*ref)->final)
   {
      node = *ref;
      node->clause_count++;
      bounds_add(node, clause->weight);
      ref = (FeatureSetGetVal(fset, node->feature) <= node->threshold)?
         &(node->u1.succ.le) : &(node->u1.succ.gt);
   }
   if(!*ref)
   {
      *ref = leaf_alloc(storage);
   }
   leaf = *ref;
   if(!leaf->entry_count)
   {
      /* A new leaf has exact bounds */
      leaf->max_weight = clause->weight;
      leaf->min_weight = clause->weight;
   }
   else
   {
      bounds_add(leaf, clause->weight);
   }
   for(entry = leaf->u1.entries; entry; entry = entry->next)
   {
      if(FeatureSetEqual(entry->fset, fset))
      {
         FeatureSetFree(clause->feature_set);
         clause->feature_set = entry->fset;
         break;
      }
   }
   if(!entry)
   {
      entry = FSEntryAlloc(storage, fset);
      leaf_append(leaf, entry);
      lists_add(lists, entry);
   }
   entry->clause_count++;
   leaf->clause_count++;
   OrdSetStore(&(entry->clauses), clause);

   if(leaf->entry_count > split->bucket_size)
   {
      leaf_split(ref, storage, split);
   }
}


/*-----------------------------------------------------------------------
//
// Function: FBIndexDelete()
//
//   Delete clause from the binary feature tree at *root. Return true
//   if the clause was found, false otherwise. An entry that becomes
//   empty is removed (together with its feature set, and from the
//   per-feature lists if lists is not NULL). A leaf without
//   entries is removed, and its parent is replaced by the other
//   successor. The caller has to reset clause->feature_set. *storage
//   is updated.
//
// Global Variables: -
//
// Side Effects    : Changes index
//
/----------------------------------------------------------------------*/

bool FBIndexDelete(FBIndex_p *root, long *storage, FTSplit_p split,
                   FBLists_p lists, Clause_p clause)
{
   FeatureSet_p  fset = clause->feature_set;
   FBIndex_p     *ref, node, leaf, parent;
   FSEntry_p     *eref = NULL, entry = NULL;
   PStack_p      path;  /* References to the decision nodes */
   PStackPointer i;
   bool          res = false;

   assert(fset);
   assert(clause->weight == ClauseStandardWeight(clause));

   path = PStackAlloc();
   ref = root;
   while(*ref && !(*ref)->final)
   {
      node = *ref;
      PStackPushP(path, ref);
      ref = (FeatureSetGetVal(fset, node->feature) <= node->threshold)?
         &(node->u1.succ.le) : &(node->u1.succ.gt);
   }
   leaf = *ref;
   if(leaf)
   {
      for(eref = &(leaf->u1.entries); *eref; eref = &((*eref)->next))
      {
         if((*eref)->fset == fset)
         {
            entry = *eref;
            break;
         }
      }
      res = entry && OrdSetDeleteEntry(&(entry->clauses), clause);
   }
   if(res)
   {
      FeatureSetSplitCount(split, fset, -1);
      for(i=0; i<PStackGetSP(path); i++)
      {
         node = *(FBIndex_p*)PStackElementP(path, i);
         node->clause_count--;
         assert(node->clause_count > 0);
         bounds_remove(node, clause->weight);
      }
      bounds_remove(leaf, clause->weight);
      leaf->clause_count--;
      entry->clause_count--;
      if(!entry->clause_count)
      {
         *eref = entry->next;
         leaf->entry_count--;
         lists_remove(lists, entry);
         FSEntryFree(storage, entry); /* Frees fset */
      }
      if(!leaf->clause_count)
      {
         assert(!leaf->entry_count);
         if(PStackEmpty(path))
         {
            *root = NULL;
         }
         else
         {
            FBIndex_p *pref = PStackPopP(path);

            parent = *pref;
            *pref = (parent->u1.succ.le == leaf)?
               parent->u1.succ.gt : parent->u1.succ.le;
            node_free(storage, parent);
         }
         node_free(storage, leaf);
      }
   }
   PStackFree(path);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: FBIndexMaxWeight()
//
//   Return the maximal standard weight of the clauses at or below
//   index (which must not be empty), recomputing invalid bounds.
//
// Global Variables: -
//
// Side Effects    : Updates the bounds
//
/----------------------------------------------------------------------*/

long FBIndexMaxWeight(FBIndex_p index)
{
   assert(index && index->clause_count);

   if(index->max_weight == FS_WEIGHT_INVALID)
   {
      long res = 0;

      if(index->final)
      {
         FSEntry_p entry;
         OrdSet_p  handle;

         for(entry = index->u1.entries; entry; entry = entry->next)
         {
            for(handle = OrdSetFirst(entry->clauses); handle;
                handle = OrdSetNext(entry->clauses, handle))
            {
               res = MAX(res, ((Clause_p)handle->key)->weight);
            }
         }
      }
      else
      {
         res = MAX(FBIndexMaxWeight(index->u1.succ.le),
                   FBIndexMaxWeight(index->u1.succ.gt));
      }
      index->max_weight = res;
   }
   return index->max_weight;
}


/*-----------------------------------------------------------------------
//
// Function: FBIndexMinWeight()
//
//   Return the minimal standard weight of the clauses at or below
//   index (which must not be empty), recomputing invalid bounds.
//
// Global Variables: -
//
// Side Effects    : Updates the bounds
//
/----------------------------------------------------------------------*/

long FBIndexMinWeight(FBIndex_p index)
{
   assert(index && index->clause_count);

   if(index->min_weight == FS_WEIGHT_INVALID)
   {
      long res = LONG_MAX;

      if(index->final)
      {
         FSEntry_p entry;
         OrdSet_p  handle;

         for(entry = index->u1.entries; entry; entry = entry->next)
         {
            for(handle = OrdSetFirst(entry->clauses); handle;
                handle = OrdSetNext(entry->clauses, handle))
            {
               res = MIN(res, ((Clause_p)handle->key)->weight);
            }
         }
      }
      else
      {
         res = MIN(FBIndexMinWeight(index->u1.succ.le),
                   FBIndexMinWeight(index->u1.succ.gt));
      }
      index->min_weight = res;
   }
   return index->min_weight;
}


/*-----------------------------------------------------------------------
//
// Function: FBIndexCountNodes()
//
//   Count the nodes of the binary feature tree (only the leaves if
//   leaves is true).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long FBIndexCountNodes(FBIndex_p index, bool leaves)
{
   if(!index)
   {
      return 0;
   }
   if(index->final)
   {
      assert(index->clause_count && index->u1.entries);
      return 1;
   }
   return (leaves? 0 : 1) +
      FBIndexCountNodes(index->u1.succ.le, leaves) +
      FBIndexCountNodes(index->u1.succ.gt, leaves);
}


/*-----------------------------------------------------------------------
//
// Function: FBIndexCollectStats()
//
//   Add the sizes of the binary feature set tree at index to stats.
//
// Global Variables: -
//
// Side Effects    : Changes stats
//
/----------------------------------------------------------------------*/

void FBIndexCollectStats(FBIndex_p index, FTStats_p stats)
{
   if(!index)
   {
      return;
   }
   if(index->final)
   {
      stats->leaves++;
      FSEntriesCollectStats(index->u1.entries, stats);
   }
   else
   {
      stats->inner++;
      FBIndexCollectStats(index->u1.succ.le, stats);
      FBIndexCollectStats(index->u1.succ.gt, stats);
   }
}


/*-----------------------------------------------------------------------
//
// Function: FBListsStorage()
//
//   Return the memory of the per-feature lists (arrays and list
//   cells), 0 for NULL.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long FBListsStorage(FBLists_p lists)
{
   if(!lists)
   {
      return 0;
   }
   return MEMSIZE(FBListsCell)+PDArrayStorage(lists->entries)+
      PDArrayStorage(lists->sizes)+lists->cells*ORDSET_CELL_MEM;
}


/*-----------------------------------------------------------------------
//
// Function: FBIndexPrint()
//
//   Pretty-print the binary feature tree.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void FBIndexPrint(FILE* out, FBIndex_p index, bool fullterms)
{
   fprintf(out, "* ROOT *\n");
   fb_index_print(out, index, fullterms, 0);
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
