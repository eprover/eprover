/*-----------------------------------------------------------------------

File  : che_prefixweight.c

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

#include "che_prefixweight.h"


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

static void prfx_insert_term(
   PDTree_p tree, 
   Term_p term, 
   VarBank_p vars, 
   VarNormStyle var_norm)
{
   Term_p norm;

   norm = TermCopyNormalizeVars(vars,term,var_norm);
   PDTreeInsertTerm(tree,norm,NULL,false);
   TermFree(norm);
}

static void prfx_insert_subterms(
   PDTree_p tree, 
   Term_p term, 
   VarBank_p vars, 
   VarNormStyle var_norm)
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
      prfx_insert_term(tree,subterm,vars,var_norm);

      for(i=0; i<subterm->arity; i++)
      {
         PStackPushP(stack, subterm->args[i]);
      }
   }

   PStackFree(stack);
}

static void prfx_insert_topgens(
   PDTree_p tree, 
   Term_p term,
   VarBank_p vars,
   Sig_p sig,
   VarNormStyle var_norm)
{
   int i;
   PStack_p topgens;
   Term_p topgen;

   topgens = ComputeTopGeneralizations(term,vars,sig);
   for (i=0; i<topgens->current; i++) {
      topgen = topgens->stack[i].p_val;
      prfx_insert_term(tree,topgen,vars,var_norm);
   }
   FreeGeneralizations(topgens);
}

static void prfx_insert_subgens(
   PDTree_p tree, 
   Term_p term, 
   VarBank_p vars, 
   VarNormStyle var_norm)
{
   int i;
   PStack_p subgens;
   Term_p genterm;

   subgens = ComputeSubtermsGeneralizations(term,vars);
   for (i=0; i<subgens->current; i++) {
      genterm = subgens->stack[i].p_val;
      prfx_insert_term(tree,genterm,vars,var_norm);
   }
   FreeGeneralizations(subgens);
}

static void prfx_init(PrefixWeightParam_p data)
{
   Clause_p clause;
   Clause_p anchor;
   Eqn_p lit;

   if (data->terms) 
   {
      return;
   }

   data->terms = PDTreeAlloc(data->proofstate->terms);
   data->vars = VarBankAlloc(TypeBankAlloc()); 

   // for each axiom ...
   anchor = data->proofstate->axioms->anchor;
   for (clause=anchor->succ; clause!=anchor; clause=clause->succ)
   {
      if(ClauseQueryTPTPType(clause)!=CPTypeNegConjecture) {
         continue;
      }

      // for each literal of a negated conjecture ...
      for (lit=clause->literals; lit; lit=lit->next)
      {
         switch (data->rel_terms) {
         case RTSConjectureTerms:
            prfx_insert_term(data->terms,lit->lterm,
               data->vars,data->var_norm);
            prfx_insert_term(data->terms,lit->rterm,
               data->vars,data->var_norm);
            break;
         case RTSConjectureSubterms:
            prfx_insert_subterms(data->terms,lit->lterm,
               data->vars,data->var_norm);
            prfx_insert_subterms(data->terms,lit->rterm,
               data->vars,data->var_norm);
            break;
         case RTSConjectureSubtermsTopGens:
            prfx_insert_subterms(data->terms,lit->lterm,
               data->vars,data->var_norm);
            prfx_insert_subterms(data->terms,lit->rterm,
               data->vars,data->var_norm);
            prfx_insert_topgens(data->terms,lit->lterm,
               data->vars,data->ocb->sig,data->var_norm);
            prfx_insert_topgens(data->terms,lit->rterm,
               data->vars,data->ocb->sig,data->var_norm);
            break;
         case RTSConjectureSubtermsAllGens:
            prfx_insert_subgens(data->terms,lit->lterm,
               data->vars,data->var_norm);
            prfx_insert_subgens(data->terms,lit->rterm,
               data->vars,data->var_norm);
            break;
         default:
            Error("ConjectureTermPrefixWeight parameters usage error (unsupported RelatedTermSet %d)", USAGE_ERROR, data->rel_terms);
            break;
         }
      }
   }

#ifdef DEBUG_TERMWEIGHTS
   PDTreeDotFile("eval-tree.dot",data->terms,data->proofstate->signature);
#endif
}

static double prfx_term_weight(Term_p term, PrefixWeightParam_p data)
{
   long matches, misses;
   double res;

   matches = 0;
   misses = 0;
   
   PDTreeMatchPrefix(data->terms,term,&matches,&misses);

   res = (matches*data->match_weight)+(misses*data->miss_weight);
   
   return res;
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

PrefixWeightParam_p PrefixWeightParamAlloc(void)
{
   PrefixWeightParam_p res = PrefixWeightParamCellAlloc();

   res->terms = NULL;

   return res;
}

void PrefixWeightParamFree(PrefixWeightParam_p junk)
{
   PrefixWeightParamCellFree(junk);
}
 
WFCB_p ConjectureTermPrefixWeightParse(
   Scanner_p in,  
   OCB_p ocb, 
   ProofState_p state)
{   
   ClausePrioFun prio_fun;
   double pos_multiplier, max_term_multiplier, max_literal_multiplier;
   TermWeightExtenstionStyle ext_style;
   VarNormStyle var_norm;
   RelatedTermSet rel_terms;
   double match_weight, miss_weight;

   AcceptInpTok(in, OpenBracket);
   prio_fun = ParsePrioFun(in);
   AcceptInpTok(in, Comma);

   var_norm = (VarNormStyle)ParseInt(in);
   AcceptInpTok(in, Comma);
   rel_terms = (RelatedTermSet)ParseInt(in);
   AcceptInpTok(in, Comma);

   match_weight = ParseFloat(in);
   AcceptInpTok(in, Comma);
   miss_weight = ParseFloat(in);
   AcceptInpTok(in, Comma);
   
   ext_style = (TermWeightExtenstionStyle)ParseInt(in);
   AcceptInpTok(in, Comma);
   max_term_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   max_literal_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   pos_multiplier = ParseFloat(in);
   AcceptInpTok(in, CloseBracket);
   
   return ConjectureTermPrefixWeightInit(
      prio_fun, 
      ocb,
      state,
      var_norm,
      rel_terms,
      match_weight,
      miss_weight,
      ext_style,
      max_term_multiplier,
      max_literal_multiplier,
      pos_multiplier);
}

WFCB_p ConjectureTermPrefixWeightInit(
   ClausePrioFun prio_fun, 
   OCB_p ocb,
   ProofState_p proofstate,
   VarNormStyle var_norm,
   RelatedTermSet rel_terms,
   double match_weight,
   double miss_weight,
   TermWeightExtenstionStyle ext_style,
   double max_term_multiplier,
   double max_literal_multiplier,
   double pos_multiplier)
{
   PrefixWeightParam_p data = PrefixWeightParamAlloc();

   data->init_fun     = prfx_init;
   data->ocb          = ocb;
   data->proofstate   = proofstate;
   data->var_norm     = var_norm;
   data->rel_terms    = rel_terms;
   data->match_weight = match_weight;
   data->miss_weight  = miss_weight;
   data->twe = TermWeightExtensionAlloc(
      max_term_multiplier,
      max_literal_multiplier,
      pos_multiplier,
      ext_style,
      (TermWeightFun)prfx_term_weight,
      data);
   
   return WFCBAlloc(
      ConjectureTermPrefixWeightCompute, 
      prio_fun,
      ConjectureTermPrefixWeightExit, 
      data);
}

double ConjectureTermPrefixWeightCompute(void* data, Clause_p clause)
{
   double res;
   PrefixWeightParam_p local;
   
   local = data;
   local->init_fun(data);

   ClauseCondMarkMaximalTerms(local->ocb, clause);
   res = ClauseTermExtWeight(clause, local->twe);

#ifdef DEBUG_TERMWEIGHTS
   fprintf(GlobalOut, "=%.2f: ", res);
   ClausePrint(GlobalOut, clause, true);
   fprintf(GlobalOut, "\n");
#endif

   return res;
}

void ConjectureTermPrefixWeightExit(void* data)
{
   PrefixWeightParam_p junk = data;
   
   PrefixWeightParamFree(junk);
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/

