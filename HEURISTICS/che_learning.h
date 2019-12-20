/*-----------------------------------------------------------------------

  File  : che_learning.h

  Author: Stephan Schulz

  Contents

  Evaluation of a clause by tsm-based learning algorithms

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Mon Aug 30 19:17:53 MET DST 1999

  -----------------------------------------------------------------------*/

#ifndef CHE_LEARNING

#define CHE_LEARNING

#include <che_wfcb.h>
#include <cle_tsmio.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

#define DEFAULT_POS_MULT 1

/* As it is fairly expensive to create a suitable TSM (about 10-40
   seconds) we only want to create it when we really need to use
   it. Therefore we hack this as follows: All relevant data is stored
   in the parameter cell, and the TSM is created if the
   evaluation function is called for the first time. */

typedef struct tsmparamcell
{
   long           fweight;
   long           vweight;
   double         max_term_multiplier;    /* For TSWRWeight only */
   double         max_literal_multiplier; /* For TSWRWeight only */
   double         pos_multiplier;         /* For TSWRWeight only */
   bool           flat_clauses;
   double         learnweight;
   char*          kb;
   ProofState_p   state;
   long           sel_no;
   double         set_part;
   double         dist_part;
   IndexType      indextype;
   TSMType        tsmtype;
   long           depth;
   double         e_weights[ANNOTATION_DEFAULT_SIZE-1];
   double         eval_base;
   double         eval_scale;
   PatternSubst_p pat_subst;
   TSMAdmin_p     tsmadmin;
}TSMParamCell, *TSMParam_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


#define TSMParamCellAlloc() (TSMParamCell*)     \
   SizeMalloc(sizeof(TSMParamCell))
#define TSMParamCellFree(junk)                  \
   SizeFree(junk, sizeof(TSMParamCell))


WFCB_p TSMWeightInit(ClausePrioFun prio_fun, int fweight,
                     int vweight, bool flat_clauses, double
                     learnweight, char* kb, ProofState_p state, long
                     sel_no, double set_part, double dist_part,
                     IndexType indextype, TSMType tsmtype, long depth,
                     double proofs_w, double dist_w,  double p_simp_w,
                     double f_simp_w, double p_gen_w, double f_gen_w);

WFCB_p TSMWeightParse(Scanner_p in, OCB_p ocb, ProofState_p state);

double TSMWeightCompute(void* data, Clause_p clause);

WFCB_p TSMRWeightInit(ClausePrioFun prio_fun, int fweight,
                      int vweight, double max_term_multiplier, double
                      max_literal_multiplier, double pos_multiplier,
                      bool flat_clauses, double
                      learnweight, char* kb, ProofState_p state, long
                      sel_no, double set_part, double dist_part,
                      IndexType indextype, TSMType tsmtype, long depth,
                      double proofs_w, double dist_w,  double p_simp_w,
                      double f_simp_w, double p_gen_w, double f_gen_w);

WFCB_p TSMRWeightParse(Scanner_p in, OCB_p ocb, ProofState_p state);

double TSMRWeightCompute(void* data, Clause_p clause);

void   TSMWeightExit(void* data);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
