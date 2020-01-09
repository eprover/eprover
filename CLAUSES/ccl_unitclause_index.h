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

typedef struct unitclause_index_cell 
{
   Clause_p clause; // The indexed clause
} UnitClauseIndexCell, *UnitClauseIndexCell_p;

typedef FPIndex_p UnitclauseIndex_p;

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define UnitClauseIndexCellAlloc() (UnitClauseIndexCell*)SizeMalloc(sizeof(UnitClauseIndexCell))
#define UnitClauseIndexCellFreeRaw(junk) SizeFree(junk, sizeof(UnitClauseIndexCell))

UnitClauseIndexCell_p UnitClauseIndexAlloc();
bool UnitclauseIndexDeleteClause(UnitclauseIndex_p index, Clause_p clause);
bool UnitclauseIndexInsertClause(UnitclauseIndex_p index, Clause_p clause);
void UnitclauseIndexFreeWrapper(void *junk);

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

void UnitClauseIndexCellFreeWrapper(void *junk);
void UnitClauseIndexCellFree(UnitClauseIndexCell_p junk);
int CmpUnitClauseIndexCells(const void* cell1, const void* cell2);
UnitClauseIndexCell_p UnitclauseInsertCell(PObjTree_p *root, Clause_p clause);
bool UnitclauseIndexDeleteIndexedClause(UnitclauseIndex_p index, 
                                        Term_p indexedterm,
                                        Clause_p indexed);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/