/*-----------------------------------------------------------------------

File  : ccl_findex.h

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Implementation of function symbol indexing into clauses/formulas.

  Copyright 2009 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Sun May 31 11:20:27 CEST 2009
    New

-----------------------------------------------------------------------*/

#ifndef CCL_FINDEX

#define CCL_FINDEX

#include <clb_plist.h>
#include <ccl_clausesets.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct findex_cell
{
   PDArray_p index; /* We cheat a bit by knowing that FunCodes are
                     * long ints and can be used as indices here. */
}FIndexCell, *FIndex_p;




/*---------------------------------------------------------------------*/
/*                      Functions and Variables                        */
/*---------------------------------------------------------------------*/


#define FIndexCellAlloc()    (FIndexCell*)SizeMalloc(sizeof(FIndexCell))
#define FIndexCellFree(junk) SizeFree(junk, sizeof(FIndexCell))

FIndex_p FIndexAlloc(void);
void     FIndexFree(FIndex_p junk);

void FIndexAddClause(FIndex_p index, Clause_p clause);
void FindexRemoveClause(FIndex_p index, Clause_p clause);
void FIndexAddClauseSet(FIndex_p index, ClauseSet_p set);

void FIndexAddPLClause(FIndex_p index, PList_p lclause);
void FIndexRemovePLClause(FIndex_p index, PList_p lclause);
void FIndexAddPLClauseSet(FIndex_p index, PList_p set);

void FIndexAddPLFormula(FIndex_p index, PList_p lform);
void FIndexRemovePLFormula(FIndex_p index, PList_p lform);
void FIndexAddPLFormulaSet(FIndex_p index, PList_p set);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





