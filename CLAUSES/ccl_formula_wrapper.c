/*-----------------------------------------------------------------------

File  : ccl_formula_wrapper.c

Author: Stephan Schulz

Contents

  Wrapped formula code.

Copyright 1998-2011 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Thu Nov 13 01:43:38 GMT 2003
    New

-----------------------------------------------------------------------*/

#include "ccl_formula_wrapper.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

long global_formula_counter = LONG_MIN;
long FormulaDefLimit        = TFORM_RENAME_LIMIT;

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: DefaultWFormulaAlloc()
//
//   Allocate and return a wrapped formula cell with all values
//   initialized to rational default values.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

WFormula_p DefaultWFormulaAlloc()
{
   WFormula_p handle = WFormulaCellAlloc();
   
   handle->properties = WPIgnoreProps; 
   handle->ident      = 0;
   handle->terms      = NULL;
   handle->info       = NULL;
   handle->derivation = NULL;
   handle->tformula   = NULL;
   handle->set        = NULL;
   handle->pred       = NULL;
   handle->succ       = NULL;

   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: WTFormulaAlloc()
//
//   Allocate a wrapped formula given the essential information. id
//   will automagically be set to a new value.
//
// Global Variables: FormulaIdentCounter
//
// Side Effects    : Via DefaultWFormulaAlloc()
//
/----------------------------------------------------------------------*/

WFormula_p WTFormulaAlloc(TB_p terms, TFormula_p formula)
{
   WFormula_p handle = DefaultWFormulaAlloc();
   
   handle->terms   = terms;
   handle->tformula = formula;
   handle->ident   = ++global_formula_counter;   
   
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: WFormulaFree()
//
//   Free a wrapped formula.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void WFormulaFree(WFormula_p form)
{
   assert(form);
   assert(form->tformula);
   assert(!form->set);
   assert(!form->pred);
   assert(!form->succ);
   
   /* tformula handled by Garbage Collection */
   
   ClauseInfoFree(form->info);
   if(form->derivation)
   {
      PStackFree(form->derivation);
   }
   WFormulaCellFree(form);
}

/*-----------------------------------------------------------------------
//
// Function: WFormulaFlatCopy()
//
//   Create a flat copy of the formula.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

WFormula_p WFormulaFlatCopy(WFormula_p form)
{
   WFormula_p res = DefaultWFormulaAlloc();

   res->properties = form->properties;
   res->ident      = form->ident;
   res->terms      = form->terms;
   res->tformula   = form->tformula;

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: WFormulaGCMarkCells()
//
//   If formula is a term formula, mark the terms. Otherwise a noop.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void WFormulaGCMarkCells(WFormula_p form)
{
      TFormulaGCMarkCells(form->terms, form->tformula);
}

/*-----------------------------------------------------------------------
//
// Function: WFormulaMarkPolarity()
//
//   Mark the polarity of all subformulas in form.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void WFormulaMarkPolarity(WFormula_p form)
{
   TFormulaMarkPolarity(form->terms, form->tformula, 1);
}

/*-----------------------------------------------------------------------
//
// Function: WFormulaTPTPParse()
//
//   Parse a formula in TPTP format.
//
// Global Variables: -
//
// Side Effects    : Input
//
/----------------------------------------------------------------------*/

WFormula_p WFormulaTPTPParse(Scanner_p in, TB_p terms)
{
   TFormula_p         tform;
   WFormulaProperties type;
   WFormula_p         handle;
   ClauseInfo_p       info;

   info = ClauseInfoAlloc(NULL, DStrView(AktToken(in)->source), 
                          AktToken(in)->line, 
                          AktToken(in)->column); 
   AcceptInpId(in, "input_formula");
   AcceptInpTok(in, OpenBracket);  
   CheckInpTok(in, Name|PosInt);
   info->name = DStrCopy(AktToken(in)->literal);
   NextToken(in);
   AcceptInpTok(in, Comma);
   CheckInpId(in, "axiom|hypothesis|negated_conjecture|conjecture|question|lemma|unknown");
   if(TestInpId(in, "conjecture"))
   {
      type = WPTypeConjecture;
   }
   else if(TestInpId(in, "question"))
   {
      type = WPTypeQuestion;
   }
   else if(TestInpId(in, "negated_conjecture"))
   {
      type = WPTypeNegConjecture;
   }
   else if(TestInpId(in, "hypothesis"))
   {
      type = WPTypeHypothesis;
   }
   else
   {
      type = WPTypeAxiom;
   }
   NextToken(in);
   AcceptInpTok(in, Comma);

   tform = TFormulaTPTPParse(in, terms);
   handle = WTFormulaAlloc(terms, tform);

   AcceptInpTok(in, CloseBracket);
   AcceptInpTok(in, Fullstop);
   FormulaSetType(handle, type);
   FormulaSetProp(handle, WPInitial|WPInputFormula);
   handle->info = info;

   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: FormulaTPTPPrint()
//
//   Print a formula in TPTP format.
//
// Global Variables: -
//
// Side Effects    : 
//
/----------------------------------------------------------------------*/

void WFormulaTPTPPrint(FILE* out, WFormula_p form, bool fullterms)
{
   char *typename;
   char prefix;
   long id;

   switch(FormulaQueryType(form))
   {
   case WPTypeAxiom:
         typename = "axiom";
	 break;
   case WPTypeHypothesis:
	 typename = "hypothesis";
	 break;       
   case WPTypeConjecture:
   case WPTypeNegConjecture:
	 typename = "conjecture";
	 break;
   case WPTypeQuestion:
         typename = "question";
         break;
   default:
	 typename = "unknown";
	 break;
   }   
   if(form->ident < 0)
   {
      id = form->ident - LONG_MIN;
      prefix = 'i';
   }
   else
   {
      id = form->ident;
      prefix = 'e';
   }
   fprintf(out, "input_formula(f%c_%ld,%s,", prefix, id, typename);

   TFormulaTPTPPrint(out, form->terms, form->tformula,fullterms, false);
   fprintf(out,").");   
}


/*-----------------------------------------------------------------------
//
// Function: WFormulaTSTPParse()
//
//   Parse a formula in TSTP format.
//
// Global Variables: -
//
// Side Effects    : Input
//
/----------------------------------------------------------------------*/

WFormula_p WFormulaTSTPParse(Scanner_p in, TB_p terms)
{
   TFormula_p         tform; 
   WFormulaProperties type = WPTypeAxiom;
   WFormulaProperties initial = WPInputFormula;
   WFormula_p         handle;
   ClauseInfo_p       info;

   info = ClauseInfoAlloc(NULL, DStrView(AktToken(in)->source), 
                          AktToken(in)->line, 
                          AktToken(in)->column); 
      
   AcceptInpId(in, "fof");
   AcceptInpTok(in, OpenBracket);
   CheckInpTok(in, Name|PosInt|SQString);
   info->name = DStrCopy(AktToken(in)->literal);
   NextToken(in);
   AcceptInpTok(in, Comma);
   
   /* This is hairy! E's internal types do not map very well to
      TSTP types, and E uses the "initial" properties in ways that
      make it highly desirable that anything in the input is
      actually initial (the CPInitialProperty is actually set in
      all clauses in the initial unprocessed clause set. So we
      ignore the "derived" modifier, and use CPTypeAxiom for plain
      clauses. */
   type = ClauseTypeParse(in, 
                          "axiom|hypothesis|definition|assumption|"
                          "lemma|theorem|conjecture|question|negated_conjecture|"
                          "plain|unknown");
   AcceptInpTok(in, Comma);

   tform = TFormulaTSTPParse(in, terms);
   handle = WTFormulaAlloc(terms, tform);

   if(TestInpTok(in, Comma))
   {
      AcceptInpTok(in, Comma);
      TSTPSkipSource(in);
      if(TestInpTok(in, Comma))
      {
         AcceptInpTok(in, Comma);
         CheckInpTok(in, OpenSquare);
         ParseSkipParenthesizedExpr(in);
      }
   }
   AcceptInpTok(in, CloseBracket);
   AcceptInpTok(in, Fullstop);
   FormulaSetType(handle, type);
   FormulaSetProp(handle, initial|WPInitial);
   handle->info = info;

   return handle;
}




/*-----------------------------------------------------------------------
//
// Function: WFormulaTSTPPrint()
//
//   Print a formula in TSTP format. If !complete, leave of the
//   trailing ")." for adding optional stuff.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void WFormulaTSTPPrint(FILE* out, WFormula_p form, bool fullterms,
		       bool complete)
{
   char *typename = "plain";
   char prefix;
   long id;

   switch(FormulaQueryType(form))
   {
   case WPTypeAxiom:
         if(FormulaQueryProp(form, WPInputFormula))
         {
            typename = "axiom";
         }
         break;
   case WPTypeHypothesis:
         typename = "hypothesis";
         break;      
   case WPTypeConjecture:
         typename = "conjecture";
         break;
   case WPTypeQuestion:
         typename = "question";
         break;
   case WPTypeLemma:
         typename = "lemma";
         break; 
   case WPTypeNegConjecture:
         typename = "negated_conjecture";
         break;
   default:
	 break;
   }   
   if(form->ident < 0)
   {
      id = form->ident - LONG_MIN;
      prefix = 'i';
   }
   else
   {
      id = form->ident;
      prefix = 'c';
   }
   fprintf(out, "fof(%c_0_%ld, %s", prefix, id, typename);
   fprintf(out, ", (");   

   TFormulaTPTPPrint(out, form->terms, form->tformula,fullterms, false);
   //fprintf(out, "<dummy %p in %p>", form->tformula, form->terms);   
   

   fprintf(out, ")");   
   if(complete)
   {
      fprintf(out, ").");
   }
}


/*-----------------------------------------------------------------------
//
// Function: WFormulaParse()
//
//   Parse a formula in any supported input format.
//
// Global Variables: -
//
// Side Effects    : Input
//
/----------------------------------------------------------------------*/

WFormula_p WFormulaParse(Scanner_p in, TB_p terms)
{
   WFormula_p wform = NULL;

   if(ClausesHaveDisjointVariables)
   {
      VarBankClearExtNamesNoReset(terms->vars);
   }   
   switch(ScannerGetFormat(in))
   {
   case LOPFormat:
         Error("LOP currently does not support full FOF!", OTHER_ERROR);
         break;
   case TPTPFormat:
         wform = WFormulaTPTPParse(in, terms);
         break;
   case TSTPFormat:
         wform = WFormulaTSTPParse(in, terms);
         break;
   default:
         assert(false);
         break;
   }
   /* WFormulaPrint(stdout, wform, true);
      printf("\n"); */
   return wform;
}

/*-----------------------------------------------------------------------
//
// Function: WFormulaPrint()
//
//   Print a (wrapped) formula in the current output format.
//
// Global Variables: OutputFormat
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void WFormulaPrint(FILE* out, WFormula_p form, bool fullterms)
{
   switch(OutputFormat)
   {
   case LOPFormat:
      Warning("Currently no LOP FOF format, using TPTP");
   case TPTPFormat:
      WFormulaTPTPPrint(out, form, fullterms);
      break;
   case TSTPFormat:
      WFormulaTSTPPrint(out, form, fullterms, true);
      break;
   default:
         assert(false&& "Unknown output format");
         break;
   }
}


/*-----------------------------------------------------------------------
//
// Function: WFormulaReturnFCodes()
//
//   Push all function symbol codes from form onto f_codes. Return
//   number of symbols found.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long WFormulaReturnFCodes(WFormula_p form, PStack_p f_codes)
{
   long res = 0;
   PStack_p stack;
   Sig_p sig;
   PStackPointer i, start;
   Term_p t;
   FunCode f;

   assert(form);
   assert(f_codes);

   sig = form->terms->sig;
   assert(sig);

   stack = PStackAlloc();
   TBTermCollectSubterms(form->tformula, stack);

   start = PStackGetSP(f_codes);
   for(i=0; i<PStackGetSP(stack);i++)
   {
      t = PStackElementP(stack,i);
      TermCellDelProp(t, TPOpFlag);
      if(!TermIsVar(t))
      {
         if(!SigQueryFuncProp(sig, t->f_code, FPOpFlag))
         {
            SigSetFuncProp(sig, t->f_code, FPOpFlag);
            PStackPushInt(f_codes, t->f_code);
            res++;
         }
      }
   }
   PStackFree(stack);

   /* Reset FPOpFlags */
   for(i=start; i<PStackGetSP(f_codes);i++)
   {
      f =  PStackElementInt(f_codes, i);
      SigDelFuncProp(sig, f, FPOpFlag);      
   }
   return res;
}




/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


