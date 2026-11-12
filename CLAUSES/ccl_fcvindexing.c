/*-----------------------------------------------------------------------

File  : ccl_fcvindexing.c

Author: Stephan Schulz

Contents

  Algorithms for frequency count vector indexing.

Copyright 1998-2011 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Tue Jul  1 13:09:10 CEST 2003
    New

-----------------------------------------------------------------------*/

#include "ccl_fcvindexing.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

PERF_CTR_DEFINE(FVIndexTimer);

FVIndexParmsCell FVIDefaultParameters =
{
   {
      FVIACFold,
      false,
      NULL,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
   },
   false,   /* use_perm_vectors */
   false,  /* eliminate_uninformative */
   FVINDEX_MAX_FEATURES_DEFAULT,
   FVINDEX_SYMBOL_SLACK_DEFAULT,
   FVITrie,       /* structure */
   FSAllFeatures, /* set_features */
   FTSplitFirst,  /* split_rule */
   1,             /* bucket_size */
   false          /* weight_bounds */
};

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
//   Prints enough dashes to indent a tree level.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void print_lvl(FILE* out, int level)
{
   for(int i=0; i<level; i++)
   {
      fprintf(out, "--");
   }
}


/*-----------------------------------------------------------------------
//
// Function: print_clauses()
//
//   Prints clauses stored in the leaf indented with level.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void print_clauses(FILE* out, OrdSet_p clauses, int level, bool fullterms)
{
   OrdSetIter_p stack = OrdSetTraverseInit(clauses);
   OrdSet_p  node  = NULL;
   Clause_p cl_handle   = NULL;

   while((node = OrdSetTraverseNext(stack)))
   {
      cl_handle = node->key;
      print_lvl(out, level);
      ClausePrint(out, cl_handle, fullterms);
      fprintf(stderr, " \n");
   }

   OrdSetTraverseExit(stack);
}


/*-----------------------------------------------------------------------
//
// Function: fv_index_print()
//
//   Driver function for printing fv index. To be initially called with
//   root for index and 0 for level.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void fv_index_print(FILE* out, FVIndex_p index, bool fullterms, int level)
{
   if(index->final)
   {
      print_clauses(out, index->u1.clauses, level+1, fullterms);
   }
   else
   {
      IntMapIter_p iterator = IntMapIterAlloc(index->u1.successors, 0, LONG_MAX);

      long key = 0;
      FVIndex_p succ;
      while((succ = IntMapIterNext(iterator, &key)))
      {
         print_lvl(out, level);
         fprintf(stderr, "Alternative %ld: \n", key);

         fv_index_print(out, succ, fullterms, level+1);
      }

      IntMapIterFree(iterator);
   }

}

/*-----------------------------------------------------------------------
//
// Function: insert_empty_node()
//
//   Insert an empty node into FVIndex at node node and key key.
//
// Global Variables: -
//
// Side Effects    : Memory operations, modifies index.
//
/----------------------------------------------------------------------*/

static FVIndex_p insert_empty_node(FVIndex_p node, FVIAnchor_p anchor, long key)
{
   FVIndex_p handle = FVIndexAlloc();

   assert(node);
   assert(key >= 0);

   if(!node->u1.successors)
   {
      node->u1.successors = IntMapAlloc();
      anchor->storage += IntMapStorage(node->u1.successors);
   }

   anchor->storage -= IntMapStorage(node->u1.successors);
   IntMapAssign(node->u1.successors, key, handle);
   anchor->storage += IntMapStorage(node->u1.successors);
   anchor->storage += FVINDEX_MEM;

   return handle;
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: FVIndexParmsInit()
//
//   Initialize a FVIndexParmCell with rational values.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void FVIndexParmsInit(FVIndexParms_p parms)
{
   *parms = FVIDefaultParameters;
}

/*-----------------------------------------------------------------------
//
// Function: FVIndexParmsAlloc()
//
//   Allocate an FVIndexParmsCell with rational values.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

FVIndexParms_p FVIndexParmsAlloc(void)
{
   FVIndexParms_p handle = FVIndexParmsCellAlloc();

   FVIndexParmsInit(handle);

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: FVIndexAlloc()
//
//   Allocate an empty and initialize FVIndexCell.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

FVIndex_p FVIndexAlloc(void)
{
   FVIndex_p handle = FVIndexCellAlloc();

   handle->clause_count  = 0;
   handle->vec           = NULL;
   handle->u1.clauses    = NULL;
   handle->u1.successors = NULL;
   handle->final         = false;

   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: FVIndexFree()
//
//   Free a FVIndex - recursively and slightly complex because of the
//   weird structure...
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/


void FVIndexFree(FVIndex_p junk)
{
   IntMapIter_p iter;
   long         i;
   FVIndex_p    succ;

   if(junk)
   {
      if(junk->final)
      {
         /* All clauses referencing the vector are gone with the
            clause sets */
         OrdSetFree(junk->u1.clauses);
         if(junk->vec)
         {
            FreqVectorFree(junk->vec);
         }
      }
      else if(junk->u1.successors)
      {
         iter = IntMapIterAlloc(junk->u1.successors, 0, LONG_MAX);
         while((succ = IntMapIterNext(iter, &i)))
         {
            FVIndexFree(succ);
         }
         IntMapIterFree(iter);
         IntMapFree(junk->u1.successors);
      }
   }
   FVIndexCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: FVIAnchorAlloc()
