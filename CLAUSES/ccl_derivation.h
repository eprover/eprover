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

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


typedef enum
{
   DONop,
   DOQuote,
   DORewriteL,
   DORewriteR,
   DOContextSR,
   DODesEqRes, 
   DOSR,
   DOParamod,
   DOSimParamod,
   DOOrderedFactor,
   DOEqFactor,
   DOEqRes,
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
   DCContextSR     = DOContextSR|Arg1Cnf,
   DCSR            = DOSR|Arg1Cnf,
   DCDesEqRes      = DODesEqRes, /* Doubled because its simplifying here */
   /* Generating inferences */
   DCParamod       = DOParamod |Arg1Cnf|Arg2Cnf,
   DCSimParamod    = DOSimParamod|Arg1Cnf|Arg2Cnf,
   DCOrderedFactor = DOOrderedFactor|Arg1Cnf,
   DCEqFactor      = DOEqFactor|Arg1Cnf,
   DCEqRes         = DOEqRes|Arg1Cnf
}DerivationCodes;


typedef enum
{
   PONoObject = 0,
   POSimpleDeriviation = 1,
   PODetailedDerivation = 2,
   POSingleStepDerivation = 3
}ProofObjectType;

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/



extern ProofObjectType BuildProofObject;

#define DCOpHasCnfArg1(op) ((op)&Arg1Cnf)
#define DCOpHasCnfArg2(op) ((op)&Arg2Cnf)

void ClausePushDerivation(Clause_p clause, DerivationCodes op, 
                          Clause_p arg1, Clause_p arg2);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





