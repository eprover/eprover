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
   /* vec->freq_vector[2] += ClauseDepth(clause); */
   
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

FVPackedClause_p FVPackClause(Clause_p clause, long symbol_limit)
{
   FVPackedClause_p res;

   if(symbol_limit)
   {
      return StandardFreqVectorCompute(clause, symbol_limit);
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



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


