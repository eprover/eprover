/*-----------------------------------------------------------------------

File  : ccl_freqvectors.c

Author: Stephan Schulz

Contents
 
  Algorithms for frequency count vectors.

  Copyright 1998-2003 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Tue Jul  8 21:50:44 CEST 2003
    New (separated functionality from ccl_fcvindexing.c)

-----------------------------------------------------------------------*/

#include "ccl_freqvectors.h"



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
// Function: tuple_3_compare_23lex()
//
//   Compare 2 tuple-2 cells lexicographically, with diff more
//   significant than value, which is more significant than pos.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int tuple3_compare_23lex(Tuple3Cell *t1, Tuple3Cell *t2)
{
   if(t1->diff < t2->diff)
   {
      return -1;
   }
   if(t1->diff > t2->diff)
   {
      return 1;
   }
   if(t1->value < t2->value)
   {
      return -1;
   }
   if(t1->value > t2->value)
   {
      return 1;
   }
   if(t1->pos > t2->pos)
   {
      return -1;
   }
   if(t1->pos < t2->pos)
   {
      return 1;
   }   
   return 0;
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: PermVectorComputeInternal()
//
//   Find a "good" permutation (and selection) vector for
//   FVIndexing by:
//   - Ordering features from lesser to higher informativity
//   - Selecting the best max_len features
//   - Optionally drop features that have no projected informational
//     value. 
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

PermVector_p PermVectorComputeInternal(FreqVector_p fmax, FreqVector_p fmin,
				       FreqVector_p fsum,
				       long max_len, 
				       bool eleminate_uninformative)
{
   Tuple3Cell *array;
   long i, size, start=0, start1=0, diff;
   PermVector_p handle;

   assert(fsum->size == fmax->size);
   assert(fsum->size == fmin->size);

   array = SizeMalloc(fsum->size * sizeof(Tuple3Cell));
   for(i=0; i<fsum->size; i++)
   {
      array[i].pos = i;
      diff = fmax->array[i]-fmin->array[i];
      array[i].diff  = diff;
      array[i].value = fsum->array[i];
   }
   qsort(array, fsum->size, sizeof(Tuple3Cell), 
	 (ComparisonFunctionType)tuple3_compare_23lex);
   
   if(fsum->size >  max_len)
   {
      start = fsum->size - max_len;
   }
   if(eleminate_uninformative)
   {
      for(i=0; i<fsum->size && !array[i].diff; i++)
      {
	 /* Intentionally empty */
      };
      start1 = i;
   }
   start = MAX(start, start1);
   if(start == fsum->size)
   {
      start--;
   }
   size = fsum->size - start;

   handle = PermVectorAlloc(size);

   for(i=0; i < size; i++)
   {
      handle->array[i] = array[i+start].pos;
   }  
   SizeFree(array, fsum->size * sizeof(Tuple3Cell));
   /* PermVectorPrint(GlobalOut, handle); */
   return handle;
}


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

   handle->size         = size;
   handle->array  = SizeMalloc(sizeof(long)*handle->size);
   FreqVectorInitialize(handle, 0);
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

void FreqVectorFreeReal(FreqVector_p junk)
{
   assert(junk);

   if(junk->array)
   {
      SizeFree(junk->array, sizeof(long)*junk->size);
#ifndef NDEBUG
      junk->array = NULL;
      junk->clause = NULL;
#endif
   }
   FreqVectorCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: FreqVectorInitialize()
//
//   Store value in all fields of vec.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void FreqVectorInitialize(FreqVector_p vec, long value)
{
   long i;

   for(i=0; i<vec->size;i++)
   {
      vec->array[i] = value;
   }
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
      fprintf(out, " %ld", vec->array[i]);
   }  
   fprintf(out, "\n");
}


/*-----------------------------------------------------------------------
//
// Function: VarFreqVectorAddVals()
//
//   Add values for up to symbol type features to the freq vector.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void VarFreqVectorAddVals(FreqVector_p vec, long symbols, FVIndexType features, 
			  Clause_p clause) 
{
   long *unused, *pfreqstart, *nfreqstart, *pdepthstart, *ndepthstart;
   long unused_size = 0;
   Eqn_p handle;

   assert(clause);
   assert((features == FVIACFeatures) || 
	  (features == FVISSFeatures) || 
	  (features == FVIAllFeatures));
   assert(vec);
   assert(vec->size == FVSize(symbols, features));

   switch(features)
   {
   case FVIACFeatures:
	 vec->array[0] += clause->pos_lit_no;
	 vec->array[1] += clause->neg_lit_no;   
	 unused_size = sizeof(long)*(symbols+1);
	 unused = SizeMalloc(unused_size);
	 pdepthstart = ndepthstart = unused;
	 nfreqstart = &(vec->array[FV_CLAUSE_FEATURES]);
	 pfreqstart = &(vec->array[FV_CLAUSE_FEATURES+1*(symbols+1)]);
	 break;
   case FVISSFeatures:
	 unused_size = sizeof(long)*(symbols+1);
	 unused = SizeMalloc(unused_size);
	 pfreqstart = nfreqstart = unused;
	 ndepthstart = &(vec->array[0]);
	 pdepthstart = &(vec->array[0+1*(symbols+1)]);
	 break;
   case FVIAllFeatures:	 
	 vec->array[0] += clause->pos_lit_no;
	 vec->array[1] += clause->neg_lit_no;
	 unused = NULL;
	 nfreqstart  = &(vec->array[FV_CLAUSE_FEATURES]);
	 pfreqstart  = &(vec->array[FV_CLAUSE_FEATURES+1*(symbols+1)]);
	 pdepthstart = &(vec->array[FV_CLAUSE_FEATURES+2*(symbols+1)]);
	 ndepthstart = &(vec->array[FV_CLAUSE_FEATURES+3*(symbols+1)]);	 
	 break;
   default:
	 assert(features == FVINoFeatures);
	 assert(false);
	 return; /* Cheapest way to fix compiler warning */
	 break;
   }
   for(handle = clause->literals; handle; handle = handle->next)
   {
      if(EqnIsPositive(handle))
      {
	 EqnAddSymbolFeaturesLimited(handle, 
				     pfreqstart,
				     pdepthstart,				    
				     symbols);
      }
      else
      {
	 EqnAddSymbolFeaturesLimited(handle, 
				     nfreqstart,
				     ndepthstart,				    
				     symbols);	 
      }
   }   
   if(unused)
   {
      SizeFree(unused, unused_size);
   }
}


