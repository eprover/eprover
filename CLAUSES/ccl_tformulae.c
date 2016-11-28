/*-----------------------------------------------------------------------

File  : ccl_tformulae.c

Author: Stephan Schulz

Contents

  Code for the full first order formulae encoded as terms.

  Copyright 2005 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Thu May 19 17:26:49 CEST 2005
    New (some taken from old implementation (ccl_formulae.c)

-----------------------------------------------------------------------*/

#include "ccl_tformulae.h"



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
// Function: tptp_operator_convert()
//
//   R eturn the f_code corresponding to a given token. Rather
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
   case FOFXor:
         res = sig->xor_code;
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

   CheckInpTok(in, AllQuantor|ExistQuantor);
   if(TestInpTok(in, ExistQuantor))
   {
      res = sig->qex_code;
   }
   else
   {
      res = sig->qall_code;
   }
   NextToken(in);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: quantified_form_tptp_parse()
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
   DStr_p     source_name, errpos;
   long       line, column;
   StreamType type;

   line = AktToken(in)->line;
   column = AktToken(in)->column;
   source_name = DStrGetRef(AktToken(in)->source);
   type = AktToken(in)->stream_type;

   /* Enter a new scope for variables (exit scope before exiting function) */
   VarBankPushEnv(terms->vars);

   var = TBTermParse(in, terms);
   if(!TermIsVar(var))
   {
      errpos = DStrAlloc();

      DStrAppendStr(errpos, PosRep(type, source_name, line, column));
      DStrAppendStr(errpos, " Variable expected, non-variable term found");
      Error(DStrView(errpos), SYNTAX_ERROR);
      DStrFree(errpos);
   }
   assert(var->sort != STNoSort);
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

   if(TestInpTok(in, AllQuantor|ExistQuantor))
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
      Eqn_p lit;
      lit = EqnFOFParse(in, terms);
      res = TFormulaLitAlloc(lit);
      EqnFree(lit);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: quantified_form_tstp_parse()
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
                                              FunCode quantor)
{
   Term_p     var;
   TFormula_p  rest, res;
   DStr_p     source_name, errpos;
   long       line, column;
   StreamType type;

   line = AktToken(in)->line;
   column = AktToken(in)->column;
   source_name = DStrGetRef(AktToken(in)->source);
   type = AktToken(in)->stream_type;

   /* Enter a new scope for variables (exit scope before exiting function) */
   VarBankPushEnv(terms->vars);

   var = TBTermParse(in, terms);
   if(!TermIsVar(var))
   {
      errpos = DStrAlloc();

      DStrAppendStr(errpos, PosRep(type, source_name, line, column));
      DStrAppendStr(errpos, " Variable expected, non-variable term found");
      Error(DStrView(errpos), SYNTAX_ERROR);
      DStrFree(errpos);
   }
   DStrReleaseRef(source_name);
   if(TestInpTok(in, Comma))
   {
      AcceptInpTok(in, Comma);
      rest = quantified_tform_tstp_parse(in, terms, quantor);
   }
   else
   {
      AcceptInpTok(in, CloseSquare);
      AcceptInpTok(in, Colon);
      rest = literal_tform_tstp_parse(in, terms);
   }
   res = TFormulaFCodeAlloc(terms, quantor, var, rest);

   VarBankPopEnv(terms->vars);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: literal_tform_tstp_parse()
//
//   Parse an elementary formula in TSTP format.
//
// Global Variables: -
//
// Side Effects    : I/O
//
/----------------------------------------------------------------------*/

static TFormula_p literal_tform_tstp_parse(Scanner_p in, TB_p terms)
{
   TFormula_p res, tmp;

   if(TestInpTok(in, AllQuantor|ExistQuantor))
   {
      FunCode quantor;
      quantor = tptp_quantor_parse(terms->sig,in);
      AcceptInpTok(in, OpenSquare);
      res = quantified_tform_tstp_parse(in, terms, quantor);
   }
   else if(TestInpTok(in, OpenBracket))
   {
      AcceptInpTok(in, OpenBracket);
      res = TFormulaTSTPParse(in, terms);
      AcceptInpTok(in, CloseBracket);
   }
   else if(TestInpTok(in, TildeSign))
   {
      AcceptInpTok(in, TildeSign);
      tmp = literal_tform_tstp_parse(in, terms);
      res = TFormulaFCodeAlloc(terms, terms->sig->not_code, tmp, NULL);
   }
   else
   {
      Eqn_p lit;
      lit = EqnFOFParse(in, terms);
      res = TFormulaLitAlloc(lit);
      EqnFree(lit);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: assoc_form_tstp_parse()
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

VarSet_p tform_compute_freevars(TB_p bank, TFormula_p form)
{
   VarSet_p free_vars = VarSetStoreGetVarSet(&(bank->freevarsets), form);
   VarSet_p arg_vars;

   if(!free_vars->valid)
   {
      // printf("Computing for %p - ", form);
      if(TFormulaIsLiteral(bank->sig, form))
      {
         // printf("literal\n");
         VarSetCollectVars(free_vars);
      }
      else if((form->f_code == bank->sig->qex_code) ||
              (form->f_code == bank->sig->qall_code))
      {
         // printf("quantified\n");
         arg_vars = tform_compute_freevars(bank, form->args[1]);
         VarSetInsertVarSet(free_vars, arg_vars);
         VarSetDeleteVar(free_vars, form->args[0]);
      }
      else
      {
         int i;

         // printf("composite\n");
         for(i=0;  i<form->arity; i++)
         {
            arg_vars = tform_compute_freevars(bank, form->args[i]);
            VarSetInsertVarSet(free_vars, arg_vars);
         }
      }
      free_vars->valid = true;
   }
   return free_vars;
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function:
//
//
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
   int arity = SigFindArity(bank->sig, op);
   TFormula_p res;

   assert(bank);
   assert((arity == 1) || (arity == 2));
   assert(EQUIV((arity==2), arg2));

   res = TermTopAlloc(op,arity);
   res->sort = STBool;
   if(SigIsPredicate(bank->sig, op))
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

   assert(literal);

   res = EqnTermsTBTermEncode(literal->bank, literal->lterm,
                               literal->rterm, EqnIsPositive(literal),
                               PENormal);
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
   assert(TermIsVar(var));
   assert(arg);

   return TFormulaFCodeAlloc(bank, quantor, var, arg);
}

/*-----------------------------------------------------------------------
//
// Function: TFormulaTPTPPrint()
//
//   Print a formula in TPTP/TSTP format.
//
// Global Variables:
//
// Side Effects    :
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
   else if(TFormulaIsQuantified(bank->sig,form))
   {
      if(form->f_code == bank->sig->qex_code)
      {
         fputs("?[", out);
      }
      else
      {
         fputs("![", out);
      }
      TermPrint(out, form->args[0], bank->sig, DEREF_NEVER);
      if(form->args[0]->sort != STIndividuals)
      {
         fputs(":", out);
         SortPrintTSTP(out, bank->sig->sort_table, form->args[0]->sort);
      }

      fputs("]:", out);
      TFormulaTPTPPrint(out, bank, form->args[1], fullterms, pcl);
   }
   else if(TFormulaIsUnary(form))
   {
      assert(form->f_code == bank->sig->not_code);
      fputs("~(", out);
      TFormulaTPTPPrint(out, bank, form->args[0], fullterms, pcl);
      fputs(")", out);
   }
   else
   {
      char* oprep = "XXX";

      assert(TFormulaIsBinary(form));
      fputs("(", out);
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
      fputs(")", out);
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

   f1 = literal_tform_tstp_parse(in, terms);

   if(TestInpTok(in, FOFAssocOp))
   {
      res = assoc_tform_tstp_parse(in, terms, f1);
   }
   else if(TestInpTok(in, FOFBinOp))
   {
      op = tptp_operator_parse(terms->sig, in);
      f2 = literal_tform_tstp_parse(in, terms);
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

   if(TFormulaIsLiteral(bank->sig, form))
   {
      res = TBTermIsSubterm(form, var);
   }
   else if((form->f_code == bank->sig->qex_code) ||
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

bool TFormulaVarIsFreeCached(TB_p bank, TFormula_p form, Term_p var)
{
   VarSet_p free_vars = tform_compute_freevars(bank, form);
   bool res;

   assert(free_vars->valid);

   res = VarSetContains(free_vars, var);
   assert(res == TFormulaVarIsFree(bank, form, var));

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: TFormulaCollectFreeVars()
//
//   Collect the _free_ variables in form in *vars. This is somewhat
//   tricky. We require that initially all variables have TPIsFreeVar
//   set.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

void TFormulaCollectFreeVars(TB_p bank, TFormula_p form, PTree_p *vars)
{
   TermProperties old_prop;

   if(TFormulaIsQuantified(bank->sig, form))
   {
      old_prop = TermCellGiveProps(form->args[0], TPIsFreeVar);
      TermCellDelProp(form->args[0], TPIsFreeVar);
      TFormulaCollectFreeVars(bank, form->args[1], vars);
      TermCellSetProp(form->args[0], old_prop);
   }
   else if(TFormulaIsLiteral(bank->sig, form))
   {
      TermCollectPropVariables(form, vars, TPIsFreeVar);
   }
   else
   {
      if(TFormulaHasSubForm1(bank->sig,form))
      {
         TFormulaCollectFreeVars(bank, form->args[0], vars);
      }
      if(TFormulaHasSubForm2(bank->sig, form))
      {
         TFormulaCollectFreeVars(bank, form->args[1], vars);
      }
   }
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
   TermCollectVariables(def_atom, &vars);
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
   //printf("# Formula without polarity: ");
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

   /*printf("tformula_collect_clause(): ");
     TFormulaTPTPPrint(GlobalOut, terms, form, true);
     printf("\n"); */

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
         assert(TFormulaIsLiteral(terms->sig, form));
         lit = EqnTBTermDecode(terms, form);
         PStackPushP(lit_stack, lit);

      }
   }
   PStackFree(stack);
   while(!PStackEmpty(lit_stack))
   {
      lit = PStackPopP(lit_stack);
      EqnListInsertFirst(&lit_list, lit);
   }
   PStackFree(lit_stack);

   if(fresh_vars)
   {
      Subst_p  normsubst = SubstAlloc();
      VarBankResetVCount(fresh_vars);
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


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
