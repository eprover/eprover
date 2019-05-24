/*-----------------------------------------------------------------------

  File  : che_random.h

  Author: Stephan Schulz

  Contents

  Clause "evaluations" incorporating random elements. Note that these
  are not, in general, fair if used with naive parameterization.

  Copyright 2019 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Fri May 24 14:50:44 CEST 2019

-----------------------------------------------------------------------*/

#ifndef CHE_RANDOM

#define CHE_RANDOM

#include <che_wfcb.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct
{
   double        fifo_counter;
   unsigned int  rand_range;
   double        fifo_weight;
   double        sc_weight;
   RandStateCell rand_state;
}RandomWeightParamCell, *RandomWeightParam_p;

#define RandomWeightParamCellAlloc() (RandomWeightParamCell*) \
        SizeMalloc(sizeof(RandomWeightParamCell))
#define RandomWeightParamCellFree(junk) \
        SizeFree(junk, sizeof(RandomWeightParamCell))

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

WFCB_p RandWeightInit(ClausePrioFun prio_fun, long range, double fifo_w,
                      double sc_w, unsigned int seed1, unsigned int seed2,
                      unsigned int seed3);

WFCB_p RandWeightParse(Scanner_p in, OCB_p ocb, ProofState_p state);

double RandWeightCompute(void* data, Clause_p clause);

void   RandWeightExit(void* data);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
