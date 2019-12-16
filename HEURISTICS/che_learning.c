/*-----------------------------------------------------------------------

  File  : che_learning.c

  Author: Stephan Schulz

  Contents

  TSM-based learning search heuristics

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Mon Aug 30 20:31:22 MET DST 1999

  -----------------------------------------------------------------------*/

#include "che_learning.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: tsm_param_init()
//
//   Return an initialized TSMParaCell.
//
// Global Variables: -
//
// Side Effects    : Memory allocation.
//
/----------------------------------------------------------------------*/

static TSMParam_p tsm_param_init(ClausePrioFun prio_fun, int fweight,
                                 int vweight, bool flat_clauses, double
                                 learnweight, char* kb, ProofState_p state, long
                                 sel_no, double set_part, double dist_part,
                                 IndexType indextype, TSMType tsmtype, long depth,
                                 double proofs_w, double dist_w, double p_simp_w,
                                 double f_simp_w, double p_gen_w,
                                 double f_gen_w)
{
   TSMParam_p local = TSMParamCellAlloc();
   int        i;
   double     pos_sum = 0, neg_sum = 0;

   local->fweight      = fweight;
   local->vweight      = vweight;
   local->flat_clauses = flat_clauses;
   local->learnweight  = learnweight;
   local->kb           = SecureStrdup(kb);
   local->state        = state;
   local->sel_no       = sel_no;
   local->set_part     = set_part;
   local->dist_part    = dist_part;
   local->indextype    = indextype;
   local->tsmtype      = tsmtype;
   local->depth        = depth;
   local->e_weights[0] = proofs_w;
   local->e_weights[1] = dist_w;
   local->e_weights[2] = p_simp_w;
   local->e_weights[3] = f_simp_w;
   local->e_weights[4] = p_gen_w;
   local->e_weights[5] = f_gen_w;
   for(i=0; i<(ANNOTATION_DEFAULT_SIZE-1); i++)
   {
      if(local->e_weights[i] > 0)
      {
         pos_sum+=local->e_weights[i];
      }
      else
      {
         neg_sum+=local->e_weights[i];
      }
   }
   local->eval_base = neg_sum;
   local->eval_scale = (pos_sum-neg_sum)==0 ? 1 : pos_sum-neg_sum;
   local->tsmadmin = NULL;
   local->pat_subst = NULL;

   return local;
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: TSMWeightInit()
//
//   Initialize a TSM-based evaluation function.
//
// Global Variables: -
//
// Side Effects    : Memory operations, I/O, may be expensive
//
/----------------------------------------------------------------------*/

WFCB_p TSMWeightInit(ClausePrioFun prio_fun, int fweight,
                     int vweight, bool flat_clauses, double
                     learnweight, char* kb, ProofState_p state, long
                     sel_no, double set_part, double dist_part,
                     IndexType indextype, TSMType tsmtype, long depth,
                     double proofs_w, double dist_w, double p_simp_w,
                     double f_simp_w, double p_gen_w,
                     double f_gen_w)
{
   TSMParam_p local = tsm_param_init(prio_fun, fweight, vweight,
                                     flat_clauses, learnweight, kb,
                                     state, sel_no, set_part,
                                     dist_part, indextype,
                                     (TSMType)tsmtype, depth,
                                     proofs_w, dist_w, p_simp_w,
                                     f_simp_w, p_gen_w, f_gen_w);

   return WFCBAlloc(TSMWeightCompute, prio_fun,
                    TSMWeightExit, local);
}


/*-----------------------------------------------------------------------
//
// Function: TSMWeightParse()
//
//   Parse a TSMWeight-definition. The format is:
//
//   TSMWeight(prio_fun, fweight, vweight, learnweight, flat|rec,
//   <kb-name>, max_proof_examples, max_proof_parts, max_dist_part,
//   tsmtype, indextype, indexdepth, proofs_w, dist_w, p_simp_w,
//   f_simp_w, p_gen_w, f_gen_w)
//
// Global Variables: -
//
// Side Effects    : Memory operations, Input
//
/----------------------------------------------------------------------*/

WFCB_p TSMWeightParse(Scanner_p in, OCB_p ocb, ProofState_p state)
{
   ClausePrioFun prio_fun;
   int fweight, vweight;
   bool flat_clauses = false;
   double learnweight;
   char* kb;
   long sel_no;
   double set_part, dist_part;
   int indextype;
   int tsmtype;
   long indexdepth;
   double proofs_w, dist_w, p_simp_w, f_simp_w, p_gen_w, f_gen_w;
   WFCB_p res;

   AcceptInpTok(in, OpenBracket);
   prio_fun = ParsePrioFun(in);
   AcceptInpTok(in, Comma);
   fweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   vweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   learnweight = ParseFloat(in);
   AcceptInpTok(in, Comma);
   CheckInpId(in, "rec|flat");
   if(TestInpId(in, "flat"))
   {
      flat_clauses = true;
   }
   NextToken(in);
   AcceptInpTok(in, Comma);
   kb = ParseFilename(in);
   AcceptInpTok(in, Comma);
   sel_no = ParseInt(in);
   AcceptInpTok(in, Comma);
   set_part =  ParseFloat(in);
   if((set_part<0.0) || (set_part>1))
   {
      AktTokenError(in,
                    "You need to specify the part of the knowledge"
                    " base to be used as a fraction between 0.0 and"
                    " 1.0!", false);
   }
   AcceptInpTok(in, Comma);
   dist_part = ParseFloat(in);
   AcceptInpTok(in, Comma);
   CheckInpTok(in, Name);
   tsmtype = GetTSMType(DStrView(AktToken(in)->literal));
   if((tsmtype == TSMTypeNoType) || (tsmtype == -1))
   {
      AktTokenError(in, "No correct TSM type specified!", false);
   }
   NextToken(in);
   AcceptInpTok(in, Comma);
   CheckInpTok(in, Name);
   indextype = GetIndexType(DStrView(AktToken(in)->literal));
   if((indextype == (int)IndexNoIndex) ||
      (indextype == (int)IndexEmpty) || (indextype == -1))
   {
      AktTokenError(in, "No correct index type specified!", false);
   }
   NextToken(in);
   AcceptInpTok(in, Comma);
   indexdepth = ParseInt(in);
   AcceptInpTok(in, Comma);
   proofs_w =  ParseFloat(in);
   AcceptInpTok(in, Comma);
   dist_w =  ParseFloat(in);
   AcceptInpTok(in, Comma);
   p_simp_w  =  ParseFloat(in);
   AcceptInpTok(in, Comma);
   f_simp_w =  ParseFloat(in);
   AcceptInpTok(in, Comma);
   p_gen_w =  ParseFloat(in);
   AcceptInpTok(in, Comma);
   f_gen_w =  ParseFloat(in);
   AcceptInpTok(in, CloseBracket);

   res = TSMWeightInit(prio_fun, fweight, vweight, flat_clauses,
                       learnweight, kb, state, sel_no, set_part,
                       dist_part, indextype, (TSMType)tsmtype,
                       indexdepth, proofs_w, dist_w, p_simp_w,
                       f_simp_w, p_gen_w, f_gen_w);

   FREE(kb);
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: TSMWeightCompute()
//
//   Compute a TSM-based weight for a clause.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

double TSMWeightCompute(void* data, Clause_p clause)
{
   TSMParam_p local = data;
   Term_p clauserep;
   PStack_p listrep;
   double   factor, res;

   if(!local->tsmadmin)
   {
      local->tsmadmin = TSMFromKB(local->flat_clauses,
                                  local->e_weights,
                                  local->kb,
                                  local->state->terms->sig,
                                  local->state->axioms,
                                  local->sel_no, local->set_part,
                                  local->dist_part,
                                  local->indextype, local->tsmtype,
                                  local->depth);
      local->pat_subst = PatternDefaultSubstAlloc(local->state->terms->sig);
      /* TSMPrintRek(stdout, local->tsmadmin, local->tsmadmin->tsm,
         0);*/
   }

   listrep = PStackAlloc();
   PatternSubstBacktrack(local->pat_subst, 0); /* Make sure everything
                                                  is at 0 */
   if(PatternClauseCompute(clause, &(local->pat_subst), &listrep))
   {
      clauserep = local->flat_clauses?
         FlatEncodeClauseListRep(local->state->terms, listrep):
         RecEncodeClauseListRep(local->state->terms, listrep);

      factor = TSMEvalTerm(local->tsmadmin, clauserep,
                           local->pat_subst);

      factor = factor - local->eval_base;
      factor = factor / local->eval_scale;

      /* printf("Factor: %f -- ", factor);
         ClausePrint(stdout, clause, 1);
         printf(" <=> ");
         PatternTermPrint(stdout, local->pat_subst, clauserep,
         local->state->terms->sig);
         printf("\n");*/
      /* TBDelete(local->state->terms, clauserep); */
      /* factor = (factor == 1)?1:0; */
   }
   else
   {
      factor = local->tsmadmin->limit;
      factor = factor - local->eval_base;
      factor = factor / local->eval_scale;
      /* printf("Default: %f -- ", factor);
         ClausePrint(stdout, clause, 1);
         printf("\n");*/
   }
   PStackFree(listrep);
   res = ((local->learnweight*factor)+1)*
      ClauseWeight(clause, 1, 1, 1, local->vweight,
                   local->fweight, 1, false);
   /* printf(" Eval: %f\n", res); */
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: TSMRWeightInit()
//
//   Initialize a TSM-based refined evaluation function.
//
// Global Variables: -
//
// Side Effects    : Memory operations, I/O, may be expensive
//
/----------------------------------------------------------------------*/

WFCB_p TSMRWeightInit(ClausePrioFun prio_fun, int fweight,
                      int vweight, double max_term_multiplier, double
                      max_literal_multiplier, double pos_multiplier,
                      bool flat_clauses, double
                      learnweight, char* kb, ProofState_p state, long
                      sel_no, double set_part, double dist_part,
                      IndexType indextype, TSMType tsmtype, long depth,
                      double proofs_w, double dist_w,  double p_simp_w,
                      double f_simp_w, double p_gen_w, double f_gen_w)
{
   TSMParam_p local = tsm_param_init(prio_fun, fweight, vweight,
                                     flat_clauses, learnweight, kb,
                                     state, sel_no, set_part,
                                     dist_part, indextype,
                                     (TSMType)tsmtype, depth,
                                     proofs_w, dist_w, p_simp_w,
                                     f_simp_w, p_gen_w, f_gen_w);

   local->pos_multiplier        = pos_multiplier;
   local->max_term_multiplier   = max_term_multiplier;
   local->max_literal_multiplier= max_literal_multiplier;

   return WFCBAlloc(TSMRWeightCompute, prio_fun,
                    TSMWeightExit, local);
}


/*-----------------------------------------------------------------------
//
// Function: TSMRWeightParse()
//
//   Parse a refine TSMWeight-definition. The format is:
//
//   TSMWeight(prio_fun, fweight, vweight, max_term, max_lit, pos_lit,
//   learnweight, flat|rec,
//   <kb-name>, max_proof_examples, max_proof_parts, max_dist_part,
//   tsmtype, indextype, indexdepth, proofs_w, dist_w, p_simp_w,
//   f_simp_w, p_gen_w, f_gen_w, subsum_w)
//
// Global Variables: -
//
// Side Effects    : Memory operations, Input
//
/----------------------------------------------------------------------*/

WFCB_p TSMRWeightParse(Scanner_p in, OCB_p ocb, ProofState_p state)
{
   ClausePrioFun prio_fun;
   int fweight, vweight;
   double pos_multiplier, max_term_multiplier, max_literal_multiplier;
   bool flat_clauses = false;
   double learnweight;
   char* kb;
   long sel_no;
   double set_part, dist_part;
   int indextype;
   int tsmtype;
   long indexdepth;
   double proofs_w, dist_w, p_simp_w, f_simp_w, p_gen_w, f_gen_w;
   WFCB_p res;

   AcceptInpTok(in, OpenBracket);
   prio_fun = ParsePrioFun(in);
   AcceptInpTok(in, Comma);
   fweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   vweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   max_term_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   max_literal_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   pos_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   learnweight = ParseFloat(in);
   AcceptInpTok(in, Comma);
   CheckInpId(in, "rec|flat");
   if(TestInpId(in, "flat"))
   {
      flat_clauses = true;
   }
   NextToken(in);
   AcceptInpTok(in, Comma);
   kb = ParseFilename(in);
   AcceptInpTok(in, Comma);
   sel_no = ParseInt(in);
   AcceptInpTok(in, Comma);
   set_part =  ParseFloat(in);
   if((set_part<0.0) || (set_part>1))
   {
      AktTokenError(in,
                    "You need to specify the part of the knowledge"
                    " base to be used as a fraction between 0.0 and"
                    " 1.0!", false);
   }
   AcceptInpTok(in, Comma);
   dist_part = ParseFloat(in);
   AcceptInpTok(in, Comma);
   CheckInpTok(in, Name);
   tsmtype = GetTSMType(DStrView(AktToken(in)->literal));
   if((tsmtype == TSMTypeNoType) || (tsmtype == -1))
   {
      AktTokenError(in, "No correct TSM type specified!", false);
   }
   NextToken(in);
   AcceptInpTok(in, Comma);
   CheckInpTok(in, Name);
   indextype = GetIndexType(DStrView(AktToken(in)->literal));
   if((indextype == (int)IndexNoIndex) ||
      (indextype == (int)IndexEmpty) || (indextype == -1))
   {
      AktTokenError(in, "No correct index type specified!", false);
   }
   NextToken(in);
   AcceptInpTok(in, Comma);
   indexdepth = ParseInt(in);
   AcceptInpTok(in, Comma);
   proofs_w =  ParseFloat(in);
   AcceptInpTok(in, Comma);
   dist_w =  ParseFloat(in);
   AcceptInpTok(in, Comma);
   p_simp_w  =  ParseFloat(in);
   AcceptInpTok(in, Comma);
   f_simp_w =  ParseFloat(in);
   AcceptInpTok(in, Comma);
   p_gen_w =  ParseFloat(in);
   AcceptInpTok(in, Comma);
   f_gen_w =  ParseFloat(in);
   AcceptInpTok(in, CloseBracket);

   res = TSMRWeightInit(prio_fun, fweight, vweight, max_term_multiplier,
                        max_literal_multiplier,
                        pos_multiplier,flat_clauses, learnweight, kb,
                        state, sel_no, set_part,
                        dist_part, indextype, (TSMType)tsmtype,
                        indexdepth, proofs_w, dist_w, p_simp_w,
                        f_simp_w, p_gen_w, f_gen_w);

   FREE(kb);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: TSMRWeightCompute()
//
//   Compute a TSM-based weight for a clause.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

double TSMRWeightCompute(void* data, Clause_p clause)
{
   TSMParam_p local = data;
   Term_p clauserep;
   PStack_p listrep;
   double   factor, res;

   if(!local->tsmadmin)
   {
      local->tsmadmin = TSMFromKB(local->flat_clauses,
                                  local->e_weights,
                                  local->kb,
                                  local->state->terms->sig,
                                  local->state->axioms,
                                  local->sel_no, local->set_part,
                                  local->dist_part,
                                  local->indextype, local->tsmtype,
                                  local->depth);
      local->pat_subst = PatternDefaultSubstAlloc(local->state->terms->sig);
      /* TSMPrintRek(stdout, local->tsmadmin, local->tsmadmin->tsm,
         0);*/
   }

   listrep = PStackAlloc();
   PatternSubstBacktrack(local->pat_subst, 0); /* Make sure everything
                                                  is at 0 */
   if(PatternClauseCompute(clause, &(local->pat_subst), &listrep))
   {
      clauserep = local->flat_clauses?
         FlatEncodeClauseListRep(local->state->terms, listrep):
         RecEncodeClauseListRep(local->state->terms, listrep);

      factor = TSMEvalTerm(local->tsmadmin, clauserep,
                           local->pat_subst);

      factor = factor - local->eval_base;
      factor = factor / local->eval_scale;

      /* printf("Factor: %f -- ", factor);
         ClausePrint(stdout, clause, 1);
         printf(" <=> ");
         PatternTermPrint(stdout, local->pat_subst, clauserep,
         local->state->terms->sig);
         printf("\n");*/
      /* TBDelete(local->state->terms, clauserep); */
      /* factor = (factor == 1)?1:0; */
   }
   else
   {
      factor = local->tsmadmin->limit;
      factor = factor - local->eval_base;
      factor = factor / local->eval_scale;
      /* printf("Default: %f -- ", factor);
         ClausePrint(stdout, clause, 1);
         printf("\n");*/
   }
   PStackFree(listrep);
   res = ((local->learnweight*factor)+1)*
      ClauseWeight(clause, local->max_term_multiplier,
                   local->max_literal_multiplier,
                   local->pos_multiplier,
                   local->vweight,
                   local->fweight,
                   1,
                   false);
   /* printf(" Eval: %f\n", res); */
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: TSMWeightExit()
//
//   Free a TSMParamCell.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void TSMWeightExit(void* data)
{
   TSMParam_p local = data;

   if(local->tsmadmin)
   {
      PatternSubstFree(local->tsmadmin->subst);
      PatternSubstFree(local->pat_subst);
      TSMAdminFree(local->tsmadmin);
   }
   FREE(local->kb);
   TSMParamCellFree(local);
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
