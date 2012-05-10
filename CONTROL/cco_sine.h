/*-----------------------------------------------------------------------

File  : cco_sine.h

Author: Stephan Schulz (schulz@eprover.org)

Contents
 
  Data types and definitions for supporting SinE-like specification
  filtering. 

  Copyright 2012 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Thu May 10 08:35:26 CEST 2012
    New

-----------------------------------------------------------------------*/

#ifndef CCO_SINE

#define CCO_SINE

#include <ccl_proofstate.h>
#include <ccl_sine.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

long ProofStateSinE(ProofState_p state, char* filter);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





