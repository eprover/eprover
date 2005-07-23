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
   NormSubstEqnList(lit_list, normsubst, fresh_vars);   
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




/*-----------------------------------------------------------------------
//
// Function: tformula_collect_clause()
//
//   Given a term-encoded formula that is a disjunction of literals,
//   transform it into a clause.
//
// Global Variables: -
//
// Side Effects    : Same as in TFormulaConjunctiveToCNF() below.
//
/----------------------------------------------------------------------*/

Clause_p tformula_collect_clause(TFormula_p form, TB_p terms, 
                                 VarBank_p fresh_vars)
{
   Clause_p res;
   Eqn_p lit_list = NULL, tmp_list = NULL, lit;
   PStack_p stack, lit_stack = PStackAlloc();
   Subst_p  normsubst = SubstAlloc();

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

   VarBankResetVCount(fresh_vars);
   NormSubstEqnList(lit_list, normsubst, fresh_vars);   
   tmp_list = EqnListCopy(lit_list, terms);
   res = ClauseAlloc(tmp_list);
   EqnListFree(lit_list); /* Created just for this */
   SubstDelete(normsubst);                    
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: verify_name()
//
//   If name_selector is NULL, return true. Otherwise, check if
//   info->name is in name_selector. Return true if yes, false
//   otherwise. 
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static bool verify_name(StrTree_p *name_selector, ClauseInfo_p info)
{
   StrTree_p handle;

   if(!(*name_selector))
   {
      return true;
   }
   handle = StrTreeFind(name_selector, info->name);
   if(!handle)
   {
      return false;
   }
   handle->val1.i_val = 1; /* Mark as found */
   return true;
}


/*-----------------------------------------------------------------------
//
// Function: check_all_found()
//
//   Check if all names in name_selector are marked as found. Print a
//   useful error message and terminate otherwise.
//
// Global Variables: -
//
// Side Effects    : Possibly exit, memory operations.
//
/----------------------------------------------------------------------*/

static void check_all_found(Scanner_p in, StrTree_p name_selector)
{
   PStack_p trav_stack, err_stack = PStackAlloc();
   StrTree_p handle;

   trav_stack = StrTreeTraverseInit(name_selector);
   
   while((handle = StrTreeTraverseNext(trav_stack)))
   {
      if(!handle->val1.i_val)
      {
         PStackPushP(err_stack, handle->key);
      }
   }
   StrTreeTraverseExit(trav_stack);

   if(!PStackEmpty(err_stack))
   {
      DStr_p err_str = DStrAlloc();
      char *sep = "";
      PStackPointer i;

      if(in->include_pos)
      {
         DStrSet(err_str, in->include_pos);
         DStrAppendStr(err_str, " ");
      }
      DStrAppendStr(err_str, "\"include\" statement cannot "
                    "find the following requested clauses/formulae in ");
      DStrAppendDStr(err_str, Source(in));
      DStrAppendStr(err_str, ": ");
      
      for(i=0; i< PStackGetSP(err_stack); i++)
      {
         DStrAppendStr(err_str, sep);
         DStrAppendStr(err_str, PStackElementP(err_stack, i));
         sep = ", ";
      }
      Error(DStrView(err_str), INPUT_SEMANTIC_ERROR);
      DStrFree(err_str);
   }   

   PStackFree(err_stack);
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

   if(FormulaQueryProp(wform, WPTypeConjecture)) 
   {
      if(FormulaTermEncoding)
      {
         wform->tformula = TFormulaFCodeAlloc(wform->terms,
                                              wform->terms->sig->not_code,
                                              wform->tformula,
                                              NULL);
      }
      else
      {
         form = FormulaRelRef(wform->formula);
         newform = FormulaOpAlloc(OpUNot, form, NULL);
         wform->formula = FormulaGetRef(newform);
      }
         FormulaSetType(wform, WPTypeNegConjecture);
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
   if(FormulaTermEncoding)
   {
      WTFormulaConjunctiveNF(form, terms);     
      return TFormulaToCNF(form, FormulaQueryType(form), 
                           set, terms, fresh_vars);
   }
   else
   {
      WFormulaConjunctiveNF(form, terms);
      return FormulaToCNF(form, FormulaQueryType(form), 
                          set, terms, fresh_vars);
   }
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

long FormulaSetCNF(FormulaSet_p set, ClauseSet_p clauseset,
                   TB_p terms, VarBank_p fresh_vars)
{
   WFormula_p handle;
   long res = 0;
   
   handle = set->anchor->succ;
   
   while(handle!=set->anchor)
   {
      res += WFormulaCNF(handle,clauseset, terms, fresh_vars);
      if(handle->tformula)
      {
         FormulaSetGCMarkCells(set);
         assert(terms == handle->terms);
         ClauseSetGCMarkTerms(clauseset);
         TBGCSweep(handle->terms);
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
//   it). Return number of elements parsed (even if discarded by
//   filter).
//
// Global Variables: -
//
// Side Effects    : Input, changes termbank and sets.
//
/----------------------------------------------------------------------*/

long FormulaAndClauseSetParse(Scanner_p in, ClauseSet_p cset, 
                              FormulaSet_p fset, TB_p terms, 
                              StrTree_p *name_selector)
{
   long res = 0;
   WFormula_p form, nextform;
   Clause_p   clause, nextclause;
   StrTree_p  stand_in = NULL;

   if(!name_selector)
   {
      name_selector = &stand_in;
   }

   switch(ScannerGetFormat(in))
   {
   case LOPFormat:
         /* LOP does not at the moment support full FOF */
         res = ClauseSetParseList(in, cset, terms);
         break;
   default:
         while(TestInpId(in, "input_formula|input_clause|fof|cnf|include"))
         {
            if(TestInpId(in, "include"))
            {
               StrTree_p new_limit = NULL;
               Scanner_p new_in;
               ClauseSet_p  ncset = ClauseSetAlloc();
               FormulaSet_p nfset = FormulaSetAlloc();
               
               new_in = ScannerParseInclude(in, &new_limit);
               res += FormulaAndClauseSetParse(new_in, 
                                               ncset, 
                                               nfset, 
                                               terms, 
                                               &new_limit);               
               DestroyScanner(new_in);
               StrTreeFree(new_limit);
               ClauseSetInsertSet(cset, ncset);
               FormulaSetInsertSet(fset, nfset);
               assert(ClauseSetEmpty(ncset));
               assert(ClauseSetEmpty(nfset));
               ClauseSetFree(ncset);
               FormulaSetFree(nfset);
            }
            else
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
         }
         break;
   }
   if(*name_selector)
   {
      form = fset->anchor->succ;
      while(form!= fset->anchor)
      {
         nextform = form->succ;
         if(!verify_name(name_selector, form->info))
         {
            FormulaSetDeleteEntry(form);
         }
         form = nextform;
      }
      clause = cset->anchor->succ;
      while(clause!= cset->anchor)
      {
         nextclause = clause->succ;
         if(!verify_name(name_selector, clause->info))
         {
            ClauseSetDeleteEntry(clause);
         }
         clause = nextclause;
      }
      check_all_found(in, *name_selector);
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
// Function: TFormulaConjunctiveToCNF()
//
//   Convert a term-encoded formula from conjunctive normal form into
//   a set of (variable-normalized) clauses. Return number of clauses
//   generated. 
//
// Global Variables: -
//
// Side Effects    : Memory operations, may allocate new variables in
//                   fresh_vars, may create new terms in the term
//                   bank.
//
/----------------------------------------------------------------------*/

long TFormulaToCNF(WFormula_p form, ClauseProperties type, ClauseSet_p set, 
                  TB_p terms, VarBank_p fresh_vars)
{
   TFormula_p handle;
   long old_clause_number = set->members;
   PStack_p stack = PStackAlloc();
   Clause_p clause;

   /* Skip quantors */
   for(handle = form->tformula;
       handle->f_code == terms->sig->qall_code;
       handle = handle->args[1])
   {
      assert(handle);
   }
   PStackPushP(stack, handle);
   while(!PStackEmpty(stack))
   {
      handle = PStackPopP(stack);
      if(handle->f_code == terms->sig->and_code)
      {
         PStackPushP(stack, handle->args[0]);
         PStackPushP(stack, handle->args[1]);
      }
      else
      {
         clause = tformula_collect_clause(handle, terms, fresh_vars);
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