/*-----------------------------------------------------------------------
//
// Function: VarFreqVectorCompute()
//
//   Allocate and return a frequency vector for clause based on the
//   other supplied parameters.
//
// Global Variables: -
//
// Side Effects    : Memory allocation.
//
/----------------------------------------------------------------------*/

FreqVector_p VarFreqVectorCompute(Clause_p clause, long symbols, FVIndexType features)
{
   long size;
   FreqVector_p vec;

   assert(clause);
   assert((features == FVIACFeatures) || 
	  (features == FVISSFeatures) || 
	  (features == FVIAllFeatures));

   size = FVSize(symbols, features);

   vec = FreqVectorAlloc(size);
   vec->clause = clause;
   FreqVectorInitialize(vec, 0);
   VarFreqVectorAddVals(vec, symbols, features, clause);
   return vec;
}


/*-----------------------------------------------------------------------
//
// Function: OptimizedVarFreqVectorCompute()
//
//   Compute an "optimized" frequency count vector, based on a given
//   permutation vector. If no permutation vector is given, return a
//   VarFreqVector. 
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

FreqVector_p OptimizedVarFreqVectorCompute(Clause_p clause, 
					   PermVector_p perm, 
					   FVIndexType features,
					   long sig_symbols)
{
   FreqVector_p vec, res;

   vec = VarFreqVectorCompute(clause, sig_symbols, features);
   if(perm)
   {
      long i;
      
      res = FreqVectorAlloc(perm->size);
      for(i=0; i<perm->size; i++)
      {
	 assert(perm->array[i]>=0);
	 assert(perm->array[i]<vec->size);
	 res->array[i] = vec->array[perm->array[i]];
      }
      res->clause = clause;
      FreqVectorFree(vec);
      return res;
   }
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

FVPackedClause_p FVPackClause(Clause_p clause, PermVector_p perm,
			      FVIndexType features, 
			      long symbol_limit)
{
   FVPackedClause_p res;

   if(symbol_limit)
   {
      return OptimizedVarFreqVectorCompute(clause, perm, features, symbol_limit);
   }
   res = FreqVectorCellAlloc();
   res->array = NULL;
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

#ifndef NDEBUG
   pack->clause = NULL;
#endif
   FreqVectorFree(pack);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: FVPackedClauseFreeReal()
//
//   Fully free a packed clause.
//
// Global Variables: 
//
// Side Effects    : 
//
/----------------------------------------------------------------------*/

void FVPackedClauseFreeReal(FVPackedClause_p pack)
{
   if(pack->clause)
   {
      ClauseFree(pack->clause);
   }
   FreqVectorFree(pack);
}


