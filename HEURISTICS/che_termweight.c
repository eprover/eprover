/*-----------------------------------------------------------------------

File  : che_termweight.c

Author: could be anyone

Contents
 
  Auto generated. Your comment goes here ;-).

  Copyright 2016 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Tue Mar  8 22:40:31 CET 2016
    New

-----------------------------------------------------------------------*/

#include "che_termweight.h"


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

static Term_p termweight_insert(
   TB_p bank, Term_p term, VarNormStyle var_norm)
{
   Term_p copy;
   Term_p repr;
   
   copy = TermCopyNormalizeVars(bank->vars,term,var_norm);
   repr = TBInsert(bank,copy,DEREF_NEVER); 
   TermCellSetProp(repr,TPTopPos);
   TermFree(copy);

   return repr;
}

static void termweight_insert_subterms(
   TB_p bank, Term_p term, VarNormStyle var_norm)
{
   int i;
   PStack_p stack;
   Term_p subterm;

   stack = PStackAlloc();

   PStackPushP(stack, term);
   while (!PStackEmpty(stack))
   {
      subterm = PStackPopP(stack);
      if(TermIsVar(subterm)) {
         continue;
      }
      termweight_insert(bank,subterm,var_norm);
      for(i=0; i<subterm->arity; i++)
      {
         PStackPushP(stack, subterm->args[i]);
      }
   }

   PStackFree(stack);
}

static void termweight_insert_topgens(
   TB_p bank, Term_p term, VarNormStyle var_norm)
{
   int i;
   PStack_p topgens;
   Term_p topgen;

   topgens = ComputeTopGeneralizations(term,bank->vars,bank->sig);
   for (i=0; i<topgens->current; i++) {
      topgen = topgens->stack[i].p_val;
      termweight_insert(bank,topgen,var_norm);
   }
   FreeGeneralizations(topgens);
}

static void termweight_insert_subgens(
   TB_p bank, Term_p term, VarNormStyle var_norm)
{
   int i;
   PStack_p subgens;
   Term_p genterm;

   subgens = ComputeSubtermsGeneralizations(term,bank->vars);
   for (i=0; i<subgens->current; i++) {
      genterm = subgens->stack[i].p_val;
      termweight_insert(bank,genterm,var_norm);
   }
   FreeGeneralizations(subgens);
}

static void termweight_init(TermWeightParam_p data)
{
   Clause_p clause;
   Clause_p anchor;

   if (data->eval_bank) 
   {
      return;
   }

   data->eval_bank = TBAlloc(data->ocb->sig);

   anchor = data->proofstate->axioms->anchor;
   for (clause=anchor->succ; clause!=anchor; clause=clause->succ)
   {
      if(ClauseQueryTPTPType(clause)!=CPTypeNegConjecture) {
         continue;
      }
      TBInsertClauseTermsNormalized(
         data->eval_bank,clause,data->var_norm,data->rel_terms);
   }
   
   TBCountTermFreqs(data->eval_bank);

#ifdef DEBUG_TERMWEIGHTS
   TBDotBankFile("eval-init.dot", data->eval_bank);
#endif
}

static void termweight_update_conjecture_freqs(
   TB_p bank, Term_p term, VarNormStyle var_norm)
{
   int i;
   PStack_p stack;
   Term_p subterm;
   Term_p subnorm;
   Term_p subrepr;

   stack = PStackAlloc();

   PStackPushP(stack, term);
   while (!PStackEmpty(stack))
   {
      subterm = PStackPopP(stack);
      if(TermIsVar(subterm)) 
      {
         continue;
      }
      subnorm = TermCopyNormalizeVars(bank->vars,subterm,var_norm);
      subrepr = TBFindRec(bank, subnorm);
      if (subrepr && (subrepr->freq>0)) {
         subterm->freq = subrepr->freq;
      }
      TermFree(subnorm);

      for(i=0; i<subterm->arity; i++)
      {
         PStackPushP(stack, subterm->args[i]);
      }
   }

   PStackFree(stack);
}

