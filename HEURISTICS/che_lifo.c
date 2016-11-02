/*-----------------------------------------------------------------------

File  : che_lifo.c

Author: Stephan Schulz

Contents

  LIFO-Clause evaluation

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1>Mon Jun 22 15:28:23 MET DST 1998
    New

-----------------------------------------------------------------------*/

#include "che_lifo.h"



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
// Function: LIFOEvalInit()
//
//   Return an initialized WFCB for FIFO evaluation.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

WFCB_p LIFOEvalInit(ClausePrioFun prio_fun)
{
   double *data;

   data = SizeMalloc(sizeof(double));
   *data = 0.0;

   return WFCBAlloc(LIFOEvalCompute, prio_fun, LIFOEvalExit, data);
}

/*-----------------------------------------------------------------------
//
// Function: LIFOEvalParse()
//
//   Parse a lifo-declaration.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

WFCB_p LIFOEvalParse(Scanner_p in, OCB_p ocb, ProofState_p state)
{
   ClausePrioFun prio_fun;

   AcceptInpTok(in, OpenBracket);
   prio_fun = ParsePrioFun(in);
   AcceptInpTok(in, CloseBracket);

   return LIFOEvalInit(prio_fun);
}


/*-----------------------------------------------------------------------
//
// Function: LIFOEvalCompute()
//
//   Compute an evaluation for a clause.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

double LIFOEvalCompute(void* data, Clause_p clause)
{
   double *local = data;

   *local = (*local)-1.0;

   return *local;
}


/*-----------------------------------------------------------------------
//
// Function: LIFOEvalExit()
//
//   Free the data entry in a LIFO WFCB.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void LIFOEvalExit(void* data)
{
   SizeFree(data, sizeof(double));
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


