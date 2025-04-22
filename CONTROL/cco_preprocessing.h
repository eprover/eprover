/*-----------------------------------------------------------------------

  File  : cco_preprocessing.h

  Author: Stephan Schulz (schulz@eprover.org)

  Contents

  This module encapsulates some of the main proofstate preprocessing,
  mostly to keep the complexity of eprover.c under control.

  Copyright 2025 by the authors.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

  Created:

-----------------------------------------------------------------------*/

#ifndef CCO_PREPROCESSING

#define CCO_PREPROCESSING

#include <ccl_proofstate.h>
#include <che_hcb.h>
#include <cco_ho_inferences.h>
#include <ccl_bce.h>
#include <ccl_pred_elim.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


long ProofStateClausalPreproc(ProofState_p proofstate, HeuristicParms_p h_parms);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
