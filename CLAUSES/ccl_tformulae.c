/*-----------------------------------------------------------------------

  File  : ccl_tformulae.c

  Author: Stephan Schulz

  Contents

  Code for the full first order formulae encoded as terms.

  Copyright 2005-2017 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: Thu May 19 17:26:49 CEST 2005 (some taken from old
  implementation (ccl_formulae.c)

  -----------------------------------------------------------------------*/

#include "ccl_tformulae.h"
#include <cte_lambda.h>
#include <ccl_pdtrees.h>
#include <ccl_formula_wrapper.h>
#include "ccl_inferencedoc.h"
#include "ccl_derivation.h"


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/

static TFormula_p elem_tform_tptp_parse(Scanner_p in, TB_p terms);
static TFormula_p literal_tform_tstp_parse(Scanner_p in, TB_p terms);

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: make_head()
//
//   Makes term that has function code that corresponds to f_name
//   and no arguments.
//   NB:  Term is unshared at this point!
//
// Global Variables: -
//
// Side Effects    : Input, memory operations, changes term bank
//
/----------------------------------------------------------------------*/

static Term_p make_head(Sig_p sig, const char* f_name)
{
   Term_p head = NULL;
   FunCode f_code = SigFindFCode(sig, f_name);

   if(f_code)
   {
      head = TermDefaultCellAlloc();
      head->f_code = f_code;
      head->arity = 0;
      head->type = SigGetType(sig, head->f_code);
   }
   return head;
}

/*-----------------------------------------------------------------------
//
// Function: parse_ho_atom()
//
//    Parses one HO symbol.
//
// Global Variables: -
//
// Side Effects    : Input, memory operations
//
/----------------------------------------------------------------------*/

static Term_p __inline__ parse_ho_atom(Scanner_p in, TB_p bank)
{
   assert(problemType == PROBLEM_HO);

   FuncSymbType   id_type;
   DStr_p id      = DStrAlloc();
   Type_p type;
   Term_p head;

   if(TestInpTok(in, IteToken))
   {
      head = ParseIte(in, bank);
   }
   else if(TestInpTok(in, LetToken))
   {
      head = ParseLet(in, bank);
      assert(head->type);
   }
   else if((id_type=TermParseOperator(in, id))==FSIdentVar)
   {
      /* A variable may be annotated with a sort */
      if(TestInpTok(in, Colon))
      {
         AcceptInpTok(in, Colon);
         type = TypeBankParseType(in, bank->sig->type_bank);
         head = VarBankExtNameAssertAllocSort(bank->vars, DStrView(id), type);
      }
      else
      {
         head = VarBankExtNameAssertAlloc(bank->vars, DStrView(id));
      }

      assert(TermIsFreeVar(head));
   }
   else
   {
      head = make_head(bank->sig, DStrView(id));
      if(!head)
      {
         DStr_p msg = DStrAlloc();
         DStrAppendStr(msg, "Function symbol ");
         DStrAppendStr(msg, DStrView(id));
         DStrAppendStr(msg, " has not been defined previously.");
         AktTokenError(in, DStrView(msg), false);
      }
      head = TBTermTopInsert(bank, head);
   }
   DStrFree(id);
   assert(TermIsShared(head));
   assert(head->type);
   return head;
}

/*-----------------------------------------------------------------------
//
// Function: normalize_head()
//
//   Makes sure that term is represented in a flattened representation.
//
// Global Variables: -
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

static Term_p normalize_head(Term_p head, Term_p* rest_args, int rest_arity, TB_p bank)
{
   assert(problemType == PROBLEM_HO);
   assert(TermIsFreeVar(head) || TermIsShared(head));
   Term_p res = NULL;

   if(rest_arity == 0)
   {
      res = head; // nothing is being applied
   }
   else
   {
      int total_arity = (TermIsPhonyAppTarget(head) ? 0 : head->arity) + rest_arity;
      if(TermIsPhonyAppTarget(head))
      {
         total_arity++; // head is going to be the first argument

         res = TermDefaultCellArityAlloc(total_arity);
         res->f_code = SIG_PHONY_APP_CODE;

         res->args[0] = head;
         for(int i=1; i<total_arity; i++)
         {
            res->args[i] = rest_args[i-1];
         }
      }
      else
      {
         assert(total_arity != 0);
         res = TermDefaultCellArityAlloc(total_arity);
         res->f_code = head->f_code;

         int i;
         for(i=0; i < head->arity; i++)
         {
            res->args[i] = head->args[i];
         }

         for(i=0; i < rest_arity; i++)
         {
            res->args[head->arity + i] = rest_args[i];
         }
      }
   }

   if(!TermIsFreeVar(res) && !TermIsShared(res))
   {
      res = TBTermTopInsert(bank, res);
   }

   assert(TermIsShared(res));
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: tptp_operator_convert()
//
//   Return the f_code corresponding to a given token. Rather
//   trivial ;-)
//
// Global Variables: -
//
// Side Effects    : Input
//
/----------------------------------------------------------------------*/

