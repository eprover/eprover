/*-----------------------------------------------------------------------

File  : ccl_clausefunc.c

Author: Stephan Schulz

Contents
 
  Clause functions that need to know about sets.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Tue Aug  7 00:02:44 CEST 2001
    New, partitioned ccl_clausesets.h

-----------------------------------------------------------------------*/


#include "ccl_clausefunc.h"


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
// Function: clause_canon_compare()
//
//   Compare two indirectly pointed to clauses with
//   ClauseStructWeightCompare().
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static int clause_canon_compare(const Clause_p *c1, const Clause_p *c2)
{
   return ClauseStructWeightLexCompare(*c1, *c2);
}


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
// Function: ClauseRemoveLiteral()
//
//   Remove *lit from clause, adjusting counters as necessary.
//
// Global Variables: -
//
// Side Effects    : Changes clause and possibly clause->set->literals
//
/----------------------------------------------------------------------*/

void ClauseRemoveLiteral(Clause_p clause, Eqn_p *lit)
{
   Eqn_p handle = *lit;

   if(EqnIsPositive(handle))
   {
      clause->pos_lit_no--;
   }
   else
   {
      clause->neg_lit_no--;
   }
   if(clause->set)
   {
      clause->set->literals--;
   }
   clause->weight -= EqnStandardWeight(handle);
   EqnListDeleteElement(lit);   
}


/*-----------------------------------------------------------------------
//
// Function: ClauseFlipLiteralSign()
//
//   Change the sign of lit, adjusting counters as necessary.
//
// Global Variables: -
//
// Side Effects    : Changes clause.
//
/----------------------------------------------------------------------*/

void ClauseFlipLiteralSign(Clause_p clause, Eqn_p lit)
{
   if(EqnIsPositive(lit))
   {
      clause->pos_lit_no--;
      clause->neg_lit_no++;
   }
   else
   {
      clause->neg_lit_no--;
      clause->pos_lit_no++;
   }
   EqnFlipProp(lit, EPIsPositive);
}


/*-----------------------------------------------------------------------
//
// Function: ClauseKillChildren()
//
//   Remove and delete children of a clause.
//
// Global Variables: -
//
// Side Effects    : Changes term sets
//
/----------------------------------------------------------------------*/

