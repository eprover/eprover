/*-----------------------------------------------------------------------

File  : ccl_derivation.c

Author: Stephan Schulz (schulz@eprover.org)

Contents
 
  Functions related to the construction, manipulation, and printing of
  explicit proof objects in E.

  Copyright 2013 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main E directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Sat Apr  6 10:58:29 CEST 2013
    New

-----------------------------------------------------------------------*/

#include "ccl_derivation.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

ProofObjectType BuildProofObject = 0; 

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: ClausePushDerivation()
//
//   Push the derivation items (op-code and suitable number of
//   arguments) onto the derivation stack.
//
// Global Variables: -
//
// Side Effects    : May allocate new derivation stack.
//
/----------------------------------------------------------------------*/

void ClausePushDerivation(Clause_p clause, DerivationCodes op, 
                          void* arg1, void* arg2)
{
   assert(clause);
   assert(op);

   if(!clause->derivation)
   {
      clause->derivation = PStackVarAlloc(3);
   }
   assert(DCOpHasCnfArg1(op)||DCOpHasFofArg1(op)||!arg1);
   assert(DCOpHasCnfArg2(op)||DCOpHasFofArg2(op)||!arg2);
   assert(DCOpHasCnfArg1(op)||!DCOpHasCnfArg2(op));

   PStackPushInt(clause->derivation, op);
   if(arg1)
   {
      PStackPushP(clause->derivation, arg1);
      if(arg2)
      {
         PStackPushP(clause->derivation, arg2);
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: DerivStackExtractParents()
//
//   Given a derivation stack (derivation-codes with arguments),
//   return all the (occurances of) all the side premises referenced
//   in the derivation (via the result stacks). Return value is the
//   number of premises found.
//
// Global Variables: 
//
// Side Effects    : 
//
/----------------------------------------------------------------------*/

long DerivStackExtractParents(PStack_p derivation, 
                              PStack_p res_clauses, 
                              PStack_p res_formulas)
{
   PStackPointer i, sp;
   long res = 0;
   DerivationCodes op;

   sp = PStackGetSP(derivation);
   
   while(i<sp)
   {
      op = PStackElementInt(derivation, i);
      i++;
      if(DCOpHasCnfArg1(op))
      {
         PStackPushP(res_clauses, PStackElementP(derivation, i));
         i++;
         res++;
      }
      else if(DCOpHasFofArg1(op))
      {
         PStackPushP(res_formulas, PStackElementP(derivation, i));
         i++;
         res++;
      }
      if(DCOpHasCnfArg2(op))
      {
         PStackPushP(res_clauses, PStackElementP(derivation, i));
         i++;
         res++;
      }
      else if(DCOpHasFofArg1(op))
      {
         PStackPushP(res_formulas, PStackElementP(derivation, i));
         i++;
         res++;
      }
   }
   return res;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


