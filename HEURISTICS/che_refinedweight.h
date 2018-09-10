/*-----------------------------------------------------------------------

File  : che_refinedweight.h

Author: Stephan Schulz

Contents

  Evaluation of a clause by refined clause weight, using weight
  penalty factors for maximal terms and literals.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed Jun 17 00:11:03 MET DST 1998
    New

-----------------------------------------------------------------------*/

#ifndef CHE_REFINEDWEIGHT

#define CHE_REFINEDWEIGHT

#include <che_clauseweight.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

#define DEFAULT_MAX_MULT 1.5

typedef struct refinedweightparamcell
{
   OCB_p  ocb;
   double max_term_multiplier;
   double max_literal_multiplier;
   double pos_multiplier;
   double app_var_mult;
   long   vweight;
   long   fweight;
}RefinedWeightParamCell, *RefinedWeightParam_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


#define RefinedWeightParamCellAlloc() (RefinedWeightParamCell*) \
        SizeMalloc(sizeof(RefinedWeightParamCell))
#define RefinedWeightParamCellFree(junk) \
        SizeFree(junk, sizeof(RefinedWeightParamCell))

WFCB_p ClauseRefinedWeightInit(ClausePrioFun prio_fun, int fweight,
                int vweight, OCB_p ocb, double
                max_term_multiplier, double
                max_literal_multiplier, double
                pos_multiplier, double app_var_mult);

WFCB_p ClauseRefinedWeightParse(Scanner_p in, OCB_p ocb, ProofState_p
            state);

double ClauseRefinedWeightCompute(void* data, Clause_p clause);

WFCB_p ClauseRefinedWeight2Parse(Scanner_p in, OCB_p ocb, ProofState_p
            state);

double ClauseRefinedWeight2Compute(void* data, Clause_p clause);

void   ClauseRefinedWeightExit(void* data);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





