/*-----------------------------------------------------------------------

File  : ccl_tformulae.c

Author: Stephan Schulz

Contents

  Code for the full first order formulae encoded as terms.

  Copyright 2005 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
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

TFormula_p elem_tform_tptp_parse(Scanner_p in, TB_p terms);

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/



/*-----------------------------------------------------------------------
//
// Function: tptp_operator_parse_fun()
//
//   Parse a TPTP operator and return the corresponding f_code. Rather
//   trivial ;-) 
//
// Global Variables: -
//
// Side Effects    : Input
//
/----------------------------------------------------------------------*/

FunCode tptp_operator_parse_fun(Sig_p sig, Scanner_p in)
{
   FunCode res=0;

   CheckInpTok(in, FOFBinOp);
   switch(AktTokenType(in))
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
   NextToken(in);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: tptp_quantor_parse_fun()
//
//   Parse and return a TPTP quantor. Rather trivial ;-)
//
// Global Variables: -
//
// Side Effects    : Input
//
/----------------------------------------------------------------------*/

FunCode tptp_quantor_parse_fun(Sig_p sig, Scanner_p in)
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

TFormula_p quantified_tform_tptp_parse(Scanner_p in, 
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
      rest = quantified_tform_tptp_parse(in, terms, quantor);
   }
   else
   {
      AcceptInpTok(in, CloseSquare);
      AcceptInpTok(in, Colon);      
      rest = elem_tform_tptp_parse(in, terms);
   }
   res = TFormulaFCodeAlloc(terms, quantor, var, rest);
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

TFormula_p elem_tform_tptp_parse(Scanner_p in, TB_p terms)
{
   TFormula_p res, tmp;
   
   if(TestInpTok(in, AllQuantor|ExistQuantor))
   {
      FunCode quantor;
      quantor = tptp_quantor_parse_fun(terms->sig,in);
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



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/



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
   if(SigIsPredicate(bank->sig, op))
   {
      TermCellSetProp(res, TPPredPos);
   }
   res->args[0] = arg1;
   res->args[1] = arg2;
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
      TermPrint(out, form->args[0], NULL, DEREF_NEVER);
      fputs("]:", out);
      TFormulaTPTPPrint(out, bank, form->args[1], fullterms, fullterms);
   }
   else if(TFormulaIsUnary(form))
   {
      assert(form->f_code == bank->sig->not_code);
      fputs("~(", out);
      TFormulaTPTPPrint(out, bank, form->args[0], fullterms, fullterms);
      fputs(")", out);
   }
   else
   {
      char* oprep = "XXX";

      assert(TFormulaIsBinary(form));
      fputs("(", out);
      TFormulaTPTPPrint(out, bank, form->args[0], fullterms, fullterms);
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
      TFormulaTPTPPrint(out, bank, form->args[1], fullterms, fullterms);      
      fputs(")", out);      
   }   
}


/*-----------------------------------------------------------------------
//
// Function: TFormulaTPTPParse()
//
//   Parse a formula in TSTP/TPTP formula.
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
      op = tptp_operator_parse_fun(terms->sig, in);
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
      res = TermIsSubterm(form, var, DEREF_NEVER, TBTermEqual);
   }
   else if((form->f_code == bank->sig->qex_code) ||
           (form->f_code == bank->sig->qall_code))
   {
      if(TBTermEqual(form->args[0], var))
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
   PTree_p vars;

   TFormulaCollectFreeVars(bank, form, &vars);
   form = TFormulaAddQuantors(bank, form, universal, vars);
   PTreeFree(vars);
   
   return form;
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