//
//   Allocate an (empty) FV index with the structure, set features
//   and split rule from parms (cspec and perm are passed
//   separately).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

FVIAnchor_p FVIAnchorAlloc(FVCollect_p cspec, PermVector_p perm,
                           FVIndexParms_p parms)
{
   FVIAnchor_p handle = FVIAnchorCellAlloc();
   bool        trie = (parms->structure == FVITrie);

   handle->perm_vector  = perm;
   handle->cspec        = cspec;
   handle->structure    = parms->structure;
   handle->set_features = parms->set_features;
   handle->index        = trie? FVIndexAlloc() : NULL;
   handle->ftindex      = NULL;
   handle->fsindex      = NULL;
   handle->split        = trie? NULL : FTSplitAlloc(parms->split_rule,
                                                parms->bucket_size);
   handle->weight_bounds = (parms->structure == FVISetTree) &&
                           parms->weight_bounds;
   handle->storage      = 0;

   return handle;
}



/*-----------------------------------------------------------------------
//
// Function: FVIAnchorFree()
//
//   Free a FV incex.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void FVIAnchorFree(FVIAnchor_p junk)
{
   assert(junk);

   /* fprintf(GlobalOut,
      COMCHAR" Freeing FVIndex. %ld leaves, %ld empty. Total nodes: %ld. Mem: %ld\n",
      FVIndexCountNodes(junk->index, true, false),
      FVIndexCountNodes(junk->index, true, true),
      FVIndexCountNodes(junk->index, false, false),
      FVIndexStorage(junk));*/

   if(junk->index)
   {
      FVIndexFree(junk->index);
   }
   FTIndexFree(junk->ftindex);
   FSIndexFree(junk->fsindex);
   FTSplitFree(junk->split);
   if(junk->perm_vector)
   {
      PermVectorFree(junk->perm_vector);
   }
   FVIAnchorCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: FVIndexGetNextNonEmptyNode()
//
//   Get the next node if it is not empty. Otherwise return NULL.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

FVIndex_p FVIndexGetNextNonEmptyNode(FVIndex_p node, long key)
{
   FVIndex_p handle;

   assert(!node->final);

   handle = IntMapGetVal(node->u1.successors, key);
   if(handle&&handle->clause_count)
   {
      return handle;
   }
   return NULL;
}



/*-----------------------------------------------------------------------
//
// Function: FVIndexInsert()
//
//   Insert clause into the index. The clause's feature vector is
//   computed if it does not have one. If the clause ends up in a new
//   leaf, the leaf takes over the vector, otherwise the clause's copy
//   is freed. Afterwards, the clause references the vector of its
//   leaf.
//
// Global Variables: -
//
// Side Effects    : Changes the index and the clause.
//
/----------------------------------------------------------------------*/

