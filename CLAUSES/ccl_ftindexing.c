/*-----------------------------------------------------------------------

  File  : ccl_ftindexing.c

  Author: Stephan Schulz (schulz@eprover.org)

  Contents

  Feature tree indexing for subsumption (see ccl_ftindexing.h).

  Copyright 2026 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: Thu Nov 12 2026

  -----------------------------------------------------------------------*/

#include "ccl_ftindexing.h"


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
// Function: ft_index_print()
//
//   Print the index rooted at node.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void ft_index_print(FILE* out, FTIndex_p node, bool fullterms,
                           int level)
{
   if(!node)
   {
      print_lvl(out, level);
      fprintf(out, "(empty)\n");
   }
   else if(node->final)
   {
      OrdSet_p handle;

      print_lvl(out, level);
      fprintf(out, "Leaf (%ld clauses): ", node->clause_count);
      FreqVectorPrint(out, node->vec);
      for(handle = OrdSetFirst(node->u1.clauses); handle;
          handle = OrdSetNext(node->u1.clauses, handle))
      {
         print_lvl(out, level+1);
         ClausePrint(out, handle->key, fullterms);
         fprintf(out, "\n");
      }
   }
   else
   {
      IntMapIter_p iter = IntMapIterAlloc(node->u1.successors, 0, LONG_MAX);
      long key = 0;
      FTIndex_p succ;

      print_lvl(out, level);
      fprintf(out, "Test feature %ld (%ld clauses)\n",
              node->feature, node->clause_count);
      while((succ = IntMapIterNext(iter, &key)))
      {
         print_lvl(out, level);
         fprintf(out, "Value %ld:\n", key);
         ft_index_print(out, succ, fullterms, level+1);
      }
      IntMapIterFree(iter);
   }
}


/*-----------------------------------------------------------------------
//
// Function: leaf_alloc()
//
//   Allocate a new, empty leaf owning vec. The leaf vector gets its
//   list of non-zero positions for fast compatibility checks.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static FTIndex_p leaf_alloc(long *storage, FreqVector_p vec)
{
   FTIndex_p handle = FTIndexCellAlloc();

   handle->final         = true;
   handle->clause_count  = 0;
   handle->feature       = -1;
   handle->vec           = vec;
   handle->u1.clauses    = NULL;
   FreqVectorComputeNZ(vec);
   *storage += FTINDEX_MEM;

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: decision_alloc()
//
//   Allocate a new decision node testing feature, without
//   successors.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static FTIndex_p decision_alloc(long *storage, long feature)
{
   FTIndex_p handle = FTIndexCellAlloc();

   handle->final         = false;
   handle->clause_count  = 0;
   handle->feature       = feature;
   handle->vec           = NULL;
   handle->u1.successors = IntMapAlloc();
   *storage += FTINDEX_MEM+IntMapStorage(handle->u1.successors);

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: node_free()
//
//   Free a single node (a leaf with its vector, or a decision node
//   with its successor map, but not the successors), and account for
//   the memory.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void node_free(long *storage, FTIndex_p junk)
{
   *storage -= FTINDEX_MEM;
   if(junk->final)
   {
      assert(!junk->u1.clauses);
      FreqVectorFree(junk->vec);
   }
   else
   {
      *storage -= IntMapStorage(junk->u1.successors);
      IntMapFree(junk->u1.successors);
   }
   FTIndexCellFree(junk);
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

static void succ_assign(long *storage, FTIndex_p node, long key,
                        FTIndex_p succ)
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

static void succ_delete(long *storage, FTIndex_p node, long key)
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

static FTIndex_p single_succ(FTIndex_p node)
{
   IntMapIter_p iter = IntMapIterAlloc(node->u1.successors, 0, LONG_MAX);
   FTIndex_p    res, other;
   long         key;

   res = IntMapIterNext(iter, &key);
   other = IntMapIterNext(iter, &key);
   IntMapIterFree(iter);

   return other? NULL : res;
}



/*-----------------------------------------------------------------------
//
// Function: ft_split_count()
//
//   Account for a clause with feature vector vec entering (delta 1)
//   or leaving (delta -1) the index, if split needs feature
//   frequencies.
//
// Global Variables: -
//
// Side Effects    : Changes split
//
/----------------------------------------------------------------------*/

