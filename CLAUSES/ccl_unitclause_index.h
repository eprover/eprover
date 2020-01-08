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
   Term_p termL;
   PTree_p right;
} UnitClauseIndexCell, *UnitClauseIndexCell_p;

typedef FPIndex_p UnitclauseIndex_p;

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

void UnitclauseIndexFreeWrapper(void *junk);

#define UnitClauseIndexCellAlloc() (UnitClauseIndexCell*)SizeMalloc(sizeof(UnitClauseIndexCell))
#define UnitClauseIndexCellFreeRaw(junk) SizeFree(junk, sizeof(UnitClauseIndexCell))

bool UnitclauseIndexDeleteClause(UnitclauseIndex_p index, Clause_p clause);
void UnitclauseIndexDeleteTerm(PObjTree_p *root, Term_p lterm);
bool UnitclauseIndexDeleteCellTerm(PObjTree_p *root, Term_p lterm, Term_p rterm);
bool UnitclauseIndexDeleteRightTerm(UnitclauseIndex_p index, Term_p lterm, Term_p rterm);

bool UnitclauseIndexInsertClause(UnitclauseIndex_p index, Clause_p clause);
bool UnitclauseIndexInsert(UnitclauseIndex_p index, Term_p lterm, Term_p rterm);

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

void UnitClauseIndexCellFree(UnitClauseIndexCell_p junk);
int CmpUnitClauseIndexCells(const void* cell1, const void* cell2);
UnitClauseIndexCell_p UnitclauseInsert(PObjTree_p *root, Term_p lterm);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/