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



/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


typedef enum
{
   NOP,
   RWL,
   RWR,
   PM,
   SPM,
   
}OpCodes;


typedef enum
{
   Arg1Fof = 1<<8,
   Arg1Cnf = 1<<9,
   Arg1Pos = 1<<10,
   Arg2Fof = 1<<11,
   Arg2Cnf = 1<<12,
   Arg2Pos = 1<<13,
   Arg3Fof = 1<<14,
   Arg3Cnf = 1<<15,
   Arg3Pos = 1<<16  
}ArgDesc;


typedef enum
{
   DCNOP = NOP, 
   DCRWL = RWL|Arg1Cnf,
   DCRWR = RWR|Arg1Cnf,
   
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



#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





