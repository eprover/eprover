/*-----------------------------------------------------------------------

  File  : ccl_propclauses.h

  Author: Stephan Schulz

  Contents

  Definitions for propositional clauses (for eground) which can be
  stored much more compactly than ordinary clauses - at the price of
  less functionality and flexibility.

  Copyright 1998-2011 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: Sun Dec 16 16:29:02 CET 2001

  -----------------------------------------------------------------------*/

#ifndef CCL_PROPCLAUSES

#define CCL_PROPCLAUSES

#include <ccl_clauses.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct proplitcell
{
   EqnProperties properties;
   Term_p        lit;
}PropLitCell, *PropLit_p;


typedef struct propclausecell
{
   int        lit_no;
   PropLit_p  literals;
   struct propclausecell *next; /* For linear lists == PropClauseSets */
}PropClauseCell, *PropClause_p;

typedef struct propclausesetcell
{
   long         members;
   long         literals;
   long         empty_clauses;
   PropClause_p list;    /* List of clauses */
   PropClause_p *inspos; /* Points to next field of last clause, so we
                            can keep inserted clauses in order - I
                            believe giving propositional provers small
                            clauses first may be beneficial */
}PropClauseSetCell, *PropClauseSet_p;





/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define PropClauseCellAlloc()    (PropClauseCell*)SizeMalloc(sizeof(PropClauseCell))
#define PropClauseCellFree(junk) SizeFree(junk, sizeof(PropClauseCell))

PropClause_p PropClauseAlloc(Clause_p clause);
void         PropClauseFree(PropClause_p clause);
Clause_p     PropClauseToClause(TB_p bank, PropClause_p clause);
void         PropClausePrint(FILE* out, TB_p bank, PropClause_p
                             clause);
long         PropClauseMaxVar(PropClause_p clause);


#define PropClauseSetCellAlloc()    (PropClauseSetCell*)SizeMalloc(sizeof(PropClauseSetCell))
#define PropClauseSetCellFree(junk) SizeFree(junk, sizeof(PropClauseSetCell))

PropClauseSet_p PropClauseSetAlloc(void);
void            PropClauseSetFree(PropClauseSet_p set);
long            PropClauseSetInsertPropClause(PropClauseSet_p set,
                                              PropClause_p clause);
long            PropClauseSetInsertClause(PropClauseSet_p set,
                                          Clause_p clause);
void            PropClauseSetPrint(FILE* out, TB_p bank,
                                   PropClauseSet_p set);
long            PropClauseSetMaxVar(PropClauseSet_p set);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
