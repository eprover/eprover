/*-----------------------------------------------------------------------

  File  : che_tfidfweight.c

  Author: Stephan Schulz, yan

  Contents
 
  Iplementation of conjecture frequency weight (TfIdf) from [CICM'16/Sec.3].
  
  Syntax
   
  ConjectureTermTfIdfWeight(
   prio,      // priority function
   varnorm,   // variable normalization:
              // 0: universal variable, 
              // 1: alhpa normalization
   relterm,   // related terms: 
              // 0: conjecture terms, 
              // 1: conjecture subterms, 
              // 2: conjecture subterms and top-level generalizations,
              // 3: conjecture subterms and subterm generalizations. 
   update,    // update documents with every generated clause:
              // 0: no (use axioms only)
              // 1: yes
   tf_mult,   // term frequency multiplier (float)
              // (set to 0 to use Idf only, set to 1 for default Tf/Idf)
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

#include "che_tfidfweight.h"


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

static void tfidf_documents_add_term(
   PDTree_p documents, 
   Term_p term,
   Clause_p clause, 
   Eqn_p lit, 
   EqnSide side, 
   VarNormStyle var_norm,
   VarBank_p vars)
{
   ClausePos_p pos;
   Term_p copy;

   copy = TermCopyNormalizeVars(vars,term,var_norm);

   pos          = ClausePosCellAlloc();
   pos->clause  = clause;
   pos->literal = lit;
   pos->side    = side;
   pos->pos     = NULL;
   PDTreeInsertTerm(documents,copy,pos,false);
   ClausePosCellFree(pos);

   TermFree(copy);
}

static void tfidf_documents_add_subterms(
   PDTree_p documents, 
   Term_p term,
   Clause_p clause, 
   Eqn_p lit, 
   EqnSide side, 
   VarNormStyle var_norm,
   VarBank_p vars)
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
      tfidf_documents_add_term(
         documents,subterm,clause,lit,side,var_norm,vars);
      for(i=0; i<subterm->arity; i++)
      {
         PStackPushP(stack, subterm->args[i]);
      }
   }

   PStackFree(stack);
}

static void tfidf_documents_add_clause(
   PDTree_p documents, 
   Clause_p clause,
   VarNormStyle var_norm,
   VarBank_p vars)
{
   Eqn_p lit;

   for (lit=clause->literals; lit; lit=lit->next)
   {
      tfidf_documents_add_subterms(
         documents,lit->lterm,clause,lit,LeftSide,var_norm,vars);
      tfidf_documents_add_subterms(
         documents,lit->rterm,clause,lit,RightSide,var_norm,vars);
   }
}

static void tfidf_init(TfIdfWeightParam_p data)
{
   Clause_p clause;
   Clause_p anchor;

   if (data->eval_bank) 
   {
      return;
   }

   data->eval_bank = TBAlloc(data->ocb->sig);
   data->documents = PDTreeAlloc(data->eval_bank);
   anchor = data->proofstate->axioms->anchor;
   for (clause=anchor->succ; clause!=anchor; clause=clause->succ)
   {
      if(ClauseQueryTPTPType(clause)==CPTypeNegConjecture) {
         TBInsertClauseTermsNormalized(
            data->eval_bank,clause,data->var_norm,data->rel_terms);
      }
      else 
      {
         tfidf_documents_add_clause(data->documents,clause,
            data->var_norm,data->eval_bank->vars);
      }
   }
   
   data->eval_freqs = TBCountTermFreqs(data->eval_bank);
}

static double tfidf_term_weight(Term_p term, TfIdfWeightParam_p data)
{
   long matched, remains;
   double tf, df;
   double idf, tfidf;
   Term_p norm, repr;
   PDTNode_p node;
   NumTree_p cell;

   norm = TermCopyNormalizeVars(data->eval_bank->vars,term,data->var_norm);
   repr = TBFindRepr(data->eval_bank,norm);
   cell = repr ? NumTreeFind(&data->eval_freqs, repr->entry_no) : NULL;
   tf = cell ? (cell->val1.i_val) : 0;
   tf = (data->tf_fact*(tf-1))+1; // make tf=1 when tf_fact=0 ("disable tf")

   node = PDTreeMatchPrefix(data->documents,norm,&matched,&remains);
   df = (remains==0)?node->ref_count:0;

   idf = log((1+data->documents->clause_count)/(1+df));
   tfidf = tf*idf; 
   TermFree(norm);

   return 1/(1+tfidf);
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: TfIdfWeightParamAlloc()
//
//   Allocate new parameter cell.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

TfIdfWeightParam_p TfIdfWeightParamAlloc(void)
{
   TfIdfWeightParam_p res = TfIdfWeightParamCellAlloc();
   res->eval_bank = NULL;
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: TfIdfWeightParamFree()
//
//   Free the parameter cell.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void TfIdfWeightParamFree(TfIdfWeightParam_p junk)
{
   if (junk->eval_bank) 
   {
      PDTreeFree(junk->documents);
      junk->documents = NULL;
      // hack to avoid assertion in TBFree: sig is freed later
      junk->eval_bank->sig = NULL; 
      TBFree(junk->eval_bank);
      junk->eval_bank = NULL;
      NumTreeFree(junk->eval_freqs);
      junk->eval_freqs = NULL;
   }
   TfIdfWeightParamCellFree(junk);
}

/*-----------------------------------------------------------------------
//
// Function: ConjectureTermTfIdfWeightParse()
//
//   Parse parameters from a scanner.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

WFCB_p ConjectureTermTfIdfWeightParse(
   Scanner_p in,  
   OCB_p ocb, 
   ProofState_p state)
{  
   int update_docs;
   double tf_fact;
   double pos_multiplier, max_term_multiplier, max_literal_multiplier;
   TermWeightExtenstionStyle ext_style;
   VarNormStyle var_norm;
   RelatedTermSet rel_terms;
   ClausePrioFun prio_fun;

   AcceptInpTok(in, OpenBracket);
   prio_fun = ParsePrioFun(in);
   AcceptInpTok(in, Comma);

   var_norm = (VarNormStyle)ParseInt(in);
   AcceptInpTok(in, Comma);
   rel_terms = (RelatedTermSet)ParseInt(in);
   AcceptInpTok(in, Comma);

   update_docs = ParseInt(in);
   AcceptInpTok(in, Comma);
   tf_fact = ParseFloat(in);
   AcceptInpTok(in, Comma);
   
   ext_style = (TermWeightExtenstionStyle)ParseInt(in);
   AcceptInpTok(in, Comma);
   max_term_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   max_literal_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   pos_multiplier = ParseFloat(in);
   AcceptInpTok(in, CloseBracket);
   
   return ConjectureTermTfIdfWeightInit(
      prio_fun, 
      ocb,
      state,
      var_norm,
      rel_terms,
      update_docs,
      tf_fact,
      ext_style,
      max_term_multiplier,
      max_literal_multiplier,
      pos_multiplier);
}

/*-----------------------------------------------------------------------
//
// Function: ConjectureTermTfIdfWeightInit()
//
//   Initialize parameters cell and create WFCB.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

WFCB_p ConjectureTermTfIdfWeightInit(
   ClausePrioFun prio_fun, 
   OCB_p ocb,
   ProofState_p proofstate,
   VarNormStyle var_norm,
   RelatedTermSet rel_terms,
   int update_docs,
   double tf_fact,
   TermWeightExtenstionStyle ext_style,
   double max_term_multiplier,
   double max_literal_multiplier,
   double pos_multiplier)
{
   TfIdfWeightParam_p data = TfIdfWeightParamAlloc();

   data->init_fun    = tfidf_init;
   data->ocb         = ocb;
   data->proofstate  = proofstate;
   data->var_norm    = var_norm;
   data->rel_terms   = rel_terms;
   data->update_docs = update_docs;
   data->eval_freqs  = NULL;
   data->twe = TermWeightExtensionAlloc(
      max_term_multiplier,
      max_literal_multiplier,
      pos_multiplier,
      ext_style,
      (TermWeightFun)tfidf_term_weight,
      data);
   
   return WFCBAlloc(
      ConjectureTermTfIdfWeightCompute, 
      prio_fun,
      ConjectureTermTfIdfWeightExit, 
      data);
}

/*-----------------------------------------------------------------------
//
// Function: ConjectureTermTfIdfWeightCompute()
//
//   Compute the clause weight.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double ConjectureTermTfIdfWeightCompute(void* data, Clause_p clause)
{
   double res;
   TfIdfWeightParam_p local;
   
   local = data;
   local->init_fun(data);

   ClauseCondMarkMaximalTerms(local->ocb, clause);
   res = ClauseTermExtWeight(clause, local->twe);
   if (local->update_docs) 
   {
      tfidf_documents_add_clause(
         local->documents,clause,local->var_norm,local->eval_bank->vars);
   }

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: ConjectureTermTfIdfWeightExit()
//
//   Clean up the parameter cell.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void ConjectureTermTfIdfWeightExit(void* data)
{
   TfIdfWeightParam_p junk = data;
  
   TermWeightExtensionFree(junk->twe);
   TfIdfWeightParamFree(junk);
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/