static FunCode tptp_operator_convert(Sig_p sig, TokenType tok)
{
   FunCode res=0;

   switch(tok)
   {
   case FOFOr:
         res = sig->or_code;
         break;
   case FOFAnd:
         res = sig->and_code;
         break;
   case FOFLRImpl:
         res = sig->impl_code;
         break;
   case FOFRLImpl:
         res = sig->bimpl_code;
         break;
   case FOFEquiv:
         res = sig->equiv_code;
         break;
   case EqualSign:
         res = sig->eqn_code;
         break;
   case FOFXor:
         res = sig->xor_code;
         break;
   case NegEqualSign:
         res = sig->neqn_code;
         break;
   case FOFNand:
         res = sig->nand_code;
         break;
   case FOFNor:
         res = sig->nor_code;
         break;
   default:
         assert(false && "Unknown/Impossibe operator.");
         break;
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: tptp_operator_parse()
//
//   Parse a TPTP operator and return the corresponding f_code. Rather
//   trivial ;-)
//
// Global Variables: -
//
// Side Effects    : Input
//
/----------------------------------------------------------------------*/

static FunCode tptp_operator_parse(Sig_p sig, Scanner_p in)
{
   FunCode res=0;

   CheckInpTok(in, FOFBinOp);
   res = tptp_operator_convert(sig, AktTokenType(in));
   NextToken(in);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: tptp_quantor_parse()
//
//   Parse and return a TPTP quantor. Rather trivial ;-)
//
// Global Variables: -
//
// Side Effects    : Input
//
/----------------------------------------------------------------------*/

static FunCode tptp_quantor_parse(Sig_p sig, Scanner_p in)
{
   FunCode res;

   CheckInpTok(in, UnivQuantor|ExistQuantor|LambdaQuantor);
   if(TestInpTok(in, ExistQuantor))
   {
      res = sig->qex_code;
   }
   else if (TestInpTok(in, UnivQuantor))
   {
      res = sig->qall_code;
   }
   else
   {
      assert (TestInpTok(in, LambdaQuantor));
      res = SIG_NAMED_LAMBDA_CODE;
   }
   NextToken(in);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: quantified_tform_tptp_parse()
//
//   Parse a quantified TPTP/TSTP formula. At this point, the quantor
//   has already been read (and is passed into the function), and we
//   are at the first (or current) variable.
//
// Global Variables: -
//
// Side Effects    : Input, memory operations
//
/----------------------------------------------------------------------*/

static TFormula_p quantified_tform_tptp_parse(Scanner_p in,
                                              TB_p terms,
                                              FunCode quantor)
{
   Term_p     var;
   TFormula_p  rest, res;
   DStr_p     source_name;
   long       line, column;
   StreamType type;

   line = AktToken(in)->line;
   column = AktToken(in)->column;
   source_name = DStrGetRef(AktToken(in)->source);
   type = AktToken(in)->stream_type;

   /* Enter a new scope for variables (exit scope before exiting function) */
   VarBankPushEnv(terms->vars);

   var = TBTermParse(in, terms);
   if(!TermIsFreeVar(var))
   {
      Error("%s Variable expected, non-variable term found",
            SYNTAX_ERROR,
            PosRep(type, source_name, line, column));
   }
   assert(var->type);
   DStrReleaseRef(source_name);
   if(TestInpTok(in, Comma))
   {
      AcceptInpTok(in, Comma);
      rest = quantified_tform_tptp_parse(in, terms, quantor);
   }
   else
   {
      AcceptInpTok(in, CloseSquare);
      AcceptInpTok(in, Colon);
      rest = elem_tform_tptp_parse(in, terms);
   }
   res = TFormulaFCodeAlloc(terms, quantor, var, rest);

   VarBankPopEnv(terms->vars);
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: parse_atom()
//
//   Parse an elementary formula in TPTP/TSTP format. New: takes care
//   of complicated forms such as $let and $ite
//
// Global Variables: -
//
// Side Effects    : I/O
//
/----------------------------------------------------------------------*/

static TFormula_p parse_atom(Scanner_p in, TB_p terms)
{
   Term_p lhs, rhs;
   bool positive = EqnParseInfix(in, terms, &lhs, &rhs);
   Term_p res;
   if(rhs == NULL)
   {
      res = lhs;
   }
   else
   {
      res = EqnTermsTBTermEncode(terms, lhs, rhs, positive, PENormal);
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: elem_tform_tptp_parse()
//
//   Parse an elementary formula in TPTP/TSTP format.
//
// Global Variables: -
//
// Side Effects    : I/O
//
/----------------------------------------------------------------------*/

static TFormula_p elem_tform_tptp_parse(Scanner_p in, TB_p terms)
{
   TFormula_p res, tmp;

   if(TestInpTok(in, UnivQuantor|ExistQuantor))
   {
      FunCode quantor;
      quantor = tptp_quantor_parse(terms->sig,in);
      AcceptInpTok(in, OpenSquare);
      res = quantified_tform_tptp_parse(in, terms, quantor);
   }
   else if(TestInpTok(in, OpenBracket))
   {
      AcceptInpTok(in, OpenBracket);
      res = TFormulaTPTPParse(in, terms);
      AcceptInpTok(in, CloseBracket);
   }
   else if(TestInpTok(in, TildeSign))
   {
      AcceptInpTok(in, TildeSign);
      tmp = elem_tform_tptp_parse(in, terms);
      res = TFormulaFCodeAlloc(terms, terms->sig->not_code, tmp, NULL);
   }
   else
   {
      res = parse_atom(in, terms);
   }
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: clause_tform_tstp_parse()
//
//   Parse a sequence of literals connected by a | operator
//   and return it.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

static TFormula_p clause_tform_tstp_parse(Scanner_p in, TB_p terms)
{
   //printf(COMCHAR" clause_tform_tstp_parse()\n");
   TFormula_p head, rest;
   Eqn_p lit;

   lit = EqnFOFParse(in, terms);
   head = TFormulaLitAlloc(lit);
   EqnFree(lit);
   //printf(COMCHAR" head parsed\n");
   while(TestInpTok(in, FOFOr))
   {
      AcceptInpTok(in, FOFOr);
      lit = EqnFOFParse(in, terms);
      //printf(COMCHAR" lit parsed:");
      //EqnPrint(stdout, lit, false, true);
      //printf("\n");
      rest = TFormulaLitAlloc(lit);
      EqnFree(lit);
      //printf(COMCHAR" rest allocated\n");
      head = TFormulaFCodeAlloc(terms, terms->sig->or_code, head, rest);
      //printf(COMCHAR" allocated\n");
   }
   // printf(COMCHAR" done:");
   //TFormulaTPTPPrint(stdout, terms, head, true, true);
   //printf("\n");
   return head;
}


/*-----------------------------------------------------------------------
//
// Function: quantified_tform_tstp_parse()
//
//   Parse a quantified TSTP formula. At this point, the quantor
//   has already been read (and is passed into the function), and we
//   are at the first (or current) variable.
//
// Global Variables: -
//
// Side Effects    : Input, memory operations
//
/----------------------------------------------------------------------*/

static TFormula_p quantified_tform_tstp_parse(Scanner_p in,
                                              TB_p terms,
                                              FunCode quantor, bool tcf)
{
   Term_p     var;
   TFormula_p  rest, res;
   DStr_p     source_name;
   long       line, column;
   StreamType type;

   line = AktToken(in)->line;
   column = AktToken(in)->column;
   source_name = DStrGetRef(AktToken(in)->source);
   type = AktToken(in)->stream_type;

   /* Enter a new scope for variables (exit scope before exiting function) */
   VarBankPushEnv(terms->vars);

   var = TBTermParse(in, terms);
   if(!TermIsFreeVar(var))
   {
      Error("%s Variable expected, non-variable term found",
            SYNTAX_ERROR,
            PosRep(type, source_name, line, column));
   }
   DStrReleaseRef(source_name);
   if(TestInpTok(in, Comma))
   {
      AcceptInpTok(in, Comma);
      rest = quantified_tform_tstp_parse(in, terms, quantor, tcf);
   }
   else
   {
      AcceptInpTok(in, CloseSquare);
      AcceptInpTok(in, Colon);
      if(tcf)
      {
         if(TestInpTok(in, OpenBracket))
         {
            AcceptInpTok(in, OpenBracket);
            rest = clause_tform_tstp_parse(in, terms);
            AcceptInpTok(in, CloseBracket);
         }
         else
         {
            rest = parse_atom(in, terms);
         }
      }
      else
      {
         rest = literal_tform_tstp_parse(in, terms);
      }
   }
   res = TFormulaFCodeAlloc(terms, quantor, var, rest);

   VarBankPopEnv(terms->vars);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: assoc_tform_tstp_parse()
//
//   Parse a sequence of formulas connected by a single AC operator
//   and return it.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

static TFormula_p assoc_tform_tstp_parse(Scanner_p in, TB_p terms, TFormula_p head)
{
   TokenType  optok;
   FunCode    op;
   TFormula_p f2;

   optok =  AktTokenType(in);
   op    =  tptp_operator_convert(terms->sig, optok);

   while(TestInpTok(in, optok))
   {
      AcceptInpTok(in, optok);
      f2 = literal_tform_tstp_parse(in, terms);
      head = TFormulaFCodeAlloc(terms, op, head, f2);
   }
   return head;
}


/*-----------------------------------------------------------------------
//
// Function: applied_tform_tstp_parse()
//
//   Parse a sequence of formulas connected by application operator
//   and normalize the term according to the invariant maintained by @:
//   If the head is a single constant F then simply apply F to arguments.
//   Otherwise, apply the head using SIG_PHONY_APP_CODE
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

static TFormula_p applied_tform_tstp_parse(Scanner_p in, TB_p terms, TFormula_p head)
{
   assert(TestInpTok(in, Application));

   // printf("applied_tform_tstp_parse()...\n");
   const Type_p hd_type = GetHeadType(terms->sig, head);
   assert(hd_type);
   const int max_args = TypeGetMaxArity(hd_type);
   int i = 0;
   const TermRef args = TermArgTmpArrayAlloc(max_args);
   bool head_is_logical = !TermIsFreeVar(head) &&
      SigQueryFuncProp(terms->sig, head->f_code, FPFOFOp);
   Term_p arg;


   while(TestInpTok(in, Application))
   {
      if(i >= max_args)
      {
         //fprintf(stderr, "max args: %d\n", max_args);
         //fprintf(stderr, "type: ");
         //TypePrintTSTP(stderr, terms->sig->type_bank, hd_type);
         //TermPrintDbg(stderr, head, terms->sig, DEREF_NEVER);
         AktTokenError(in, " Too many arguments applied to the term",
                       false);
      }
      AcceptInpTok(in, Application);
      arg = literal_tform_tstp_parse(in, terms);
      args[i++] = head_is_logical ? EncodePredicateAsEqn(terms, arg) : arg;
   }

   TFormula_p res =
      EncodePredicateAsEqn(terms, normalize_head(head, args, i, terms));
   TermArgTmpArrayFree(args, max_args);
   // printf("...applied_tform_tstp_parse()\n");
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: literal_tform_tstp_parse()
//
//   Parse an elementary formula in TSTP format.
//   Parses:
//   (1) quantified formulas (includes lambda in HO)
//   (2) '(' full formula ')'
//   (3) ~ full formula
//   FO: (4) equation / predicate term
//   HO: (4) variable or constant
//
// Global Variables: -
//
// Side Effects    : I/O
//
/----------------------------------------------------------------------*/

static TFormula_p literal_tform_tstp_parse(Scanner_p in, TB_p terms)
{
   TFormula_p res=NULL, tmp=NULL;

   if(TestInpTok(in, UnivQuantor|ExistQuantor|LambdaQuantor))
   {
      FunCode quantor;
      quantor = tptp_quantor_parse(terms->sig,in);
      AcceptInpTok(in, OpenSquare);
      res = quantified_tform_tstp_parse(in, terms, quantor, false);
   }
   else if(TestInpTok(in, OpenBracket))
   {
      AcceptInpTok(in, OpenBracket);

      FunCode log_op = -1;
      // cases where in HO syntax we can have logical symbol at the top
      if(TestInpTok(in, FOFBinOp) && TestTok(LookToken(in, 1), CloseBracket))
      {
         log_op = tptp_operator_parse(terms->sig, in);
      }
      else if (TestInpTok(in, TildeSign) && TestTok(LookToken(in, 1), CloseBracket))
      {
         AcceptInpTok(in, TildeSign);
         log_op = terms->sig->not_code;
      }

      if(log_op != -1)
      {
         res = TBTermTopInsert(terms, TermTopAlloc(log_op, 0));
      }
      else
      {
         res = TFormulaTSTPParse(in, terms);
      }
      AcceptInpTok(in, CloseBracket);
   }
   else if(TestInpTok(in, TildeSign))
   {
      AcceptInpTok(in, TildeSign);
      if (TestInpTok(in, Application))
      {
         AcceptInpTok(in, Application);
      }
      tmp = literal_tform_tstp_parse(in, terms);
      res = TFormulaFCodeAlloc(terms, terms->sig->not_code, tmp, NULL);
   }
   else
   {
      if(problemType == PROBLEM_FO)
      {
         res = parse_atom(in, terms);
      }
      else
      {
         res = parse_ho_atom(in, terms);
      }
   }
   return EncodePredicateAsEqn(terms, res);
}


/*-----------------------------------------------------------------------
//
// Function: tform_compute_freevars()
//
//   Return the set of free variables in form. If necessary, compute
//   it and update bank->freevars.
//
// Global Variables: -
//
// Side Effects    : Updates bank->freevars.
//
/----------------------------------------------------------------------*/

/* VarSet_p tform_compute_freevars(TB_p bank, TFormula_p form) */
/* { */
/*    VarSet_p free_vars = VarSetStoreGetVarSet(&(bank->freevarsets), form); */
/*    VarSet_p arg_vars; */

/*    if(!free_vars->valid) */
/*    { */
/*       // printf("Computing for %p - ", form); */
/*       if(TFormulaIsLiteral(bank->sig, form)) */
/*       { */
/*          // printf("literal\n"); */
/*          VarSetCollectVars(free_vars); */
/*       } */
/*       else if((form->f_code == bank->sig->qex_code) || */
/*               (form->f_code == bank->sig->qall_code)) */
/*       { */
/*          // printf("quantified\n"); */
/*          arg_vars = tform_compute_freevars(bank, form->args[1]); */
/*          VarSetInsertVarSet(free_vars, arg_vars); */
/*          VarSetDeleteVar(free_vars, form->args[0]); */
/*       } */
/*       else */
/*       { */
/*          int i; */

/*          // printf("composite\n"); */
/*          for(i=0;  i<form->arity; i++) */
/*          { */
/*             arg_vars = tform_compute_freevars(bank, form->args[i]); */
/*             VarSetInsertVarSet(free_vars, arg_vars); */
/*          } */
/*       } */
/*       free_vars->valid = true; */
/*    } */
/*    return free_vars; */
/* } */




/*-----------------------------------------------------------------------
//
// Function: tformula_collect_freevars()
//
//   Collect the _free_ variables in form in *vars. This is somewhat
//   tricky. We require that initially all variables have TPIsFreeVar
//   set.
//
// Global Variables: -
//
// Side Effects    : Memory operations, changes TPIsFreeVar.
//
/----------------------------------------------------------------------*/

static void tformula_collect_freevars(TB_p bank, TFormula_p form, PTree_p *vars)
{
   TermProperties old_prop;

   if(form->f_code == SIG_LET_CODE)
   {
      tformula_collect_freevars(bank, form->args[form->arity-1], vars);
   }
   else if(TermIsDBVar(form))
   {
      return;
   }
   else if(TFormulaIsQuantified(bank->sig, form) && form->arity == 2)
   {
      old_prop = TermCellGiveProps(form->args[0], TPIsFreeVar);
      TermCellDelProp(form->args[0], TPIsFreeVar);
      tformula_collect_freevars(bank, form->args[1], vars);
      TermCellSetProp(form->args[0], old_prop);
   }
   else if(TermIsFreeVar(form))
   {
      if(TermCellQueryProp(form, TPIsFreeVar))
      {
         PTreeStore(vars, form);
      }
   }
   else
   {
      for(int i=0; i<form->arity; i++)
      {
         if(TermIsFreeVar(form->args[i]) &&
            TermCellQueryProp(form->args[i], TPIsFreeVar))
         {
            PTreeStore(vars, form->args[i]);
         }
         else
         {
            tformula_collect_freevars(bank, form->args[i], vars);
         }
      }
   }
}

/*-----------------------------------------------------------------------
//
// Function: lambda_eq_to_forall()
//
//   If the term is an equation between terms where at least one is a lambda,
//   then turn it into equation of non-lambdas
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static Term_p lambda_eq_to_forall(TB_p terms, Term_p t)
{
   Sig_p sig = terms->sig;
   if(!TermHasEqNeq(t))
   {
      t = NULL;
   }
   else if((t->f_code == sig->eqn_code
            || t->f_code == sig->neqn_code)
           && t->arity == 2)
   {
      if((TermIsLambda(t->args[0]) || TermIsLambda(t->args[1])))
      {
         PStack_p lhs_vars = PStackAlloc();
         PStack_p rhs_vars = PStackAlloc();
         UNUSED(UnfoldLambda(t->args[0], lhs_vars));
         UNUSED(UnfoldLambda(t->args[1], rhs_vars));
         assert(!PStackEmpty(lhs_vars) || !PStackEmpty(rhs_vars));

         PStack_p more_vars =
            PStackGetSP(lhs_vars) > PStackGetSP(rhs_vars) ? lhs_vars : rhs_vars;
         PStack_p fresh_vars = PStackAlloc();
         PStack_p encoded_vars = PStackAlloc();
         for(long i=0; i<PStackGetSP(more_vars); i++)
         {
            Term_p db = PStackElementP(more_vars, i);
            Term_p fvar = VarBankGetFreshVar(terms->vars, db->type);
            PStackPushP(fresh_vars, fvar);
            PStackPushP(encoded_vars, EncodePredicateAsEqn(terms, fvar));
         }
         Term_p lhs = BetaNormalizeDB(terms, ApplyTerms(terms, t->args[0], encoded_vars));
         Term_p rhs = BetaNormalizeDB(terms, ApplyTerms(terms, t->args[1], encoded_vars));
         if(lhs->type == sig->type_bank->bool_type)
         {

            t = TFormulaFCodeAlloc(terms,
                                   t->f_code == sig->eqn_code
                                   ? sig->equiv_code : sig->xor_code,
                                   EncodePredicateAsEqn(terms, lhs),
                                   EncodePredicateAsEqn(terms, rhs));
         }
         else
         {
            t = TFormulaFCodeAlloc(terms, t->f_code, lhs, rhs);
         }

         bool universal = t->f_code == sig->eqn_code
            || t->f_code == sig->equiv_code;
         while(!PStackEmpty(fresh_vars))
         {
            t = TFormulaAddQuantor(terms, t, universal, PStackPopP(fresh_vars));
         }

         PStackFree(lhs_vars);
         PStackFree(rhs_vars);
         PStackFree(fresh_vars);
         PStackFree(encoded_vars);
      }
   }
   return t;
}

/*-----------------------------------------------------------------------
//
// Function: find_generalization()
//
//   Check if there is already a name for lambda term query. If so,
//   return the defining formula and store the name in *name. Assumes
//   that in query fresh variables that represent loosely bound vars
//   are bound to their corresponding DB vars.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

WFormula_p find_generalization(PDTree_p liftings, Term_p query, TermRef name)
{
   ClausePos_p pos;
   Subst_p    subst = SubstAlloc();
   WFormula_p res = NULL;

   PDTreeSearchInit(liftings, query, PDTREE_IGNORE_NF_DATE, false);
   while(!res && (pos = PDTreeFindNextDemodulator(liftings, subst)))
   {
      //once to deref variables that matched the definition
      //to query
      Term_p matcher_derefed =
         TBInsertInstantiated(liftings->bank, pos->literal->rterm);

      // to make sure that previously derefed vars are not derefed
      // again, we temporarily unbind them
      Term_p old_bindings[PStackGetSP(subst)];
      for(long i=0; i<PStackGetSP(subst); i++)
      {
         Term_p var = PStackElementP(subst, i);
         old_bindings[i] = var->binding;
         var->binding = NULL;
      }


      //the second time to bind free variables to loosely bound ones
      matcher_derefed =
         TBInsertInstantiated(liftings->bank, matcher_derefed);

      Term_p candidate =
         LambdaEtaReduceDB(liftings->bank,
            BetaNormalizeDB(liftings->bank, matcher_derefed));
      if(!TermHasLambdaSubterm(candidate))
      {
         *name = candidate;
         res = pos->data;
         DBGTermCheckUnownedSubterm(stdout, res->tformula, "find_generalization");
      }
      else
      {
         for(long i=0; i<PStackGetSP(subst); i++)
         {
            ((Term_p)PStackElementP(subst, i))->binding = old_bindings[i];
         }
      }
   }
   PDTreeSearchExit(liftings);
   SubstDelete(subst);
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: store_lifting()
//
//   Check if there is already a name for lambda term query. If so,
//   return the defining formula and store the name in *name.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void store_lifting(PDTree_p liftings, Term_p def_head, Term_p body, WFormula_p def)
{
   Eqn_p eqn = EqnAlloc(body, def_head, liftings->bank, true);
   ClausePos_p pos = ClausePosCellAlloc();
   pos->literal = eqn;
   pos->data = def;
   pos->side = LeftSide;
   pos->pos = NULL;
   pos->clause = NULL;
   if(!PDTreeInsert(liftings, pos))
   {
      EqnFree(eqn);
      ClausePosCellFree(pos);
   }
}

/*-----------------------------------------------------------------------
//
// Function: store_lifting()
//
//   Renames every loosely bound DB variable (>= depth) to a fresh
//   free variable using db_map. NB: Each fresh free variable
//   is bound back to corresponding loosely bound DB (- depth).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
Term_p unbind_loose(TB_p terms, IntMap_p db_map, long depth, Term_p t)
{
   assert(!TermIsLambda(t));
   Term_p res;
   if(!TermHasDBSubterm(t))
   {
      res = t;
   }
   else if (TermIsDBVar(t))
   {
      if(t->f_code < depth)
      {
         res = t;
      }
      else
      {
         Term_p* fvar_ref = (Term_p*)IntMapGetRef(db_map, t->f_code);
         if(!*fvar_ref)
         {
            *fvar_ref = VarBankGetFreshVar(terms->vars, t->type);
            (*fvar_ref)->binding =
               TBRequestDBVar(terms, t->type, t->f_code-depth);
         }
         assert ((*fvar_ref)->type == t->type);
         res = *fvar_ref;
      }
   }
   else
   {
      res = TermTopCopyWithoutArgs(t);
      bool changed = false;
      for(long i=0; i<res->arity; i++)
      {
         res->args[i] = unbind_loose(terms, db_map, depth, t->args[i]);
         changed = changed || res->args[i] != t->args[i];
      }

      if(changed)
      {
         res = TBTermTopInsert(terms, res);
      }
      else
      {
         TermTopFree(res);
         res = t;
      }
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: lift_lambda()
//
//   Convert lambda term: ^[...bound vars...]:s[...free vars...]
//   into a definiton f ..free vars.. ..bound vars.. = s
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p lift_lambda(TB_p terms, PStack_p bound_vars, Term_p body,
                   PStack_p definitions, PDTree_p liftings)
{
   //printf("### body (original): ");
   //TermPrintDbg(stdout, body, terms->sig, DEREF_NEVER);
   //printf("\n");
   Term_p lifted; // the result holding variable
   assert(!TermHasLambdaSubterm(body)); // LiftLambda recursively called before

   // storing all free variables of body in free_var_stack
   PTree_p free_vars = NULL;
   TFormulaCollectFreeVars(terms, body, &free_vars);
   PStack_p free_var_stack = PStackAlloc();
   PTreeToPStack(free_var_stack, free_vars);
   PTreeFree(free_vars);

   // creating a stack of fresh variables that correspond to the
   // the lambda prefix of the term
   PStack_p bound_to_fresh = PStackAlloc();
   for(long i=0; i<PStackGetSP(bound_vars); i++)
   {
      Type_p fresh_ty = ((Term_p)PStackElementP(bound_vars,i))->type;
      Term_p tmp = VarBankGetFreshVar(terms->vars, fresh_ty);
      PStackPushP(bound_to_fresh, tmp);
   }

   IntMap_p loosely_bound_to_fresh = IntMapAlloc();
   Term_p body_no_loose =
      unbind_loose(terms, loosely_bound_to_fresh, PStackGetSP(bound_vars), body);
   IntMapIter_p iter =  IntMapIterAlloc(loosely_bound_to_fresh, 0, LONG_MAX);

   // closed will be like body, but all loosely bound vars are replaced by
   // fresh vars
   Term_p closed = body_no_loose;

   //printf("### closed (fresh): ");
   //TermPrintDbg(stdout, closed, terms->sig, DEREF_NEVER);
   //printf("\n");

   for(long i=PStackGetSP(bound_vars)-1; i>=0; i--)
   {
      closed =
         CloseWithDBVar(terms, ((Term_p)PStackElementP(bound_vars, i))->type,
                        closed);
   }

   // unbind_loose binds FRESH variables to DB vars
   // As those are fresh, no variable in PDT should be bound.
   WFormula_p generalization = find_generalization(liftings, closed, &lifted);
   if(generalization)
   {
      long dummy;
      Term_p var;
      while((var = IntMapIterNext(iter, &dummy)))
      {
         var->binding=NULL;
      }
      PStackPushP(definitions, generalization);
   }
   else
   {
      PStack_p lb_stack_fresh_vars = PStackAlloc();
      PStack_p lb_stack_db_vars = PStackAlloc();
      long dummy;
      Term_p var;
      while((var = IntMapIterNext(iter, &dummy)))
      {
         PStackPushP(lb_stack_fresh_vars, var);
         PStackPushP(lb_stack_db_vars, var->binding);
         var->binding=NULL;
      }

      // next to free variables, additional argument to lambda name symbol
      // are fresh variables respresenting loosely bound vars and
      // fresh vars representing bound vars in the lambda prefix
      Type_p* lift_sym_ty_args = TypeArgArrayAlloc(PStackGetSP(lb_stack_db_vars));
      for(long i=0; i<PStackGetSP(lb_stack_db_vars); i++)
      {
         lift_sym_ty_args[i] = ((Term_p)PStackElementP(lb_stack_db_vars,i))->type;
      }
      Type_p res_ty =
         TypeBankInsertTypeShared(terms->sig->type_bank,
            ArrowTypeFlattened(lift_sym_ty_args,
                               PStackGetSP(lb_stack_db_vars), closed->type));
      TypeArgArrayFree(lift_sym_ty_args, PStackGetSP(lb_stack_db_vars));

      Term_p def_head =  TermAllocNewSkolem(terms->sig, free_var_stack, res_ty);
      def_head = TBTermTopInsert(terms, def_head);

      Term_p repl_t = ApplyTerms(terms, def_head, lb_stack_db_vars);
      Term_p lhs_wo_bound = ApplyTerms(terms, def_head, lb_stack_fresh_vars);
      Term_p repl_lhs = ApplyTerms(terms, lhs_wo_bound, bound_to_fresh);

      //printf("### closed: ");
      //TermPrintDbg(stdout, closed, terms->sig, DEREF_NEVER);
      //printf("\n");
      Term_p repl_rhs = WHNF_step(terms, ApplyTerms(terms, closed, bound_to_fresh));

      //printf("### repl_rhs: ");
      //TermPrintDbg(stdout, repl_rhs, terms->sig, DEREF_NEVER);
      //printf("\n");


      TFormula_p def_f;
      if(TypeIsBool(body->type))
      {
         def_f = TFormulaFCodeAlloc(terms, terms->sig->equiv_code,
                                    EncodePredicateAsEqn(terms, repl_lhs),
                                    EncodePredicateAsEqn(terms, repl_rhs));
      }
      else
      {
         def_f = TFormulaFCodeAlloc(terms, terms->sig->eqn_code, repl_lhs, repl_rhs);
      }

      for(long i=0; i<repl_lhs->arity; i++)
      {
         def_f = TFormulaAddQuantor(terms, def_f, true, repl_lhs->args[i]);
      }
      DBGTermCheckUnownedSubterm(stdout, def_f, "lift_lambda(def)");
      WFormula_p def = WTFormulaAlloc(terms, def_f);
      DocFormulaCreationDefault(def, inf_fof_intro_def, NULL, NULL);
      WFormulaPushDerivation(def, DCIntroDef, NULL, NULL);

      // NB: we are storing the definition of the kind
      // lift_name(FREE_VARS, LOOSE_BOUND_VARS) = %x. BODY
      store_lifting(liftings, lhs_wo_bound, closed, def);

      PStackPushP(definitions, def);

      PStackFree(lb_stack_fresh_vars);
      PStackFree(lb_stack_db_vars);
      lifted = repl_t;
   }

   IntMapIterFree(iter);
   IntMapFree(loosely_bound_to_fresh);
   PStackFree(free_var_stack);
   PStackFree(bound_to_fresh);
   DBGTermCheckUnownedSubterm(stdout, lifted, "lift_lambdaX");
   return lifted;
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: EncodePredicateAsEqn()
//
//   If a term is of the from p(s) where p is an uninterpreted predicate
//   symbol it will be converted to equation p(s) = T, to maintain
//   E's interal invariants
//
// Global Variables: -
//
// Side Effects    : Input, memory operations, changes term bank
//
/----------------------------------------------------------------------*/

Term_p EncodePredicateAsEqn(TB_p bank, TFormula_p f)
{
   Sig_p sig = bank->sig;
   bool positive;

   /* if(f->f_code == bank->sig->answer_code) */
   /* { */
   /*    printf("#X# %d: ", f->type == sig->type_bank->bool_type); */
   /*    TermPrint(stdout, f, bank->sig, DEREF_NEVER); */
   /*    printf("\n"); */
   /* } */

   if((TermIsAnyVar(f) ||
       !SigIsLogicalSymbol(bank->sig, f->f_code) ||
       f->f_code == bank->sig->answer_code ||
       f->f_code == SIG_TRUE_CODE ||
       f->f_code == SIG_FALSE_CODE ||
       f->f_code == SIG_ITE_CODE ||
       f->f_code == SIG_LET_CODE ||
       TermIsPhonyApp(f)) &&
      f->type == sig->type_bank->bool_type)
   {
      Term_p lside, rside;

      if(TermIsAnyVar(f))
      {
         lside = f;
         positive = true;
      }
      else
      {
         lside = f->f_code == SIG_FALSE_CODE ? bank->true_term : f;
         positive = f->f_code != SIG_FALSE_CODE;
      }
      rside = bank->true_term;

      // making sure we encode $false as $true!=$true
      f = EqnTermsTBTermEncode(bank, lside, rside, positive, PENormal);
      printf("xxx %ld - %ld: ", f->f_code, bank->sig->eqn_code);
      TermPrint(stdout, f, bank->sig, DEREF_NEVER);
      printf("\n");
   }
   return f;
}


/*-----------------------------------------------------------------------
//
// Function: TFormulaIsPropConst()
//
//   Return true iff the formula is the encoding of one of the
//   propositional constants i.e. $eqn($true,$true)$ (if posive is
//   true) or $neqn($true, $true).
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

bool TFormulaIsPropConst(Sig_p sig, TFormula_p form, bool positive)
{
   FunCode f_code = SigGetEqnCode(sig, positive);

   if(form->f_code!=f_code)
   {
      return false;
   }
   return (form->args[0]->f_code == SIG_TRUE_CODE)&&
      (form->args[1]->f_code == SIG_TRUE_CODE);
}

/*-----------------------------------------------------------------------
//
// Function: TFormulaFCodeAlloc()
//
//   Allocate a formula given an f_code and two subformulas (the
//   second one may be NULL).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

TFormula_p TFormulaFCodeAlloc(TB_p bank, FunCode op, TFormula_p arg1, TFormula_p arg2)
{
   Sig_p sig = bank->sig;
   int arity = SigFindArity(sig, op);
   TFormula_p res;

   assert(bank);
   assert((arity == 1) || (arity == 2));
   assert(EQUIV((arity==2), arg2));

   res = TermTopAlloc(op,arity);
   if(op != SIG_NAMED_LAMBDA_CODE)
   {
      res->type = sig->type_bank->bool_type;
   }
   if(SigIsPredicate(sig, op))
   {
      TermCellSetProp(res, TPPredPos);
   }
   if(arity > 0)
   {
      res->args[0] = arg1;
      if(arity > 1)
      {
         res->args[1] = arg2;
      }
   }
   assert(bank);
   res = TBTermTopInsert(bank, res);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: TFormulaLitAlloc()
//
//   Allocate a literal term formula. The equation is _not_ freed!
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

TFormula_p TFormulaLitAlloc(Eqn_p literal)
{
   TFormula_p res;
   TB_p bank = literal->bank;

   assert(literal);

   if(problemType == PROBLEM_FO)
   {
      res = EqnTermsTBTermEncode(literal->bank, literal->lterm,
                                 literal->rterm, EqnIsPositive(literal),
                                 PENormal);
   }
   else if(literal->rterm == bank->true_term)
   {
      res = DecodeFormulasForCNF(bank, literal->lterm);
      if(EqnIsNegative(literal))
      {
         res = TFormulaFCodeAlloc(bank, bank->sig->not_code, res, NULL);
      }
   }
   else if(EqnIsClausifiable(literal))
   {
      Term_p lterm = DecodeFormulasForCNF(bank, literal->lterm);
      Term_p rterm = DecodeFormulasForCNF(bank, literal->rterm);
      res =
         TFormulaFCodeAlloc(bank,
            EqnIsPositive(literal) ? bank->sig->equiv_code : bank->sig->xor_code,
            lterm, rterm);
   }
   else
   {
      Term_p lterm = DecodeFormulasForCNF(bank, literal->lterm);
      Term_p rterm = DecodeFormulasForCNF(bank, literal->rterm);
      res =
         EqnTermsTBTermEncode(bank, lterm, rterm, EqnIsPositive(literal), PENormal);
   }

   return res;

}

/*-----------------------------------------------------------------------
//
// Function: TFormulaPropConstantAlloc()
//
//   Allocate a formula representing a propositional constant (true or
//   false).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

TFormula_p TFormulaPropConstantAlloc(TB_p terms, bool positive)
{
   Eqn_p handle;
   TFormula_p res;

   handle = EqnAlloc(terms->true_term, terms->true_term, terms, positive);
   res =  TFormulaLitAlloc(handle);
   EqnFree(handle);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: TFormulaQuantorAlloc()
//
//   Allocate a formula with a quantor.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

TFormula_p TFormulaQuantorAlloc(TB_p bank, FunCode quantor, Term_p var, TFormula_p arg)
{
   assert(var);
   assert(TermIsFreeVar(var));
   assert(arg);

   return TFormulaFCodeAlloc(bank, quantor, var, arg);
}


/*-----------------------------------------------------------------------
//
// Function: tformula_print_or_chain()
//
//   Print a formula of |-connectect subformula as a flat list
//   without parentheses.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void tformula_print_or_chain(FILE* out, TB_p bank, TFormula_p form,
                              bool fullterms, bool pcl)
{
   if(form->f_code!=bank->sig->or_code)
   {
      TFormulaTPTPPrint(out, bank, form, fullterms, pcl);
   }
   else
   {
      tformula_print_or_chain(out, bank, form->args[0], fullterms, pcl);
      fputs("|", out);
      TFormulaTPTPPrint(out, bank, form->args[1], fullterms, pcl);
   }
}


/*-----------------------------------------------------------------------
//
// Function: tformula_appencode_or_chain()
//
//   Prints app-encoded version of the formula form to out.
//   Original formula is not chagned.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void tformula_appencode_or_chain(FILE* out, TB_p bank, TFormula_p form)
{
   if(form->f_code!=bank->sig->or_code)
   {
      TFormulaAppEncode(out, bank, form);
   }
   else
   {
      tformula_appencode_or_chain(out, bank, form->args[0]);
      fputs("|", out);
      TFormulaAppEncode(out, bank, form->args[1]);
   }
}


/*-----------------------------------------------------------------------
//
// Function: TFormulaTPTPPrint()
//
//   Print a formula in TPTP/TSTP format.
//
// Global Variables:
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void TFormulaTPTPPrint(FILE* out, TB_p bank, TFormula_p form, bool fullterms, bool pcl)
{
   assert(form);

   if(TFormulaIsLiteral(bank->sig, form))
   {
      Eqn_p tmp;

      assert(form->f_code == bank->sig->eqn_code ||
             form->f_code == bank->sig->neqn_code);

      tmp = EqnAlloc(form->args[0], form->args[1], bank, true);

      EqnFOFPrint(out, tmp, form->f_code == bank->sig->neqn_code, fullterms, pcl);
      EqnFree(tmp);
   }
   else if(TermIsFreeVar(form))
   {
      TermPrint(out, form, bank->sig, DEREF_NEVER);
   }
   else if(form->f_code == SIG_PHONY_APP_CODE)
   {
      fprintf(out, "(");
      TermPrint(out, form, bank->sig, DEREF_NEVER);
      fprintf(out, ")");
   }
   else if(TFormulaIsQuantified(bank->sig,form))
   {
      if(form->arity==2)
      {
         FunCode quantifier = form->f_code;
         if(form->f_code == bank->sig->qex_code)
         {
            fputs("?[", out);
         }
         else if(form->f_code == bank->sig->qall_code)
         {
            fputs("![", out);
         }
         else
         {
            fputs("^[", out);
         }
         TermPrint(out, form->args[0], bank->sig, DEREF_NEVER);
         if(problemType == PROBLEM_HO || !TypeIsIndividual(form->args[0]->type))
         {
            fputs(":", out);
            TypePrintTSTP(out, bank->sig->type_bank, form->args[0]->type);
         }
         while(form->args[1]->f_code == quantifier)
         {
            form = form->args[1];
            fputs(", ", out);
            TermPrint(out, form->args[0], bank->sig, DEREF_NEVER);
            if(problemType == PROBLEM_HO || !TypeIsIndividual(form->args[0]->type))
            {
               fputs(":", out);
               TypePrintTSTP(out, bank->sig->type_bank, form->args[0]->type);
            }
         }
         fputs("]:(", out);
         TFormulaTPTPPrint(out, bank, form->args[1], fullterms, pcl);
         fputs(")", out);
      }
      else
      {
         TermPrintDbg(out, form, bank->sig, DEREF_NEVER);
      }
   }
   else if(TFormulaIsUnary(form))
   {
      printf("#### ");
      TermPrintSExpr(out, form, bank->sig);
      printf("\n");
      assert(form->f_code == bank->sig->not_code);
      fputs("~(", out);
      TFormulaTPTPPrint(out, bank, form->args[0], fullterms, pcl);
      fputs(")", out);
   }
   else if(form->arity == 0)
   {
      fprintf(out, "%s", SigFindName(bank->sig, form->f_code));
   }
   else if(form->f_code == bank->sig->distinct_code)
   {
      TBPrintTerm(out, bank, form, fullterms);
   }
   else
   {
      char* oprep = "XXX";

      assert(form->arity);
      assert(form->f_code == SIG_LET_CODE || form->f_code == SIG_ITE_CODE ||
             TFormulaIsBinary(form));
      fputs("(", out);
      if(form->f_code == SIG_LET_CODE || form->f_code == SIG_ITE_CODE)
      {
         TermPrint(out, form, bank->sig, DEREF_NEVER);
      }
      else if(form->f_code == bank->sig->or_code)
      {
         tformula_print_or_chain(out, bank, form, fullterms, pcl);
      }
      else
      {
         TFormulaTPTPPrint(out, bank, form->args[0], fullterms, pcl);
         if(form->f_code == bank->sig->and_code)
         {
            oprep = "&";
         }
         else if(form->f_code == bank->sig->or_code)
         {
            oprep = "|";
         }
         else if(form->f_code == bank->sig->impl_code)
         {
            oprep = "=>";
         }
         else if(form->f_code == bank->sig->equiv_code)
         {
            oprep = "<=>";
         }
         else if(form->f_code == bank->sig->nand_code)
         {
            oprep = "~&";
         }
         else if(form->f_code == bank->sig->nor_code)
         {
         oprep = "~|";
         }
         else if(form->f_code == bank->sig->bimpl_code)
         {
            oprep = "<=";
         }
         else if(form->f_code == bank->sig->xor_code)
         {
            oprep = "<~>";
         }
         else
         {
            assert(false && "Wrong operator");
         }
         fputs(oprep, out);
         TFormulaTPTPPrint(out, bank, form->args[1], fullterms, pcl);
      }
      fputs(")", out);
   }
}


/*-----------------------------------------------------------------------
//
// Function: TFormulaAppEncode()
//
//   Appencodes TFormula and prints result to out.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void TFormulaAppEncode(FILE* out, TB_p bank, TFormula_p form)
{
   assert(form);

   if(TFormulaIsLiteral(bank->sig, form))
   {
      Eqn_p tmp;

      assert(form->f_code == bank->sig->eqn_code ||
             form->f_code == bank->sig->neqn_code);

      tmp = EqnAlloc(form->args[0], form->args[1], bank, true);

      EqnAppEncode(out, tmp, form->f_code == bank->sig->neqn_code);
      EqnFree(tmp);
   }
   else if(TFormulaIsQuantified(bank->sig,form))
   {
      FunCode quantifier = form->f_code;
      if(form->f_code == bank->sig->qex_code)
      {
         fputs("?[", out);
      }
      else
      {
         fputs("![", out);
      }
      assert(TermIsFreeVar(form->args[0]));
      VarPrint(out, form->args[0]->f_code);
      fputs(":", out);

      DStr_p type_name = TypeAppEncodedName(form->args[0]->type);

      fprintf(out, "%s", DStrView(type_name));

      DStrFree(type_name);


      while(form->args[1]->f_code == quantifier)
      {
         form = form->args[1];
         fputs(", ", out);

         assert(TermIsFreeVar(form->args[0]));
         VarPrint(out, form->args[0]->f_code);
         fputs(":", out);
         type_name = TypeAppEncodedName(form->args[0]->type);

         fprintf(out, "%s", DStrView(type_name));

         DStrFree(type_name);
      }
      fputs("]:", out);
      TFormulaAppEncode(out, bank, form->args[1]);
   }
   else if(TFormulaIsUnary(form))
   {
      assert(form->f_code == bank->sig->not_code);
      fputs("~(", out);
      TFormulaAppEncode(out, bank, form->args[0]);
      fputs(")", out);
   }
   else
   {
      char* oprep = "XXX";

      assert(TFormulaIsBinary(form));
      fputs("(", out);
      if(form->f_code == bank->sig->or_code)
      {
         tformula_appencode_or_chain(out, bank, form);
      }
      else
      {
         TFormulaAppEncode(out, bank, form->args[0]);
         if(form->f_code == bank->sig->and_code)
         {
            oprep = "&";
         }
         else if(form->f_code == bank->sig->or_code)
         {
            oprep = "|";
         }
         else if(form->f_code == bank->sig->impl_code)
         {
            oprep = "=>";
         }
         else if(form->f_code == bank->sig->equiv_code)
         {
            oprep = "<=>";
         }
         else if(form->f_code == bank->sig->nand_code)
         {
            oprep = "~&";
         }
         else if(form->f_code == bank->sig->nor_code)
         {
         oprep = "~|";
         }
         else if(form->f_code == bank->sig->bimpl_code)
         {
            oprep = "<=";
         }
         else if(form->f_code == bank->sig->xor_code)
         {
            oprep = "<~>";
         }
         else
         {
            assert(false && "Wrong operator");
         }
         fputs(oprep, out);
         TFormulaAppEncode(out, bank, form->args[1]);
      }
      fputs(")", out);
   }
}


/*-----------------------------------------------------------------------
//
// Function: PreloadTypes()
//
//   Make sure that all intermediate types needed for app-encoding of
//   the formula are already inserted in the type bank. For example
//   if type a > b > c > d appears in the type bank insert types
//   b > c > d and c > d to the type bank.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void PreloadTypes(TB_p bank, TFormula_p form)
{
   assert(form);

   if(TFormulaIsLiteral(bank->sig, form))
   {
      assert(form->f_code == bank->sig->eqn_code ||
             form->f_code == bank->sig->neqn_code);

      /* This would app encode the terms and create needed types */
      TermFree(TermAppEncode(form->args[0], bank->sig));
      TermFree(TermAppEncode(form->args[1], bank->sig));
   }
   else if(TFormulaIsQuantified(bank->sig,form))
   {
      PreloadTypes(bank, form->args[1]);
   }
   else if(TFormulaIsUnary(form))
   {
      PreloadTypes(bank, form->args[0]);
   }
   else
   {
      PreloadTypes(bank, form->args[0]);
      PreloadTypes(bank, form->args[1]);
   }
}


/*-----------------------------------------------------------------------
//
// Function: TFormulaTPTPParse()
//
//   Parse a formula in TPTP format.
//
// Global Variables: -
//
// Side Effects    : I/O, memory operations
//
/----------------------------------------------------------------------*/

TFormula_p TFormulaTPTPParse(Scanner_p in, TB_p terms)
{
   TFormula_p      f1, f2, res;
   FunCode op;
   f1 = elem_tform_tptp_parse(in, terms);
   if(TestInpTok(in, FOFBinOp))
   {
      op = tptp_operator_parse(terms->sig, in);
      f2 = TFormulaTPTPParse(in, terms);
      res = TFormulaFCodeAlloc(terms, op, f1, f2);
   }
   else
   {
      res = f1;
   }
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: TFormulaTSTPParse()
//
//   Parse a formula in TSTP formuat.
//
// Global Variables: -
//
// Side Effects    : I/O, memory operations
//
/----------------------------------------------------------------------*/

TFormula_p TFormulaTSTPParse(Scanner_p in, TB_p terms)
{
   TFormula_p      f1, f2, res;
   FunCode op;
   Sig_p   sig = terms->sig;

   f1 = literal_tform_tstp_parse(in, terms);
   if(TestInpTok(in, FOFAssocOp))
   {
      res = assoc_tform_tstp_parse(in, terms, f1);
   }
   else if(TestInpTok(in, Application))
   {
      res = applied_tform_tstp_parse(in, terms, f1);
   }
   else if(TestInpTok(in, FOFBinOp))
   {
      op = tptp_operator_parse(terms->sig, in);
      f2 = literal_tform_tstp_parse(in, terms);

      if(f1->type == sig->type_bank->bool_type &&
        (op == sig->eqn_code || op == sig->neqn_code))
      {
         assert(f2->type == sig->type_bank->bool_type);
         // if it is bool it is either a literal ((dis)equation) or a formula
         assert(SigIsLogicalSymbol(sig, f1->f_code));
         assert(SigIsLogicalSymbol(sig, f2->f_code));

         op = (op == sig->eqn_code) ? sig->equiv_code : sig->xor_code;
      }


      res = TFormulaFCodeAlloc(terms, op, f1, f2);
   }
   else
   {
      res = f1;
   }
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: TcfTSTPParse()
//
//   Parse a TCF formula (potentially typed clause) in TSTP format.
//
// Global Variables: -
//
// Side Effects    : I/O, memory operations
//
/----------------------------------------------------------------------*/

TFormula_p TcfTSTPParse(Scanner_p in, TB_p terms)
{
   TFormula_p res;

   CheckInpTok(in, TermStartToken|TildeSign|UnivQuantor|OpenBracket);

   bool in_parens = false;
   if(TestInpTok(in, OpenBracket))
   {
      AcceptInpTok(in, OpenBracket);
      in_parens = true;
   }
   if(TestInpTok(in, UnivQuantor))
   {
      FunCode quantor;
      quantor = tptp_quantor_parse(terms->sig,in);
      AcceptInpTok(in, OpenSquare);
      // printf(COMCHAR" Begin  quantified_tform_tstp_parse()\n");
      res = quantified_tform_tstp_parse(in, terms, quantor, true);
   }
   else
   {
      res = clause_tform_tstp_parse(in, terms);
   }
   if(in_parens)
   {
      AcceptInpTok(in, CloseBracket);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: parse_constant_term()
//
//   Parse a constant term (only constants allowed).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p parse_constant_term(Scanner_p in, TB_p terms)
{
   Term_p       handle;
   DStr_p       id = DStrAlloc();
   DStr_p       source_name = DStrGetRef(AktToken(in)->source);
   StreamType   type_stream = AktToken(in)->stream_type;
   long
      line   = AktToken(in)->line,
      column = AktToken(in)->column;

   FuncSymbType id_type = FuncSymbParse(in, id);
   FunCode      f_code = TermSigInsert(terms->sig,
                                       DStrView(id),
                                       0,
                                       false, id_type);
   if(!f_code)
   {
      Error("%s: constant expected but %s registered with arity %d",
            SYNTAX_ERROR,
            PosRep(type_stream, source_name, line, column),
            DStrView(id),
            (terms->sig)->
            f_info[SigFindFCode(terms->sig, DStrView(id))].arity);
   }
   DStrReleaseRef(source_name);

   handle = TermDefaultCellAlloc();
   handle->arity = 0;
   handle->f_code = f_code;
   handle = TBTermTopInsert(terms, handle);
   DStrFree(id);
   return handle;
}




/*-----------------------------------------------------------------------
//
// Function: TSTPDistinctParse()
//
//   Parse a $distinct()-pseudo-term.
//
// Global Variables: -
//
// Side Effects    : I/O, memory operation
//
/----------------------------------------------------------------------*/

TFormula_p TSTPDistinctParse(Scanner_p in, TB_p terms)
{
   Term_p        handle, arg;
   DStr_p        id = DStrAlloc();
   PStack_p      args = PStackAlloc();
   int           arity, i;
   Type_p        type;

   AcceptInpId(in, "$distinct");
   AcceptInpTok(in, OpenBracket);

   arg = parse_constant_term(in, terms);
   type = arg->type;
   PStackPushP(args, arg);

   DStrReset(id);
   while(TestInpTok(in, Comma))
   {
      NextToken(in);

      DStr_p       source_name = DStrGetRef(AktToken(in)->source);
      StreamType   type_stream = AktToken(in)->stream_type;
      long
         line   = AktToken(in)->line,
         column = AktToken(in)->column;

      arg = parse_constant_term(in, terms);
      if(arg->type!=type)
      {
         Error("%s All $distinct arguments have to be constants of the same type",
               TYPE_ERROR,
               PosRep(type_stream, source_name, line, column));
      }
      PStackPushP(args, arg);
      DStrReleaseRef(source_name);
      DStrReset(id);
   }
   AcceptInpTok(in, CloseBracket);
   arity = PStackGetSP(args);
   handle = TermDefaultCellArityAlloc(arity);
   handle->f_code = terms->sig->distinct_code;

   for(i=0;i<arity;i++)
   {
      handle->args[i] = PStackElementP(args,i);
   }
   handle = TBTermTopInsert(terms, handle);

   PStackFree(args);
   DStrFree(id);
   return handle;
}



/*-----------------------------------------------------------------------
//
// Function: TFormulaVarIsFree()
//
//   Return true iff var is a free variable in form.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool TFormulaVarIsFree(TB_p bank, TFormula_p form, Term_p var)
{
   bool res = false;
   int i;

   if(!form->v_count)
   {
      return false;
   }
   if(form==var)
   {
      return true;
   }
   if((form->f_code == bank->sig->qex_code) ||
           (form->f_code == bank->sig->qall_code))
   {
      if(form->args[0] == var)
      {
         res = false;
      }
      else
      {
         res = TFormulaVarIsFree(bank, form->args[1], var);
      }
   }
   else
   {
      for(i=0;  i<form->arity; i++)
      {
         res = TFormulaVarIsFree(bank, form->args[i], var);
         if(res)
         {
            break;
         }
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: TFormulaVarIsFreeCached()
//
//   Return true iff var is a free variable in form. Also cache the
//   local variable set in bank->freevarset.
//
//   Not really an improvement in the original use case, kept as a
//   historical recode...
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

/* bool TFormulaVarIsFreeCached(TB_p bank, TFormula_p form, Term_p var) */
/* { */
/*    VarSet_p free_vars = tform_compute_freevars(bank, form); */
/*    bool res; */

/*    assert(free_vars->valid); */

/*    res = VarSetContains(free_vars, var); */
/*    assert(res == TFormulaVarIsFree(bank, form, var)); */

/*    return res; */
/* } */


/*-----------------------------------------------------------------------
//
// Function: TFormulaCollectFreeVars()
//
//   Collect the _free_ variables in form in *vars.
//
// Global Variables: -
//
// Side Effects    : Memory operations, changes TPIsFreeVar.
//
/----------------------------------------------------------------------*/

void TFormulaCollectFreeVars(TB_p bank, TFormula_p form, PTree_p *vars)
{
   VarBankVarsSetProp(bank->vars, TPIsFreeVar);
   tformula_collect_freevars(bank, form, vars);
}

/*-----------------------------------------------------------------------
//
// Function: TFormulaIsClosed()
//
//   Returns true if forula has no free vars.
//
// Global Variables: -
//
// Side Effects    : Memory operations, changes TPIsFreeVar.
//
/----------------------------------------------------------------------*/

bool TFormulaIsClosed(TB_p bank, TFormula_p form)
{
   PTree_p vars = NULL;
   TFormulaCollectFreeVars(bank, form, &vars);
   bool ans  = vars == NULL;
   PTreeFree(vars);
   return ans;
}


/*-----------------------------------------------------------------------
//
// Function: TFormulaHasFreeVars()
//
//   Check if the formula has at least one free variable. If so,
//   return one of them, otherweise NULL.
//
// Global Variables: -
//
// Side Effects    : Changes TPIsFreeVar in variable cells.
//
/----------------------------------------------------------------------*/

TFormula_p TFormulaHasFreeVars(TB_p bank, TFormula_p form)
{
   TFormula_p res = NULL;
   PTree_p dummy = NULL;

   TFormulaCollectFreeVars(bank, form, &dummy);
   if (dummy!=NULL)
   {
      res = dummy->key;
   }
   PTreeFree(dummy);
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: TFormulaAddQuantor()
//
//   Given F and X, create !X.F or ?X.F. Requires F and X to be in the
//   term bank!
//
// Global Variables: -
//
// Side Effects    : (potentially) Changes term bank
//
/----------------------------------------------------------------------*/

TFormula_p TFormulaAddQuantor(TB_p bank, TFormula_p form, bool universal, Term_p var)
{
   FunCode quantor;
   TFormula_p new_form;

   quantor = universal?bank->sig->qall_code:bank->sig->qex_code;
   new_form = TFormulaFCodeAlloc(bank, quantor, var, form);

   return new_form;
}


/*-----------------------------------------------------------------------
//
// Function: TFormulaAddQuantors()
//
//   Given F and X1...Xn, create Q[X1...Xn]:F, where Q is ? or ! as
//   requested.
//
// Global Variables:
//
// Side Effects    : As TFormulaAddQuantor.
//
/----------------------------------------------------------------------*/

TFormula_p TFormulaAddQuantors(TB_p bank, TFormula_p form, bool universal,
                               PTree_p vars)
{
   PStack_p var_stack = PStackAlloc();
   PStackPointer i;
   Term_p var;

   PTreeToPStack(var_stack,vars);
   for(i=0; i<PStackGetSP(var_stack); i++)
   {
      var = PStackElementP(var_stack, i);
      form = TFormulaAddQuantor(bank, form, universal, var);
   }
   PStackFree(var_stack);
   return form;
}


/*-----------------------------------------------------------------------
//
// Function: TFormulaClosure()
//
//   Create the existential or universal closure of form.
//
// Global Variables: -
//
// Side Effects    : As TFormulaAddQuantor.
//
/----------------------------------------------------------------------*/

TFormula_p TFormulaClosure(TB_p bank, TFormula_p form, bool universal)
{
   PTree_p vars = NULL;

   VarBankVarsSetProp(bank->vars, TPIsFreeVar);
   TFormulaCollectFreeVars(bank, form, &vars);
   form = TFormulaAddQuantors(bank, form, universal, vars);
   PTreeFree(vars);

   return form;
}


/*-----------------------------------------------------------------------
//
// Function: TFormulaCreateDef()
//
//   Given an fresh, suitable atom, a formula, and the polarity,
//   return the correct defining formula.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

TFormula_p TFormulaCreateDef(TB_p bank, TFormula_p def_atom, TFormula_p defined,
                             int polarity)
{
   PTree_p vars=NULL;
   TFormula_p res = NULL;

   switch(polarity)
   {
   case -1:
         res = TFormulaFCodeAlloc(bank, bank->sig->impl_code, defined, def_atom);
         assert(!TermCellQueryProp(defined, TPPosPolarity));
         break;
   case 0:
         res = TFormulaFCodeAlloc(bank, bank->sig->equiv_code, def_atom, defined);
         break;
   case 1:
         res = TFormulaFCodeAlloc(bank, bank->sig->impl_code, def_atom, defined);
         assert(!TermCellQueryProp(defined, TPNegPolarity));
         break;
   default:
         assert(false && "Illegal polarity");
         break;
   }
   // This is correct - the atom is already created with the proper vars
   TermCollectVariables(def_atom, &vars);
   //TFormulaCollectFreeVars(bank, def_atom, &vars);
   res = TFormulaAddQuantors(bank, res, true, vars);
   PTreeFree(vars);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: TFormulaClauseEncode()
//
//   Given a clause, return a TFormula representing it. Quantors are
//   not added for the universal closure!
//
// Global Variables: -
//
// Side Effects    : Memory operations, changes bank
//
/----------------------------------------------------------------------*/

TFormula_p TFormulaClauseEncode(TB_p bank, Clause_p clause)
{
   TFormula_p res = NULL, tmp;
   Eqn_p      handle;

   assert(clause);
   if(ClauseIsEmpty(clause))
   {
      res = TFormulaPropConstantAlloc(bank, false);
   }
   else
   {
      //printf("Encoding: ");ClausePrintList(stdout, clause, true);printf("\n");
      res = TFormulaLitAlloc(clause->literals);
      for(handle = clause->literals->next;
          handle;
          handle = handle->next)
      {
         tmp = TFormulaLitAlloc(handle);
         res = TFormulaFCodeAlloc(bank, bank->sig->or_code, res, tmp);
      }
   }
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: TFormulaMarkPolarity()
//
//   For all subformulas of form, mark if they occur with positive
//   and/or negative polarity. Assumes that the properties are
//   properly reset!
//
// Global Variables: -
//
// Side Effects    :  -
//
/----------------------------------------------------------------------*/

void TFormulaMarkPolarity(TB_p bank, TFormula_p form, int polarity)
{
   assert((polarity<=1) && (polarity >=-1));

   if(TFormulaIsLiteral(bank->sig, form))
   {
      return;
   }
   switch(polarity)
   {
   case -1:
         TermCellSetProp(form, TPNegPolarity);
         break;
   case 0:
         TermCellSetProp(form, TPPosPolarity|TPNegPolarity);
         break;
   case 1:
         TermCellSetProp(form, TPPosPolarity);
         break;
   default:
         assert(false && "Impossible polarity in TFormulaMarkPolarity");
   }

   if((form->f_code == bank->sig->and_code)||
      (form->f_code == bank->sig->or_code))
   {
      TFormulaMarkPolarity(bank, form->args[0], polarity);
   }
   else if((form->f_code == bank->sig->not_code)||
           (form->f_code == bank->sig->impl_code))
   {
      TFormulaMarkPolarity(bank, form->args[0], -polarity);
   }
   else if(form->f_code == bank->sig->equiv_code)
   {
      TFormulaMarkPolarity(bank, form->args[0], 0);
   }
   /* Handle args[1] */
   if((form->f_code == bank->sig->and_code)||
      (form->f_code == bank->sig->or_code) ||
      (form->f_code == bank->sig->impl_code)||
      (form->f_code == bank->sig->qex_code)||
      (form->f_code == bank->sig->qall_code))
   {
      TFormulaMarkPolarity(bank, form->args[1], polarity);
   }
   else if(form->f_code == bank->sig->equiv_code)
   {
      TFormulaMarkPolarity(bank, form->args[1], 0);
   }
}


/*-----------------------------------------------------------------------
//
// Function: TFormulaDecodePolarity()
//
//   Return the polarity indicated by the polarity properties.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int TFormulaDecodePolarity(TB_p bank, TFormula_p form)
{
   if(TermCellQueryProp(form, TPPosPolarity|TPNegPolarity))
   {
      return 0;
   }
   if(TermCellQueryProp(form, TPPosPolarity))
   {
      return 1;
   }
   if(TermCellQueryProp(form, TPNegPolarity))
   {
      return -1;
   }
   //printf(COMCHAR" Formula without polarity: ");
   //TFormulaTPTPPrint(stdout, bank, form, true, false);
   //printf("\n");
   assert(false && "Formula without polarity !?!");
   return 0;
}


/*-----------------------------------------------------------------------
//
// Function: TFormulaClauseClosedEncode()
//
//   Generate a tform-representation of clause with explicit
//   universal quantification.
//
// Global Variables: -
//
// Side Effects    : Via TFormulaClauseEncode(), Memory operations
//
/----------------------------------------------------------------------*/

TFormula_p TFormulaClauseClosedEncode(TB_p bank, Clause_p clause)
{
   TFormula_p res = TFormulaClauseEncode(bank, clause);

   res = TFormulaClosure(bank, res, true);
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: TFormulaCollectClause()
//
//   Given a term-encoded formula that is a disjunction of literals,
//   transform it into a clause. If the optional parameter fresh_vars
//   is given, variables in the result will be normalized.
//
// Global Variables: -
//
// Side Effects    : Same as in TFormulaConjunctiveToCNF() below.
//
/----------------------------------------------------------------------*/

Clause_p TFormulaCollectClause(TFormula_p form, TB_p terms,
                               VarBank_p fresh_vars)
{
   Clause_p res;
   Eqn_p lit_list = NULL, tmp_list = NULL, lit;
   PStack_p stack, lit_stack = PStackAlloc();
   PStackPointer i;

   // fprintf(stderr, "tformula_collect_clause(): ");
   // TermPrintDbg(stderr, form, terms->sig, DEREF_NEVER);
   // fprintf(stderr, "\n");

   stack = PStackAlloc();
   PStackPushP(stack, form);
   while(!PStackEmpty(stack))
   {
      form = PStackPopP(stack);
      if(form->f_code == terms->sig->or_code)
      {
         PStackPushP(stack, form->args[0]);
         PStackPushP(stack, form->args[1]);
      }
      else
      {
         if(TFormulaIsLiteral(terms->sig, form))
         {
            lit = EqnTBTermDecode(terms, form);
            PStackPushP(lit_stack, lit);
         }
         else if(TermIsTrueTerm(form))
         {
            lit = EqnAlloc(form, form, terms, true);
            PStackPushP(lit_stack, lit);
         }
         else if(TermIsFalseTerm(form))
         {
            // Nothing to do - drop trivial literal
         }
      }
   }
   PStackFree(stack);
   //while(!PStackEmpty(lit_stack))
   //{
   //lit = PStackPopP(lit_stack);
   //EqnListInsertFirst(&lit_list, lit);
   //}
   for(i=0; i<PStackGetSP(lit_stack); i++)
   {
      lit = PStackElementP(lit_stack, i);
      EqnListInsertFirst(&lit_list, lit);
   }
   PStackFree(lit_stack);

   if(fresh_vars)
   {
      Subst_p  normsubst = SubstAlloc();
      VarBankResetVCounts(fresh_vars);
      NormSubstEqnList(lit_list, normsubst, fresh_vars);
      tmp_list = EqnListCopy(lit_list, terms);
      res = ClauseAlloc(tmp_list);
      EqnListFree(lit_list); /* Created just for this */
      SubstDelete(normsubst);
   }
   else
   {
      res = ClauseAlloc(lit_list);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: TFormulaIsUntyped
//
//   returns true if the formula only contains basic types (individual/prop)
//
// Global Variables: -
//
// Side Effects    : memory operations
//
/----------------------------------------------------------------------*/
bool TFormulaIsUntyped(TFormula_p form)
{
   return TermIsUntyped(form);
}

/*-----------------------------------------------------------------------
//
// Function: TFormulaNegate
//
//   If formula is literal, it negates the $(n)eq symbol. Otherwise,
//   if formula is \alpha, it returns \neg alpha
//
// Global Variables: -
//
// Side Effects    : memory operations
//
/----------------------------------------------------------------------*/

TFormula_p TFormulaNegate(TFormula_p form, TB_p terms)
{
   TFormula_p res = NULL;
   if(TFormulaIsLiteral(terms->sig, form))
   {
      FunCode f_code = SigGetOtherEqnCode(terms->sig, form->f_code);
      res = TFormulaFCodeAlloc(terms, f_code,
                               form->args[0],
                               form->args[1]);
   }
   else
   {
      res = TFormulaFCodeAlloc(terms, terms->sig->not_code,
                               form, NULL);
   }

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: TFormulaStackToForm()
//
//   Given a stack of formulas, combine them into a formula conjoined
//   by the given op. I'm to tired to think about structure, so better
//   use only conjunction and disjunction here ;-)
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

TFormula_p TFormulaStackToForm(TB_p bank, PStack_p stack, FunCode op)
{
   TFormula_p res, handle;

   if(PStackEmpty(stack))
   {
      return bank->true_term;
   }
   res = PStackPopP(stack);
   while(!PStackEmpty(stack))
   {
      handle = PStackPopP(stack);
      res = TFormulaFCodeAlloc(bank, op, handle, res);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: TFormulaExpandDistinct()
//
//   Create a conjunction of disequations expressing a $distinct
//   statment: $distinct(a,b,c) => neqn(a,b)&neqn(b,c)&neqn(a,c).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

TFormula_p TFormulaExpandDistinct(TB_p bank, TFormula_p distinct)
{
   int i,j;
   PStack_p disequs = PStackAlloc();
   TFormula_p handle;

   for(i=0; i<distinct->arity; i++)
   {
      for(j=i+1; j<distinct->arity; j++)
      {
         handle = TFormulaFCodeAlloc(bank, bank->sig->neqn_code,
                                     distinct->args[i],
                                     distinct->args[j]);

         PStackPushP(disequs, handle);
      }
   }
   handle = TFormulaStackToForm(bank, disequs, bank->sig->and_code);
   PStackFree(disequs);

   return handle;
}



/*-----------------------------------------------------------------------
//
// Function: LiftLambdas()
//
//   Turns equation (^[X]:s)=t into ![X]:(s = (t @ X))
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

TFormula_p LiftLambdas(TB_p terms, TFormula_p t, PStack_p definitions, PDTree_p liftings)
{
   Term_p res;

   PStack_p vars = NULL;
   //printf("### LiftLambdas: ");
   //TermPrintDbg(stdout, t, terms->sig, DEREF_NEVER);

   t = BetaNormalizeDB(terms, t);

   //fprintf(stdout, " => ");
   //TermPrintDbg(stdout, t, terms->sig, DEREF_NEVER);
   //fprintf(stdout, "\n");

   DBGTermCheckUnownedSubterm(stdout, t, "UnownedLL");
   if(TermIsLambda(t))
   {
      vars = PStackAlloc();
      t = UnfoldLambda(t, vars);
   }

   if(!TermHasLambdaSubterm(t))
   {
      res = t;
   }
   else
   {
      res = TermTopCopyWithoutArgs(t);
      for(int i=0; i<t->arity; i++)
      {
         res->args[i] = LiftLambdas(terms, t->args[i], definitions, liftings);
      }

      res = TBTermTopInsert(terms, res);
      assert(res != t);
   }


   if(vars)
   {
      res = lift_lambda(terms, vars, res, definitions, liftings);
      PStackFree(vars);
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: Lambda2Forall()
//
//   Turns equation (^[X]:s)=t into ![X]:(s = (t @ X))
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

TFormula_p LambdaToForall(TB_p terms, TFormula_p t)
{
   VarBankSetVCountsToUsed(terms->vars);
   return TermMap(terms, t, lambda_eq_to_forall);
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
