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
      OrdSet_p handle;

      print_lvl(out, level);
      fprintf(out, "Leaf (%ld clauses): ", node->clause_count);
      FeatureSetPrint(out, node->fset);
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
//   Allocate a new, empty leaf owning fset.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static FSIndex_p leaf_alloc(long *storage, FeatureSet_p fset)
{
   FSIndex_p handle = FSIndexCellAlloc();

   handle->final         = true;
   handle->clause_count  = 0;
   handle->feature       = -1;
   handle->fset          = fset;
   handle->u1.clauses    = NULL;
   *storage += FSINDEX_MEM;

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

static FSIndex_p decision_alloc(long *storage, long feature)
{
   FSIndex_p handle = FSIndexCellAlloc();

   handle->final         = false;
   handle->clause_count  = 0;
   handle->feature       = feature;
   handle->fset          = NULL;
   handle->u1.successors = IntMapAlloc();
   *storage += FSINDEX_MEM+IntMapStorage(handle->u1.successors);

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: node_free()
//
//   Free a single node (a leaf with its feature set, or a decision node
//   with its successor map, but not the successors), and account for
//   the memory.
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
      assert(!junk->u1.clauses);
      FeatureSetFree(junk->fset);
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



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


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
   long         i;
   FSIndex_p    succ;

   if(!junk)
   {
      return;
   }
   if(junk->final)
   {
      OrdSetFree(junk->u1.clauses);
      FeatureSetFree(junk->fset);
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
//   leaf is added. If it reaches a leaf with the same feature set, it
//   is stored there and references the leaf's set (its own is
//   freed). If the leaf has a different set, the leaf is replaced
//   by a decision node on the first feature in which the sets
//   differ, with the old leaf and a new leaf for the clause as
//   successors. *storage is updated.
//
// Global Variables: -
//
// Side Effects    : Changes index, may change clause->feature_set
//
/----------------------------------------------------------------------*/

void FSIndexInsert(FSIndex_p *root, long *storage, Clause_p clause)
{
   FSIndex_p    *ref, node, leaf = NULL;
   FeatureSet_p fset = clause->feature_set;
   long         key, feature;

   assert(fset);

   ref = root;
   while(*ref && !(*ref)->final)
   {
      node = *ref;
      node->clause_count++;
      key = FeatureSetGetVal(fset, node->feature);
      if(!IntMapGetVal(node->u1.successors, key))
      {
         leaf = leaf_alloc(storage, fset);
         succ_assign(storage, node, key, leaf);
         break;
      }
      ref = (FSIndex_p*)IntMapGetRef(node->u1.successors, key);
   }
   if(!leaf)
   {
      if(!*ref)
      {
         leaf = leaf_alloc(storage, fset);
         *ref = leaf;
      }
      else
      {
         feature = FeatureSetFirstDiff((*ref)->fset, fset);
         if(feature == -1)
         {
            leaf = *ref;
            FeatureSetFree(clause->feature_set);
            clause->feature_set = leaf->fset;
         }
         else
         {
            node = decision_alloc(storage, feature);
            node->clause_count = (*ref)->clause_count+1;
            succ_assign(storage, node,
                        FeatureSetGetVal((*ref)->fset, feature), *ref);
            leaf = leaf_alloc(storage, fset);
            succ_assign(storage, node, FeatureSetGetVal(fset, feature), leaf);
            *ref = node;
         }
      }
   }
   leaf->clause_count++;
   OrdSetStore(&(leaf->u1.clauses), clause);
}


/*-----------------------------------------------------------------------
//
// Function: FSIndexDelete()
//
//   Delete clause from the feature tree at *root. Return true if the
//   clause was found, false otherwise. A leaf that becomes empty is
//   removed (together with its feature set), and a decision node left
//   with a single successor is replaced by that successor. The caller
//   has to reset clause->feature_set. *storage is updated.
//
// Global Variables: -
//
// Side Effects    : Changes index
//
/----------------------------------------------------------------------*/

bool FSIndexDelete(FSIndex_p *root, long *storage, Clause_p clause)
{
   FeatureSet_p  fset = clause->feature_set;
   FSIndex_p     node, parent, succ;
   PStack_p      path;
   PStackPointer i;
   bool          res;

   assert(fset);

   path = PStackAlloc();
   node = *root;
   while(node && !node->final)
   {
      PStackPushP(path, node);
      node = IntMapGetVal(node->u1.successors,
                          FeatureSetGetVal(fset, node->feature));
   }
   res = node && OrdSetDeleteEntry(&(node->u1.clauses), clause);
   if(res)
   {
      assert(node->fset == fset);
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
         node_free(storage, node); /* Frees the set */
      }
   }
   PStackFree(path);
   return res;
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
