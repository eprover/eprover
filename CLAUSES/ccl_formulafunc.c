/*-----------------------------------------------------------------------

File  : ccl_formulafunc.c

Author: Stephan Schulz

Contents

  Higher level formula functions that need to know about sets (and
  CNFing).

  Copyright 2004 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Sun Apr  4 14:12:05 CEST 2004
    New

-----------------------------------------------------------------------*/

#include "ccl_formulafunc.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: formula_collect_clause()
//
//   Given a formula that is a disjunction of literals, transform it
//   into a clause.
//
// Global Variables: -
//
// Side Effects    : Same as in FormulaConjunctiveToCNF() below.
//
/----------------------------------------------------------------------*/

Clause_p formula_collect_clause(Formula_p form, TB_p terms, 
                                VarBank_p fresh_vars)
{
   Clause_p res;
   Eqn_p lit_list = NULL, tmp_list = NULL, lit;
   PStack_p stack, lit_stack = PStackAlloc();
   Subst_p  normsubst = SubstAlloc();

   stack = PStackAlloc();
   PStackPushP(stack, form);
   while(!PStackEmpty(stack))
   {
      form = PStackPopP(stack);
      switch(form->op)
      {
      case OpIsLit:
            PStackPushP(lit_stack, form->special.literal);
            EqnDelProp(form->special.literal, EPIsUsed);
            break;
      case OpBOr:
            PStackPushP(stack, form->arg1);
            PStackPushP(stack, form->arg2);
            break;
      default:
            assert(false && "Formula does not describe a clause.");
            break;
      }
   }
   PStackFree(stack);
   while(!PStackEmpty(lit_stack))
   {
      lit = PStackPopP(lit_stack);
      if(!EqnQueryProp(lit, EPIsUsed))
      {
         EqnListInsertFirst(&lit_list, lit);
         EqnSetProp(lit, EPIsUsed);
      }
   }
   PStackFree(lit_stack);

   VarBankResetVCount(fresh_vars);
   NormSubstEqnList(tmp_list, normsubst, fresh_vars);   
   tmp_list = EqnListCopy(lit_list, terms);
   res = ClauseAlloc(tmp_list);   
   /* ClausePrint(stdout, res, true);
      printf("\n");*/
   /* We could disassemble lit_list now, but don't have to. tmp_list
    * is, *OF COURSE* consumed by ClauseAlloc(). This comment has no
    * background story.*/
   SubstDelete(normsubst);                    
   return res;
}



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


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

   if(FormulaQueryProp(wform, WPTypeConjecture)
      &&!FormulaQueryProp(wform,WPInitialConjecture)) 
   {
      form = FormulaRelRef(wform->formula);
      newform = FormulaOpAlloc(OpUNot, form, NULL);
      wform->formula = FormulaGetRef(newform);
      FormulaSetProp(wform, WPInitialConjecture);
      DocFormulaModificationDefault(wform, inf_neg_conjecture);
      return true;
   }
   return false;
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
// Function: WFormulaCNF()
//
//   Transform the formula of a wrapped formula into CNF.
//
// Global Variables: -
//
// Side Effects    : Changes formula, memory operations
//
/----------------------------------------------------------------------*/

long WFormulaCNF(WFormula_p form, ClauseSet_p set, 
                 TB_p terms, VarBank_p fresh_vars)
{
   WFormulaConjunctiveNF(form, terms);
   return FormulaToCNF(form, FormulaQueryType(form), 
                       set, terms, fresh_vars);
}


/*-----------------------------------------------------------------------
//
// Function: FormulaSetCNF()
//
//   Transform all formulae in set into CNF. Return number of clauses
//   generated. 
//
// Global Variables: -
//
// Side Effects    : Changes formulae (yes, really)
//
/----------------------------------------------------------------------*/

void FormulaSetCNF(FormulaSet_p set, ClauseSet_p clauseset,
                   TB_p terms, VarBank_p fresh_vars)
{
   WFormula_p handle;
   long res = 0;
   
   handle = set->anchor->succ;
   
   while(handle!=set->anchor)
   {
      res += WFormulaCNF(handle,clauseset, terms, fresh_vars);
      handle = handle->succ;
   }
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


/*-----------------------------------------------------------------------
//
// Function: FormulaConjunctiveToCNF()
//
//   Convert a formula from conjunctive normal form into a set of
//   (variable-normalized) clauses. Return number of clauses
//   generated. 
//
// Global Variables: -
//
// Side Effects    : Memory operations, may allocate new variables in
//                   fresh_vars, may create new terms in the term
//                   bank, manipulates next pointers in literals (they
//                   keep pointing somewhere, but that should have no
//                   significance). 
//
/----------------------------------------------------------------------*/

long FormulaToCNF(WFormula_p form, ClauseProperties type, ClauseSet_p set, 
                  TB_p terms, VarBank_p fresh_vars)
{
   Formula_p handle;
   long old_clause_number = set->members;
   PStack_p stack = PStackAlloc();
   Clause_p clause;

   /* Skip quantors */
   for(handle = form->formula; handle->op == OpQAll; handle = handle->arg1)
   {
      assert(handle);
   }
   PStackPushP(stack, handle);
   while(!PStackEmpty(stack))
   {
      handle = PStackPopP(stack);
      if(handle->op == OpBAnd)
      {
         PStackPushP(stack, handle->arg1);
         PStackPushP(stack, handle->arg2);
      }
      else
      {
         clause = formula_collect_clause(handle, terms, fresh_vars);
         ClauseSetTPTPType(clause, type);
         DocClauseFromForm(GlobalOut, OutputLevel, clause, form);
         ClauseSetInsert(set, clause);
      }
   }
   PStackFree(stack);
   return set->members - old_clause_number;
}


/*-----------------------------------------------------------------------
//
// Function: FormulaSetDocInital()
//
//   If level >= 2, print all formula as initials.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void FormulaSetDocInital(FILE* out, long level, FormulaSet_p set)
{
   WFormula_p handle;
   
   if(level>=2)
   {
      for(handle = set->anchor->succ; handle!=set->anchor; handle =
	     handle->succ)
      {
	 DocFormulaCreationDefault(handle, inf_initial, NULL,NULL);
      }
   }   
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


