/*-----------------------------------------------------------------------

File  : ccl_freqvectors.h

Author: Stephan Schulz

Contents

  Functions for handling frequency count vectors.

  2003 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Tue Jul  8 21:48:35 CEST 2003  
    New (separated FreqVector from fcvindexing.*)

-----------------------------------------------------------------------*/

#ifndef CCL_FREQVECTORS

#define CCL_FREQVECTORS

#include <clb_pdarrays.h>
#include <ccl_clauses.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct freq_vector_cell
{
   long size;        /* How many fields? */
   long *freq_vector;
   Clause_p clause; /* Just an unprotected reference */
}FreqVectorCell, *FreqVector_p, *FVPackedClause_p;

#define FV_MAX_SYMBOL_COUNT 50
#define NON_SIG_FEATURES 2



/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


#define FreqVectorCellAlloc()    (FreqVectorCell*)SizeMalloc(sizeof(FreqVectorCell))
#define FreqVectorCellFree(junk) SizeFree(junk, sizeof(FreqVectorCell))

#define SigSizeToFreqVectorSize(size) (size*2-2+NON_SIG_FEATURES)
FreqVector_p FreqVectorAlloc(long size);
void         FreqVectorFree(FreqVector_p junk);

void         FreqVectorPrint(FILE* out, FreqVector_p vec);

void             StandardFreqVectorAddVals(FreqVector_p vec, long sig_symbols, 
					   Clause_p clause);
FreqVector_p     StandardFreqVectorCompute(Clause_p clause, long sig_symbols);
FVPackedClause_p FVPackClause(Clause_p clause, long symbol_limit);
Clause_p         FVUnpackClause(FVPackedClause_p pack);
void             FVPackedClauseFree(FVPackedClause_p pack);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





