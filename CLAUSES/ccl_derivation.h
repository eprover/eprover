/*-----------------------------------------------------------------------

  File  : ccl_derivation.h

  Author: Stephan Schulz (schulz@eprover.org)

  Contents

  Datatypes and definitions for compact representation of derivations
  of a clause.

  Copyright 2013-2018 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

  Changes

  Created: Thu Feb 14 00:21:15 CET 2013

  -----------------------------------------------------------------------*/

#ifndef CCL_DERIVATION

#define CCL_DERIVATION

#include <ccl_inferencedoc.h>
#include <ccl_clauses.h>
#include <ccl_formula_wrapper.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef enum
{
   PONone,
   POList,
   POGraph1,
   POGraph2,
   POGraph3,
}ProofOutput;


typedef enum
{
   DONop,
   DOQuote,
   DOAddCnfArg, /* Extra arguments for multi-argument inferences */
   /* For simplifying inferences, the main premise is implicit */
   DOEvalGC,
   DORewrite,
   DOLocalRewrite,
   DOUnfold,
   DOApplyDef,
   DOContextSR,
   DODesEqRes,
   DOSR,
   DOACRes,
   DOCondense,
   DONormalize,
   DOEvalAnswers,
   /* Simplification/Modfication for FOF */
   DONegateConjecture,
   DOFofSimplify,
   DOFNNF,
   DOShiftQuantors,
   DOVarRename,
   DOSkolemize,
   DODistDisjunctions,
   DOAnnoQuestion,
   DOExpandDistinct,
   /* Generating inferences */
   DOParamod,
   DOSimParamod,
   DOOrderedFactor,
   DOEqFactor,
   DOEqRes,
   DODisEqDecompose,
   DOSatGen,
   DOPEResolve,
   /* CNF conversion and similar */
   DOSplitEquiv,
   DOIntroDef,
   DOSplitConjunct,
   DOEqToEq,
   DOLiftLambdas,
   DOFoolUnroll,
   DOLiftIte,
   DOEliminateBVar,
   /* HO inferences */
   DODynamicCNF,
   DOFlexResolve,
   DOArgCong,
   DONegExt,
   DOPosExt,
   DOExtSup,
   DOExtEqRes,
   DOExtEqFact,
   DOInvRec,
   DOChoiceAx,
   DOLeibnizElim,
   DOPrimEnum,
   DOChoiceInst,
   DOTrigger,
   DOPruneArg
}OpCode;


typedef enum
{
   Arg1Fof = 1<<8,
   Arg1Cnf = 1<<9,
   Arg1Num = 1<<10,
   Arg2Fof = 1<<11,
   Arg2Cnf = 1<<12,
   Arg2Num = 1<<13,
   ArgIsHO = 1<<14,
}ArgDesc;


