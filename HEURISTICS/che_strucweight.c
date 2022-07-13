/*-----------------------------------------------------------------------

  File  : che_strucweight.c

  Author: Stephan Schulz, yan

  Contents
 
  Iplementation of conjecture structural distance weight (Struc) 
  from [CICM'16/Sec.3].
  
  Syntax
   
  ConjectureStrucDistanceWeight(
   prio,      // priority function
   varnorm,   // variable normalization:
              // 0: universal variable, 
              // 1: alhpa normalization
   relterm,   // related terms: 
              // 0: conjecture terms, 
              // 1: conjecture subterms, 
              // 2: conjecture subterms and top-level generalizations,
              // 3: conjecture subterms and subterm generalizations. 
   var_mis,   // upper bound on variable distance (float)
   sym_mis,   // multiplicator of arguments distance on symbol 
              // mismatch (float)
   inst_fact, // instatiation multiplier/cost (float)
   gen_fact,  // generalization multiplier/cost (float)
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

#include <float.h>
#include "che_strucweight.h"


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

static void strc_insert_term(
   PStack_p terms,
   Term_p term, 
   VarBank_p vars, 
   VarNormStyle var_norm)
{
   Term_p norm;
  
   norm = TermCopyNormalizeVars(vars,term,var_norm);
   PStackPushP(terms,norm);
}

static void strc_insert_subterms(
   PStack_p terms,
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
      if (TermIsFreeVar(subterm)) 
      {
         continue;
      }
      strc_insert_term(terms,subterm,vars,var_norm);

      for(i=0; i<subterm->arity; i++)
      {
         PStackPushP(stack, subterm->args[i]);
      }
   }

   PStackFree(stack);
}

static void strc_insert_topgens(
   PStack_p terms, 
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
      strc_insert_term(terms,topgen,vars,var_norm);
   }
   FreeGeneralizations(topgens);
}

static void strc_insert_subgens(
   PStack_p terms, 
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
      strc_insert_term(terms,genterm,vars,var_norm);
   }
   FreeGeneralizations(subgens);
}

static void strc_init(StrucWeightParam_p data)
{
   Clause_p clause;
   Clause_p anchor;
   Eqn_p lit;

   if (data->terms) 
   {
      return;
   }

   data->terms = PStackAlloc();
   data->vars = VarBankAlloc(data->proofstate->signature->type_bank);
   
   // for each axiom ...
   anchor = data->proofstate->axioms->anchor;
   for (clause=anchor->succ; clause!=anchor; clause=clause->succ)
   {
      if(ClauseQueryTPTPType(clause) != CPTypeNegConjecture) 
      {
         continue;
      }

      // for each literal of a negated conjecture ...
      for (lit=clause->literals; lit; lit=lit->next)
      {
         switch (data->rel_terms) 
         {
         case RTSConjectureTerms:
            strc_insert_term(
               data->terms,lit->lterm,data->vars,data->var_norm);
            strc_insert_term(
               data->terms,lit->rterm,data->vars,data->var_norm);
            break;
         case RTSConjectureSubterms:
            strc_insert_subterms(
               data->terms,lit->lterm,data->vars,data->var_norm);
            strc_insert_subterms(
               data->terms,lit->rterm,data->vars,data->var_norm);
            break;
         case RTSConjectureSubtermsTopGens:
            strc_insert_subterms(
               data->terms,lit->lterm,data->vars,data->var_norm);
            strc_insert_subterms(
               data->terms,lit->rterm,data->vars,data->var_norm);
            strc_insert_topgens(data->terms,lit->lterm,
               data->vars,data->ocb->sig,data->var_norm);
            strc_insert_topgens(data->terms,lit->rterm,
               data->vars,data->ocb->sig,data->var_norm);
            break;
         case RTSConjectureSubtermsAllGens:
            strc_insert_subgens(
               data->terms,lit->lterm,data->vars,data->var_norm);
            strc_insert_subgens(
               data->terms,lit->rterm,data->vars,data->var_norm);
            break;
         default:
            Error("ConjectureStrucDistanceWeight parameters usage error (unsupported RelatedTermSet %d)", USAGE_ERROR, data->rel_terms);
            break;
         }
      }
   }
}

double strc_terms_distance(
   Term_p term1,
   Term_p term2,
   VarBank_p vars,
   VarNormStyle var_norm,
   double var_mismatch,
   double sym_mismatch,
   double inst_factor,
   double gen_factor)
{
   int i;

   // X == Y   or   X == f(t1,...,tn)
   if (TermIsFreeVar(term1)) 
   {
      if (TermIsFreeVar(term2)) 
      {
         // X == Y
         return (term1->f_code==term2->f_code ? 0 : MIN(inst_factor+gen_factor,
                                                        var_mismatch));
         //return 0;
      }
      // X == f(t1,...,tn)
      //return inst_factor*term_struc_weight(term2,init_terms,vars,var_norm,var_mismatch,inst_factor,gen_factor);
      return inst_factor*TermWeight(term2,1,1); 
   }

   // f(t1,...,tn) == Y
   if (TermIsFreeVar(term2)) 
   {
      //return gen_factor*term_struc_weight(term2,init_terms,vars,var_norm,var_mismatch,inst_factor,gen_factor);
      return gen_factor*TermWeight(term1,1,1);
   }

   if ((term1->f_code != term2->f_code) && (term1->arity != term2->arity)) 
   {
      return (gen_factor*TermWeight(term1,1,1))+(inst_factor*TermWeight(term2,1,1));
   }

   double argdist = 0;
   for (i=0; i<term1->arity; i++) 
   {
      argdist += strc_terms_distance(term1->args[i],term2->args[i],
         vars,var_norm,var_mismatch,sym_mismatch,inst_factor,gen_factor);
   }
      
   double geninst = (gen_factor*TermWeight(term1,1,1))+(inst_factor*TermWeight(term2,1,1));
   double fact = (term1->f_code==term2->f_code?1:sym_mismatch);
   
   return MIN(fact*argdist,geninst);
}

static double strc_term_weight(Term_p term, StrucWeightParam_p data)
{
   int i;
   double min,cur;
   Term_p norm;
   Term_p conj;

   norm = TermCopyNormalizeVars(data->vars,term,data->var_norm);

   min = DBL_MAX;
   for (i=0; i<data->terms->current; i++) 
   {
      conj = data->terms->stack[i].p_val;
      cur = strc_terms_distance(norm,conj,
         data->vars,data->var_norm,
         data->var_mismatch,data->sym_mismatch,
         data->inst_factor,data->gen_factor);
      min = MIN(min,cur);
      //printf("STRUC-DISTANCE %f %f %f   ", data->var_mismatch,data->inst_factor,data->gen_factor);
      //TermPrint(GlobalOut,conj,data->ocb->sig,DEREF_NEVER);
      //printf(" &&  ");
      //TermPrint(GlobalOut,norm,data->ocb->sig,DEREF_NEVER);
      //printf(" = %f\n",cur);
   }

   TermFree(norm);
   
   return min;
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: StrucWeightParamAlloc()
//
//   Allocate new parameter cell.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

StrucWeightParam_p StrucWeightParamAlloc(void)
{
   StrucWeightParam_p res = StrucWeightParamCellAlloc();
   
   res->terms = NULL;
   res->vars = NULL;
   
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: StrucWeightParamFree()
//
//   Free the parameter cell.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void StrucWeightParamFree(StrucWeightParam_p junk)
{
   Term_p term;

   if (junk->terms) 
   {
      while (!PStackEmpty(junk->terms)) 
      {
         term = PStackPopP(junk->terms);
         TermFree(term);
      }
      PStackFree(junk->terms);
      junk->terms = NULL;
   }
   if (junk->vars)
   {
      VarBankFree(junk->vars);
      junk->vars = NULL;
   }

   StrucWeightParamCellFree(junk);
}
 
/*-----------------------------------------------------------------------
//
// Function: ConjectureStrucDistanceWeightParse()
//
//   Parse parameters from a scanner.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

WFCB_p ConjectureStrucDistanceWeightParse(
   Scanner_p in,  
   OCB_p ocb, 
   ProofState_p state)
{   
   ClausePrioFun prio_fun;
   double pos_multiplier, max_term_multiplier, max_literal_multiplier;
   TermWeightExtenstionStyle ext_style;
   VarNormStyle var_norm;
   RelatedTermSet rel_terms;
   double var_mismatch, sym_mismatch, inst_factor, gen_factor;

   AcceptInpTok(in, OpenBracket);
   prio_fun = ParsePrioFun(in);
   AcceptInpTok(in, Comma);

   var_norm = (VarNormStyle)ParseInt(in);
   AcceptInpTok(in, Comma);
   rel_terms = (RelatedTermSet)ParseInt(in);
   AcceptInpTok(in, Comma);

   var_mismatch = ParseFloat(in);   
   AcceptInpTok(in, Comma);
   sym_mismatch = ParseFloat(in);   
   AcceptInpTok(in, Comma);
   inst_factor = ParseFloat(in);   
   AcceptInpTok(in, Comma);
   gen_factor = ParseFloat(in);   
   AcceptInpTok(in, Comma);
   
   ext_style = (TermWeightExtenstionStyle)ParseInt(in);
   AcceptInpTok(in, Comma);
   max_term_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   max_literal_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   pos_multiplier = ParseFloat(in);
   AcceptInpTok(in, CloseBracket);
   
   return ConjectureStrucDistanceWeightInit(
      prio_fun, 
      ocb,
      state,
      var_norm,
      rel_terms,
      var_mismatch,
      sym_mismatch,
      inst_factor,
      gen_factor,
      ext_style,
      max_term_multiplier,
      max_literal_multiplier,
      pos_multiplier);
}

/*-----------------------------------------------------------------------
//
// Function: ConjectureStrucDistanceWeightInit()
//
//   Initialize parameters cell and create WFCB.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

WFCB_p ConjectureStrucDistanceWeightInit(
   ClausePrioFun prio_fun, 
   OCB_p ocb,
   ProofState_p proofstate,
   VarNormStyle var_norm,
   RelatedTermSet rel_terms,
   double var_mismatch,
   double sym_mismatch,
   double inst_factor,
   double gen_factor,
   TermWeightExtenstionStyle ext_style,
   double max_term_multiplier,
   double max_literal_multiplier,
   double pos_multiplier)
{
   StrucWeightParam_p data = StrucWeightParamAlloc();

   data->init_fun   = strc_init;
   data->ocb        = ocb;
   data->proofstate = proofstate;
   data->var_norm   = var_norm;
   data->rel_terms  = rel_terms;

   data->var_mismatch = var_mismatch;
   data->sym_mismatch = sym_mismatch;
   data->inst_factor     = inst_factor;
   data->gen_factor      = gen_factor;
   
   data->twe = TermWeightExtensionAlloc(
      max_term_multiplier,
      max_literal_multiplier,
      pos_multiplier,
      ext_style,
      (TermWeightFun)strc_term_weight,
      data);
   
   return WFCBAlloc(
      ConjectureStrucDistanceWeightCompute, 
      prio_fun,
      ConjectureStrucDistanceWeightExit, 
      data);
}

/*-----------------------------------------------------------------------
//
// Function: ConjectureStrucDistanceWeightCompute()
//
//   Compute the clause weight.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double ConjectureStrucDistanceWeightCompute(void* data, Clause_p clause)
{
   double res;
   StrucWeightParam_p local;
   
   local = data;
   local->init_fun(data);

   ClauseCondMarkMaximalTerms(local->ocb, clause);
   res = ClauseTermExtWeight(clause, local->twe);

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: ConjectureStrucDistanceWeightExit()
//
//   Clean up the parameter cell.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void ConjectureStrucDistanceWeightExit(void* data)
{
   StrucWeightParam_p junk = data;
   
   TermWeightExtensionFree(junk->twe);
   StrucWeightParamFree(junk);
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/

