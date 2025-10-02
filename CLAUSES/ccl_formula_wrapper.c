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
#include <clb_simple_stuff.h>


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

long global_formula_counter = LONG_MIN;
bool FormulasKeepInputNames = true;


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: handle_ho_def()
//
//   Parse higher order definitions of form s = t where both s and t
//   are non-formula terms or p = f where p is a predicate symbol
//   and f is a formula.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

TFormula_p handle_ho_def(Scanner_p in, TB_p bank)
{
   assert(problemType == PROBLEM_HO);

   bool in_parens = false;

   if(TestInpTok(in, OpenBracket))
   {
      AcceptInpTok(in, OpenBracket);
      in_parens = true;
   }

   Term_p lside_term = TBTermParse(in, bank);
   if(TypeIsBool(lside_term->type))
   {
      TFormula_p lside = EqnTermsTBTermEncode(bank, lside_term,
                                              bank->true_term, true, PENormal);
      if(!TestInpTok(in, EqualSign))
      {
         AktTokenError(in, "E currently supports definitions of type <predicate "
                           " symbol> = <closed LFHOL formula>",
                       false);
      }

      AcceptInpTok(in, EqualSign);
      TFormula_p res =  TFormulaFCodeAlloc(bank, bank->sig->equiv_code,
                                           lside, TFormulaTSTPParse(in, bank));
      if(in_parens)
      {
         AcceptInpTok(in, CloseBracket);
      }
      return res;
   }
   else
   {
      bool positive = true;
      if(TestInpTok(in, NegEqualSign))
      {
         positive = false;
      }
      AcceptInpTok(in, EqualSign|NegEqualSign);
      Term_p rside = TBTermParse(in, bank);
      TFormula_p res = EqnTermsTBTermEncode(bank, lside_term, rside,
                                            positive, PENormal);
      if(in_parens)
      {
         AcceptInpTok(in, CloseBracket);
      }
      return res;
   }
   return NULL;
}

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

