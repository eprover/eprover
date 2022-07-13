/*-----------------------------------------------------------------------

  File  : che_termweight.c

  Author: Stephan Schulz, yan

  Contents
 
  Iplementation of conjecture subterm weight (Term) from [CICM'16/Sec.3].
  
  Syntax
   
  ConjectureRelativeTermWeight(
   prio,      // priority function
   varnorm,   // variable normalization:
              // 0: universal variable, 
              // 1: alhpa normalization
   relterm,   // related terms: 
              // 0: conjecture terms, 
              // 1: conjecture subterms, 
              // 2: conjecture subterms and top-level generalizations,
              // 3: conjecture subterms and subterm generalizations. 
   conj_mult, // conjecture multiplier (float)
   fweight,   // function symbol weight (int)
   cweight,   // constant symbol weight (int)
   pweight,   // predicate symbol weight (int)
   vweight,   // variable weight (int)
   ext_style, // term extension style:
              // 0: apply to literals and sum
              // 1: apply to all subterms and sum
              // 2: take the max of all subterms
   maxtm,     // maximal term multiplier (float)
   maxlm,     // maximal literal multiplier (float)
   poslm)     // positive literal multiplier (float)

  References

  [CICM'16]: Jan Jakubův and Josef Urban: "Extending E Prover with 
    Similarity Based Clause Selection Strategies", CICM, 2016.
    https://doi.org/10.1007/978-3-319-42547-4_11

  Copyright 1998-2018 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Wed Nov  7 21:37:27 CET 2018

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
      if(TermIsFreeVar(subterm)) {
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
   
   data->eval_freqs = TBCountTermFreqs(data->eval_bank);
}

static void termweight_update_conjecture_freqs(
   TB_p bank, NumTree_p* freqs, Term_p term, VarNormStyle var_norm)
{
   int i;
   PStack_p stack;
   Term_p subterm;
   Term_p subnorm;
   Term_p subrepr;
   NumTree_p cell;

   stack = PStackAlloc();

   PStackPushP(stack, term);
   while (!PStackEmpty(stack))
   {
      subterm = PStackPopP(stack);
      if (TermIsFreeVar(subterm)) 
      {
         continue;
      }
      subnorm = TermCopyNormalizeVars(bank->vars, subterm, var_norm);
      subrepr = TBFindRepr(bank, subnorm);
      if (subrepr) 
      {
         cell = NumTreeFind(freqs, subrepr->entry_no);
         if (cell && cell->val1.i_val > 0)
         {
            NumTreeStore(freqs, subterm->entry_no, cell->val1, cell->val2);
         }
      }

      //if (subrepr && (subrepr->freq>0)) 
      //{
      //   subterm->freq = subrepr->freq;
      //}
      TermFree(subnorm);

      for (i=0; i<subterm->arity; i++)
      {
         PStackPushP(stack, subterm->args[i]);
      }
   }

   PStackFree(stack);
}

static double termweight_term_weight(Term_p term, TermWeightParam_p data)
{
   Term_p repr;
   long freq;
   NumTree_p cell;

   repr = termweight_insert(data->eval_bank, term, data->var_norm);
   termweight_update_conjecture_freqs(data->eval_bank, &data->eval_freqs, 
      repr, data->var_norm);

   if (TermIsFreeVar(repr))
   {
      return data->vweight;
   }

   cell = NumTreeFind(&data->eval_freqs, repr->entry_no);
   freq = cell ? (cell->val1.i_val) : 0;
   if (TermIsConst(repr)) 
   {
      return (freq>0) ? data->conj_cweight : data->cweight;
   }
   if (TermCellQueryProp(repr, TPPredPos))
   {
      return (freq>0) ? data->conj_pweight : data->pweight;
   }
   else
   {
      return (freq>0) ? data->conj_fweight : data->fweight;
   }
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: TBInsertClauseTermsNormalized()
//
//   Insert clause related terms into the term bank with a given 
//   normalizations.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

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

/*-----------------------------------------------------------------------
//
// Function: TermWeightParamAlloc()
//
//   Allocate new parameter cell.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

TermWeightParam_p TermWeightParamAlloc(void)
{
   TermWeightParam_p res = TermWeightParamCellAlloc();

   res->eval_bank = NULL;

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: TermWeightParamFree()
//
//   Free the parameter cell.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void TermWeightParamFree(TermWeightParam_p junk)
{
   if (junk->eval_bank) 
   {
      junk->eval_bank->sig = NULL;
      TBFree(junk->eval_bank);
      junk->eval_bank = NULL;
      NumTreeFree(junk->eval_freqs);
   }
   TermWeightParamCellFree(junk);
}
 
/*-----------------------------------------------------------------------
//
// Function: ConjectureRelativeTermWeightParse()
//
//   Parse parameters from a scanner.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

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

/*-----------------------------------------------------------------------
//
// Function: ConjectureRelativeTermWeightInit()
//
//   Initialize parameters cell and create WFCB.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

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
   data->eval_freqs = NULL;
   
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

/*-----------------------------------------------------------------------
//
// Function: ConjectureRelativeTermWeightCompute()
//
//   Compute the clause weight.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double ConjectureRelativeTermWeightCompute(void* data, Clause_p clause)
{
   TermWeightParam_p local;
   double res;
   
   local = data;
   local->init_fun(data);

   ClauseCondMarkMaximalTerms(local->ocb, clause);
   res = ClauseTermExtWeight(clause, local->twe);

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: ConjectureRelativeTermWeightExit()
//
//   Clean up the parameter cell.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void ConjectureRelativeTermWeightExit(void* data)
{
   TermWeightParam_p junk = data;
   
   TermWeightExtensionFree(junk->twe);
   TermWeightParamFree(junk);
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/

