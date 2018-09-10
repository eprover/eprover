/*-----------------------------------------------------------------------

File  : che_orientweight.h

Author: Stephan Schulz

Contents

  Evaluation of a clause by orientable clause weight, using penalties
  for unorientable and maximal literals.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed Jun 17 00:11:03 MET DST 1998
    New

-----------------------------------------------------------------------*/

#ifndef CHE_ORIENTWEIGHT

#define CHE_ORIENTWEIGHT

#include <che_clauseweight.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

#define DEFAULT_MAX_MULT 1.5

typedef struct orientweightparamcell
{
   OCB_p  ocb;
   double unorientable_literal_multiplier;
   double max_literal_multiplier;
   double pos_multiplier;
   double app_var_mult;
   long   vweight;
   long   fweight;
}OrientWeightParamCell, *OrientWeightParam_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


#define OrientWeightParamCellAlloc() (OrientWeightParamCell*) \
        SizeMalloc(sizeof(OrientWeightParamCell))
#define OrientWeightParamCellFree(junk) \
        SizeFree(junk, sizeof(OrientWeightParamCell))

WFCB_p ClauseOrientWeightInit(ClausePrioFun prio_fun, int fweight,
                int vweight, OCB_p ocb, double
                unorientable_literal_multiplier, double
                max_literal_multiplier, double
                pos_multiplier, double app_var_mult);

WFCB_p ClauseOrientWeightParse(Scanner_p in, OCB_p ocb, ProofState_p
            state);

double ClauseOrientWeightCompute(void* data, Clause_p clause);

WFCB_p OrientLMaxWeightInit(ClausePrioFun prio_fun, int fweight,
             int vweight, OCB_p ocb, double
             unorientable_literal_multiplier, double
             max_literal_multiplier, double
             pos_multiplier, double app_var_mult);

WFCB_p OrientLMaxWeightParse(Scanner_p in, OCB_p ocb, ProofState_p
              state);

double OrientLMaxWeightCompute(void* data, Clause_p clause);

void   ClauseOrientWeightExit(void* data);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





