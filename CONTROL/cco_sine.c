/*-----------------------------------------------------------------------

File  : cco_sine.c

Author: Stephan Schulz (schulz@eprover.org)

Contents

  SinE-like specification filtering. 

  Copyright 2012 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Thu May 10 15:39:26 CEST 2012
    New

-----------------------------------------------------------------------*/

#include "cco_sine.h"




/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


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
// Function: ProofStateSinE()
//
//   Apply SinE with the specified filter to the proofstate (in
//   particular state->f_axioms and state->axioms). This is
//   destructive. Returns number of axioms deleted.
//
// Global Variables: -
//
// Side Effects    : Memory operations aplenty.
//
/----------------------------------------------------------------------*/

long ProofStateSinE(ProofState_p state, char* filter)
{
   long res = 0;

   printf("# Magic happens here!\n");

   return res;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


