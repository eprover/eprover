/*-----------------------------------------------------------------------

File  : ccl_derivations.h

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Datatypes and definitions for compact representation of derivations
  of a clause. 

  Copyright 2013 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Thu Feb 14 00:21:15 CET 2013
    New

-----------------------------------------------------------------------*/

#ifndef CCL_DERIVATION

#define CCL_DERIVATION

#include <ccl_clauses.h>
#include <ccl_formula_wrapper.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


typedef enum
{
   DONop,
   DOQuote,
   DORewriteL,
   DORewriteR,
   DOApplyDef,
   DOContextSR,
   DODesEqRes, 
   DOSR,
   DOParamod,
   DOSimParamod,
   DOOrderedFactor,
   DOEqFactor,
   DOEqRes,
   DOSplitEquiv
}OpCodes;


typedef enum
{
   Arg1Fof = 1<<8,
   Arg1Cnf = 1<<9,
   Arg2Fof = 1<<10,
   Arg2Cnf = 1<<11,
}ArgDesc;


typedef enum
{
   DCNop           = DONop,
   DCCnfQuote      = DOQuote|Arg1Cnf,
   /* For simplifying inferences, the main premise is implicit */
   DCRewriteL      = DORewriteL|Arg1Cnf,
   DCRewriteR      = DORewriteR|Arg1Cnf,
   DCApplyDef      = DOApplyDef|Arg1Fof,
   DCContextSR     = DOContextSR|Arg1Cnf,
   DCSR            = DOSR|Arg1Cnf,
   DCDesEqRes      = DODesEqRes, /* Doubled because its simplifying here */
   /* Generating inferences */
   DCParamod       = DOParamod |Arg1Cnf|Arg2Cnf,
   DCSimParamod    = DOSimParamod|Arg1Cnf|Arg2Cnf,
   DCOrderedFactor = DOOrderedFactor|Arg1Cnf,
   DCEqFactor      = DOEqFactor|Arg1Cnf,
   DCEqRes         = DOEqRes|Arg1Cnf,
   /* CNF conversion and similar */
   DCSplitEquiv    = DOSplitEquiv|Arg1Fof
}DerivationCodes;


typedef enum
{
   PONoObject = 0,
   POSimpleDeriviation = 1,
   PODetailedDerivation = 2,
   POSingleStepDerivation = 3
}ProofObjectType;


typedef struct derived_cell
{
   long       ref_count;
   Clause_p   clause;
   WFormula_p formula;
}DerivedCell, *Derived_p;


typedef struct derivation_cell
{
   PObjTree_p deriv;
   PStack_p   orfdered_deriv;
}DerivationCell, *Derivation_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


extern ProofObjectType BuildProofObject;

#define DCOpHasCnfArg1(op) ((op)&Arg1Cnf)
#define DCOpHasCnfArg2(op) ((op)&Arg2Cnf)
#define DCOpHasFofArg1(op) ((op)&Arg1Fof)
#define DCOpHasFofArg2(op) ((op)&Arg2Fof)

#define DerivedCellAlloc() (DerivedCell*)SizeMalloc(sizeof(DerivedCell))
#define DerivedCellFree(junk) SizeFree(junk, sizeof(DerivedCell))

Derived_p DerivedAlloc(void);
#define DerivedFree(junk) DerivedCellFree(junk)

#define DerivationCellAlloc() (DerivationCell*)SizeMalloc(sizeof(DerivationCell))
#define DerivationCellFree(junk) SizeFree(junk, sizeof(DerivationCell))

Derivation_p DerivationAlloc(void);
void         DerivationFree(Derivation_p junk);

void ClausePushDerivation(Clause_p clause, DerivationCodes op, 
                          void* arg1, void* arg2);

long DerivStackExtractParents(PStack_p derivation, 
                              PStack_p res_clauses, 
                              PStack_p res_formulas);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





