/*-----------------------------------------------------------------------

File  : ccl_fcvindexing.c

Author: Stephan Schulz

Contents
 
  Algorithms for frequency count vector indexing.

  Copyright 1998-2003 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Tue Jul  1 13:09:10 CEST 2003
    New

-----------------------------------------------------------------------*/

#include "ccl_fcvindexing.h"



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
// Function: insert_empty_node()
//
//   Insert an empty node into FVIndex  at node node and key key.
//
// Global Variables: -
//
// Side Effects    : Memory operations, modifies index.
//
/----------------------------------------------------------------------*/

static FVIndex_p insert_empty_node(FVIndex_p node, FVIAnchor_p anchor, long key)
{
   FVIndex_p handle = FVIndexAlloc(), tmp;
   long i, new_limit;

   assert(node);
   assert(!FVIndexFinalNode(node));
   assert(key >= 0);

   if(FVIndexEmptyNode(node))
   {
      assert(key!=node->type_or_key);      
      node->u1.succ = handle;
      node->type_or_key = key;
      assert(node->array_size == 0);
   }
   else if(FVIndexUnaryNode(node))
   {
      tmp = node->u1.succ;
      new_limit = MAX(key, node->type_or_key)+1;
      node->u1.successors = SecureMalloc(sizeof(long)*new_limit);
      anchor->array_count += new_limit;
      for(i=0; i<new_limit; i++)
      {
	 node->u1.successors[i] = NULL;
      }
      node->u1.successors[key] = handle;
      node->u1.successors[node->type_or_key] = tmp;
      node->type_or_key = FVINDEXTYPE_MANY;
      node->array_size = new_limit;
   }
   else
   {
      assert(FVIndexManySuccNode(node));
      if(key >= node->array_size)
      {
	 new_limit = MAX(key, node->array_size)+1;
	 node->u1.successors = SecureRealloc(node->u1.successors,
					     sizeof(long)*new_limit);
	 anchor->array_count += (new_limit-node->array_size);

	 for(i=node->array_size; i<new_limit; i++)
	 {
	    node->u1.successors[i] = NULL;
	 }
	 node->array_size = new_limit;
      }
      node->u1.successors[key] = handle;      
   }
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: fv_index_get_next_node()
//
//   Given a key, return a pointer to the corresponding cell, or NULL
//   if no such cell exists (up to now).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static
FVIndex_p fv_index_get_next_node(FVIndex_p node, long key)
{
   assert(!FVIndexFinalNode(node));
   
   if(FVIndexEmptyNode(node))
   {
      return NULL;
   }
   if(FVIndexUnaryNode(node))
   {
      if(node->type_or_key == key)
      {
	 return node->u1.succ;
      }
      return NULL;
   }
   assert(FVIndexManySuccNode(node));
   if(key < node->array_size)
   {
      return node->u1.successors[key];
   }
   return NULL;
}



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


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
   
   handle->features                = FVIACFeatures;
   handle->use_perm_vectors        = true;
   handle->eleminate_uninformative = false;
   handle->max_symbols             = FVINDEX_MAX_FEATURES_DEFAULT;
   handle->symbol_slack            = FVINDEX_SYMBOL_SLACK_DEFAULT;
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

   handle->type_or_key   = FVINDEXTYPE_EMPTY;
   handle->clause_count  = 0;
   handle->array_size    = 0;
   handle->u1.successors = NULL;
   
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
   if(junk)
   {
      switch(junk->type_or_key)
      {
      case FVINDEXTYPE_FINAL:
	    PTreeFree(junk->u1.clauses);
	    break;
      case FVINDEXTYPE_MANY:
	    if(junk->array_size > 0)
	    {
	       long i;
	       for(i=0; i<junk->array_size; i++)
	       {	       
		  FVIndexFree(junk->u1.successors[i]);
	       }
	    }
	    FREE(junk->u1.successors);
	    break;
      case FVINDEXTYPE_EMPTY:
	    break;
      default:
	    FVIndexFree(junk->u1.succ);
	    break;
      }
      FVIndexCellFree(junk);
   }
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

FVIAnchor_p FVIAnchorAlloc(long symbol_limit, FVIndexType features, PermVector_p perm)
{
   FVIAnchor_p handle = FVIAnchorCellAlloc();
   
   handle->symbol_limit = symbol_limit;
   handle->perm_vector  = perm;
   handle->features     = features;   
   handle->node_count   = 0;
   handle->array_count  = 0;
   handle->index        = FVIndexAlloc();

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

   fprintf(GlobalOut, 
	   "# Freeing FVIndex. %ld leaves, %ld empty. Total nodes: %ld. Mem: %ld\n",
	   FVIndexCountNodes(junk->index, true, false),
	   FVIndexCountNodes(junk->index, true, true),
	   junk->node_count,
	   FVIndexStorage(junk));

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
   
   assert(!FVIndexFinalNode(node));
   
   handle = fv_index_get_next_node(node, key);
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
   FVIndex_p handle, newnode;
   long i;

   handle = index->index;
   handle->clause_count++;

   for(i=0; i<vec_clause->size; i++)
   {
      assert(!FVIndexFinalNode(handle));
      newnode = fv_index_get_next_node(handle, vec_clause->array[i]);
      if(!newnode)
      {
	 newnode = insert_empty_node(handle, 
				     index,
				     vec_clause->array[i]);
	 index->node_count++;
      }      
      handle = newnode;
      handle->clause_count++;
   }
   handle->type_or_key = FVINDEXTYPE_FINAL;
   PTreeStore(&(handle->u1.clauses), vec_clause->clause);
   ClauseSetProp(vec_clause->clause, CPIsSIndexed);
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

   vec = OptimizedVarFreqVectorCompute(clause, index->perm_vector, 
				       index->features,
				       index->symbol_limit);   
   handle = index->index;
   handle->clause_count--;

   for(i=0; i<vec->size; i++)
   {
      assert(!FVIndexFinalNode(handle));
      handle = fv_index_get_next_node(handle, vec->array[i]);
      if(!handle)
      {
	 break;
      }
      handle->clause_count--;
   }
   FreqVectorFree(vec);
   ClauseDelProp(clause, CPIsSIndexed);
   return handle?PTreeDeleteEntry(&(handle->u1.clauses), clause):false;
}

/*-----------------------------------------------------------------------
//
// Function: FVIndexCountNodes()
//
//   Count the number of nodes. If empty is true, count empty leaves
//   only. If leaves it true, count leaves only.
//
// Global Variables: 
//
// Side Effects    : 
//
/----------------------------------------------------------------------*/

long FVIndexCountNodes(FVIndex_p index, bool leafs, bool empty)
{
   long res = 0;
   
   if(index)
   {
      if(FVIndexFinalNode(index))
      {
	 if(!empty || !index->u1.clauses)
	 {
	    res++;
	 }
	 assert(EQUIV(index->clause_count,index->u1.clauses));
      }
      else 
      {
	 if(!(empty||leafs))
	 {
	    res++;
	 }
	 if(FVIndexUnaryNode(index))
	 {
	    res+=FVIndexCountNodes(index->u1.succ,leafs,empty);
	 }
	 else
	 {
	    long i;
	    for(i=0; i<index->array_size; i++)
	    {	       
	       res += FVIndexCountNodes(index->u1.successors[i], leafs, empty);
	    }
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
      return FVPackClause(clause, NULL,FVINoFeatures,0);
   }
   return FVPackClause(clause, anchor->perm_vector, 
		       anchor->features,
		       anchor->symbol_limit);
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


