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
                          Clause_p arg1, Clause_p arg2)
{
   assert(clause);
   assert(op);

   if(!clause->derivation)
   {
      clause->derivation = PStackVarAlloc(3);
   }
   assert(DCOpHasCnfArg1(op)|!arg1);
   assert(DCOpHasCnfArg2(op)|!arg2);
   assert(DCOpHasCnfArg1(op)|!DCOpHasCnfArg2(op));

   PStackPushInt(clause->derivation, op);
   if(arg1)
   {
      PStackPushP(clause->derivation, arg1);
      if(arg2)
      {
         PStackPushP(clause->derivation, arg1);
      }
   }
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


