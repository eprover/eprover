/*-----------------------------------------------------------------------

File  : ccl_efficient_subsumption_index.h

Author: Constantin Ruhdorfer

Contents

  Interface for indexing clauses for subsumption.

Copyright 2019-2020 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

-----------------------------------------------------------------------*/

#ifndef CCL_EFFICIENT_SUBSUMPTION_INDEX
#define CCL_EFFICIENT_SUBSUMPTION_INDEX

#include <ccl_fcvindexing.h>
#include <ccl_unitclause_index.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct efficient_subsumption_index
{
   FVIAnchor_p       fvindex;          /* Used for non-unit subsumption */
   UCIndex_p         unitclasue_index; /* Used for unit clauses subsuption */

   /* Clause abstraction */
   bool              wl_constants_abstraction;
   bool              wl_skolemsym_abstraction;
   PDArray_p         wl_abstraction_symbols;
} ESIndex, *ESIndex_p;

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define ESIndexAllocRaw()\
        (ESIndex*)SizeMalloc(sizeof(ESIndex))
#define ESIndexFreeRaw(junk) SizeFree(junk, sizeof(junk))

ESIndex_p ESIndexAlloc(FVCollect_p cspec, PermVector_p perm);
void ESIndexFree(ESIndex_p clauseset_indexes);
void ESIndexUCIndexInit(ESIndex_p index, Sig_p sig, 
                        char* unitclause_index_type);
void ESIndexInsertClause(ESIndex_p index, Clause_p clause);
                                          
Clause_p ClausesetIndexDeleteEntry(ESIndex_p index, Clause_p junk);

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

void esindex_insert(ESIndex_p index, FVPackedClause_p newclause);

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/

#endif