typedef enum
{
   DCNop              = DONop,
   DCCnfQuote         = DOQuote|Arg1Cnf,
   DCFofQuote         = DOQuote|Arg1Fof,
   DCCnfAddArg        = DOAddCnfArg|Arg1Cnf,
   /* For simplifying inferences, the main premise is implicit */
   DCCnfEvalGC        = DOEvalGC,
   DCRewrite          = DORewrite|Arg1Cnf,
   DCLocalRewrite     = DOLocalRewrite,
   DCUnfold           = DOUnfold|Arg1Cnf,
   DCApplyDef         = DOApplyDef|Arg1Fof,
   DCContextSR        = DOContextSR|Arg1Cnf,
   DCSR               = DOSR|Arg1Cnf,
   DCDesEqRes         = DODesEqRes, /* Doubled because its simplifying here */
   DCACRes            = DOACRes|Arg1Num,
   DCCondense         = DOCondense,
   DCNormalize        = DONormalize,
   DCEvalAnswers      = DOEvalAnswers,
   /* Simplification/Modfication for FOF */
   DCNegateConjecture = DONegateConjecture,
   DCFofSimplify      = DOFofSimplify,
   DCFNNF             = DOFNNF,
   DCShiftQuantors    = DOShiftQuantors,
   DCVarRename        = DOVarRename,
   DCSkolemize        = DOSkolemize,
   DCDistDisjunctions = DODistDisjunctions,
   DCAnnoQuestion     = DOAnnoQuestion,
   DCExpandDistinct   = DOExpandDistinct |Arg1Fof,
   /* Generating inferences */
   DCParamod          = DOParamod |Arg1Cnf|Arg2Cnf,
   DCSimParamod       = DOSimParamod|Arg1Cnf|Arg2Cnf,
   DCOrderedFactor    = DOOrderedFactor|Arg1Cnf,
   DCEqFactor         = DOEqFactor|Arg1Cnf,
   DCEqRes            = DOEqRes|Arg1Cnf,
   DCDisEqDecompose   = DODisEqDecompose|Arg1Cnf,
   DCSatGen           = DOSatGen|Arg1Cnf,
   DCPEResolve        = DOPEResolve|Arg1Cnf|Arg2Cnf,
   /* CNF conversion and similar */
   DCSplitEquiv       = DOSplitEquiv|Arg1Fof,
   DCIntroDef         = DOIntroDef,
   DCSplitConjunct    = DOSplitConjunct|Arg1Fof,
   DCEqToEq           = DOEqToEq,
   DCLiftLambdas      = DOLiftLambdas|Arg1Fof,
   DCFoolUnroll       = DOFoolUnroll,
   DCLiftIte          = DOLiftIte,
   DCEliminateBVar    = DOEliminateBVar,
   /* HO inferences */
   DCDynamicCNF       = DODynamicCNF|Arg1Cnf|ArgIsHO,
   DCFlexResolve      = DOFlexResolve|ArgIsHO,
   DCArgCong          = DOArgCong|Arg1Cnf|ArgIsHO,
   DCNegExt           = DONegExt|Arg1Cnf|ArgIsHO,
   DCPosExt           = DOPosExt|Arg1Cnf|ArgIsHO,
   DCExtSup           = DOExtSup|Arg1Cnf|Arg2Cnf|ArgIsHO,
   DCExtEqRes         = DOExtEqRes|Arg1Cnf|ArgIsHO,
   DCExtEqFact        = DOExtEqFact|Arg1Cnf|ArgIsHO,
   DCInvRec           = DOInvRec|Arg1Cnf|ArgIsHO,
   DCChoiceAx         = DOChoiceAx|ArgIsHO,
   DCLeibnizElim      = DOLeibnizElim|Arg1Cnf|ArgIsHO,
   DCPrimEnum         = DOPrimEnum|Arg1Cnf|ArgIsHO,
   DCChoiceInst       = DOChoiceInst|Arg1Cnf|Arg2Cnf|ArgIsHO,
   DCTrigger          = DOTrigger|Arg1Cnf|Arg2Cnf|ArgIsHO,
   DCPruneArg         = DOPruneArg|ArgIsHO
}DerivationCode;


typedef enum
{
   POInvalidObject = -1,
   PONoObject = 0,
   POSimpleDeriviation = 1,
   PODetailedDerivation = 2,
   POSingleStepDerivation = 3
}ProofObjectType;


typedef struct derived_cell
{
   long       ref_count;
   bool       is_root;
   bool       is_fresh;
   Clause_p   clause;
   WFormula_p formula;
}DerivedCell, *Derived_p;



typedef struct derivation_cell
{
   bool       ordered;
   bool       has_conjecture;
   Sig_p      sig;
   PObjTree_p deriv;
   PStack_p   roots;
   PStack_p   ordered_deriv;
   unsigned long clause_step_count;
   unsigned long formula_step_count;
   unsigned long initial_clause_count;
   unsigned long initial_formula_count;
   unsigned long clause_conjecture_count;
   unsigned long formula_conjecture_count;
   unsigned long generating_inf_count;
   unsigned long simplifying_inf_count;
}DerivationCell, *Derivation_p;




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


extern ProofObjectType PrintProofObject;
extern bool            ProofObjectRecordsGCSelection;


#define DCOpHasCnfArg1(op)   ((op)&Arg1Cnf)
#define DCOpHasFofArg1(op)   ((op)&Arg1Fof)
#define DCOpHasNumArg1(op)    ((op)&Arg1Num)
#define DCOpHasParentArg1(op) ((op)&(Arg1Cnf|Arg1Fof))
#define DCOpHasArg1(op)    ((op)&(Arg1Cnf|Arg1Fof|Arg1Num))

