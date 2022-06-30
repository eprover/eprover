/*-----------------------------------------------------------------------

  File  : che_dagweight.h

Author: Stephan Schulz

Contents

  Evaluation of a clause by DAG weight (i.e. counting multiple
  occurrences of subterms only once).

  Copyright 2021 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Thu Apr  8 09:21:56 CEST 2021

-----------------------------------------------------------------------*/

#ifndef CHE_DAGWEIGHT

#define CHE_DAGWEIGHT

#include <che_clauseweight.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

#define DEFAULT_DAG_DUP_WEIGHT 1

typedef struct dagweightparamcell
{
   double pos_multiplier;
   long   vweight;
   long   fweight;
   long   dup_weight;    /* Weight used for repeated subterm - set to
                            0 for perfect DAG weight */
   bool   pos_use_dag;
   bool   pos_term_reset;
   bool   pos_eqn_reset;
   bool   neg_use_dag;
   bool   neg_term_reset;
   bool   neg_eqn_reset;
   bool   pos_neg_reset;
}DAGWeightParamCell, *DAGWeightParam_p;

/* There are many ways to consider the terms in a clause as a dag:
   - Each term in a literal is considered as an independent dag.
   - Each pair of terms in a literal is considered as one indepdendent
     dag (with two "pseudo-roots").
   - All terms in a set of literals are considered as one independent
     dag.

   We support starting a new DAG after each term, each literal
   (separately for positive and negative literals), all positive
   literals,  or never.
*/


/* Simplified with respect to DAG, but allowing for different weights
 * for maximal terms. Also reused for Twee-style RDAGWeight2 and
 * extended for RDAGWeight3 */

typedef struct RDAGWeightparamcell
{
   OCB_p  ocb;
   long   vweight;
   long   fweight;
   long   nvweight;
   long   nfweight;
   long   dup_weight;
   double uniqmax_term_multiplier;
   double max_term_multiplier;
   double max_literal_multiplier;
   double pos_multiplier;
   double pneq_multiplier; /* bonus/malus for non-equational pos literals */
   double nneq_multiplier; /* bonus/malus for non-equational neg literals */
}RDAGWeightParamCell, *RDAGWeightParam_p;



/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


#define DAGWeightParamCellAlloc() (DAGWeightParamCell*) \
        SizeMalloc(sizeof(DAGWeightParamCell))
#define DAGWeightParamCellFree(junk) \
        SizeFree(junk, sizeof(DAGWeightParamCell))


#define RDAGWeightParamCellAlloc() (RDAGWeightParamCell*) \
        SizeMalloc(sizeof(RDAGWeightParamCell))
#define RDAGWeightParamCellFree(junk) \
        SizeFree(junk, sizeof(RDAGWeightParamCell))


WFCB_p DAGWeightInit(ClausePrioFun prio_fun, int fweight, int
                     vweight, double pos_multiplier, long dup_weight,
                     bool pos_use_dag, bool pos_term_reset,
                     bool pos_eqn_reset, bool neg_use_dag,
                     bool neg_term_reset, bool neg_eqn_reset,
                     bool pos_neg_reset);

WFCB_p DAGWeightParse(Scanner_p in, OCB_p ocb, ProofState_p state);

double DAGWeightCompute(void* data, Clause_p clause);

void DAGWeightExit(void* data);


WFCB_p RDAGWeightInit(ClausePrioFun prio_fun,
                      OCB_p ocb,
                      long fweight,
                      long vweight,
                      long dup_weight,
                      double uniqmax_term_multiplier,
                      double max_term_multiplier,
                      double max_literal_multiplier,
                      double pos_multiplier);

WFCB_p RDAGWeightParse(Scanner_p in, OCB_p ocb, ProofState_p state);

double RDAGWeightCompute(void* data, Clause_p clause);

void RDAGWeightExit(void* data);


WFCB_p RDAGWeight2Init(ClausePrioFun prio_fun,
                       OCB_p ocb,
                       long fweight,
                       long vweight,
                       long dup_weight,
                       double max_term_multiplier,
                       double pos_multiplier);

WFCB_p RDAGWeight2Parse(Scanner_p in, OCB_p ocb, ProofState_p state);

double RDAGWeight2Compute(void* data, Clause_p clause);

WFCB_p RDAGWeight3Init(ClausePrioFun prio_fun,
                       OCB_p ocb,
                       long fweight,
                       long vweight,
                       long nfweight,
                       long nvweight,
                       long dup_weight,
                       double max_term_multiplier,
                       double pos_multiplier,
                       double pneq_multiplier,
                       double nneq_multiplier);

WFCB_p RDAGWeight3Parse(Scanner_p in, OCB_p ocb, ProofState_p state);

double RDAGWeight3Compute(void* data, Clause_p clause);



#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
