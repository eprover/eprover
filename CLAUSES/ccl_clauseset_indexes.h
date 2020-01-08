/*-----------------------------------------------------------------------

File  : ccl_clauseset_indexes.h

Author: Constantin Ruhdorfer

Contents

  Interface for indexing watchlist clauses.

Copyright 2019-2020 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

-----------------------------------------------------------------------*/

#ifndef CCL_CLAUSESET_INDEXES

#define CCL_CLAUSESET_INDEXES

#include <ccl_fcvindexing.h>
#include <ccl_pdtrees.h>
#include <ccl_unitclause_index.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct clauseset_indexes
{
   PDTree_p  demod_index; /* If used for demodulators */
   FVIAnchor_p fvindex; /* Used for non-unit subsumption */
   UnitclauseIndex_p unitclasue_index; /* Used for unit clauses subsuption */
} ClausesetIndexes, *ClausesetIndexes_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

ClausesetIndexes_p ClausesetIndexesAlloc();
void ClausesetIndexInsertNewClause(ClausesetIndexes_p clauseset_indexes, FVPackedClause_p newclause);
Clause_p ClausesetIndexExtractEntry(ClausesetIndexes_p clauseset_indexes, Clause_p junk);
void ClausesetIndexesFree(ClausesetIndexes_p clauseset_indexes);
void ClausesetIndexesUCIndexededInsert(ClausesetIndexes_p clauseset_indexes, Clause_p newclause);
void ClausesetIndexesPDTIndexedInsert(ClausesetIndexes_p clauseset_indexes, Clause_p newclause);


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

ClausesetIndexes_p ClausesetIndexesAllocRaw();
void ClausesetIndexesFreeRaw(ClausesetIndexes_p clauseset_indexes);
void ClausesetIndexesPDTDelteClause(ClausesetIndexes_p clauseset_indexes, 
                                    Clause_p junk);

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/

#endif