/*-----------------------------------------------------------------------

  File  : ccl_fsindexing.c

  Author: Stephan Schulz (schulz@eprover.org)

  Contents

  Feature set tree indexing for subsumption (see ccl_fsindexing.h).

  Copyright 2026 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: Thu Nov 12 2026

  -----------------------------------------------------------------------*/

#include "ccl_fsindexing.h"


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
// Function: fs_index_print()
//
//   Print the index rooted at node.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void fs_index_print(FILE* out, FSIndex_p node, bool fullterms,
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
      IntMapIter_p iter = IntMapIterAlloc(node->u1.successors, 0, LONG_MAX);
      long key = 0;
      FSIndex_p succ;

      print_lvl(out, level);
      fprintf(out, "Test feature %ld (%ld clauses)\n",
              node->feature, node->clause_count);
      while((succ = IntMapIterNext(iter, &key)))
      {
         print_lvl(out, level);
         fprintf(out, "Value %ld:\n", key);
         fs_index_print(out, succ, fullterms, level+1);
      }
      IntMapIterFree(iter);
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

static FSIndex_p leaf_alloc(long *storage)
{
   FSIndex_p handle = FSIndexCellAlloc();

   handle->final         = true;
   handle->clause_count  = 0;
   handle->feature       = -1;
   handle->entry_count   = 0;
   handle->max_weight    = FS_WEIGHT_INVALID;
   handle->min_weight    = FS_WEIGHT_INVALID;
   handle->u1.entries    = NULL;
   *storage += FSINDEX_MEM;

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

static void leaf_append(FSIndex_p leaf, FSEntry_p entry)
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
// Function: decision_alloc()
//
//   Allocate a new decision node testing feature, without
//   successors (and with invalid weight bounds).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static FSIndex_p decision_alloc(long *storage, long feature)
{
   FSIndex_p handle = FSIndexCellAlloc();

   handle->final         = false;
   handle->clause_count  = 0;
   handle->feature       = feature;
   handle->entry_count   = 0;
   handle->max_weight    = FS_WEIGHT_INVALID;
   handle->min_weight    = FS_WEIGHT_INVALID;
   handle->u1.successors = IntMapAlloc();
   *storage += FSINDEX_MEM+IntMapStorage(handle->u1.successors);

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: node_free()
//
//   Free a single node (a leaf, which has to be without entries, or a
//   decision node with its successor map, but not the successors),
//   and account for the memory.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void node_free(long *storage, FSIndex_p junk)
{
   *storage -= FSINDEX_MEM;
   if(junk->final)
   {
      assert(!junk->u1.entries);
   }
   else
   {
      *storage -= IntMapStorage(junk->u1.successors);
      IntMapFree(junk->u1.successors);
   }
   FSIndexCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: succ_assign()
//
//   Make succ the successor of the decision node for value key.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void succ_assign(long *storage, FSIndex_p node, long key,
                        FSIndex_p succ)
{
   assert(!node->final);
   assert(key >= 0);

   *storage -= IntMapStorage(node->u1.successors);
   IntMapAssign(node->u1.successors, key, succ);
   *storage += IntMapStorage(node->u1.successors);
}


/*-----------------------------------------------------------------------
//
// Function: succ_delete()
//
//   Remove the successor for value key from the decision node.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void succ_delete(long *storage, FSIndex_p node, long key)
{
   assert(!node->final);

   *storage -= IntMapStorage(node->u1.successors);
   IntMapDelKey(node->u1.successors, key);
   *storage += IntMapStorage(node->u1.successors);
}


/*-----------------------------------------------------------------------
//
// Function: single_succ()
//
//   If the decision node has exactly one successor, return it,
//   otherwise return NULL.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static FSIndex_p single_succ(FSIndex_p node)
{
   IntMapIter_p iter = IntMapIterAlloc(node->u1.successors, 0, LONG_MAX);
   FSIndex_p    res, other;
   long         key;

   res = IntMapIterNext(iter, &key);
   other = IntMapIterNext(iter, &key);
   IntMapIterFree(iter);

   return other? NULL : res;
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

static void bounds_add(FSIndex_p node, long weight)
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

static void bounds_remove(FSIndex_p node, long weight)
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
// Function: leaf_split()
//
//   Replace the leaf at *ref (with more than one entry) by a decision
//   node on a feature on which its entries do not all agree (selected
//   by split), with new leaves for the entries as successors. The
//   entries keep their relative order.
//
// Global Variables: -
//
// Side Effects    : Changes index, memory operations
//
/----------------------------------------------------------------------*/

static void leaf_split(FSIndex_p *ref, long *storage, FTSplit_p split)
{
   FSIndex_p leaf = *ref, node, succ;
   FSEntry_p entry, next;
   long      feature = -1, score = 0, key;

   assert(leaf->final);
   assert(leaf->entry_count > 1);

   for(entry = leaf->u1.entries->next; entry; entry = entry->next)
   {
      FeatureSetSplitConsider(leaf->u1.entries->fset, entry->fset,
                              split, &feature, &score);
   }
   assert(feature != -1);

   node = decision_alloc(storage, feature);
   node->clause_count = leaf->clause_count;
   node->max_weight   = leaf->max_weight;
   node->min_weight   = leaf->min_weight;

   for(entry = leaf->u1.entries; entry; entry = next)
   {
      next = entry->next;
      key  = FeatureSetGetVal(entry->fset, feature);
      succ = IntMapGetVal(node->u1.successors, key);
      if(!succ)
      {
         succ = leaf_alloc(storage);
         succ_assign(storage, node, key, succ);
      }
      leaf_append(succ, entry);
   }
   leaf->u1.entries = NULL;
   node_free(storage, leaf);
   *ref = node;
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: FSEntryAlloc()
//
//   Allocate a new, empty leaf entry owning fset.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

FSEntry_p FSEntryAlloc(long *storage, FeatureSet_p fset)
{
   FSEntry_p handle = FSEntryCellAlloc();

   handle->fset         = fset;
   handle->clause_count = 0;
   handle->clauses      = NULL;
   handle->next         = NULL;
   *storage += FSENTRY_MEM;

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: FSEntryFree()
//
//   Free a leaf entry, its feature set, and its clause set (but not
//   the clauses).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void FSEntryFree(long *storage, FSEntry_p junk)
{
   *storage -= FSENTRY_MEM;
   OrdSetFree(junk->clauses);
   FeatureSetFree(junk->fset);
   FSEntryCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: FSIndexFree()
//
//   Free an index (subtree), including the feature sets of
//   its leaves. Clauses referencing these sets have to be gone
//   already (they go with their clause sets).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void FSIndexFree(FSIndex_p junk)
{
   IntMapIter_p iter;
   long         i, dummy = 0;
   FSIndex_p    succ;
   FSEntry_p    entry, next;

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
      iter = IntMapIterAlloc(junk->u1.successors, 0, LONG_MAX);
      while((succ = IntMapIterNext(iter, &i)))
      {
         FSIndexFree(succ);
      }
      IntMapIterFree(iter);
      IntMapFree(junk->u1.successors);
   }
   FSIndexCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: FSIndexInsert()
//
//   Insert clause (with its feature set in clause->feature_set)
//   into the feature tree at *root. The clause follows the tests of
//   the decision nodes. If there is no successor for its value, a new
//   leaf is added. If it reaches a leaf with an entry with the same
//   feature set, it is stored there and references the entry's set
//   (its own is freed). Otherwise, a new entry is added to the leaf,
//   and if the leaf then has more entries than the bucket size, it is
//   split (see leaf_split()). *storage is updated.
//
// Global Variables: -
//
// Side Effects    : Changes index, may change clause->feature_set
//
/----------------------------------------------------------------------*/

void FSIndexInsert(FSIndex_p *root, long *storage, FTSplit_p split,
                   Clause_p clause)
{
   FSIndex_p    *ref, node, leaf;
   FSEntry_p    entry = NULL;
   FeatureSet_p fset = clause->feature_set;
   long         key;

   assert(fset);

   FeatureSetSplitCount(split, fset, 1);
   ref = root;
   while(*ref && !(*ref)->final)
   {
      node = *ref;
      node->clause_count++;
      bounds_add(node, clause->weight);
      key = FeatureSetGetVal(fset, node->feature);
      if(!IntMapGetVal(node->u1.successors, key))
      {
         succ_assign(storage, node, key, leaf_alloc(storage));
      }
      ref = (FSIndex_p*)IntMapGetRef(node->u1.successors, key);
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
// Function: FSIndexDelete()
//
//   Delete clause from the feature tree at *root. Return true if the
//   clause was found, false otherwise. An entry that becomes empty is
//   removed (together with its feature set), a leaf without entries is
//   removed, and a decision node left with a single successor is
//   replaced by that successor. The caller has to reset
//   clause->feature_set. *storage is updated.
//
// Global Variables: -
//
// Side Effects    : Changes index
//
/----------------------------------------------------------------------*/

bool FSIndexDelete(FSIndex_p *root, long *storage, FTSplit_p split,
                   Clause_p clause)
{
   FeatureSet_p  fset = clause->feature_set;
   FSIndex_p     node, parent, succ;
   FSEntry_p     *eref = NULL, entry = NULL;
   PStack_p      path;
   PStackPointer i;
   bool          res = false;

   assert(fset);
   assert(clause->weight == ClauseStandardWeight(clause));

   path = PStackAlloc();
   node = *root;
   while(node && !node->final)
   {
      PStackPushP(path, node);
      node = IntMapGetVal(node->u1.successors,
                          FeatureSetGetVal(fset, node->feature));
   }
   if(node)
   {
      for(eref = &(node->u1.entries); *eref; eref = &((*eref)->next))
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
         parent = PStackElementP(path, i);
         parent->clause_count--;
         assert(parent->clause_count > 0);
         bounds_remove(parent, clause->weight);
      }
      bounds_remove(node, clause->weight);
      node->clause_count--;
      entry->clause_count--;
      if(!entry->clause_count)
      {
         /* Freed at the end, fset is still needed for the path */
         *eref = entry->next;
         node->entry_count--;
      }
      if(!node->clause_count)
      {
         assert(!node->entry_count);
         if(PStackEmpty(path))
         {
            *root = NULL;
         }
         else
         {
            parent = PStackPopP(path);
            succ_delete(storage, parent,
                        FeatureSetGetVal(fset, parent->feature));
            succ = single_succ(parent);
            if(succ)
            {
               if(PStackEmpty(path))
               {
                  *root = succ;
               }
               else
               {
                  FSIndex_p grandparent = PStackTopP(path);
                  succ_assign(storage, grandparent,
                              FeatureSetGetVal(fset, grandparent->feature),
                              succ);
               }
               node_free(storage, parent);
            }
         }
         node_free(storage, node);
      }
      if(!entry->clause_count)
      {
         FSEntryFree(storage, entry); /* Frees fset */
      }
   }
   PStackFree(path);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: FSIndexMaxWeight()
//
//   Return the maximal standard weight of the clauses at or below
//   index (which must not be empty), recomputing invalid bounds.
//
// Global Variables: -
//
// Side Effects    : Updates the bounds
//
/----------------------------------------------------------------------*/

long FSIndexMaxWeight(FSIndex_p index)
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
         IntMapIter_p iter = IntMapIterAlloc(index->u1.successors,
                                             0, LONG_MAX);
         FSIndex_p    succ;
         long         key;

         while((succ = IntMapIterNext(iter, &key)))
         {
            res = MAX(res, FSIndexMaxWeight(succ));
         }
         IntMapIterFree(iter);
      }
      index->max_weight = res;
   }
   return index->max_weight;
}


/*-----------------------------------------------------------------------
//
// Function: FSIndexMinWeight()
//
//   Return the minimal standard weight of the clauses at or below
//   index (which must not be empty), recomputing invalid bounds.
//
// Global Variables: -
//
// Side Effects    : Updates the bounds
//
/----------------------------------------------------------------------*/

long FSIndexMinWeight(FSIndex_p index)
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
         IntMapIter_p iter = IntMapIterAlloc(index->u1.successors,
                                             0, LONG_MAX);
         FSIndex_p    succ;
         long         key;

         while((succ = IntMapIterNext(iter, &key)))
         {
            res = MIN(res, FSIndexMinWeight(succ));
         }
         IntMapIterFree(iter);
      }
      index->min_weight = res;
   }
   return index->min_weight;
}


/*-----------------------------------------------------------------------
//
// Function: FSIndexCountNodes()
//
//   Count the nodes of the feature tree (only the leaves if leaves is
//   true).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long FSIndexCountNodes(FSIndex_p index, bool leaves)
{
   long         res = 0, i;
   IntMapIter_p iter;
   FSIndex_p    succ;

   if(index)
   {
      if(index->final)
      {
         assert(index->clause_count && index->u1.entries);
         res++;
      }
      else
      {
         if(!leaves)
         {
            res++;
         }
         iter = IntMapIterAlloc(index->u1.successors, 0, LONG_MAX);
         while((succ = IntMapIterNext(iter, &i)))
         {
            res += FSIndexCountNodes(succ, leaves);
         }
         IntMapIterFree(iter);
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: FSIndexPrint()
//
//   Pretty-print the feature tree.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void FSIndexPrint(FILE* out, FSIndex_p index, bool fullterms)
{
   fprintf(out, "* ROOT *\n");
   fs_index_print(out, index, fullterms, 0);
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
