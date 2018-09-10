/*-----------------------------------------------------------------------

File  : che_varweights.h

Author: Stephan Schulz, schulz@eprover.org

Contents

  Weight functions that play around a bit ;-)

  Copyright 1998, 1999, 2005 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed Jun 17 00:11:03 MET DST 1998
    New

-----------------------------------------------------------------------*/

#ifndef CHE_VARWEIGHTS

#define CHE_VARWEIGHTS

#include <che_refinedweight.h>
#include <che_clausesetfeatures.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


typedef struct varweightparamcell
{
   OCB_p  ocb;
   double max_term_multiplier;
   double max_literal_multiplier;
   double pos_multiplier;
   double conjecture_multiplier;
   double hypothesis_multiplier;
   double sig_size_multiplier;
   double proof_size_multiplier;
   double proof_depth_multiplier;
   double term_weight_multiplier;
   double term_depth_multiplier;
   double weight_multiplier;
   double app_var_mult;
   long   vlweight;
   long   vweight;
   long   fweight;
   long   nvweight;
   long   nfweight;
   long   cweight;
   long   pweight;
   long   stagger_limit;
}VarWeightParamCell, *VarWeightParam_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


#define VarWeightParamCellAlloc() (VarWeightParamCell*) \
        SizeMalloc(sizeof(VarWeightParamCell))
#define VarWeightParamCellFree(junk) \
        SizeFree(junk, sizeof(VarWeightParamCell))

WFCB_p TPTPTypeWeightInit(ClausePrioFun prio_fun, int fweight,
           int vweight, OCB_p ocb, double
           max_term_multiplier, double
           max_literal_multiplier, double
           pos_multiplier, double
           conjecture_multiplier, double
           hypothesis_multiplier, double app_var_mult);
WFCB_p TPTPTypeWeightParse(Scanner_p in, OCB_p ocb, ProofState_p
            state);
double TPTPTypeWeightCompute(void* data, Clause_p clause);


WFCB_p SigWeightInit(ClausePrioFun prio_fun, int fweight,
           int vweight, OCB_p ocb, double
           max_term_multiplier, double
           max_literal_multiplier, double
           pos_multiplier, double sig_size_multiplier,
           double app_var_mult);
WFCB_p SigWeightParse(Scanner_p in, OCB_p ocb, ProofState_p
            state);
double SigWeightCompute(void* data, Clause_p clause);


WFCB_p ProofWeightInit(ClausePrioFun prio_fun, int fweight,
             int vweight, OCB_p ocb, double
             max_term_multiplier, double
             max_literal_multiplier, double
             pos_multiplier, double
             proof_size_multiplier, double
             proof_depth_multiplier, double
             app_var_mult);
WFCB_p ProofWeightParse(Scanner_p in, OCB_p ocb, ProofState_p
         state);
double ProofWeightCompute(void* data, Clause_p clause);


WFCB_p DepthWeightInit(ClausePrioFun prio_fun, int fweight,
             int vweight, OCB_p ocb, double
             max_term_multiplier, double
             max_literal_multiplier, double
             pos_multiplier, double term_weight_multiplier,
             double app_var_mult);
WFCB_p DepthWeightParse(Scanner_p in, OCB_p ocb, ProofState_p
            state);
double DepthWeightCompute(void* data, Clause_p clause);


WFCB_p WeightLessDepthInit(ClausePrioFun prio_fun, int fweight,
             int vweight, OCB_p ocb, double
             max_term_multiplier, double
             max_literal_multiplier, double
             pos_multiplier, double depth_weight_multiplier,
             double app_var_mult);
WFCB_p WeightLessDepthParse(Scanner_p in, OCB_p ocb, ProofState_p
            state);
double WeightLessDepthCompute(void* data, Clause_p clause);


WFCB_p NLWeightInit(ClausePrioFun prio_fun, int fweight,
          int vlweight, int vweight, OCB_p ocb, double
          max_term_multiplier, double
          max_literal_multiplier, double
           pos_multiplier, double app_var_mult);

WFCB_p NLWeightParse(Scanner_p in, OCB_p ocb, ProofState_p
            state);
double NLWeightCompute(void* data, Clause_p clause);

WFCB_p PNRefinedWeightInit(ClausePrioFun prio_fun, int fweight,
            int vweight, int nfweight, int nvweight,
            OCB_p ocb, double max_term_multiplier,
            double max_literal_multiplier, double
            pos_multiplier, double app_var_mult);
WFCB_p PNRefinedWeightParse(Scanner_p in, OCB_p ocb, ProofState_p
             state);
double PNRefinedWeightCompute(void* data, Clause_p clause);


WFCB_p SymTypeWeightInit(ClausePrioFun prio_fun, int fweight,
          int vweight, int cweight, int pweight, OCB_p
          ocb, double max_term_multiplier, double
          max_literal_multiplier, double
          pos_multiplier, double app_var_mult);

WFCB_p SymTypeWeightParse(Scanner_p in, OCB_p ocb, ProofState_p
           state);
double SymTypeWeightCompute(void* data, Clause_p clause);

WFCB_p ClauseWeightAgeInit(ClausePrioFun prio_fun, int fweight, int
            vweight, double pos_multiplier,
            double weight_multiplier, double app_var_mult);

WFCB_p ClauseWeightAgeParse(Scanner_p in, OCB_p ocb, ProofState_p state);

double ClauseWeightAgeCompute(void* data, Clause_p clause);


WFCB_p StaggeredWeightInit(ClausePrioFun prio_fun,
            double stagger_factor, ClauseSet_p axioms);

WFCB_p StaggeredWeightParse(Scanner_p in, OCB_p ocb, ProofState_p
             state);

double StaggeredWeightCompute(void* data, Clause_p clause);


void   VarWeightExit(void* data);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





