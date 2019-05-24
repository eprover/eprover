/*-----------------------------------------------------------------------

  File  : che_random.c

  Author: Stephan Schulz

  Contents

  "Random" clause "evaluation". ;-)

  Copyright 2019 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Fri Jun  5 22:51:52 MET DST 1998
    New

-----------------------------------------------------------------------*/

#include "che_random.h"



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
// Function: RandWeightInit()
//
//   Return an initialized WFCB for Random evaluation.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

WFCB_p RandWeightInit(ClausePrioFun prio_fun, long range, double fifo_w,
                    double sc_w, unsigned int seed1, unsigned int seed2,
                    unsigned int seed3)
{
   RandomWeightParam_p data = RandomWeightParamCellAlloc();

   data->fifo_counter = 0.0;
   data->rand_range   = range;
   data->fifo_weight  = fifo_w;
   data->sc_weight    = sc_w;
   data->rand_state.xstate = 684291357;
   data->rand_state.ystate = 123459876;
   data->rand_state.zstate = 918273645;
   data->rand_state.cstate = 129834675;

   if(seed1)
   {
      data->rand_state.xstate = seed1;
   }
   if(seed2)
   {
      data->rand_state.ystate = seed2;
   }
   if(seed3)
   {
      data->rand_state.zstate = seed3;
   }
   return WFCBAlloc(RandWeightCompute, prio_fun, RandWeightExit, data);
}


/*-----------------------------------------------------------------------
//
// Function: RandWeightParse()
//
//   Parse a Random declaration of the form
//   (priofun, range, fifo_w, sc_w [, seed1 [, seed2 [,seed3]]])
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

WFCB_p RandWeightParse(Scanner_p in, OCB_p ocb, ProofState_p state)
{
   ClausePrioFun prio_fun;
   unsigned int rand_range;
   double        fifo_w;
   double        sc_w;
   unsigned int  seed1 = 0, seed2 = 0, seed3 = 0;

   AcceptInpTok(in, OpenBracket);
   prio_fun = ParsePrioFun(in);

   AcceptInpTok(in, Comma);
   rand_range = ParseInt(in);
   AcceptInpTok(in, Comma);
   fifo_w = ParseFloat(in);
   AcceptInpTok(in, Comma);
   sc_w = ParseFloat(in);
   if(TestInpTok(in, Comma))
   {
      AcceptInpTok(in, Comma);
      seed1=ParseInt(in);
   }
   if(TestInpTok(in, Comma))
   {
      AcceptInpTok(in, Comma);
      seed2=ParseInt(in);
   }
   if(TestInpTok(in, Comma))
   {
      AcceptInpTok(in, Comma);
      seed3=ParseInt(in);
   }

   AcceptInpTok(in, CloseBracket);

   return RandWeightInit(prio_fun, rand_range,
                       fifo_w, sc_w, seed1, seed2, seed3);
}


/*-----------------------------------------------------------------------
//
// Function: RandWeightCompute()
//
//   Compute an evaluation for a clause.
//
// aGlobal Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

double RandWeightCompute(void* data, Clause_p clause)
{
   RandomWeightParam_p local = data;

   double fifo, sc, rnd;

   sc   = ClauseStandardWeight(clause);
   fifo = (local->fifo_counter)++;
   rnd  = JKISSRandDouble(&(local->rand_state));

   //printf("%d  %f  %f\n", local->rand_range,
   //       local->sc_weight, local->fifo_weight);

   return rnd*local->rand_range
      +sc*local->sc_weight
      +fifo*local->fifo_weight;
}


/*-----------------------------------------------------------------------
//
// Function: RandWeightExit()
//
//   Free the data entry in a Random WFCB.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void RandWeightExit(void* data)
{
   RandomWeightParamCellFree(data);
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
