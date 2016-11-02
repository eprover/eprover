/*-----------------------------------------------------------------------

File  : che_lifo.h

Author: Stephan Schulz

Contents

  LIFO-Evaluation of a clause (unfair!)

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Mon Jun 22 15:28:23 MET DST 1998
    New

-----------------------------------------------------------------------*/

#ifndef CHE_LIFO

#define CHE_LIFO

#include <che_wfcb.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/



/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


WFCB_p LIFOEvalInit(ClausePrioFun prio_fun);

WFCB_p LIFOEvalParse(Scanner_p in, OCB_p ocb, ProofState_p state);

double LIFOEvalCompute(void* data, Clause_p clause);

void   LIFOEvalExit(void* data);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





