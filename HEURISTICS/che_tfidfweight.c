/*-----------------------------------------------------------------------

File  : che_tfidfweight.c

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
      if(TermIsVar(subterm)) {
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
   TBCountTermFreqs(data->eval_bank);
#ifdef DEBUG_TERMWEIGHTS
   TBDotBankFile("eval-init.dot",data->eval_bank);
   PDTreeDotFile("documents.dot",data->documents,data->ocb->sig);
#endif
}

static double tfidf_term_weight(Term_p term, TfIdfWeightParam_p data)
{
   long matched, remains;
   double tf, df;
   double idf, tfidf;
   Term_p norm, repr;
   PDTNode_p node;

   norm = TermCopyNormalizeVars(data->eval_bank->vars,term,data->var_norm);
   repr = TBFindRepr(data->eval_bank,norm);
   tf = repr?(repr->freq):0;
   tf = (data->tf_fact*(tf-1))+1; // make tf=1 when tf_fact=0 ("disable tf")

   node = PDTreeMatchPrefix(data->documents,norm,&matched,&remains);
   df = (remains==0)?node->ref_count:0;

   idf = log((1+data->documents->clause_count)/(1+df));
   tfidf = tf*idf; // TODO: try idf only

#ifdef DEBUG_TERMWEIGHTS
   printf("   >>> ");
   TermPrint(GlobalOut,norm,data->eval_bank->sig,DEREF_NEVER);
   printf("   tf=%f df=%f idf=%f tfidf=%f count=%ld\n",tf,df,idf,tfidf,data->documents->clause_count);
#endif

   TermFree(norm);

   return 1/(1+tfidf);
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

TfIdfWeightParam_p TfIdfWeightParamAlloc(void)
{
   TfIdfWeightParam_p res = TfIdfWeightParamCellAlloc();
   res->eval_bank = NULL;
   return res;
}

void TfIdfWeightParamFree(TfIdfWeightParam_p junk)
{
   if (junk->eval_bank) 
   {
      TBFree(junk->eval_bank);
      junk->eval_bank = NULL;
      PDTreeFree(junk->documents);
      junk->documents = NULL;
   }
   TfIdfWeightParamCellFree(junk);
}
 
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

double ConjectureTermTfIdfWeightCompute(void* data, Clause_p clause)
{
   TfIdfWeightParam_p local;
   double res = 1.0;
   
   local = data;
   local->init_fun(data);
   return res;

   //ClauseCondMarkMaximalTerms(local->ocb, clause);
   //res = ClauseTermExtWeight(clause, local->twe);
   //if (local->update_docs) {
   //   tfidf_documents_add_clause(
   //      local->documents,clause,local->var_norm,local->eval_bank->vars);
   //}

#ifdef DEBUG_TERMWEIGHTS
   fprintf(GlobalOut, "=%.2f: ", res);
   ClausePrint(GlobalOut, clause, true);
   fprintf(GlobalOut, "\n");
#endif
   return res;
}

void ConjectureTermTfIdfWeightExit(void* data)
{
   TfIdfWeightParam_p junk = data;
   
   TfIdfWeightParamFree(junk);
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/

