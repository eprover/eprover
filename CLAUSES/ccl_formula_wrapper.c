/*-----------------------------------------------------------------------

File  : ccl_formula_wrapper.c

Author: Stephan Schulz

Contents

  Wrapped formula code.

  Copyright 1998-2003 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
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
   
   handle->properties = FPIgnoreProps; 
   handle->ident      = 0;
   handle->ext_ident  = NULL;
   handle->formula    = 0;
   handle->set        = NULL;
   handle->pred       = NULL;
   handle->succ       = NULL;

   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: WFormulaAlloc()
//
//   Allocate a wrapped formula given the essential information. id
//   will automagically be set to a new value.
//
// Global Variables: FormulaIdentCounter
//
// Side Effects    : Via DefaultWFormulaAlloc()
//
/----------------------------------------------------------------------*/

WFormula_p WFormulaAlloc(Formula_p formula)
{
   WFormula_p handle = DefaultWFormulaAlloc();
   
   handle->formula = FormulaGetRef(formula);
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
   assert(form->formula);
   assert(!form->set);
   assert(!form->pred);
   assert(!form->succ);
   
   FormulaRelRef(form->formula);
   FormulaFree(form->formula);
   if(form->ext_ident)
   {
      FREE(form->ext_ident);
   }
   WFormulaCellFree(form);
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
   char*              name;
   Formula_p          form;
   WFormulaProperties type;
   WFormula_p         handle;
   
   AcceptInpId(in, "input_formula");
   AcceptInpTok(in, OpenBracket);
   CheckInpTok(in, Name|PosInt);
   name = DStrCopy(AktToken(in)->literal);
   NextToken(in);
   AcceptInpTok(in, Comma);
   CheckInpId(in, "axiom|hypothesis|conjecture|lemma|unknown");
   if(TestInpId(in, "conjecture"))
   {
      type = WPTypeConjecture;
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
   form = FormulaTPTPParse(in, terms); /* TSTP = TPTP! */
   AcceptInpTok(in, CloseBracket);
   AcceptInpTok(in, Fullstop);
   handle = WFormulaAlloc(form);
   handle->ext_ident = name;
   FormulaSetType(handle, type);
   FormulaSetProp(handle, WPInitial);

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
	 typename = "conjecture";
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
   FormulaTPTPPrint(out, form->formula,fullterms);
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
   char*              name;
   Formula_p          form;
   WFormulaProperties type = WPTypeAxiom;
   WFormulaProperties initial = WPInitial;
   WFormula_p         handle;
   bool               check_type = true;
      
   AcceptInpId(in, "fof");
   AcceptInpTok(in, OpenBracket);
   CheckInpTok(in, Name);
   name = DStrCopy(AktToken(in)->literal);
   NextToken(in);
   AcceptInpTok(in, Comma);
   if(TestInpId(in, "initial|derived"))
   {
      if(TestInpId(in, "derived"))
      {
	 initial = WPIgnoreProps;
      }
      AcceptInpTok(in, Ident);
      type = WPTypeAxiom;
      check_type = false;
      if(TestInpTok(in, Hyphen))
      {
	 AcceptInpTok(in, Hyphen);
	 check_type = true;
      }
   }
   if(check_type)
   {
      CheckInpId(in, 
		 "axiom|definition|knowledge|assumption|"
		 "hypothesis|conjecture|lemma|unknown");
      if(TestInpId(in, "hypothesis"))
      {
	 type = WPTypeHypothesis;
      }
      else if(TestInpId(in, "conjecture"))
      {
	 type = WPTypeConjecture;
      }
      else
      {
	 type = WPTypeAxiom;
      }
   }   
   NextToken(in);
   AcceptInpTok(in, Comma);
   form = FormulaTPTPParse(in, terms); /* TSTP = TPTP! */
   AcceptInpTok(in, CloseBracket);
   AcceptInpTok(in, Fullstop);
   handle = WFormulaAlloc(form);
   handle->ext_ident = name;
   FormulaSetType(handle, type);
   FormulaSetProp(handle, initial);

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
   char *typename, *initial="derived";
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
	 typename = "conjecture";
	 break;
   default:
	 typename = "unknown";
	 break;
   }   
   if(FormulaQueryProp(form, WPInitial))
   {
      initial = "initial";
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
   fprintf(out, "fof(f%c_%ld,%s-%s,", prefix, id, initial,typename);
   FormulaTPTPPrint(out, form->formula,fullterms);
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
   return wform;
}



/*-----------------------------------------------------------------------
//
// Function: WFormulaConjectureNegate()
//
//   If formula is a conjecture, negate it and delete that property
//   (but set WPInitialConjecture). Returns true if formula was a
//   conjecture. 
//
// Global Variables: -
//
// Side Effects    : Changes formula
//
/----------------------------------------------------------------------*/

bool WFormulaConjectureNegate(WFormula_p wform)
{
   Formula_p form, newform;

   if(FormulaQueryProp(wform, WPTypeConjecture))
   {
      form = FormulaRelRef(wform->formula);
      newform = FormulaOpAlloc(OpUNot, form, NULL);
      wform->formula = FormulaGetRef(newform);
      FormulaDelProp(wform, WPTypeConjecture);
      FormulaSetProp(wform, WPInitialConjecture);
      return true;
   }
   return false;
}


/*-----------------------------------------------------------------------
//
// Function: FormulaSetAlloc()
//
//   Allocate and initialize a formula set.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

FormulaSet_p FormulaSetAlloc()
{
   FormulaSet_p set = FormulaSetCellAlloc();

   set->members = 0;
   set->anchor  = WFormulaCellAlloc();
   set->anchor->succ = set->anchor;
   set->anchor->pred = set->anchor;
   
   return set;
}

/*-----------------------------------------------------------------------
//
// Function: FormulaSetFree(set)
//
//   Free a formula set (and all its formulas).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void FormulaSetFree(FormulaSet_p set)
{
   assert(set);

   while(!FormulaSetEmpty(set))
   {
      FormulaSetDeleteEntry(set->anchor->succ);
   }
   WFormulaCellFree(set->anchor);
   FormulaSetCellFree(set);
}


/*-----------------------------------------------------------------------
//
// Function: FormulaSetInsert()
//
//   Insert new into set.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void FormulaSetInsert(FormulaSet_p set, WFormula_p new)
{
   assert(set);
   assert(new);
   assert(!new->set);
   
   new->succ = set->anchor;
   new->pred = set->anchor->pred;
   set->anchor->pred->succ = new;
   set->anchor->pred = new;
   new->set = set;
   set->members++;
}

/*-----------------------------------------------------------------------
//
// Function: FormulaSetExtractEntry()
//
//   Extract a given formula from a formula set and return it.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

WFormula_p FormulaSetExtractEntry(WFormula_p form)
{
   assert(form);
   assert(form->set);

   form->pred->succ = form->succ;
   form->succ->pred = form->pred;
   form->set->members--;
   form->set = NULL;
   form->succ = NULL;
   form->pred = NULL;

   return form;
}

/*-----------------------------------------------------------------------
//
// Function: FormulaSetExtractFirst()
//
//   Extract and return the first formula from set, if any, otherwise
//   return NULL.
//
// Global Variables: 
//
// Side Effects    : 
//
/----------------------------------------------------------------------*/

WFormula_p FormulaSetExtractFirst(FormulaSet_p set)
{
   assert(set);

   if(FormulaSetEmpty(set))
   {
      return NULL;
   }
   return FormulaSetExtractEntry(set->anchor->succ);
}


/*-----------------------------------------------------------------------
//
// Function: FormulaSetDeleteEntry()
//
//   Delete an element of a formulaset.
//
// Global Variables: 
//
// Side Effects    : 
//
/----------------------------------------------------------------------*/

void FormulaSetDeleteEntry(WFormula_p form)
{
   assert(form);

   FormulaSetExtractEntry(form);
   WFormulaFree(form);
}



/*-----------------------------------------------------------------------
//
// Function: FormulaSetPreprocConjectures()
//
//   Negate all conjectures to make the implication to prove into an
//   formula set that is inconsistent if the implication is true. Note
//   that multiple conjectures are implicitely disjunctively
//   connected! Returns number of conjectures.
//
// Global Variables: -
//
// Side Effects    : Changes formula, may print warning if number of
//                   conjectures is different from 1.
//
/----------------------------------------------------------------------*/

long FormulaSetPreprocConjectures(FormulaSet_p set)
{
   long res = 0;
   WFormula_p handle;

   handle = set->anchor->succ;
   
   while(handle!=set->anchor)
   {
      if(WFormulaConjectureNegate(handle))
      {
         res++;
      }
      handle = handle->succ;
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: FormulaAndClauseSetParse()
//
//   Parse a mixture of clauses and formulas (if the syntax supports
//   it). Return number of elements parsed.
//
// Global Variables: -
//
// Side Effects    : Input, changes termbank and sets.
//
/----------------------------------------------------------------------*/

long FormulaAndClauseSetParse(Scanner_p in, ClauseSet_p cset, 
                              FormulaSet_p fset, TB_p terms)  
{
   long res = 0;
   WFormula_p form;
   Clause_p   clause;
   
   switch(ScannerGetFormat(in))
   {
   case LOPFormat:
         /* LOP does not at the moment support full FOF */
         res = ClauseSetParseList(in, cset, terms);
         break;
   default:
         while(TestInpId(in, "input_formula|input_clause|fof|cnf"))
         {
            if(TestInpId(in, "input_formula|fof"))
            {
               form = WFormulaParse(in, terms);
               FormulaSetInsert(fset, form);
            }
            else
            {
               assert(TestInpId(in, "input_clause|cnf"));
               clause = ClauseParse(in, terms);
               ClauseSetInsert(cset, clause);
            }
            res++;
         }
         break;
   }
   return res;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


