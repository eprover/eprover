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
//   Parse an return a TPTP quantor. Rather trivial ;-)
//
// Global Variables: -
//
// Side Effects    : Input
//
/----------------------------------------------------------------------*/

FOFOperatorType tptp_operator_parse(Scanner_p in)
{
   FOFOperatorType res;

   CheckInpTok(in, TildeSign|Ampersand|Pipe|EqualSign|LesserSign);
   if(TestInpTok(in, TildeSign))
   {
      NextToken(in);
      CheckInpTokNoSkip(in, Ampersand|Pipe);
      if(TestInpTok(in, Ampersand))
      {
         res = OpBNand;         
      }
      else
      {
         res = OpBNor;
      }
      NextToken(in);
   }
   else
   {
      CheckInpTok(in, Ampersand|Pipe|EqualSign|LesserSign);
      if(TestInpTok(in, Ampersand))
      {
         res = OpBAnd;
         NextToken(in);
            
      }
      else if(TestInpTok(in, Pipe))
      {
         res = OpBOr;
         NextToken(in);
      }
      else if(TestInpTok(in, EqualSign))
      {
         NextToken(in);
         CheckInpTokNoSkip(in, GreaterSign);
         res = OpBImpl;
         NextToken(in);
      }
      else
      {
         AcceptInpTok(in, LesserSign);
         if(TestInpTok(in, TildeSign))
         {
            AcceptInpTokNoSkip(in, TildeSign);
            AcceptInpTokNoSkip(in, GreaterSign);
            res = OpBXor;
         }
         else
         {
            AcceptInpTokNoSkip(in, EqualSign);
            res = OpBImpl;
            if(TestInpTok(in, GreaterSign))
            {            
               AcceptInpTokNoSkip(in, GreaterSign);
               res = OpBEquiv;
            }
         }
      }
   }
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
      lit = EqnTSTPParse(in, terms);
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

void FormulaTPTPPrint(FILE* out, Formula_p form, bool fullterms)
{
   assert(form);

   if(FormulaIsLiteral(form))
   {      
      EqnTSTPPrint(out, form->special.literal, fullterms);
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
      FormulaTPTPPrint(out, form->arg1, fullterms);
   }
   else if(FormulaIsUnary(form))
   {
      assert(form->op == OpUNot);
      if(FormulaIsBinary(form->arg1))
      {
	 fputs("~(", out);
	 FormulaTPTPPrint(out, form->arg1, fullterms);
	 fputs(")", out);
      }
      else
      {
	 fputc('~', out);
	 FormulaTPTPPrint(out, form->arg1, fullterms);
      }
   }
   else
   {
      char* oprep = "XXX";

      assert(FormulaIsBinary(form));
      fputs("(", out);
      FormulaTPTPPrint(out, form->arg1, fullterms);
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
      FormulaTPTPPrint(out, form->arg2, fullterms);      
      fputs(")", out);      
   }   
}

/*-----------------------------------------------------------------------
//
// Function: FormulaTPTPParse()
//
//   Parse a formula in TSTP/TPTP formula.
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
   if(TestInpTok(in, Ampersand|LesserSign|EqualSign|Pipe|TildeSign))
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
         res = EqnEqual(form1->special.literal, 
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
// Function: FormulaHasFreeVar()
//
//   Return true iff var is a free variable in form.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool FormulaHasFreeVar(Formula_p form, Term_p var)
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
         res = FormulaHasFreeVar(form->arg1, var);
         break;
   case OpQEx:
   case OpQAll:
         if(TBTermEqual(form->special.var, var))
         {
            res = false;
         }
         else
         {
            res = FormulaHasFreeVar(form->arg1, var);
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
         res = FormulaHasFreeVar(form->arg1,var)
            || FormulaHasFreeVar(form->arg2,var);
         break; 
   default:
         assert(false && "Illegal operator in formula");
         break;
   }
   return res;
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


