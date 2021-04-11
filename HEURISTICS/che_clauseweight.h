/*-----------------------------------------------------------------------

File  : che_clauseweight.h

Author: Stephan Schulz

Contents

  Evaluation of a clause by clause weight, also an example for setting
  up an evaluation function. Contains some additional evaluation
  functions as well.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sat Jul  5 02:28:25 MET DST 1997
    New

-----------------------------------------------------------------------*/

#ifndef CHE_CLAUSEWEIGHT

#define CHE_CLAUSEWEIGHT

#include <che_wfcb.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

#define DEFAULT_POS_MULT 1

typedef struct weightparamcell
{
   double pos_multiplier;
   double app_var_mult;
   long   vweight;
   long   fweight;
}WeightParamCell, *WeightParam_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


#define WeightParamCellAlloc() (WeightParamCell*) \
        SizeMalloc(sizeof(WeightParamCell))
#define WeightParamCellFree(junk) \
        SizeFree(junk, sizeof(WeightParamCell))

WFCB_p ClauseWeightInit(ClausePrioFun prio_fun, int fweight, int
         vweight, double pos_multiplier, double app_var_mult);

WFCB_p ClauseWeightParse(Scanner_p in, OCB_p ocb, ProofState_p state);

double ClauseWeightCompute(void* data, Clause_p clause);

void   ClauseWeightExit(void* data);

WFCB_p LMaxWeightInit(ClausePrioFun prio_fun, int fweight, int
             vweight, double pos_multiplier, double app_var_mult);

WFCB_p LMaxWeightParse(Scanner_p in, OCB_p ocb, ProofState_p state);

double LMaxWeightCompute(void* data, Clause_p clause);

WFCB_p CMaxWeightInit(ClausePrioFun prio_fun, int fweight, int
             vweight, double pos_multiplier, double app_var_mult);

WFCB_p CMaxWeightParse(Scanner_p in, OCB_p ocb, ProofState_p state);

double CMaxWeightCompute(void* data, Clause_p clause);

WFCB_p UniqWeightInit(ClausePrioFun prio_fun);

WFCB_p UniqWeightParse(Scanner_p in, OCB_p ocb, ProofState_p
         state);

double UniqWeightCompute(void* data, Clause_p clause);

WFCB_p DefaultWeightInit(ClausePrioFun prio_fun);

WFCB_p DefaultWeightParse(Scanner_p in, OCB_p ocb, ProofState_p
           state);

double DefaultWeightCompute(void* data, Clause_p clause);


void TrivialWeightExit(void* data);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
