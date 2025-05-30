/*-----------------------------------------------------------------------

  File  : che_heuristics.h

  Author: Stephan Schulz

  Contents

  High-Level interface functions to the heuristics module.

  Copyright 1998-2020 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Mon Jun  8 02:14:51 MET DST 1998

  -----------------------------------------------------------------------*/

#ifndef CHE_HEURISTICS

#define CHE_HEURISTICS

#include <che_proofcontrol.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


HCB_p GetHeuristic(char* source, HCBARGUMENTS);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
