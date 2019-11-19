/*-----------------------------------------------------------------------

File  : ccl_clauseset_indexes.h

Author: Constantin Ruhdorfer

Contents

  Interface for indexing watchlist clauses.

Copyright 1998-2020 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

-----------------------------------------------------------------------*/

#ifndef CCL_CLAUSESET_INDEXES

#define CCL_CLAUSESET_INDEXES

#include <ccl_fcvindexing.h>
#include <ccl_pdtrees.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct clauseset_indexes
{
   PDTree_p  demod_index; /* If used for demodulators */
   FVIAnchor_p fvindex; /* Used for non-unit subsumption */
}ClausesetIndexes, *ClausesetIndexes_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

ClausesetIndexes_p ClausesetIndexesAlloc();
void ClausesetIndexesFree(ClausesetIndexes_p clauseset_indexes);
void ClausesetIndexesPDTIndexedInsert(ClausesetIndexes_p clauseset_indexes, Clause_p newclause);

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/


ClausesetIndexes_p ClausesetIndexesAllocRaw();

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/

#endif