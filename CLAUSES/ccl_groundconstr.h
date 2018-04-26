/*-----------------------------------------------------------------------

  File  : ccl_groundconstr.h

  Author: Stephan Schulz

  Contents

  Computing constraints on the possible instances of groundable
  clauses.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: Thu Jun  7 23:45:05 MEST 2001

  -----------------------------------------------------------------------*/

#ifndef CCL_GROUNDCONSTR

#define CCL_GROUNDCONSTR

#include <ccl_clausesets.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


typedef struct lit_constr_cell
{
   bool    constrained;
   PTree_p constraints;
}LitConstrCell;

typedef struct lit_occ_table
{
   long          sig_size;
   int           maxarity;
   LitConstrCell *matrix;
}LitOccTableCell, *LitOccTable_p;



/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


#define LitOccTableCellAlloc()    (LitOccTableCell*)SizeMalloc(sizeof(LitOccTableCell))
#define LitOccTableCellFree(junk) SizeFree(junk, sizeof(LitOccTableCell))

#define LIT_OCC_TABLE_REF(table, pred, arity) \
        (&((table)->matrix[(((table)->sig_size)*(arity)+(pred))]))

#define LIT_OCC_TABLE_ENTRY(table, pred, arity) (*LIT_OCC_TABLE_REF(table,pred,arity))

LitOccTable_p LitOccTableAlloc(Sig_p sig);
void          LitOccTableFree(LitOccTable_p junk);

bool    LitPosGetConstrState(LitOccTable_p table, FunCode pred, int pos);
void    LitPosSetConstrState(LitOccTable_p table, FunCode pred, int
                             pos, bool value);

PTree_p LitPosGetConstraints(LitOccTable_p table, FunCode pred, int pos);
bool    LitPosAddConstraint(LitOccTable_p table, FunCode pred, int
                            pos, Term_p term);

void    LitOccAddLitAdd(LitOccTable_p p_table, LitOccTable_p n_table,
                        Eqn_p eqn);

void    LitOccAddClauseAdd(LitOccTable_p p_table, LitOccTable_p
                           n_table, Clause_p clause);

void    LitOccAddClauseSetAlt(LitOccTable_p p_table, LitOccTable_p
                              n_table, ClauseSet_p set);

long    SigCollectConstantTerms(TB_p bank, PStack_p stack, FunCode uniq);

void    EqnCollectVarConstr(LitOccTable_p p_table, LitOccTable_p
                            n_table, PDArray_p var_constr, Eqn_p
                            eqn);
void    ClauseCollectVarConstr(LitOccTable_p p_table, LitOccTable_p
                               n_table, Clause_p clause, PTree_p
                               ground_terms, PDArray_p var_constr);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