static void ft_split_count(FTSplit_p split, FreqVector_p vec, long delta)
{
   if(FTSplitUsesFreq(split))
   {
      FTSplitCountClause(split, delta);
      for(long i=0; i<vec->size; i++)
      {
         if(vec->array[i])
         {
            FTSplitCountFeature(split, i, delta);
         }
      }
   }
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: FTIndexFree()
//
//   Free an index (subtree), including the vectors of its
//   leaves. Clauses referencing these vectors have to be gone
//   already (they go with their clause sets).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void FTIndexFree(FTIndex_p junk)
{
   IntMapIter_p iter;
   long         i;
   FTIndex_p    succ;

   if(!junk)
   {
      return;
   }
   if(junk->final)
   {
      OrdSetFree(junk->u1.clauses);
      FreqVectorFree(junk->vec);
   }
   else
   {
      iter = IntMapIterAlloc(junk->u1.successors, 0, LONG_MAX);
      while((succ = IntMapIterNext(iter, &i)))
      {
         FTIndexFree(succ);
      }
      IntMapIterFree(iter);
      IntMapFree(junk->u1.successors);
   }
   FTIndexCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: FTIndexInsert()
//
//   Insert clause (with its feature vector in clause->feature_vec)
//   into the feature tree at *root. The clause follows the tests of
//   the decision nodes. If there is no successor for its value, a new
//   leaf is added. If it reaches a leaf with the same vector, it is
//   stored there and references the leaf's vector (its own is
//   freed). If the leaf has a different vector, the leaf is replaced
//   by a decision node on a feature in which the vectors differ,
//   selected by split, with the old leaf and a new leaf for the clause as
//   successors. *storage is updated.
//
// Global Variables: -
//
// Side Effects    : Changes index, may change clause->feature_vec
//
/----------------------------------------------------------------------*/

void FTIndexInsert(FTIndex_p *root, long *storage, FTSplit_p split,
                   Clause_p clause)
{
   FTIndex_p    *ref, node, leaf = NULL;
   FreqVector_p vec = clause->feature_vec;
   long         key, feature;

   assert(vec);

   ft_split_count(split, vec, 1);
   ref = root;
   while(*ref && !(*ref)->final)
   {
      node = *ref;
      node->clause_count++;
      key = vec->array[node->feature];
      if(!IntMapGetVal(node->u1.successors, key))
      {
         leaf = leaf_alloc(storage, vec);
         succ_assign(storage, node, key, leaf);
         break;
      }
      ref = (FTIndex_p*)IntMapGetRef(node->u1.successors, key);
   }
   if(!leaf)
   {
      if(!*ref)
      {
         leaf = leaf_alloc(storage, vec);
         *ref = leaf;
      }
      else
      {
         feature = FreqVectorSplitFeature((*ref)->vec, vec, split);
         if(feature == -1)
         {
            leaf = *ref;
            FreqVectorFree(clause->feature_vec);
            clause->feature_vec = leaf->vec;
         }
         else
         {
            node = decision_alloc(storage, feature);
            node->clause_count = (*ref)->clause_count+1;
            succ_assign(storage, node, (*ref)->vec->array[feature], *ref);
            leaf = leaf_alloc(storage, vec);
            succ_assign(storage, node, vec->array[feature], leaf);
            *ref = node;
         }
      }
   }
   leaf->clause_count++;
   OrdSetStore(&(leaf->u1.clauses), clause);
}


/*-----------------------------------------------------------------------
//
// Function: FTIndexDelete()
//
//   Delete clause from the feature tree at *root. Return true if the
//   clause was found, false otherwise. A leaf that becomes empty is
//   removed (together with its vector), and a decision node left
//   with a single successor is replaced by that successor. The caller
//   has to reset clause->feature_vec. *storage is updated.
//
// Global Variables: -
//
// Side Effects    : Changes index
//
/----------------------------------------------------------------------*/

bool FTIndexDelete(FTIndex_p *root, long *storage, FTSplit_p split,
                   Clause_p clause)
{
   FreqVector_p  vec = clause->feature_vec;
   FTIndex_p     node, parent, succ;
   PStack_p      path;
   PStackPointer i;
   bool          res;

   assert(vec);

   path = PStackAlloc();
   node = *root;
   while(node && !node->final)
   {
      PStackPushP(path, node);
      node = IntMapGetVal(node->u1.successors, vec->array[node->feature]);
   }
   res = node && OrdSetDeleteEntry(&(node->u1.clauses), clause);
   if(res)
   {
      ft_split_count(split, vec, -1);
      assert(node->vec == vec);
      for(i=0; i<PStackGetSP(path); i++)
      {
         parent = PStackElementP(path, i);
         parent->clause_count--;
         assert(parent->clause_count > 0);
      }
      node->clause_count--;
      if(!node->clause_count)
      {
         if(PStackEmpty(path))
         {
            *root = NULL;
         }
         else
         {
            parent = PStackPopP(path);
            succ_delete(storage, parent, vec->array[parent->feature]);
            succ = single_succ(parent);
            if(succ)
            {
               if(PStackEmpty(path))
               {
                  *root = succ;
               }
               else
               {
                  FTIndex_p grandparent = PStackTopP(path);
                  succ_assign(storage, grandparent,
                              vec->array[grandparent->feature], succ);
               }
               node_free(storage, parent);
            }
         }
         node_free(storage, node); /* Frees vec */
      }
   }
   PStackFree(path);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: FTIndexCountNodes()
//
//   Count the nodes of the feature tree (only the leaves if leaves is
//   true).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long FTIndexCountNodes(FTIndex_p index, bool leaves)
{
   long         res = 0, i;
   IntMapIter_p iter;
   FTIndex_p    succ;

   if(index)
   {
      if(index->final)
      {
         assert(index->clause_count && index->u1.clauses);
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
            res += FTIndexCountNodes(succ, leaves);
         }
         IntMapIterFree(iter);
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: FTIndexCollectStats()
//
//   Add the sizes of the feature tree at index to stats.
//
// Global Variables: -
//
// Side Effects    : Changes stats
//
/----------------------------------------------------------------------*/

void FTIndexCollectStats(FTIndex_p index, FTStats_p stats)
{
   IntMapIter_p iter;
   FTIndex_p    succ;
   long         i;

   if(!index)
   {
      return;
   }
   if(index->final)
   {
      stats->leaves++;
      stats->sets++;
      stats->clauses += index->clause_count;
      stats->set_mem += FreqVectorStorage(index->vec);
   }
   else
   {
      stats->inner++;
      iter = IntMapIterAlloc(index->u1.successors, 0, LONG_MAX);
      while((succ = IntMapIterNext(iter, &i)))
      {
         FTIndexCollectStats(succ, stats);
      }
      IntMapIterFree(iter);
   }
}


/*-----------------------------------------------------------------------
//
// Function: FTIndexPrint()
//
//   Pretty-print the feature tree.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void FTIndexPrint(FILE* out, FTIndex_p index, bool fullterms)
{
   fprintf(out, "* ROOT *\n");
   ft_index_print(out, index, fullterms, 0);
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