WFormula_p DefaultWFormulaAlloc(void)
{
   WFormula_p handle = WFormulaCellAlloc();

   handle->properties = CPIgnoreProps;
   handle->is_clause  = false;
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
   res->is_clause  = form->is_clause;
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
// Function: WFormulaGetId()
//
//   Return an identifier for the formula. The pointer to the
//   identifier is good until the next call to this function or until
//   the formula is being  destroyed, whichever comes first.
//
// Global Variables: -
//
// Side Effects    : FormulasKeepInputNames
//
/----------------------------------------------------------------------*/

char* WFormulaGetId(WFormula_p form)
{
   static char ident[32]; //big enough for 64 bit integers and then some
   char prefix;
   long id;

   if(FormulasKeepInputNames
      &&form->info
      &&form->info->name)
   {
      return form->info->name;
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
   snprintf(ident, 31, "%c_0_%ld", prefix, id);
   return ident;
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
   FormulaProperties  type;
   WFormula_p         handle;
   ClauseInfo_p       info;

   info = ClauseInfoAlloc(NULL, DStrView(AktToken(in)->source),
                          AktToken(in)->line,
                          AktToken(in)->column);
   AcceptInpId(in, "input_formula");
   SetProblemType(PROBLEM_FO);
   AcceptInpTok(in, OpenBracket);
   CheckInpTok(in, Name|PosInt);
   info->name = DStrCopy(AktToken(in)->literal);
   NextToken(in);
   AcceptInpTok(in, Comma);
   CheckInpId(in, "axiom|hypothesis|negated_conjecture|conjecture|question|lemma|unknown");
   if(TestInpId(in, "conjecture"))
   {
      type = CPTypeConjecture;
   }
   else if(TestInpId(in, "question"))
   {
      type = CPTypeQuestion;
   }
   else if(TestInpId(in, "negated_conjecture"))
   {
      type = CPTypeNegConjecture;
   }
   else if(TestInpId(in, "hypothesis"))
   {
      type = CPTypeHypothesis;
   }
   else
   {
      type = CPTypeAxiom;
   }
   NextToken(in);
   AcceptInpTok(in, Comma);

   tform = TFormulaTPTPParse(in, terms);
   handle = WTFormulaAlloc(terms, tform);

   AcceptInpTok(in, CloseBracket);
   AcceptInpTok(in, Fullstop);
   FormulaSetType(handle, type);
   FormulaSetProp(handle, CPInitial|CPInputFormula);
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

   switch(FormulaQueryType(form))
   {
   case CPTypeAxiom:
         typename = "axiom";
    break;
   case CPTypeHypothesis:
    typename = "hypothesis";
    break;
   case CPTypeConjecture:
   case CPTypeNegConjecture:
    typename = "conjecture";
    break;
   case CPTypeQuestion:
         typename = "question";
         break;
   default:
    typename = "unknown";
    break;
   }
   fprintf(out, "input_formula(%s,%s,", WFormulaGetId(form), typename);

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
   TFormula_p        tform;
   FormulaProperties type = CPTypeAxiom;
   FormulaProperties initial = CPInputFormula;
   WFormula_p        handle;
   ClauseInfo_p      info;
   bool              is_tcf = false;
   DStr_p            source_name;
   long              line, column;
   StreamType        inptype;

   info = ClauseInfoAlloc(NULL, DStrView(AktToken(in)->source),
                          AktToken(in)->line,
                          AktToken(in)->column);

   if(TestInpId(in, "tcf"))
   {
      is_tcf = true;
   }

   if(TestInpId(in, "thf"))
   {
      SetProblemType(PROBLEM_HO);
   }
   else if(TestInpId(in, "fof|tff|tcf"))
   {
      SetProblemType(PROBLEM_FO);
   }

   AcceptInpId(in, "fof|tff|thf|tcf");
   AcceptInpTok(in, OpenBracket);
   CheckInpTok(in, Name|PosInt|SQString);
   info->name = DStrCopy(AktToken(in)->literal);
   // printf(COMCHAR" Parsing: %s\n", info->name);
   NextToken(in);
   AcceptInpTok(in, Comma);

   /* This is hairy! E's internal types do not map very well to
      TSTP types, and E uses the "initial" properties in ways that
      make it highly desirable that anything in the input is
      actually initial (the CPInitialProperty is actually set in
      all clauses in the initial unprocessed clause set. So we
      ignore the "derived" modifier, and use CPTypeAxiom for plain
      clauses.
      With typing, it gets more complex, as a type declaration is
      not a proper clause. However, the simplest thing to do is
      to parse "$true" and modify the signature.
      */
   if(TestInpId(in, "type"))
   {
      NextToken(in);
      AcceptInpTok(in, Comma);

      /* Parse declaration, modifies signature */
      SigParseTFFTypeDeclaration(in, terms->sig);

      tform = TFormulaPropConstantAlloc(terms, true);
      handle = WTFormulaAlloc(terms, tform);
   }
   else
   {
      type = (FormulaProperties)
         ClauseTypeParse(in,is_tcf?
                         "axiom|hypothesis|definition|assumption|"
                         "lemma|theorem|conjecture|question|negated_conjecture|"
                         "plain|unknown|watchlist":
                         "axiom|hypothesis|definition|assumption|"
                         "lemma|theorem|conjecture|question|negated_conjecture|"
                         "plain|unknown");
      AcceptInpTok(in, Comma);

      // printf(COMCHAR" Formula Start!\n");

      source_name = DStrGetRef(AktToken(in)->source);
      inptype     = AktToken(in)->stream_type;
      line        = AktToken(in)->line;
      column      = AktToken(in)->column;

      if(TestInpId(in, "$distinct"))
      {
         tform = TSTPDistinctParse(in, terms);
         // fprintf(stderr, COMCHAR" $distinct parsed!: ");
         // TFormulaTPTPPrint(stderr, terms, tform, true, false);
         // fprintf(stderr, " : ");
         // TermPrintDbg(stderr, tform, terms->sig, DEREF_NEVER);
         // fprintf(stderr, ";\n");
      }
      else if(is_tcf)
      {
         // printf(COMCHAR" Tcf Start!\n");
         tform = TcfTSTPParse(in, terms);
         // printf(COMCHAR" Tcf Done!\n");
      }
      else
      {
         //fprintf(stderr, COMCHAR" TFormula Start!\n");
         tform = TFormulaTSTPParse(in, terms);
         //fprintf(stderr, COMCHAR" TFormula parsed!: ");
         //TFormulaTPTPPrint(stderr, terms, tform, true, false);
         //fprintf(stderr, " : ");
         //TermPrintDbg(stderr, tform, terms->sig, DEREF_NEVER);
         //fprintf(stderr, ";\n");
      }


      if(TFormulaHasFreeVars(terms, tform))
      {
         Error("%s Formula has free variables (check parentheses "
               "and quantifier precedence)",
               SYNTAX_ERROR,
               PosRep(inptype, source_name, line, column));
      }
      DStrReleaseRef(source_name);

      handle = WTFormulaAlloc(terms, tform);
   }

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
   FormulaSetProp(handle, initial|CPInitial);
   handle->info = info;
   //WFormulaTSTPPrintFlex(stdout, handle, true, true, false);
   //printf("\n");
   // printf(COMCHAR" Formula complete!\n");
   return handle;
}





/*-----------------------------------------------------------------------
//
// Function: WFormulaTSTPPrintFlex()
//
//   Print a formula in TSTP format. If !complete, leave of the
//   trailing ")." for adding optional stuff. If "as_formula" is true,
//   print clauses as (universally quantified) formulas.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void WFormulaTSTPPrintFlex(FILE* out, WFormula_p form, bool fullterms,
                           bool complete, bool as_formula)
{
   char *typename = "plain", *formula_kind = "fof";
   bool is_untyped = TFormulaIsUntyped(form->tformula);

   //SigPrint(stdout, form->terms->sig);

   if(problemType == PROBLEM_FO)
   {
      if(form->is_clause && is_untyped && !as_formula)
      {
         formula_kind = "cnf";
      }
      else if(form->is_clause && !as_formula)
      {
         formula_kind = "tcf";
      }
      else if(!is_untyped)
      {
         formula_kind = "tff";
      }
   }
   else
   {
      formula_kind = "thf";
   }

   switch(FormulaQueryType(form))
   {
   case CPTypeAxiom:
         if(FormulaQueryProp(form, CPInputFormula))
         {
            typename = "axiom";
         }
         break;
   case CPTypeHypothesis:
         typename = "hypothesis";
         break;
   case CPTypeConjecture:
         typename = "conjecture";
         break;
   case CPTypeQuestion:
         typename = "question";
         break;
   case CPTypeLemma:
         typename = "lemma";
         break;
   case CPTypeNegConjecture:
         typename = "negated_conjecture";
         break;
   default:
    break;
   }
   fprintf(out, "%s(%s, %s", formula_kind, WFormulaGetId(form), typename);
   fprintf(out, ", ");

   if(form->is_clause)
   {
      if(as_formula)
      {
         TFormula_p closure =  TFormulaClosure(form->terms, form->tformula, true);
         TFormulaTPTPPrint(out, form->terms, closure, fullterms, false);
      }
      else
      {
         Clause_p clause = WFormClauseToClause(form);
         ClauseTSTPCorePrint(out, clause, fullterms);
         ClauseFree(clause);
      }
   }
   else
   {
      TFormulaTPTPPrint(out, form->terms, form->tformula,fullterms, false);
      //fprintf(out, "<dummy %p in %p>", form->tformula, form->terms);
   }
   if(complete)
   {
      fprintf(out, ").");
   }
}

/*-----------------------------------------------------------------------
//
// Function: WFormulaAppEncode()
//
//   Encodes terms in wrapped formula's literals using app encoding.
//   Initial WFormula is not changed.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void WFormulaAppEncode(FILE* out, WFormula_p form)
{
   char *typename = "plain", *formula_kind = "tff";

   switch(FormulaQueryType(form))
   {
   case CPTypeAxiom:
         if(FormulaQueryProp(form, CPInputFormula))
         {
            typename = "axiom";
         }
         break;
   case CPTypeHypothesis:
         typename = "hypothesis";
         break;
   case CPTypeConjecture:
         typename = "conjecture";
         break;
   case CPTypeQuestion:
         typename = "question";
         break;
   case CPTypeLemma:
         typename = "lemma";
         break;
   case CPTypeNegConjecture:
         typename = "negated_conjecture";
         break;
   default:
    break;
   }
   fprintf(out, "%s(%s, %s", formula_kind, WFormulaGetId(form), typename);
   fprintf(out, ", ");

   assert(!form->is_clause);
   TFormulaAppEncode(out, form->terms, form->tformula);
   fprintf(out, ").");
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
// Function: WFormClauseParse()
//
//   Parse a clause into a a WFormula disjunction.
//
// Global Variables: -
//
// Side Effects    : Input
//
/----------------------------------------------------------------------*/

WFormula_p WFormClauseParse(Scanner_p in, TB_p terms)
{
   WFormula_p wform  = NULL;
   TFormula_p form;
   Clause_p   handle = ClauseParse(in, terms);

   form = TFormulaClauseEncode(terms, handle);

   wform = WTFormulaAlloc(terms, form);
   wform->is_clause  = true;
   wform->properties = (FormulaProperties)handle->properties;
   wform->info = handle->info;
   handle->info = NULL;
   ClauseFree(handle);

   //printf(COMCHAR" WFormClauseParse: ");
   //WFormulaPrint(stdout, wform, true);
   //printf("\n");
   return wform;
}


/*-----------------------------------------------------------------------
//
// Function: WFormClauseToClause()
//
//   Convert a WFormula-encoded clause to a clause proper.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Clause_p WFormClauseToClause(WFormula_p form)
{
   Clause_p res  = TFormulaCollectClause(form->tformula, form->terms, NULL);

   res->properties = form->properties;
   if(form->info)
   {
      res->info = ClauseInfoAlloc(form->info->name,
                                  form->info->source,
                                  form->info->line,
                                  form->info->column);
   }
   return res;
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
   if(form->is_clause)
   {
      Clause_p clause = WFormClauseToClause(form);
      ClausePrint(out, clause, fullterms);
      ClauseFree(clause);
   }
   else
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
      if(!TermIsAnyVar(t) && !TermIsPhonyApp(t))
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


/*-----------------------------------------------------------------------
//
// Function: WFormulaSymbolDiversity()
//
//   Return number of different symbols in form.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

long WFormulaSymbolDiversity(WFormula_p form)
{
   long res;
   PStack_p stack = PStackAlloc();
   // TODO: Should we filter for non-internal symbols?
   res = WFormulaReturnFCodes(form, stack);
   PStackFree(stack);
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: WFormulaOfClause
//
//   Universally quantifies the disjunction of the literals of
//   the clause, and return it as a fresh formula.
//
//   Allocate a formula.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/
WFormula_p WFormulaOfClause(Clause_p clause, TB_p bank)
{
   TFormula_p form = NULL;
   WFormula_p res = NULL;

   form = TFormulaClauseEncode(bank, clause);
   form = TFormulaClosure(bank, form, true);

   res = WTFormulaAlloc(bank, form);
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: WFormulaGetLambdaDefinedSym
//
//   If the formula is an axiom tagged with 'definition' role then
//   extract the function code of the defined symbol. In case
//   of a format error (definition tag, but nothing is defined)
//   or tag is not present  return -1.
//
//   Allocate a formula.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

FunCode WFormulaGetLambdaDefinedSym(WFormula_p form)
{
   FunCode res = -1;
   if(FormulaQueryProp(form, CPIsLambdaDef))
   {
      Term_p tform = form->tformula;
      Sig_p sig = form->terms->sig;
      while (tform->f_code == sig->qall_code && tform->arity == 2)
      {
         tform = tform->args[1];
      }

      Term_p lhs = NULL;
      if (tform->f_code == sig->eqn_code)
      {
         lhs = tform->args[0];
      }
      else if(tform->f_code == sig->equiv_code &&
              tform->args[0]->f_code == sig->eqn_code)
      {
         lhs = tform->args[0]->args[0];
      }

      if(lhs && lhs->f_code > sig->internal_symbols)
      {
         res = lhs->f_code;
      }
   }
   return res;
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
