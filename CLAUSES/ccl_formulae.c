/*-----------------------------------------------------------------------

File  : ccl_formulae.c

Author: Stephan Schulz

Contents

  Code for the full first order datatype.

  Copyright 2003 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Wed Nov  5 10:18:51 GMT 2003
    New

-----------------------------------------------------------------------*/

#include "ccl_formulae.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/

Formula_p elem_form_tptp_parse(Scanner_p in, TB_p terms);

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/



/*-----------------------------------------------------------------------
//
// Function: tptp_operator_parse()
//
//   Parse and return a TPTP operator. Rather trivial ;-)
//
// Global Variables: -
//
// Side Effects    : Input
//
/----------------------------------------------------------------------*/

FOFOperatorType tptp_operator_parse(Scanner_p in)
{
   FOFOperatorType res=OpNoOp;

   CheckInpTok(in, FOFBinOp);
   switch(AktTokenType(in))
   {
   case FOFOr:
         res = OpBOr;
         break;
   case FOFAnd:
         res = OpBAnd;
         break;
   case FOFLRImpl:
         res = OpBImpl;
         break;
   case FOFRLImpl:
         res = OpBNImpl;
         break;
   case FOFEquiv:
         res = OpBEquiv;
         break;
   case FOFXor:
         res = OpBXor;
         break;
   case FOFNand:
         res = OpBNand;
         break;
   case FOFNor:
         res = OpBNor;
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
// Function: tptp_quantor_parse()
//
//   Parse an return a TPTP quantor. Rather trivial ;-)
//
// Global Variables: -
//
// Side Effects    : Input
//
/----------------------------------------------------------------------*/

FOFOperatorType tptp_quantor_parse(Scanner_p in)
{
   FOFOperatorType res;

   CheckInpTok(in, AllQuantor|ExistQuantor);
   if(TestInpTok(in, ExistQuantor))
   {
      res = OpQEx;
   }
   else
   {
      res = OpQAll;      
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

Formula_p quantified_form_tptp_parse(Scanner_p in, 
                                     TB_p terms, 
                                     FOFOperatorType quantor)
{
   Term_p     var;
   Formula_p  rest, res;
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
      rest = quantified_form_tptp_parse(in, terms, quantor);
   }
   else
   {
      AcceptInpTok(in, CloseSquare);
      AcceptInpTok(in, Colon);      
      rest = elem_form_tptp_parse(in, terms);
   }
   res = FormulaQuantorAlloc(quantor, var, rest);
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: elem_form_tptp_parse()
//
//   Parse an elementary formula in TPTP/TSTP format.
//
// Global Variables: -
//
// Side Effects    : I/O
//
/----------------------------------------------------------------------*/

Formula_p elem_form_tptp_parse(Scanner_p in, TB_p terms)
{
   Formula_p res, tmp;
   
   if(TestInpTok(in, AllQuantor|ExistQuantor))
   {
      FOFOperatorType quantor;
      quantor = tptp_quantor_parse(in);
      AcceptInpTok(in, OpenSquare);
      res = quantified_form_tptp_parse(in, terms, quantor);
   }
   else if(TestInpTok(in, OpenBracket))
   {
      AcceptInpTok(in, OpenBracket);
      res = FormulaTPTPParse(in, terms);
      AcceptInpTok(in, CloseBracket);
   }
   else if(TestInpTok(in, TildeSign))
   {
      AcceptInpTok(in, TildeSign);
      tmp = elem_form_tptp_parse(in, terms);
      res = FormulaOpAlloc(OpUNot, tmp, NULL);
   }
   else 
   {
      Eqn_p lit;
      lit = EqnFOFParse(in, terms);
      res = FormulaLitAlloc(lit);
   }
   return res;
}



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: FormulaAlloc()
//
//   Return an empty, initialized FormulaCell.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Formula_p FormulaAlloc(void)
{
   Formula_p res = FormulaCellAlloc();
   
   res->special.var = NULL;
   res->ref_count = 0;
   res->arg1 = NULL;
   res->arg2 = NULL;
   return res;
}
   


/*-----------------------------------------------------------------------
//
// Function: FormulaFree()
//
//   Free a formula and its subformulae. Does free the real literals,
//   but not terms or variables, which are supposed to be shared.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void FormulaFree(Formula_p form)
{
   assert(form);
   assert(form->ref_count>=0);

   if(form->ref_count)
   {
      return;
   }
   if(FormulaIsLiteral(form))
   {
      EqnFree(form->special.literal);
   }
   else
   {
      if(FormulaHasSubForm1(form))
      {
	 FormulaRelRef(form->arg1);
         FormulaFree(form->arg1);
      }
      if(FormulaHasSubForm2(form))
      {
	 FormulaRelRef(form->arg2);
         FormulaFree(form->arg2);
      }    
   }
   FormulaCellFree(form);
}


/*-----------------------------------------------------------------------
//
// Function: FormulaOpAlloc()
//
//   Allocate a formula given two subformulas (the second one may be
//   NULL) and an operator.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Formula_p FormulaOpAlloc(FOFOperatorType op, Formula_p arg1, Formula_p arg2)
{
   Formula_p res = FormulaAlloc();
   
   assert(OpIsUnary(op)||OpIsBinary(op));
   assert(EQUIV(OpIsBinary(op),arg2));
   
   res->op = op;
   res->special.var = NULL;
   res->arg1 = FormulaGetRef(arg1);
   res->arg2 = FormulaGetRef(arg2);

   return res;          
}


/*-----------------------------------------------------------------------
//
// Function: FormulaLitAlloc()
//
//   Allocate a literal formula.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Formula_p FormulaLitAlloc(Eqn_p literal)
{
   Formula_p res = FormulaAlloc();
   
   assert(literal);

   res->op = OpIsLit;
   res->special.literal =literal;
   res->arg1 = NULL;
   res->arg2 = NULL;

   return res;          

}

/*-----------------------------------------------------------------------
//
// Function: FormulaPropConstantAlloc() 
//
//   Allocate a formula representing a propositional constant (true or
//   false). 
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Formula_p FormulaPropConstantAlloc(TB_p terms, bool positive)
{
   Eqn_p handle;

   handle = EqnAlloc(terms->true_term, terms->true_term, terms, positive);
   return FormulaLitAlloc(handle);
}


/*-----------------------------------------------------------------------
//
// Function: FormulaQuantorAlloc()
//
//   Allocate a formula with a quantor.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Formula_p FormulaQuantorAlloc(FOFOperatorType quantor, Term_p var, Formula_p arg1)
{
   Formula_p res = FormulaAlloc();
   
   assert(OpIsQuantor(quantor));
   assert(var);
   assert(TermIsVar(var));
   assert(arg1);

   res->op = quantor;
   res->special.var = var;
   res->arg1 = FormulaGetRef(arg1);
   res->arg2 = NULL;

   return res;
}   

/*-----------------------------------------------------------------------
//
// Function: FormulaTPTPPrint()
//
//   Print a formula in TPTP/TSTP format.
//
// Global Variables: 
//
// Side Effects    : 
//
/----------------------------------------------------------------------*/

void FormulaTPTPPrint(FILE* out, Formula_p form, bool fullterms, bool pcl)
{
   assert(form);

   if(FormulaIsLiteral(form))
   {      
      EqnFOFPrint(out, form->special.literal, false, fullterms, pcl);
   }
   else if(FormulaIsQuantified(form))
   {
      switch(form->op)
      {
      case OpQEx:
            fputs("?[", out);
            break;
      case OpQAll:
            fputs("![", out);
            break;
      default:
            assert(false && "Wrong quantor");
      }
      TermPrint(out, form->special.var, NULL, DEREF_NEVER);
      fputs("]:", out);
      FormulaTPTPPrint(out, form->arg1, fullterms, pcl);
   }
   else if(FormulaIsUnary(form))
   {
      assert(form->op == OpUNot);
      if(true /*FormulaIsBinary(form->arg1)*/)
      {
	 fputs("~(", out);
	 FormulaTPTPPrint(out, form->arg1, fullterms, pcl);
	 fputs(")", out);
      }
      else
      {
	 fputc('~', out);
	 FormulaTPTPPrint(out, form->arg1, fullterms, pcl);
      }
   }
   else
   {
      char* oprep = "XXX";

      assert(FormulaIsBinary(form));
      fputs("(", out);
      FormulaTPTPPrint(out, form->arg1, fullterms, pcl);
      switch(form->op)
      {
      case OpBAnd:
            oprep = "&";
            break;
      case OpBOr:
            oprep = "|";
            break;
      case OpBImpl:
            oprep = "=>";
            break;
      case OpBEquiv:
            oprep = "<=>";
            break;
      case OpBNand:
            oprep = "~&";
            break;
      case OpBNor:
            oprep = "~|";           
            break;
      case OpBNImpl:
            oprep = "<=";           
            break;
      case OpBXor:
            oprep = "<~>";
            break;            
      default:
            assert(false && "Wrong operator");
      }
      fputs(oprep, out);
      FormulaTPTPPrint(out, form->arg2, fullterms, pcl);
      fputs(")", out);      
   }   
}



/*-----------------------------------------------------------------------
//
// Function: FormulaTPTPParse()
//
//   Parse a formula in TPTP-2 format..
//
// Global Variables: -
//
// Side Effects    : I/O, memory operations
//
/----------------------------------------------------------------------*/

Formula_p FormulaTPTPParse(Scanner_p in, TB_p terms)
{
   Formula_p      f1, f2, res;
   FOFOperatorType op;
   f1 = elem_form_tptp_parse(in, terms);   
   if(TestInpTok(in, FOFBinOp))
   {
      op = tptp_operator_parse(in);
      f2 = FormulaTPTPParse(in, terms);
      res = FormulaOpAlloc(op, f1, f2);
   }
   else
   {
      res = f1;
   }
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: FormulaTSTPParse()
//
//   Parse a formula in TPTP-3/TSTP format. This now differs from the
//   above since the requirements for parenthesis are stricter.
//
// Global Variables: -
//
// Side Effects    : I/O, memory operations
//
/----------------------------------------------------------------------*/

Formula_p FormulaTSTPParse(Scanner_p in, TB_p terms)
{
   Formula_p      f1, f2, res;
   FOFOperatorType op, op_old;
   f1 = elem_form_tptp_parse(in, terms);   
   if(TestInpTok(in, FOFBinOp))
   {
      op = tptp_operator_parse(in);
      f2 = FormulaTSTPParse(in, terms);
      res = FormulaOpAlloc(op, f1, f2);
   }
   else
   {
      res = f1;
   }
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: FormulaEqual()
//
//   Return true if form1 and form2 are equal. Terms are supposed to
//   be shared. We only check syntactic equality and ignore AC here.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool FormulaEqual(Formula_p form1, Formula_p form2)
{
   bool res = false;

   /* printf("FormulaEqual()...\n"); */

   if(form1 == form2)
   {
      return true;
   }
   if(form1->op != form2->op)
   {
      return false;
   }
   switch(form1->op)
   {
   case OpIsLit:
         res = LiteralEqual(form1->special.literal, 
                            form2->special.literal, 
                            TBTermEqual);
         break;
   case OpUNot:
         res = FormulaEqual(form1->arg1, form2->arg1);
         break;
   case OpQEx:
   case OpQAll:
         res = TBTermEqual(form1->special.var, form2->special.var)
            && FormulaEqual(form1->arg1, form2->arg1);
         break;
   case OpBAnd:
   case OpBOr:
   case OpBImpl:
   case OpBEquiv:
   case OpBNand:
   case OpBNor:
   case OpBNImpl:
   case OpBXor:
         res = FormulaEqual(form1->arg1, form2->arg1)
           && FormulaEqual(form1->arg2, form2->arg2);
         break; 
   default:
         assert(false && "Illegal operator in formula");
         break;
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: FormulaVarIsFree()
//
//   Return true iff var is a free variable in form.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool FormulaVarIsFree(Formula_p form, Term_p var)
{
   bool res = false;

   switch(form->op)
   {
   case OpIsLit:
         res = TermIsSubterm(form->special.literal->lterm, var, 
                             DEREF_NEVER, TBTermEqual)
            ||TermIsSubterm(form->special.literal->rterm, var, 
                            DEREF_NEVER, TBTermEqual);         
         break;
   case OpUNot:
         res = FormulaVarIsFree(form->arg1, var);
         break;
   case OpQEx:
   case OpQAll:
         if(TBTermEqual(form->special.var, var))
         {
            res = false;
         }
         else
         {
            res = FormulaVarIsFree(form->arg1, var);
         }
         break;
   case OpBAnd:
   case OpBOr:
   case OpBImpl:
   case OpBEquiv:
   case OpBNand:
   case OpBNor:
   case OpBNImpl:
   case OpBXor:
         res = FormulaVarIsFree(form->arg1,var)
            || FormulaVarIsFree(form->arg2,var);
         break; 
   default:
         assert(false && "Illegal operator in formula");
         break;
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: FormulaCopy()
//
//   Return an (instantiated) copy of form. This will _not_ work if an
//   instantiated variable will be quantified somewhere, and there is
//   an assertion to catch this case.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Formula_p FormulaCopy(Formula_p form, TB_p terms)
{
   Formula_p handle, arg1, arg2;

   if(!form)
   {
      return NULL;
   }

   if(FormulaIsQuantified(form))
   {
      assert(!form->special.var->binding);
      
      arg1 = FormulaCopy(form->arg1, terms);
      handle = FormulaQuantorAlloc(form->op, form->special.var, arg1);
   }
   else if(FormulaIsLiteral(form))
   {
      Eqn_p lit;
      
      lit = EqnCopy(form->special.literal, terms);
      handle = FormulaLitAlloc(lit);
   }
   else
   {
      arg1 = FormulaCopy(form->arg1, terms);
      arg2 = FormulaCopy(form->arg2, terms);
      handle = FormulaOpAlloc(form->op, arg1, arg2);
   }         
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: FormulaCollectFreeVars()
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

void FormulaCollectFreeVars(Formula_p form, PTree_p *vars)
{
   TermProperties old_prop;
   
   if(FormulaIsQuantified(form))
   {
      old_prop = TermCellGiveProps(form->special.var, TPIsFreeVar);
      TermCellDelProp(form->special.var, TPIsFreeVar);
      FormulaCollectFreeVars(form->arg1, vars);
      TermCellSetProp(form->special.var, old_prop);      
   }
   else if(FormulaIsLiteral(form))
   {
      TermCollectPropVariables(form->special.literal->lterm, vars, TPIsFreeVar);
      TermCollectPropVariables(form->special.literal->rterm, vars, TPIsFreeVar);
   }
   else
   {
      if(FormulaHasSubForm1(form))
      {
         FormulaCollectFreeVars(form->arg1, vars);
      }
      if(FormulaHasSubForm2(form))
      {
         FormulaCollectFreeVars(form->arg2, vars);
      }
   }
}

/*-----------------------------------------------------------------------
//
// Function: FormulaFindMaxVarCode()
//
//   Return largest (absolute, i.e. largest negative) f_code of any
//   variable in form.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

FunCode FormulaFindMaxVarCode(Formula_p form)
{
   FunCode res1=0, res2=0;

   if(FormulaIsLiteral(form))
   {
      res1 = TermFindMaxVarCode(form->special.literal->lterm);
      res2 = TermFindMaxVarCode(form->special.literal->rterm);
   }
   else if(FormulaIsQuantified(form))
   {
      res1 = FormulaFindMaxVarCode(form->arg1);
      res2 = form->special.var->f_code;
   }
   else
   {
      if(FormulaHasSubForm1(form))
      {
         res1 = FormulaFindMaxVarCode(form->arg1);
      }
      if(FormulaHasSubForm2(form))
      {
         res2 = FormulaFindMaxVarCode(form->arg2);
      }
   }
   return MIN(res1, res2); /* Remember that var f_codes are negative */
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