/*-----------------------------------------------------------------------
//
// Function: FreqVectorAdd()
//
//   Component-wise addition of both sources. Guaranteed to work if
//   dest is a source (but not maximally efficient - who cares). Yes,
//   it's worth mentioning it ;-)
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void FreqVectorAdd(FreqVector_p dest, FreqVector_p s1, FreqVector_p s2)
{
   long i;

   assert(s1 && s2 && dest);
   assert(s1->size == dest->size);
   assert(s2->size == dest->size);

   for(i=0; i<dest->size; i++)
   {
      dest->array[i] = s1->array[i]+s2->array[i];
   }
}


/*-----------------------------------------------------------------------
//
// Function: FreqVectorMulAdd()
//
//   Component-wise addition of both weighted sources. Guaranteed to
//   work if dest is a source (but not maximally efficient - who
//   cares). Yes, it's worth mentioning it ;-)
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void FreqVectorMulAdd(FreqVector_p dest, FreqVector_p s1, long f1, 
		      FreqVector_p s2, long f2)
{
   long i;

   assert(s1 && s2 && dest);
   assert(s1->size == dest->size);
   assert(s2->size == dest->size);

   for(i=0; i<dest->size; i++)
   {
      dest->array[i] = f1*s1->array[i]+f2*s2->array[i];
   }
}


/*-----------------------------------------------------------------------
//
// Function: FreqVectorMax()
//
//   Compute componentwise  max of vectors. See above.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void FreqVectorMax(FreqVector_p dest, FreqVector_p s1, FreqVector_p s2)
{
   long i;

   assert(s1 && s2 && dest);
   assert(s1->size == dest->size);
   assert(s2->size == dest->size);

   for(i=0; i<dest->size; i++)
   {
      dest->array[i] = MAX(s1->array[i],s2->array[i]);
   }
}


/*-----------------------------------------------------------------------
//
// Function: FreqVectorMin()
//
//   Compute componentwise  min of vectors. See above.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void FreqVectorMin(FreqVector_p dest, FreqVector_p s1, FreqVector_p s2)
{
   long i;

   assert(s1 && s2 && dest);
   assert(s1->size == dest->size);
   assert(s2->size == dest->size);

   for(i=0; i<dest->size; i++)
   {
      dest->array[i] = MIN(s1->array[i],s2->array[i]);
   }
}

#ifdef NEVER_DEFINED

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
   long *pfreqstart, *nfreqstart, *pdepthstart, *ndepthstart;
   Eqn_p handle;

   vec->array[0] += clause->pos_lit_no;
   vec->array[1] += clause->neg_lit_no;
   
   nfreqstart  = &(vec->array[FV_CLAUSE_FEATURES]);
   pfreqstart  = &(vec->array[FV_CLAUSE_FEATURES+1*(sig_symbols+1)]);
   pdepthstart = &(vec->array[FV_CLAUSE_FEATURES+2*(sig_symbols+1)]);
   ndepthstart = &(vec->array[FV_CLAUSE_FEATURES+3*(sig_symbols+1)]);
   for(handle = clause->literals; handle; handle = handle->next)
   {
      if(EqnIsPositive(handle))
      {
	 EqnAddSymbolFeaturesLimited(handle, 
				     pfreqstart,
				     pdepthstart,				    
				     sig_symbols);
      }
      else
      {
	 EqnAddSymbolFeaturesLimited(handle, 
				     nfreqstart,
				     ndepthstart,				    
				     sig_symbols);	 
      }
   }
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

   assert(clause);
   vec = FreqVectorAlloc(FVFullSize(sig_symbols));
   vec->clause = clause;
   StandardFreqVectorAddVals(vec, sig_symbols, clause);
   /* FreqVectorPrint(GlobalOut, vec); */
   return vec;
}


/*-----------------------------------------------------------------------
//
// Function: OptimizedFreqVectorCompute()
//
//   Compute an "optimized" frequency count vector, based on a given
//   permutation vector. If no permutation vector is given, return a
//   StandardFreqVector. 
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

FreqVector_p OptimizedFreqVectorCompute(Clause_p clause, 
					PermVector_p perm, 
					long sig_symbols)
{
   FreqVector_p vec, res;

   vec = StandardFreqVectorCompute(clause, sig_symbols);
   if(perm)
   {
      long i;
      
      res = FreqVectorAlloc(perm->size);
      for(i=0; i<perm->size; i++)
      {
	 res->array[i] = vec->array[perm->array[i]];
      }
      res->clause = clause;
      FreqVectorFree(vec);
      return res;
   }
   return vec;
}

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


