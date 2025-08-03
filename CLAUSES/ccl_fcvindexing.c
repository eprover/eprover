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

void print_clauses(FILE* out, PTree_p clauses, int level, bool fullterms)
{
   PStack_p stack = PTreeTraverseInit(clauses);
   PTree_p  node  = NULL;
   Clause_p cl_handle   = NULL;

   while((node = PTreeTraverseNext(stack)))
   {
      cl_handle = node->key;
      print_lvl(out, level);
      ClausePrint(out, cl_handle, fullterms);
      fprintf(stderr, " \n");
   }

   PTreeTraverseExit(stack);
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
         PTreeFree(junk->u1.clauses);
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
//   Insert a FreqVector (with associated clause) into the index.
//
// Global Variables: -
//
// Side Effects    : Changes the index.
//
/----------------------------------------------------------------------*/

void FVIndexInsert(FVIAnchor_p index, FreqVector_p vec_clause)
{
   FVIndex_p handle, newnode = NULL;
   long i;

   PERF_CTR_ENTRY(FVIndexTimer);

   assert(vec_clause);
   assert(vec_clause->clause);

   ClauseSubsumeOrderSortLits(vec_clause->clause);

   handle = index->index;
   handle->clause_count++;

   for(i=0; i<vec_clause->size; i++)
   {
      assert(!handle->final);

      newnode = IntMapGetVal(handle->u1.successors, vec_clause->array[i]);
      if(!newnode)
      {
         newnode = insert_empty_node(handle,
                                     index,
                                     vec_clause->array[i]);
      }
      handle = newnode;
      handle->clause_count++;
   }
   handle->final = true;
   PTreeStore(&(handle->u1.clauses), vec_clause->clause);
   /* ClauseSetProp(vec_clause->clause, CPIsSIndexed); */
   PERF_CTR_EXIT(FVIndexTimer);
}


/*-----------------------------------------------------------------------
//
// Function: FVIndexDelete()
//
//   Delete a clause from a FVIndex. At the moment, just removes the
//   clause from the final cell (I expect removals to be rare enough
//   that it pays to leave the structure intact. Returns true if the
//   clause was in the index, false otherwise.
//
// Global Variables: -
//
// Side Effects    : Changes index.
//
/----------------------------------------------------------------------*/

bool FVIndexDelete(FVIAnchor_p index, Clause_p clause)
{
   FreqVector_p vec;
   FVIndex_p handle;
   long i;
   bool res;

   vec = OptimizedVarFreqVectorCompute(clause, index->perm_vector,
                   index->cspec);
   /* FreqVector-Computation is measured independently */
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
   FreqVectorFree(vec);
   /* ClauseDelProp(clause, CPIsSIndexed); */
   res = handle?PTreeDeleteEntry(&(handle->u1.clauses), clause):false;
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
// Function: FVIndexPackClause()
//
//   Pack a clause into an apropriate FVPackedClauseStructure for the
//   index.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

FVPackedClause_p FVIndexPackClause(Clause_p clause, FVIAnchor_p anchor)
{
   if(!anchor)
   {
      return FVPackClause(clause, NULL, NULL);
   }
   return FVPackClause(clause, anchor->perm_vector,
             anchor->cspec);
}

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
