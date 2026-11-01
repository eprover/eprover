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
   DOPruneArg,
   DOStepQuote  /* A quote of the copy that --proof-object=3 keeps
                   before a simplification step */
}OpCode;


/* A DerivationCode is an OpCode (bits 0-7), a 4 bit kind field for
   each of up to DC_MAX_ARGS arguments (bits 8-23), and the HO flag
   (bit 24). Arguments are used in order (argument n implies
   argument n-1). Only formulas and clauses are parents. */

#define DC_MAX_ARGS    4
#define DC_OPCODE_MASK 0xFF
#define DCArgShift(n)  (4*(n)+4)

typedef enum
{
   ArgFof  = 1,
   ArgCnf  = 2,
   ArgNum  = 4,
   ArgTerm = 8,
   ArgKindMask = ArgFof|ArgCnf|ArgNum|ArgTerm
}ArgKind;

typedef enum
{
   Arg1Fof  = ArgFof<<DCArgShift(1),
   Arg1Cnf  = ArgCnf<<DCArgShift(1),
   Arg1Num  = ArgNum<<DCArgShift(1),
   Arg1Term = ArgTerm<<DCArgShift(1),
   Arg2Fof  = ArgFof<<DCArgShift(2),
   Arg2Cnf  = ArgCnf<<DCArgShift(2),
   Arg2Num  = ArgNum<<DCArgShift(2),
   Arg2Term = ArgTerm<<DCArgShift(2),
   Arg3Fof  = ArgFof<<DCArgShift(3),
   Arg3Cnf  = ArgCnf<<DCArgShift(3),
   Arg3Num  = ArgNum<<DCArgShift(3),
   Arg3Term = ArgTerm<<DCArgShift(3),
   Arg4Fof  = ArgFof<<DCArgShift(4),
   Arg4Cnf  = ArgCnf<<DCArgShift(4),
   Arg4Num  = ArgNum<<DCArgShift(4),
   Arg4Term = ArgTerm<<DCArgShift(4),
   ArgIsHO  = 1<<DCArgShift(DC_MAX_ARGS+1)
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
   /* A single Skolemization step with its premise, the eliminated
      variable and the Skolem term replacing it */
   DCSkolemizeSym     = DOSkolemize|Arg1Fof|Arg2Term|Arg3Term,
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
   DCIntroDef         = DOIntroDef|Arg1Num,
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
   DCPruneArg         = DOPruneArg|ArgIsHO,
   DCCnfStepQuote     = DOStepQuote|Arg1Cnf
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


/* Argument n (1..DC_MAX_ARGS) of a derivation code */
#define DCOpArgKind(op,n)       (((op)>>DCArgShift(n))&ArgKindMask)
#define DCOpHasArg(op,n)        (DCOpArgKind(op,n)!=0)
#define DCOpHasFofArg(op,n)     (DCOpArgKind(op,n)&ArgFof)
#define DCOpHasCnfArg(op,n)     (DCOpArgKind(op,n)&ArgCnf)
#define DCOpHasNumArg(op,n)     (DCOpArgKind(op,n)&ArgNum)
#define DCOpHasTermArg(op,n)    (DCOpArgKind(op,n)&ArgTerm)
#define DCOpHasParentArg(op,n)  (DCOpArgKind(op,n)&(ArgFof|ArgCnf))

/* Number of stack slots taken by the arguments */
#define DCOpArgCount(op) (DCOpHasArg(op,1)+DCOpHasArg(op,2)+   \
                          DCOpHasArg(op,3)+DCOpHasArg(op,4))
#define DCOpHasTermArgs(op) ((op)&(Arg1Term|Arg2Term|Arg3Term|Arg4Term))

#define DPOpGetOpCode(op)  ((op)&DC_OPCODE_MASK)
#define DCOpIsGenerating(op) ((DPOpGetOpCode(op) >= DOParamod)&&(DPOpGetOpCode(op) <= DOSatGen))

#define DPSetIsHO(op) ((op) | ArgIsHO)
#define DPGetIsHO(op) ((op) & ArgIsHO)
#define DPClearIsHO(op) ((op) & ~ArgIsHO)

void ClausePushDerivation(Clause_p clause, DerivationCode op, ...);

void ClausePushACResDerivation(Clause_p clause, Sig_p sig);

void WFormulaPushDerivation(WFormula_p form, DerivationCode op, ...);

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


void DerivStackGCMarkTerms(TB_p bank, PStack_p derivation);
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

Derivation_p DerivationCompute(PStack_p root_clauses, Sig_p sig,
                               PStack_p step_sides);

void DerivationAnalyse(Derivation_p derivationt);
long DerivationCollectFCodes(Derivation_p derived, NumTree_p *tree);

void DerivationPrint(FILE* out, Derivation_p derivation);
void DerivationDotPrint(FILE* out, Derivation_p derivation,
                        ProofOutput print_derivation);

void DerivationPrintConditional(FILE* out, char* status, Derivation_p derivation,
                                Sig_p sig, ProofOutput print_derivation,
                                bool print_analysis);

void DerivationComputeAndPrint(FILE* out, char* status, PStack_p root_clauses,
                               Sig_p sig, PStack_p step_sides,
                               ProofOutput print_derivation,
                               bool print_analysis);

//void print_opdata(FILE* out);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