static double termweight_term_weight(Term_p term, TermWeightParam_p data)
{
   Term_p repr;

   repr = termweight_insert(data->eval_bank, term, data->var_norm);
   termweight_update_conjecture_freqs(data->eval_bank, repr, data->var_norm);

   if (TermIsVar(repr))
   {
      return data->vweight;
   }
   if (TermIsConst(repr)) 
   {
      return (repr->freq>0)?data->conj_fweight:data->cweight;
   }
   if (TermCellQueryProp(repr,TPPredPos))
   {
      return (repr->freq>0)?data->conj_pweight:data->pweight;
   }
   else
   {
      return (repr->freq>0)?data->conj_fweight:data->fweight;
   }
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

TermWeightParam_p TermWeightParamAlloc(void)
{
   TermWeightParam_p res = TermWeightParamCellAlloc();

   res->eval_bank = NULL;

   return res;
}

void TermWeightParamFree(TermWeightParam_p junk)
{
   if (junk->eval_bank) 
   {
      TBFree(junk->eval_bank);
      junk->eval_bank = NULL;
   }
   TermWeightParamCellFree(junk);
}
 
WFCB_p ConjectureRelativeTermWeightParse(
   Scanner_p in,  
   OCB_p ocb, 
   ProofState_p state)
{   
   ClausePrioFun prio_fun;
   double pos_multiplier, max_term_multiplier, max_literal_multiplier;
   int fweight, pweight, cweight, vweight;
   double conj_multiplier;
   TermWeightExtenstionStyle ext_style;
   VarNormStyle var_norm;
   RelatedTermSet rel_terms;

   AcceptInpTok(in, OpenBracket);
   prio_fun = ParsePrioFun(in);
   AcceptInpTok(in, Comma);

   var_norm = (VarNormStyle)ParseInt(in);
   AcceptInpTok(in, Comma);
   rel_terms = (RelatedTermSet)ParseInt(in);
   AcceptInpTok(in, Comma);

   conj_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   fweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   cweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   pweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   vweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   
   ext_style = (TermWeightExtenstionStyle)ParseInt(in);
   AcceptInpTok(in, Comma);
   max_term_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   max_literal_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   pos_multiplier = ParseFloat(in);
   AcceptInpTok(in, CloseBracket);
   
   return ConjectureRelativeTermWeightInit(
      prio_fun, 
      ocb,
      state,
      var_norm,
      rel_terms,
      vweight,
      fweight,
      cweight,
      pweight,
      conj_multiplier*fweight,
      conj_multiplier*cweight,
      conj_multiplier*pweight,
      ext_style,
      max_term_multiplier,
      max_literal_multiplier,
      pos_multiplier);
}

WFCB_p ConjectureRelativeTermWeightInit(
   ClausePrioFun prio_fun, 
   OCB_p ocb,
   ProofState_p proofstate,
   VarNormStyle var_norm,
   RelatedTermSet rel_terms,
   long vweight,
   long fweight,
   long cweight,
   long pweight,
   long conj_fweight,
   long conj_cweight,
   long conj_pweight,
   TermWeightExtenstionStyle ext_style,
   double max_term_multiplier,
   double max_literal_multiplier,
   double pos_multiplier)
{
   TermWeightParam_p data = TermWeightParamAlloc();

   data->init_fun   = termweight_init;
   data->ocb        = ocb;
   data->proofstate = proofstate;
   data->var_norm   = var_norm;
   data->rel_terms  = rel_terms;
   
   data->vweight      = vweight;
   data->fweight      = fweight;
   data->cweight      = cweight;
   data->pweight      = pweight;
   data->conj_fweight = conj_fweight;
   data->conj_cweight = conj_cweight;
   data->conj_pweight = conj_pweight;

   data->twe = TermWeightExtensionAlloc(
      max_term_multiplier,
      max_literal_multiplier,
      pos_multiplier,
      ext_style,
      (TermWeightFun)termweight_term_weight,
      data);
   
   return WFCBAlloc(
      ConjectureRelativeTermWeightCompute, 
      prio_fun,
      ConjectureRelativeTermWeightExit, 
      data);
}

double ConjectureRelativeTermWeightCompute(void* data, Clause_p clause)
{
   TermWeightParam_p local;
   double res;
   
   local = data;
   local->init_fun(data);

   ClauseCondMarkMaximalTerms(local->ocb, clause);
   res = ClauseTermExtWeight(clause, local->twe);

#ifdef DEBUG_TERMWEIGHTS
   static long cnt = 0;
   char dot[64];
   sprintf(dot, "eval-tb-%06ld.dot", cnt++);
   TBDotBankFile(dot, local->eval_bank);
   
   fprintf(GlobalOut, "=%.2f: ", res);
   ClausePrint(GlobalOut, clause, true);
   fprintf(GlobalOut, "\n");
#endif

   return res;
}

void ConjectureRelativeTermWeightExit(void* data)
{
   TermWeightParam_p junk = data;
   
   TermWeightParamFree(junk);
}

void TBInsertClauseTermsNormalized(
   TB_p bank, 
   Clause_p clause, 
   VarNormStyle var_norm, 
   RelatedTermSet rel_terms)
{
   Eqn_p lit;

   for (lit=clause->literals; lit; lit=lit->next)
   {
      switch (rel_terms) {
         case RTSConjectureTerms:
            termweight_insert(bank,lit->lterm,var_norm);
            termweight_insert(bank,lit->rterm,var_norm);
            break;
         case RTSConjectureSubterms:
            termweight_insert_subterms(bank,lit->lterm,var_norm);
            termweight_insert_subterms(bank,lit->rterm,var_norm);
            break;
         case RTSConjectureSubtermsTopGens:
            termweight_insert_subterms(bank,lit->lterm,var_norm);
            termweight_insert_subterms(bank,lit->rterm,var_norm);
            termweight_insert_topgens(bank,lit->lterm,var_norm);
            termweight_insert_topgens(bank,lit->rterm,var_norm);
            break;
         case RTSConjectureSubtermsAllGens:
            termweight_insert_subgens(bank,lit->lterm,var_norm);
            termweight_insert_subgens(bank,lit->rterm,var_norm);
            break;
         default:
            Error("TBInsertClauseNormalized: Parameters syntax error (unsupported RelatedTermSet %d)", USAGE_ERROR, rel_terms);
            break;
      }
   }
}

void TBIncSubtermsFreqs(Term_p term)
{
   int i;
   PStack_p stack;
   Term_p subterm;

   stack = PStackAlloc();

   PStackPushP(stack, term);
   while (!PStackEmpty(stack))
   {
      subterm = PStackPopP(stack);
      if (TermIsVar(subterm))
      {
         continue;
      }
         
      subterm->freq++;
      
      for(i=0; i<subterm->arity; i++)
      {
         PStackPushP(stack, subterm->args[i]);
      }
   }

   PStackFree(stack);
}

void TBCountTermFreqs(TB_p bank)
{
   PStack_p stack = PStackAlloc();
   Term_p term;
   int i;

   for(i=0; i<TERM_STORE_HASH_SIZE; i++)
   {      
      PStackPushP(stack, bank->term_store.store[i]);
      while(!PStackEmpty(stack))
      {
         term = PStackPopP(stack);
         if ((!term) || (!TermCellQueryProp(term,TPTopPos)))
         {
            continue;
         }
         TBIncSubtermsFreqs(term);
         if(term)
         {
            PStackPushP(stack, term->lson);
            PStackPushP(stack, term->rson);
         }
      }
   }
   PStackFree(stack);
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/