void FVIndexInsert(FVIAnchor_p index, Clause_p clause)
{
   FVIndex_p handle, newnode = NULL;
   FreqVector_p vec;
   long i;

   assert(clause);
   assert(!ClauseQueryProp(clause, CPIsSIndexed));
   assert(ClauseFeaturesAreValid(clause, index));

   if(index->structure == FVISetTree)
   {
      if(!clause->feature_set)
      {
         clause->feature_set =
            ClauseFeatureSetCompute(clause, index->set_features);
      }
      PERF_CTR_ENTRY(FVIndexTimer);
      ClauseSubsumeOrderSortLits(clause);
      FSIndexInsert(&(index->fsindex), &(index->storage), index->split,
                    clause);
      PERF_CTR_EXIT(FVIndexTimer);
      return;
   }

   if(!clause->feature_vec)
   {
      clause->feature_vec = FVIndexComputeVector(index, clause);
   }
   vec = clause->feature_vec;

   PERF_CTR_ENTRY(FVIndexTimer);

   ClauseSubsumeOrderSortLits(clause);

   if(index->structure == FVITree)
   {
      FTIndexInsert(&(index->ftindex), &(index->storage), index->split,
                    clause);
      PERF_CTR_EXIT(FVIndexTimer);
      return;
   }

   handle = index->index;
   handle->clause_count++;

   for(i=0; i<vec->size; i++)
   {
      assert(!handle->final);

      newnode = IntMapGetVal(handle->u1.successors, vec->array[i]);
      if(!newnode)
      {
         newnode = insert_empty_node(handle,
                                     index,
                                     vec->array[i]);
      }
      handle = newnode;
      handle->clause_count++;
   }
   handle->final = true;
   if(handle->vec)
   {
      FreqVectorFree(clause->feature_vec);
      clause->feature_vec = handle->vec;
   }
   else
   {
      handle->vec = vec;
   }
   OrdSetStore(&(handle->u1.clauses), clause);
   PERF_CTR_EXIT(FVIndexTimer);
}


/*-----------------------------------------------------------------------
//
// Function: FVIndexDelete()
//
//   Delete a clause from a FVIndex. At the moment, just removes the
//   clause from the final cell (I expect removals to be rare enough
//   that it pays to leave the structure intact). The clause counts
//   along the path are only decreased if the clause was found. In
//   any case, the clause loses access to the feature vector of its
//   leaf. Returns true if the clause was in the index, false
//   otherwise.
//
// Global Variables: -
//
// Side Effects    : Changes index and clause.
//
/----------------------------------------------------------------------*/

