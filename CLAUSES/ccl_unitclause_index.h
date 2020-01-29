/*-----------------------------------------------------------------------

File  : ccl_unitclause_index.h

Author: Constantin Ruhdorfer

Contents

  A simple index for unitclauses.

Copyright 2019-2020 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

-----------------------------------------------------------------------*/

#ifndef CCL_UNITCLAUSESE_INDEXES

#define CCL_UNITCLAUSESE_INDEXES

#include <ccl_clauses.h>
#include <cte_fp_index.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
Datastructure that indexes unit clauses by representing them as a single 
equals term (e.g. =(lterm, rterm)). This indexing technique should use a
special Fingerprintindexing function that ignores the first symbol as it 
is always the same.
The leaves store the indexed term.
-----------------------------------------------------------------------*/
typedef struct unitclause_index_cell 
{
   Clause_p clause; // The indexed clause
} UnitClauseIndexCell, *UnitClauseIndexCell_p;

typedef FPIndex_p UnitclauseIndex_p;

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define UnitClauseIndexCellAllocRaw() (UnitClauseIndexCell*)SizeMalloc(sizeof(UnitClauseIndexCell))
#define UnitClauseIndexCellFreeRaw(junk) SizeFree(junk, sizeof(UnitClauseIndexCell))

UnitClauseIndexCell_p UnitClauseIndexCellAlloc();
bool UnitclauseIndexDeleteClause(UnitclauseIndex_p index, Clause_p clause);
bool UnitclauseIndexInsertClause(UnitclauseIndex_p index, Clause_p clause);
long UnitClauseIndexFindSubsumedCandidates(UnitclauseIndex_p index, 
                                           Clause_p clause, PStack_p candidates);
void UnitclauseIndexFreeWrapper(void *junk);

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

// void UnitClauseIndexCellFreeWrapper(void *junk);
void UnitClauseIndexCellFree(UnitClauseIndexCell_p junk);
int CmpUnitClauseIndexCells(const void* cell1, const void* cell2);
// UnitClauseIndexCell_p UnitclauseInsertCell(PObjTree_p *root, Clause_p clause);
bool UnitclauseInsertCell(PTree_p *root, Clause_p clause);
// bool UnitclauseIndexDeleteIndexedClause(UnitclauseIndex_p index, 
//                                         Term_p indexedterm,
//                                         Clause_p indexed);
bool UnitclauseIndexDeletClauseCell(PTree_p *root, Clause_p indexed);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/