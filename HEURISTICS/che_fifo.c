/*-----------------------------------------------------------------------

File  : che_fifo.c

Author: Stephan Schulz

Contents

  FIFO-Clause evaluation

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Fri Jun  5 22:51:52 MET DST 1998
    New

-----------------------------------------------------------------------*/

#include "che_fifo.h"



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
// Function: FIFOEvalInit()
//
//   Return an initialized WFCB for FIFO evaluation.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

WFCB_p FIFOEvalInit(ClausePrioFun prio_fun)
{
   double *data;

   data = SizeMalloc(sizeof(double));
   *data = 0.0;

   return WFCBAlloc(FIFOEvalCompute, prio_fun, FIFOEvalExit, data);
}


/*-----------------------------------------------------------------------
//
// Function: FIFOEvalParse()
//
//   Parse a fifo-declaration.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

WFCB_p FIFOEvalParse(Scanner_p in, OCB_p ocb, ProofState_p state)
{
   ClausePrioFun prio_fun;

   AcceptInpTok(in, OpenBracket);
   prio_fun = ParsePrioFun(in);
   AcceptInpTok(in, CloseBracket);

   return FIFOEvalInit(prio_fun);
}



/*-----------------------------------------------------------------------
//
// Function: FIFOEvalCompute()
//
//   Compute an evaluation for a clause.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

double FIFOEvalCompute(void* data, Clause_p clause)
{
   double *local = data;

   *local = (*local)+1.0;
   return *local;
}


/*-----------------------------------------------------------------------
//
// Function: FIFOEvalExit()
//
//   Free the data entry in a FIFO WFCB.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void FIFOEvalExit(void* data)
{
   SizeFree(data, sizeof(double));
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