void ClauseKillChildren(Clause_p clause)
{
   Clause_p current;

   while(clause->children)
   {
      current = clause->children->key;
      assert(clause == current->parent1 || clause == current->parent2);
      /* printf("ClauseKillChildren(%d) -> %d\n",
	 (int)clause,(int)current); */
      ClauseDetachParents(current);
      if(!ClauseQueryProp(current, CPIsProtected))
      {
	 ClauseSetDeleteEntry(current);
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: ClauseRemoveSuperfluousLiterals()
//
//   Remove duplicate and trivial negative literals from the
//   clause. Return number of removed literals.
//
// Global Variables: -
//
// Side Effects    : Changes clause, termbanks
//
/----------------------------------------------------------------------*/

int ClauseRemoveSuperfluousLiterals(Clause_p clause)
{
   Eqn_p handle;
   int   removed = 0;

   assert(!ClauseIsAnyPropSet(clause, CPIsDIndexed|CPIsSIndexed));

   removed += EqnListRemoveResolved(&(clause->literals), TBTermEqual);
   removed += EqnListRemoveDuplicates(clause->literals, TBTermEqual);
   
   if(removed)
   {
      clause->neg_lit_no = 0;
      clause->pos_lit_no = 0;   
      handle = clause->literals;
      ClauseDelProp(clause, CPInitial);

      while(handle)
      {
	 if(EqnIsPositive(handle))
	 {
	    clause->pos_lit_no++;
	 }
	 else
	 {
	    clause->neg_lit_no++;
	 }
	 handle = handle->next;
      }
      
      if(clause->set)
      {
	 clause->set->literals-=removed;
      }
   }
   return removed;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetRemoveSuperflousLiterals()
//
//   For all clauses in set remove the trivial and duplicated
//   literals. Return number of literals removed.
//
// Global Variables: -
//
// Side Effects    : Only as described
//
/----------------------------------------------------------------------*/

long ClauseSetRemoveSuperfluousLiterals(ClauseSet_p set)
{
   Clause_p handle;
   long res = 0;

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
	  handle->succ)
   {
      res += ClauseRemoveSuperfluousLiterals(handle);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetCanonize()
//
//   Canonize a clause set by canonizing all
//   clauses, and sorting them in the order defined by
//   ClauseStructWeightCompare(). 
//
// Global Variables: -
//
// Side Effects    : Memory usage.
//
/----------------------------------------------------------------------*/
 
void ClauseSetCanonize(ClauseSet_p set)
{
   Clause_p handle;

   for(handle = set->anchor->succ; handle!= set->anchor;
       handle = handle->succ)
   {
      ClauseRemoveSuperfluousLiterals(handle);
      ClauseCanonize(handle);
   }
   ClauseSetSort(set, clause_canon_compare);
   
   /* printf("Canonized: \n");
      ClauseSetPrint(stdout, set, true); */
}

/*-----------------------------------------------------------------------
//
// Function: ClauseRemoveACResolved()
//
//   Remove AC-resolved literals.
//
// Global Variables: -
//
// Side Effects    : As above
//
/----------------------------------------------------------------------*/

int ClauseRemoveACResolved(Clause_p clause)
{
   int   removed = 0;
   Sig_p sig;

   if(clause->neg_lit_no==0)
   {
      return 0;
   }
   sig = clause->literals->bank->sig;
   removed += EqnListRemoveACResolved(&(clause->literals));
   clause->neg_lit_no -= removed ;
   if(removed)
   {
      ClauseDelProp(clause, CPInitial);
      DocClauseModification(GlobalOut, OutputLevel, clause,
			    inf_ac_resolution, NULL, sig, NULL);	 
   }
   if(clause->set)
   {
      clause->set->literals-=removed;
   }
   return removed;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseUnitSimplifyTest()
//
//   Return true if clause can be simplified by a top-simplify-reflect
//   step with the (non-orientable) unit clause simplifier.
//
// Global Variables: -
//
// Side Effects    : Changes clause and values in set
//
/----------------------------------------------------------------------*/

bool ClauseUnitSimplifyTest(Clause_p clause, Clause_p simplifier)
{
   bool positive,tmp;
   EqnRef handle;
   Eqn_p  simpl;
   
   assert(ClauseIsUnit(simplifier));
   simpl = simplifier->literals;
   assert(EqnIsNegative(simpl)||!EqnIsOriented(simpl));
   
   positive = EqnIsPositive(simpl);
   
   if(EQUIV(positive, ClauseIsPositive(clause)))
   {
      return 0;
   }

   handle = &(clause->literals);

   while(*handle)
   {
      tmp = EqnIsPositive(*handle);
      if(XOR(positive,tmp)&&EqnSubsumeP(simpl,*handle,TBTermEqual))
      {
	 return true;
      }
      handle = &((*handle)->next);
   }
   return false;
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
   FormulaConjunctiveNF(&(form->formula), terms);
   return FormulaToCNF(form->formula, FormulaQueryType(form), 
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

long FormulaToCNF(Formula_p form, ClauseProperties type, ClauseSet_p set, 
                  TB_p terms, VarBank_p fresh_vars)
{
   Formula_p handle;
   long old_clause_number = set->members;
   PStack_p stack = PStackAlloc();
   Clause_p clause;

   /* Skip quantors */
   for(handle = form; handle->op == OpQAll; handle = handle->arg1)
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
         ClauseSetInsert(set, clause);
      }
   }
   PStackFree(stack);
   return set->members - old_clause_number;
}




/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/






