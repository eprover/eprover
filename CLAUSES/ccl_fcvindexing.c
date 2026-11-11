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
//   Allocate an (empty) FV index.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

FVIAnchor_p FVIAnchorAlloc(FVCollect_p cspec, PermVector_p perm)
{
   FVIAnchor_p handle = FVIAnchorCellAlloc();

   handle->perm_vector  = perm;
   handle->cspec        = cspec;
   handle->index        = FVIndexAlloc();
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

   FVIndexFree(junk->index);
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
   assert(ClauseFeatureVectorIsValid(clause, index));

   if(!clause->feature_vec)
   {
      clause->feature_vec = FVIndexComputeVector(index, clause);
   }
   vec = clause->feature_vec;

   PERF_CTR_ENTRY(FVIndexTimer);

   ClauseSubsumeOrderSortLits(clause);

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
//   that it pays to leave the structure intact). The clause loses
//   access to the feature vector of its leaf. Returns true if the
//   clause was in the index, false otherwise.
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

   assert(clause->feature_vec);
   assert(ClauseFeatureVectorIsValid(clause, index));

   vec = clause->feature_vec;
   PERF_CTR_ENTRY(FVIndexTimer);
   handle = index->index;
   handle->clause_count--;

   for(i=0; i<vec->size; i++)
   {
      assert(!handle->final);
      handle = IntMapGetVal(handle->u1.successors, vec->array[i]);
      if(!handle)
      {
         break;
      }
      handle->clause_count--;
   }
   assert(!handle || handle->vec == clause->feature_vec);
   clause->feature_vec = NULL;
   res = handle?OrdSetDeleteEntry(&(handle->u1.clauses), clause):false;
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
// Function: ClauseComputeFeatureVector()
//
//   Give the (not indexed) clause its own feature vector for indices
//   with anchor's features, replacing any old one. If anchor is NULL
//   (no indexing), the clause gets no vector.
//
// Global Variables: -
//
// Side Effects    : Memory operations, changes clause.
//
/----------------------------------------------------------------------*/

void ClauseComputeFeatureVector(Clause_p clause, FVIAnchor_p anchor)
{
   ClauseDropFeatureVector(clause);
   if(anchor)
   {
      clause->feature_vec = FVIndexComputeVector(anchor, clause);
   }
}


/*-----------------------------------------------------------------------
//
// Function: ClauseDropFeatureVector()
//
//   Free the feature vector of a (not indexed) clause, if any.
//
// Global Variables: -
//
// Side Effects    : Memory operations, changes clause.
//
/----------------------------------------------------------------------*/

void ClauseDropFeatureVector(Clause_p clause)
{
   assert(!ClauseQueryProp(clause, CPIsSIndexed));

   if(clause->feature_vec)
   {
      FreqVectorFree(clause->feature_vec);
      clause->feature_vec = NULL;
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
   assert(ClauseFeatureVectorIsValid(clause, anchor));

   *is_temp = !clause->feature_vec;
   if(*is_temp)
   {
      return FVIndexComputeVector(anchor, clause);
   }
   return clause->feature_vec;
}


#ifndef NDEBUG
/*-----------------------------------------------------------------------
//
// Function: ClauseFeatureVectorIsValid()
//
//   Return true if clause has no feature vector or its vector is the
//   one computed for anchor (i.e. it is not stale and was computed
//   with the same features). Debugging only.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

bool ClauseFeatureVectorIsValid(Clause_p clause, FVIAnchor_p anchor)
{
   FreqVector_p fresh;
   bool res;

   if(!clause->feature_vec)
   {
      return true;
   }
   fresh = FVIndexComputeVector(anchor, clause);
   res = FreqVectorEqual(fresh, clause->feature_vec);
   FreqVectorFree(fresh);
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