#define DCOpHasCnfArg2(op)    ((op)&Arg2Cnf)
#define DCOpHasFofArg2(op)    ((op)&Arg2Fof)
#define DCOpHasNumArg2(op)    ((op)&Arg2Num)
#define DCOpHasParentArg2(op) ((op)&(Arg2Cnf|Arg2Fof))
#define DCOpHasArg2(op)       ((op)&(Arg2Cnf|Arg2Fof|Arg2Num))

#define DPOpGetOpCode(op)  ((op)&127)
#define DCOpIsGenerating(op) ((DPOpGetOpCode(op) >= DOParamod)&&(DPOpGetOpCode(op) <= DOSatGen))

#define DPSetIsHO(op) ((op) | ArgIsHO)
#define DPGetIsHO(op) ((op) & ArgIsHO)

void ClausePushDerivation(Clause_p clause, DerivationCode op,
                          void* arg1, void* arg2);

void ClausePushACResDerivation(Clause_p clause, Sig_p sig);


void WFormulaPushDerivation(WFormula_p form, DerivationCode op,
                            void* arg1, void* arg2);

bool ClauseIsEvalGC(Clause_p clause);

bool ClauseIsDummyQuote(Clause_p clause);
Clause_p ClauseDerivFindFirst(Clause_p clause);
WFormula_p WFormulaDerivFindFirst(WFormula_p form);


long DerivStackExtractParents(PStack_p derivation,
                              Sig_p sig,
                              PStack_p res_clauses,
                              PStack_p res_formulas);
long DerivStackExtractOptParents(PStack_p derivation,
                                 Sig_p sig,
                                 PStack_p res_clauses,
                                 PStack_p res_formulas);


void DerivStackCountSearchInferences(PStack_p derivation,
                                     unsigned long *generating_count,
                                     unsigned long *simplifying_count);

#define DerivedCellAlloc() (DerivedCell*)SizeMalloc(sizeof(DerivedCell))
#define DerivedCellFree(junk) SizeFree(junk, sizeof(DerivedCell))

Derived_p DerivedAlloc(void);
#define DerivedFree(junk) DerivedCellFree(junk)
#define DerivedGetDerivstack(d)                                         \
   ((d)->clause?(d)->clause->derivation:(d)->formula->derivation)

bool DerivedInProof(Derived_p derived);
void DerivedSetInProof(Derived_p derived, bool in_proof);
long DerivedCollectFCodes(Derived_p derived, NumTree_p *tree);

void DerivationStackPCLPrint(FILE* out, Sig_p sig, PStack_p derivation);
void DerivationStackTSTPPrint(FILE* out, Sig_p sig, PStack_p derivation);
void DerivationDebugPrint(FILE* out, PStack_p derivation);

void DerivedPCLPrint(FILE* out, Sig_p sig, Derived_p derived);
void DerivedTSTPPrint(FILE* out, Sig_p sig, Derived_p derived);
void DerivedDotPrint(FILE* out, Sig_p sig, Derived_p derived,
                     ProofOutput print_derivation);

bool DerivedIsEvalGC(Derived_p derived);

#define DerivationCellAlloc() (DerivationCell*)SizeMalloc(sizeof(DerivationCell))
#define DerivationCellFree(junk) SizeFree(junk, sizeof(DerivationCell))

Derivation_p DerivationAlloc(Sig_p sig);
void         DerivationFree(Derivation_p junk);

Derived_p DerivationGetDerived(Derivation_p derivation, Clause_p clause,
                               WFormula_p formula);


long DerivationExtract(Derivation_p derivation, PStack_p root_clauses);
long DerivationMarkProofSteps(Derivation_p derivation);
long DerivationTopoSort(Derivation_p derivation);
void DerivationRenumber(Derivation_p derivation);

Derivation_p DerivationCompute(PStack_p root_clauses, Sig_p sig);

void DerivationAnalyse(Derivation_p derivationt);
long DerivationCollectFCodes(Derivation_p derived, NumTree_p *tree);

void DerivationPrint(FILE* out, Derivation_p derivation);
void DerivationDotPrint(FILE* out, Derivation_p derivation,
                        ProofOutput print_derivation);

void DerivationPrintConditional(FILE* out, char* status, Derivation_p derivation,
                                Sig_p sig, ProofOutput print_derivation,
                                bool print_analysis);

void DerivationComputeAndPrint(FILE* out, char* status, PStack_p root_clauses,
                               Sig_p sig, ProofOutput print_derivation,
                               bool print_analysis);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