bool FVIndexDelete(FVIAnchor_p index, Clause_p clause)
{
   FreqVector_p vec;
   FVIndex_p handle;
   long i;
   bool res;

   assert(ClauseFeaturesAreValid(clause, index));

   if(index->structure == FVISetTree)
   {
      assert(clause->feature_set);
      PERF_CTR_ENTRY(FVIndexTimer);
      res = FSIndexDelete(&(index->fsindex), &(index->storage),
                          index->split, clause);
      clause->feature_set = NULL;
      PERF_CTR_EXIT(FVIndexTimer);
      return res;
   }

   assert(clause->feature_vec);
   vec = clause->feature_vec;
   PERF_CTR_ENTRY(FVIndexTimer);

   if(index->structure == FVITree)
   {
      res = FTIndexDelete(&(index->ftindex), &(index->storage),
                          index->split, clause);
      clause->feature_vec = NULL;
      PERF_CTR_EXIT(FVIndexTimer);
      return res;
   }

   handle = index->index;
   for(i=0; handle && (i<vec->size); i++)
   {
      assert(!handle->final);
      handle = IntMapGetVal(handle->u1.successors, vec->array[i]);
   }
   res = handle && OrdSetDeleteEntry(&(handle->u1.clauses), clause);
   if(res)
   {
      assert(handle->vec == vec);
      handle = index->index;
      handle->clause_count--;
      for(i=0; i<vec->size; i++)
      {
         handle = IntMapGetVal(handle->u1.successors, vec->array[i]);
         handle->clause_count--;
         assert(handle->clause_count >= 0);
      }
   }
   clause->feature_vec = NULL;
   PERF_CTR_EXIT(FVIndexTimer);
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: FVIndexCountNodes()
//
//   Count the number of nodes. If empty is true, count empty leaves
//   only. If leaves it true, count leaves only.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long FVIndexCountNodes(FVIndex_p index, bool leaves, bool empty)
{
   long res = 0, i;
   IntMapIter_p iter;
   FVIndex_p succ;

   if(index)
   {
      if(index->final)
      {
    if(!empty || !index->u1.clauses)
    {
       res++;
    }
    assert(EQUIV(index->clause_count,index->u1.clauses));
      }
      else
      {
    if(!(empty||leaves))
    {
       res++;
    }
         if(index->u1.successors)
         {
            iter = IntMapIterAlloc(index->u1.successors, 0, LONG_MAX);
            {
               while((succ = IntMapIterNext(iter, &i)))
               {
                  res += FVIndexCountNodes(succ, leaves, empty);
               }
            }
            IntMapIterFree(iter);
    }
      }
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: FVIndexComputeVector()
//
//   Compute and return the feature vector of clause for the index.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

FreqVector_p FVIndexComputeVector(FVIAnchor_p anchor, Clause_p clause)
{
   return OptimizedVarFreqVectorCompute(clause, anchor->perm_vector,
                                        anchor->cspec);
}


/*-----------------------------------------------------------------------
//
// Function: ClauseComputeFeatures()
//
//   Give the (not indexed) clause its own features for indices with
//   anchor's structure and features (a feature set for feature set
//   trees, a feature vector otherwise), replacing any old ones. If
//   anchor is NULL (no indexing), the clause gets no features.
//
// Global Variables: -
//
// Side Effects    : Memory operations, changes clause.
//
/----------------------------------------------------------------------*/

void ClauseComputeFeatures(Clause_p clause, FVIAnchor_p anchor)
{
   ClauseDropFeatures(clause);
   if(anchor)
   {
      if(anchor->structure == FVISetTree)
      {
         clause->feature_set =
            ClauseFeatureSetCompute(clause, anchor->set_features);
      }
      else
      {
         clause->feature_vec = FVIndexComputeVector(anchor, clause);
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: ClauseDropFeatures()
//
//   Free the feature vector or set of a (not indexed) clause, if any.
//
// Global Variables: -
//
// Side Effects    : Memory operations, changes clause.
//
/----------------------------------------------------------------------*/

void ClauseDropFeatures(Clause_p clause)
{
   assert(!ClauseQueryProp(clause, CPIsSIndexed));

   if(clause->feature_vec)
   {
      FreqVectorFree(clause->feature_vec);
      clause->feature_vec = NULL;
   }
   if(clause->feature_set)
   {
      FeatureSetFree(clause->feature_set);
      clause->feature_set = NULL;
   }
}


/*-----------------------------------------------------------------------
//
// Function: ClauseGetFeatureVector()
//
//   Return the feature vector of clause for indices with anchor's
//   features: the clause's own one if it has one, otherwise a newly
//   computed one, in which case *is_temp is set and the caller has to
//   free it.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

FreqVector_p ClauseGetFeatureVector(Clause_p clause, FVIAnchor_p anchor,
                                    bool *is_temp)
{
   assert(ClauseFeaturesAreValid(clause, anchor));

   *is_temp = !clause->feature_vec;
   if(*is_temp)
   {
      return FVIndexComputeVector(anchor, clause);
   }
   return clause->feature_vec;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseGetFeatureSet()
//
//   Return the feature set of clause for indices with anchor's
//   features: the clause's own one if it has one, otherwise a newly
//   computed one, in which case *is_temp is set and the caller has to
//   free it.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

FeatureSet_p ClauseGetFeatureSet(Clause_p clause, FVIAnchor_p anchor,
                                 bool *is_temp)
{
   assert(ClauseFeaturesAreValid(clause, anchor));

   *is_temp = !clause->feature_set;
   if(*is_temp)
   {
      return ClauseFeatureSetCompute(clause, anchor->set_features);
   }
   return clause->feature_set;
}


#ifndef NDEBUG
/*-----------------------------------------------------------------------
//
// Function: ClauseFeaturesAreValid()
//
//   Return true if the feature vector and set of clause (where
//   present) are the ones computed for anchor (i.e. they are not
//   stale and were computed with the same features), and the clause
//   has the kind of features anchor uses (or none). Debugging only.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

bool ClauseFeaturesAreValid(Clause_p clause, FVIAnchor_p anchor)
{
   bool res = true;

   if(clause->feature_vec)
   {
      FreqVector_p fresh = FVIndexComputeVector(anchor, clause);
      res = (anchor->structure != FVISetTree) &&
         FreqVectorEqual(fresh, clause->feature_vec);
      FreqVectorFree(fresh);
   }
   if(res && clause->feature_set)
   {
      FeatureSet_p fresh =
         ClauseFeatureSetCompute(clause, anchor->set_features);
      res = (anchor->structure == FVISetTree) &&
         FeatureSetEqual(fresh, clause->feature_set);
      FeatureSetFree(fresh);
   }
   return res;
}
#endif


/*-----------------------------------------------------------------------
//
// Function: FVIndexPrint()
//
//   Pretty prints FVIndex.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void FVIndexPrint(FILE* out, FVIndex_p index, bool fullterms)
{
   fprintf(stderr, "* ROOT *\n");
   fv_index_print(out, index, fullterms, 0);
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
