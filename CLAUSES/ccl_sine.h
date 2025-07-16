/*-----------------------------------------------------------------------

File  : ccl_sine.h

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Code for a (generalized) version of the SinE formula selection
  algorithm. See http://www.cs.man.ac.uk/~hoderk/sine/.

  Copyright 2010 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Fri Jul  2 00:55:03 CEST 2010
    New

-----------------------------------------------------------------------*/

#ifndef CCL_SINE

#define CCL_SINE

#include <ccl_f_generality.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* Associate a single function symbol with its "defining" clauses and
 * formulas. */

typedef struct d_rel_cell
{
   FunCode  f_code;
   bool     activated;
   PStack_p d_clauses;
   PStack_p d_formulas;
}DRelCell, *DRel_p;


/* Represent the complete D-Relation for all function symbols. */

typedef struct d_relation_cell
{
   PDArray_p relation; /* Of DRel_p */
}DRelationCell, *DRelation_p;


/* Types of axioms */

typedef enum
{
   ATNoType,
   ATClause,
   ATFormula
}AxiomType;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define DRelCellAlloc()    (DRelCell*)SizeMalloc(sizeof(DRelCell))
#define DRelCellFree(junk) SizeFree(junk, sizeof(DRelCell))

DRel_p DRelAlloc(FunCode f_code);
void   DRelFree(DRel_p rel);

#define DRelationCellAlloc()    (DRelationCell*)SizeMalloc(sizeof(DRelationCell))
#define DRelationCellFree(junk) SizeFree(junk, sizeof(DRelationCell))

DRelation_p DRelationAlloc(void);
void        DRelationFree(DRelation_p rel);
DRel_p DRelationGetFEntry(DRelation_p rel, FunCode f_code);


void DRelationAddClause(DRelation_p drel,
                        GenDistrib_p generality,
                        GeneralityMeasure gentype,
                        double benevolence,
                        long generosity,
                        Clause_p clause);

void DRelationAddFormula(DRelation_p drel,
                         GenDistrib_p generality,
                         GeneralityMeasure gentype,
                         double benevolence,
                         long generosity,
                         bool trim,
                         bool force_def,
                         WFormula_p form);

void DRelationAddClauseSet(DRelation_p drel,
                           GenDistrib_p generality,
                           GeneralityMeasure gentype,
                           double benevolence,
                           long generosity,
                           ClauseSet_p set);

void DRelationAddFormulaSet(DRelation_p drel,
                            GenDistrib_p generality,
                            GeneralityMeasure gentype,
                            double benevolence,
                            long generosity,
                            bool trim,
                            bool force_def,
                            FormulaSet_p set);

void DRelationAddClauseSets(DRelation_p drel,
                            GenDistrib_p generality,
                            GeneralityMeasure gentype,
                            double benevolence,
                            long generosity,
                            PStack_p sets);

void DRelationAddFormulaSets(DRelation_p drel,
                             GenDistrib_p generality,
                             GeneralityMeasure gentype,
                             double benevolence,
                             long generosity,
                             bool trim,
                             bool force_def,
                             PStack_p sets);

void PQueueStoreClause(PQueue_p axioms, Clause_p clause);
void PQueueStoreFormula(PQueue_p axioms, WFormula_p form);

long ClauseSetFindAxSelectionSeeds(ClauseSet_p set,
                                   PQueue_p res,
                                   bool inc_hypos);
long FormulaSetFindAxSelectionSeeds(FormulaSet_p set,
                                    PQueue_p res,
                                    bool inc_hypos);

long SelectDefiningAxioms(DRelation_p drel,
                          Sig_p sig,
                          int max_recursion_depth,
                          long max_set_size,
                          bool trim,
                          PQueue_p axioms,
                          PStack_p res_clauses,
                          PStack_p res_formulas);


long SelectAxioms(GenDistrib_p      f_distrib,
                  PStack_p          clause_sets,
                  PStack_p          formula_sets,
                  PStackPointer     hyp_start,
                  AxFilter_p        ax_filter,
                  PStack_p          res_clauses,
                  PStack_p          res_formulas);

long SelectThreshold(PStack_p          clause_sets,
                     PStack_p          formula_sets,
                     AxFilter_p        ax_filter,
                     PStack_p          res_clauses,
                     PStack_p          res_formulas);

long SelectDefinitions(PStack_p clause_sets, PStack_p formula_sets,
                       PStack_p res_clauses, PStack_p res_formulas);


void DRelPrintDebug(FILE* out, DRel_p rel, Sig_p sig);
void DRelationPrintDebug(FILE* out, DRelation_p rel, Sig_p sig);
long DRelationTotalEntries(DRelation_p rel);

void PStackClauseDelProp(PStack_p stack, FormulaProperties prop);
void PStackFormulaDelProp(PStack_p stack, FormulaProperties prop);

void PStackClausePrintTSTP(FILE* out, PStack_p stack);
void PStackFormulaPrintTSTP(FILE* out, PStack_p stack);

void PStackClausesMove(PStack_p stack, ClauseSet_p set);
void PStackFormulasMove(PStack_p stack, FormulaSet_p set);



#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
