/*-----------------------------------------------------------------------

  File  : che_diversityweight.h

  Author: Stephan Schulz

  Contents

  Evaluation of a clause by refined diversity clause weight, using
  weight penalty factors for maximal terms and literals, and penalties
  for clauses with many different function symbols and variables.

  Copyright 2019 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Mon May 27 00:55:33 CEST 2019

-----------------------------------------------------------------------*/

#ifndef CHE_DIVERSITYWEIGHT

#define CHE_DIVERSITYWEIGHT

#include <che_clauseweight.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

#define DEFAULT_MAX_MULT 1.5

typedef struct diversityweightparamcell
{
   OCB_p  ocb;
   double max_term_multiplier;
   double max_literal_multiplier;
   double pos_multiplier;
   double app_var_mult;
   long   vweight;
   long   fweight;
   double fdiff1weight;
   double fdiff2weight;
   double vdiff1weight;
   double vdiff2weight;
}DiversityWeightParamCell, *DiversityWeightParam_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


#define DiversityWeightParamCellAlloc() (DiversityWeightParamCell*) \
        SizeMalloc(sizeof(DiversityWeightParamCell))
#define DiversityWeightParamCellFree(junk) \
        SizeFree(junk, sizeof(DiversityWeightParamCell))

WFCB_p DiversityWeightInit(ClausePrioFun prio_fun, int fweight,
                           int vweight, OCB_p ocb,
                           double max_term_multiplier,
                           double max_literal_multiplier,
                           double pos_multiplier,
                           double fdiff1weight,
                           double fdiff2weight,
                           double vdiff1weight,
                           double vdiff2weight,
                           double app_var_mult);

WFCB_p DiversityWeightParse(Scanner_p in, OCB_p ocb, ProofState_p
                            state);

double DiversityWeightCompute(void* data, Clause_p clause);

void   DiversityWeightExit(void* data);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
