/*-----------------------------------------------------------------------

File  : che_simweight.h

Author: Stephan Schulz

Contents

  Evaluation of a clause by similarity of terms (equations with
  similar terms have low weight).

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sun Jun 28 18:18:00 MET DST 1998
    New

-----------------------------------------------------------------------*/

#ifndef CHE_SIMWEIGHT

#define CHE_SIMWEIGHT

#include <che_wfcb.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


typedef struct simparamcell
{
   double equal_weight;
   double var_var_clash;
   double var_term_clash;
   double term_term_clash;
   double app_var_mult;
}SimParamCell, *SimParam_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


#define SimParamCellAlloc() (SimParamCell*) \
        SizeMalloc(sizeof(SimParamCell))
#define SimParamCellFree(junk) \
        SizeFree(junk, sizeof(SimParamCell))

WFCB_p SimWeightInit(ClausePrioFun prio_fun, double equal_weight,
           double var_var_clash, double var_term_clash,
           double term_term_clash, double app_var_mult);

WFCB_p SimWeightParse(Scanner_p in, OCB_p ocb, ProofState_p state);

double SimWeightCompute(void* data, Clause_p clause);

void   SimWeightExit(void* data);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





