/*-----------------------------------------------------------------------

File  : che_fifo.h

Author: Stephan Schulz

Contents

  FIFO-Evaluation of a clause/

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sat Jul  5 02:28:25 MET DST 1997
    New

-----------------------------------------------------------------------*/

#ifndef CHE_FIFO

#define CHE_FIFO

#include <che_wfcb.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/



/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

WFCB_p FIFOEvalInit(ClausePrioFun prio_fun);

WFCB_p FIFOEvalParse(Scanner_p in, OCB_p ocb, ProofState_p state);

double FIFOEvalCompute(void* data, Clause_p clause);

void   FIFOEvalExit(void* data);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





