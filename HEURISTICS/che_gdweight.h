/*-----------------------------------------------------------------------

File  : che_gdweight.h

Author: Stephan Schulz

Contents

  Evaluation of a clause by E's version of TWEE-inspired goal-direced
  weight. Conjecture ground terms get a lower (better) weight here.

  Copyright 2026 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Wed May 20 08:44:52 CEST 2026

-----------------------------------------------------------------------*/

#ifndef CHE_GDWEIGHT

#define CHE_GDWEIGHT

#include <che_wfcb.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

#define DEFAULT_POS_MULT 1

typedef struct gd_weightparamcell
{
   double pos_multiplier;
   double app_var_mult;
   long   vweight;
   long   fweight;
   double goal_multiplier;
   long   goal_const;
   bool   goal_terms_initialized;
   ClauseSet_p axioms;
}GDWeightParamCell, *GDWeightParam_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


#define GDWeightParamCellAlloc() (GDWeightParamCell*) \
        SizeMalloc(sizeof(GDWeightParamCell))
#define GDWeightParamCellFree(junk) \
        SizeFree(junk, sizeof(GDWeightParamCell))

WFCB_p GDClauseWeightInit(ClausePrioFun prio_fun,
                          ClauseSet_p axioms,
                          int fweight, int vweight,
                          double pos_multiplier,
                          double goal_multiplier,
                          long goal_const, double app_var_mult);


WFCB_p GDClauseWeightParse(Scanner_p in, OCB_p ocb, ProofState_p state);

double GDClauseWeightCompute(void* data, Clause_p clause);

void   GDClauseWeightExit(void* data);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
