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
// Function: FreqVectorAlloc()
// 
//   Allocate a frequency vector that can hold up to sig_start
//   non function symbol count features and sig_count function symbol
//   counts (in both positive and negative variety).
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

FreqVector_p FreqVectorAlloc(long size)
{
   FreqVector_p handle = FreqVectorCellAlloc();
   long i;

   handle->size         = size;
   handle->freq_vector  = SizeMalloc(sizeof(long)*handle->size);
   for(i=0; i<handle->size;i++)
   {
      handle->freq_vector[i] = 0;
   }
   handle->clause = NULL;
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: FreqVectorFree()
//
//   Free a frequency vector.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void FreqVectorFree(FreqVector_p junk)
{
   assert(junk);

   if(junk->freq_vector)
   {
      SizeFree(junk->freq_vector, sizeof(long)*junk->size);
   }
   FreqVectorCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: FreqVectorPrint()
//
//   Print a frequency vector.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void FreqVectorPrint(FILE* out, FreqVector_p vec)
{
   long i;

   assert(vec);
   if(vec->clause)
   {
      fprintf(out, "# FV for: ");
      ClausePrint(out, vec->clause, true);
      fprintf(out, "\n");
   }
   else
   {
      fprintf(out, "# FV, no clause given.\n");
   }
   fprintf(out, "# FV:");
   for(i=0; i<vec->size; i++)

   {
      fprintf(out, " %ld", vec->freq_vector[i]);
   }  
   fprintf(out, "\n");
}


/*-----------------------------------------------------------------------
//
// Function: StandardFreqVectorAddVals()
// 
//   Add the numerical features of the clause to the corresponding
//   positions in the frequency vector.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void StandardFreqVectorAddVals(FreqVector_p vec, long sig_symbols, 
			       Clause_p clause)
{   
   long *pstart, *nstart;
   Eqn_p handle;

   assert(sig_symbols<=FV_MAX_SYMBOL_COUNT);
   
   vec->freq_vector[0] += clause->pos_lit_no;
   vec->freq_vector[1] += clause->neg_lit_no;
   /* vec->freq_vector[2] += 0; */
   nstart = &(vec->freq_vector[NON_SIG_FEATURES-1]);
   pstart = &(vec->freq_vector[sig_symbols+NON_SIG_FEATURES-2]);
   for(handle = clause->literals; handle; handle = handle->next)
   {
      if(EqnIsPositive(handle))
      {

	 EqnAddSymbolDistributionLimited(handle, 
					 pstart, 
					 sig_symbols);
      }
      else
      {
	 EqnAddSymbolDistributionLimited(handle, 
					 nstart, 
					 sig_symbols);	 
      }
   }
   /* SWAP(long,vec->freq_vector[2],vec->freq_vector[SigSizeToFreqVectorSize(sig_symbols)-1]); */
}


/*-----------------------------------------------------------------------
//
// Function: StandardFreqVectorCompute()
//
//   Compute a frequency count vector for clause.
//
// Global Variables: 
//
// Side Effects    : 
//
/----------------------------------------------------------------------*/

FreqVector_p StandardFreqVectorCompute(Clause_p clause, long sig_symbols)
{
   FreqVector_p vec;

   assert(sig_symbols<=FV_MAX_SYMBOL_COUNT);
   assert(clause);
   vec = FreqVectorAlloc(SigSizeToFreqVectorSize(sig_symbols));
   vec->clause = clause;
   StandardFreqVectorAddVals(vec, sig_symbols, clause);
   /* FreqVectorPrint(GlobalOut, vec); */
   return vec;
}

/*-----------------------------------------------------------------------
//
// Function: FVPackClause()
//
//   If index is an index, compute and return a StandardFreqVector for
//   clause, otherwise pack clause into a dummy frequency vector cell
//   and return than.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

FVPackedClause_p FVPackClause(Clause_p clause, FVIAnchor_p index)
{
   FVPackedClause_p res;

   if(index)
   {
      return StandardFreqVectorCompute(clause, index->symbol_limit);
   }
   res = FreqVectorCellAlloc();
   res->freq_vector = NULL;
   res->clause = clause;

   return res; 
}


/*-----------------------------------------------------------------------
//
// Function: FVUnpackClause()
//
//   Unpack a packed clause, i.e. return the clause and throw away the
//   container.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Clause_p FVUnpackClause(FVPackedClause_p pack)
{
   Clause_p res = pack->clause;

   FreqVectorFree(pack);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: FVPackedClauseFree()
//
//   Fully free a packed clause.
//
// Global Variables: 
//
// Side Effects    : 
//
/----------------------------------------------------------------------*/

void FVPackedClauseFree(FVPackedClause_p pack)
{
   if(pack->clause)
   {
      ClauseFree(pack->clause);
   }
   FreqVectorFree(pack);
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

FVIAnchor_p FVIAnchorAlloc(long symbol_limit)
{
   FVIAnchor_p handle = FVIAnchorCellAlloc();
   
   handle->symbol_limit = symbol_limit;
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
   fprintf(GlobalOut, 
	   "# Freeing FVIndex. %ld leaves, %ld empty. Total nodes: %ld. Mem: %ld\n",
	   FVIndexCountNodes(junk->index, true, false),
	   FVIndexCountNodes(junk->index, true, true),
	   junk->node_count,
	   FVIndexStorage(junk));

   FVIndexFree(junk->index);
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
      newnode = fv_index_get_next_node(handle, vec_clause->freq_vector[i]);
      if(!newnode)
      {
	 newnode = insert_empty_node(handle, 
				     index,
				     vec_clause->freq_vector[i]);
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

   vec = StandardFreqVectorCompute(clause, index->symbol_limit);
   
   handle = index->index;
   handle->clause_count--;

   for(i=0; i<vec->size; i++)
   {
      assert(!FVIndexFinalNode(handle));
      handle = fv_index_get_next_node(handle, vec->freq_vector[i]);
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



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


