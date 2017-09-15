/*-----------------------------------------------------------------------

  File  : ccl_grounding.h

  Author: Stephan Schulz

  Contents

  Definitions for functions (and possibly later data types)
  implementing grounding of near-propositional clause sets.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: Tue May 29 02:20:15 CEST 2001

  -----------------------------------------------------------------------*/

#ifndef CCL_GROUNDING

#define CCL_GROUNDING

#include <ccl_g_lithash.h>
#include <ccl_propclauses.h>
#include <ccl_groundconstr.h>
#include <cio_signals.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* For a variable, keep track of possible and done instantiations */

typedef struct var_inst
{
   Term_p        variable;
   PStack_p      alternatives;
   PStackPointer position;
}VarInstCell, *VarInst_p;

/* For many variables, do the same */

typedef struct varset_inst
{
   long size;
   VarInst_p cells;
}VarSetInstCell, *VarSetInst_p;


/* We represent a set of ground clauses in a special way, with
   unit-clauses in a single array for efficient unit subsumption and
   unit resolution */

typedef enum
{
   GCUNone = 0,
   GCUPos  = 1,
   GCUNeg  = 2,
   GCUBoth = GCUPos|GCUNeg
}GCUEncoding;

#define DEFAULT_LIT_NO   4096
#define DEFAULT_LIT_GROW 8192

typedef enum
{
   cpl_complete,
   cpl_lowmem,
   cpl_timeout,
   cpl_unknown
}
   GroundSetState;

typedef struct ground_set_cell
{
   TB_p            lit_bank;  /* Only reference, not administered
                                 from here! */
   long            max_literal; /* Maximal literal number */
   long            unit_no;
   GroundSetState  complete;    /* Is the proofstate complete? */
   PDArray_p       units;       /* Wich ones are present? */
   PDArray_p       unit_terms;  /* And how do they look? */
   PropClauseSet_p non_units;
}GroundSetCell, *GroundSet_p;

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define VarSetInstCellAlloc()    (VarSetInstCell*)SizeMalloc(sizeof(VarSetInstCell))
#define VarSetInstCellFree(junk) SizeFree(junk, sizeof(VarSetInstCell))

VarSetInst_p VarSetInstAlloc(Clause_p clause);
void         VarSetInstFree(VarSetInst_p junk);

VarSetInst_p VarSetConstrInstAlloc(LitOccTable_p p_table,
                                   LitOccTable_p n_table, Clause_p
                                   clause, PTree_p ground_terms);

void VarSetConstrInstFree(VarSetInst_p junk);

int  ClauseCmpByLen(const void* clause1, const void* clause2);
bool EqnEqlitRecode(Eqn_p lit);
bool ClauseEqlitRecode(Clause_p clause);
long ClauseSetEqlitRecode(ClauseSet_p set);

#define EqnLitCode(eq) ((eq)->lterm->entry_no)

#define GroundSetCellAlloc()    (GroundSetCell*)SizeMalloc(sizeof(GroundSetCell))
#define GroundSetCellFree(junk) SizeFree(junk, sizeof(GroundSetCell))

void        PrintDimacsHeader(FILE* out, long max_lit, long members);
void        ClausePrintDimacs(FILE* out, Clause_p clause);
void        ClauseSetPrintDimacs(FILE* out, ClauseSet_p set);
GroundSet_p GroundSetAlloc(TB_p bank);
void        GroundSetFree(GroundSet_p junk);

#define     GroundSetMembers(set) ((set)->unit_no+((set)->non_units->members))

long        GroundSetMaxVar(GroundSet_p set);

/* Dimacs format provers oven cannot cope with empty clauses, so we
   print them as a set of two trivially complementary clauses */
#define     GroundSetDimacsPrintMembers(set)                    \
   (GroundSetMembers(set)+(set)->non_units->empty_clauses)
#define     GroundSetLiterals(set) ((set)->unit_no+((set)->non_units->literals))
bool        GroundSetInsert(GroundSet_p set, Clause_p clause);
void        GroundSetPrint(FILE* out, GroundSet_p set);
void        GroundSetPrintDimacs(FILE* out, GroundSet_p set);

bool        GroundSetUnitSimplifyClause(GroundSet_p set, Clause_p clause,
                                        bool subsume, bool resolve);

bool ClauseCreateGroundInstances(TB_p bank, Clause_p clause,
                                 VarSetInst_p inst, GroundSet_p
                                 groundset, bool subsume, bool
                                 resolve, bool taut_check);

bool ClauseSetCreateGroundInstances(TB_p bank, ClauseSet_p set,
                                    GroundSet_p groundset, bool
                                    subsume, bool resolve, bool
                                    taut_check, long give_up);

bool ClauseSetCreateConstrGroundInstances(TB_p bank, ClauseSet_p set,
                                          GroundSet_p groundset, bool
                                          subsume, bool resolve,
                                          bool taut_check, long
                                          give_up, long
                                          just_one_instance);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
