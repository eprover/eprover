/*-----------------------------------------------------------------------

File  : ccl_splitting.c

Author: Stephan Schulz

Contents
 
  Implements functions for destructive splitting of clauses with at
  least two non-propositional variable disjoint subsets of literals.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Wed Apr 18 18:24:18 MET DST 2001
    New

-----------------------------------------------------------------------*/

#include "ccl_splitting.h"



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
// Function: gen_prop_lit()
//
//   Generate a propositional literal with terms from bank.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static Eqn_p gen_prop_lit(TB_p bank, FunCode pred, bool positive)
{
   Term_p lside;
   Eqn_p  res;

   assert(bank);assert(pred > 0);
   assert(SigFindArity(bank->sig, pred) == 0);
   
   lside = TBTermtopInsert(bank, TermConstCellAlloc(pred));
   res = EqnAlloc(lside, bank->true_term, bank, positive);
   EqnSetProp(res, EPIsSplitLit);
   
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: gen_split_lit()
//
//   Generate a split literal with terms from bank.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static Eqn_p gen_split_lit(TB_p bank, FunCode pred, bool positive,
			  PStack_p split_vars)
{
   Term_p lside;
   Eqn_p  res;

   assert(bank);assert(pred > 0);
   assert(SigFindArity(bank->sig, pred) == PStackGetSP(split_vars));
   
   if(PStackEmpty(split_vars))
   {
      lside = TermConstCellAlloc(pred);
   }
   else
   {
      int arity = PStackGetSP(split_vars), i;
      
      lside = TermDefaultCellAlloc();
      lside->f_code = pred;
      lside->arity = arity;
      lside->args = TermArgArrayAlloc(arity);
      for(i=0; i<arity; i++)
      {
	 lside->args[i] = PStackElementP(split_vars, i);
      }
   }   
   lside = TBTermtopInsert(bank, lside);
   res = EqnAlloc(lside, bank->true_term, bank, positive);
   EqnSetProp(res, EPIsSplitLit);
   
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: initialize_lit_table()
//
//   Initialize the literal table.
//
// Global Variables: 
//
// Side Effects    : 
//
/----------------------------------------------------------------------*/

static void initialize_lit_table(LitSplitDesc_p lit_table,Clause_p
				 clause, SplitType how, TermProperties
				 var_filter)
{
   int i, j, lit_no = ClauseLiteralNumber(clause);
   Eqn_p handle = clause->literals;

   for(i=0; i<lit_no; i++)
   {
      assert(handle);
      lit_table[i].literal = handle;
      lit_table[i].part    = 0;
      lit_table[i].varset  = NULL;
      j = EqnCollectPropVariables(handle, &(lit_table[i].varset),
				  var_filter);
      /* j (mis)used as dummy to stiffle warnings - we are
	 only calling EqnCollectVariables for the side
	 effect here */
      if(how == SplitGroundOne || how == SplitGroundNone)
      {
	 if(!lit_table[i].varset)
	 {
	    lit_table[i].part = 1;
	 }
      }
      handle = handle->next;
   }
}


/*-----------------------------------------------------------------------
//
// Function: find_free_literal()
//
//   Find the first entry in lit_table that corresponds to a literal
//   not yet assigned to any clause part and return its index. If none
//   exists, return -1.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static int find_free_literal(LitSplitDesc_p lit_table, int lit_no)
{
   int res = -1,i;

   for(i=0; i<lit_no; i++)
   {
      if(!lit_table[i].part)
      {
	 res = i;
	 break;
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: build_part()
//
//   Given the index of the first unassigned literal in lit_table and
//   a part number, assign this number to all literals that are
//   transitively variable-linked to this first literal.
//
// Global Variables: -
//
// Side Effects    : Accumulates all part variables in
//                   lit_table[lit_index].varset 
//
/----------------------------------------------------------------------*/

static void build_part(LitSplitDesc_p lit_table, int lit_no, int
		       lit_index, int part)
{
   bool new_vars = true;
   int  i;

   lit_table[lit_index].part = part;
   while(new_vars)
   {
      new_vars = false;
      for(i=lit_index+1; i< lit_no; i++)
      {
	 if(!lit_table[i].part)
	 {
	    if(PTreeSharedElement(&(lit_table[lit_index].varset),
				  lit_table[i].varset))
	    {
	       lit_table[i].part = part;
	       new_vars = PTreeMerge(&(lit_table[lit_index].varset),
				     lit_table[i].varset)||new_vars;
	       lit_table[i].varset = NULL;
	    }
	 }
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: clause_split_general()
//
//   Try to split clause into different clauses according to the
//   inference rule below. If successful,
//   deposit split clauses into set and return number of clauses
//   created. Otherwise return 0.
//
//   L1(X) v L2(X) v L3(X) ...
//   ----------------------------------------------------
//   T1(X) v T2(X) v T3(X) ...., 
//   ~T1(X) v L1(X), ~T2(X) v L2(X), ~T3(X) v L3(X), ...
//
//   if the Li are subsets of the clause that do not share any
//   variables but those in the set X and the Ti
//   are _new_ predicate symbols. X is a parameter to the function!
//
// Global Variables: -
//
// Side Effects    : Memory allocation, if successful, destroys
//                   clause!
//
/----------------------------------------------------------------------*/

int clause_split_general(Clause_p clause, ClauseSet_p set, SplitType
			 how, PStack_p split_vars)
{
   int            res = 0, part = 0,i,j,size, lit_no, split_var_no;
   LitSplitDesc_p lit_table;
   Eqn_p          handle,tmp, join;
   FunCode        new_pred;
   TB_p           bank;
   Clause_p       new;

   assert(clause);
   assert(!clause->children);
   assert(!clause->set);
   assert(set);

   lit_no = ClauseLiteralNumber(clause);

   if(lit_no<=1 || ClauseHasSplitLiteral(clause))
   {
      return 0;
   }
   bank = clause->literals->bank;
   size = lit_no*sizeof(LitSplitDescCell);
   lit_table = SizeMalloc(size);
      
   split_var_no = PStackGetSP(split_vars);
   
   if(!split_var_no)
   {
      initialize_lit_table(lit_table, clause, how, TPIgnoreProps);
   }
   else
   {
      PStackPointer sp;
      Term_p var;

      ClauseTermSetProp(clause, TPCheckFlag);
      for(sp=0; sp<split_var_no; sp++)
      {
	 var = PStackElementP(split_vars,sp);
	 assert(TermCellQueryProp(var, TPCheckFlag));
	 TermCellDelProp(var,TPCheckFlag);	 
      }
      initialize_lit_table(lit_table, clause, how, TPCheckFlag);
   }
   
   if((how == SplitGroundOne) && find_free_literal(lit_table, lit_no))
   {
      part++;
   }
   /* Find the variable disjoint parts of the clause */
   while((i=find_free_literal(lit_table, lit_no))!=-1)
   {
      part++;
      build_part(lit_table, lit_no, i, part);
   }
   if(part>1)
   {
      Clause_p parent1 = clause->parent1;
      Clause_p parent2 = clause->parent2;

      ClauseDetachParents(clause);
      
      /* fprintf(GlobalOut, "# SplitI: ");
      ClausePrint(GlobalOut, clause, true);
      fputc('\n', GlobalOut); */
      
      
      /* Build split clauses from original literals */
      join = NULL;
      clause->literals = NULL; /* Literals are recycled in new
				  clauses, clause skeleton is
				  deallocated below */
      for(i=1; i<=part; i++)
      {
	 new_pred = SigGetNewPredicateCode(bank->sig, split_var_no);
	 tmp    = gen_split_lit(bank, new_pred, false, split_vars);
	 tmp->next = join;
	 join = tmp;
	 handle = gen_split_lit(bank, new_pred, true, split_vars);
	 for(j=0; j<lit_no; j++)
	 {
	    if(lit_table[j].part == i)
	    {
	       tmp = lit_table[j].literal;
	       tmp->next = handle; 
	       handle = tmp;
	    }
	 }
	 new = ClauseAlloc(handle);
	 if(parent1)
	 {
	    new->parent1 = parent1;
	    ClauseRegisterChild(parent1, new);
	 }
	 if(parent2)
	 {
	    new->parent2 = parent2;
	    ClauseRegisterChild(parent2, new);
	 }
	 ClauseSetInsert(set, new);
	 /* if(OutputLevel>=1)
	    {
	    fprintf(GlobalOut, "# Split1: ");
	    ClausePrint(GlobalOut, new, true);
	    fputc('\n',GlobalOut);
	    }*/
	 DocClauseCreationDefault(new, inf_split, clause, NULL);
      }
      new = ClauseAlloc(join);
      if(parent1)
      {
	 new->parent1 = parent1;
	 ClauseRegisterChild(parent1, new);
      }
      if(parent2)
      {
	 new->parent2 = parent2;
	 ClauseRegisterChild(parent2, new);
      }
      ClauseSetInsert(set, new);
      /* if(OutputLevel>=1)
      {
	 fprintf(GlobalOut, "# Split2: ");
	 ClausePrint(GlobalOut, new, true);
	 fputc('\n',GlobalOut);
	 }*/
      DocClauseCreationDefault(new, inf_split, clause, NULL);
      ClauseFree(clause);      /* We still retain the literals in the
				  split clauses! */
      res = part+1;
   }
   for(i=0; i<lit_no; i++)
   {
      PTreeFree(lit_table[i].varset);
   }
   SizeFree(lit_table, size);
   
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: initialize_permute_stack()
//
//   We want to generate unordered n-tuples from k elements. This
//   initializes a stack to contain the first valid sample (1, 2,
//   ...n) of size n.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void initialize_permute_stack(PStack_p stack, int size)
{
   int i;
   
   PStackReset(stack);
   for(i=0; i<size; i++)
   {
      PStackPushInt(stack, i);
   }
}

/*-----------------------------------------------------------------------
//
// Function: permute_stack_next()
//
//   Generate the next valid permutation and return true if it exists,
//   otherwise return false.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static bool permute_stack_next(PStack_p permute_stack, int var_no)
{
   PStackPointer limit = PStackGetSP(permute_stack), i;
   long tmp;
   
   i = 0;
   while(i < limit)
   {
      tmp = PStackPopInt(permute_stack);
      tmp++;
      if((tmp) < (var_no-i))
      {	 
	 while(i>=0)
	 {
	    PStackPushInt(permute_stack, tmp);
	    tmp++;
	    i--;
	 }
	 return true;
      }
      i++;
   }
   return false;
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: ClauseHasSplitLiteral()
//
//   Return true if a literal in the clause is a split literal.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool ClauseHasSplitLiteral(Clause_p clause)
{
   Eqn_p handle;

   for(handle = clause->literals; handle; handle = handle->next)
   {
      if(EqnQueryProp(handle, EPIsSplitLit))
      {
	 return true;
      }
   }
   return false;   
}



/*-----------------------------------------------------------------------
//
// Function: ClauseSplit()
//
//   Try to split clause into different clauses according to the
//   inference rule below. If successful,
//   deposit split clauses into set and return number of clauses
//   created. Otherwise return 0.
//
//   L1 v L2 v L3 ...
//   ----------------------------------------------------
//   T1 v T2 v T3 ...., ~T1 v L1, ~T2 v L2, ~T3 v L3, ...
//
//   if the Li are variable-disjoint subsets of the clause and the Ti
//   are _new_ propositional variables.
//
// Global Variables: -
//
// Side Effects    : Memory allocation, if successful, destroys
//                   clause!
//
/----------------------------------------------------------------------*/

int ClauseSplit(Clause_p clause, ClauseSet_p set, SplitType how)
{
   int res ;
   PStack_p dummy = PStackAlloc();
   
   res = clause_split_general(clause, set, how, dummy);
   PStackFree(dummy);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSplitGeneral()
//
//   Wrapper for clause_split_general(). Tries tries different
//   variable subsets (partially ordered by cardinality) to find a
//   subset that splits the clause. Only used for eground, so I skimp
//   on options.
//
// Global Variables: -
//
// Side Effects    : Memory allocation, if successful, destroys
//                   clause!
//
/----------------------------------------------------------------------*/

int ClauseSplitGeneral(Clause_p clause, ClauseSet_p set, long tries)
{
   int res, var_no, set_size;
   PStackPointer i;
   PStack_p vars, split_vars, permute_stack;
   PTree_p  vars_tree = NULL;

   res =  ClauseSplit(clause, set, SplitGroundOne);
   if(res)
   {
      return res;
   }
   
   var_no = ClauseCollectVariables(clause, &vars_tree);
   if(var_no<=2) /* Non-ground splitting is useless here */
   {
      PTreeFree(vars_tree);
      return 0;
   }
   vars = PStackAlloc();
   split_vars = PStackAlloc();
   permute_stack = PStackAlloc();

   PTreeToPStack(vars, vars_tree);
   PTreeFree(vars_tree);   

   set_size = 1;
   
   initialize_permute_stack(permute_stack, set_size);
   
   while(tries)
   {      
      PStackReset(split_vars);
      for(i=0; i<set_size; i++)
      {
	 PStackPushP(split_vars, 
		     PStackElementP(vars,
				    PStackElementInt(permute_stack, i)));	 
      }
      res = clause_split_general(clause, set, SplitGroundNone, split_vars);
      if(res)
      {
	 break;
      }
      if(!permute_stack_next(permute_stack, var_no))
      {
	 if(set_size == var_no-2)
	 {
	    break;
	 }
	 set_size++;
	 initialize_permute_stack(permute_stack, set_size);
      }
      tries--;
   }
   PStackFree(permute_stack);
   PStackFree(split_vars);
   PStackFree(vars);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetSplitClauses()
//
//   Split all clauses in from_set and put the result into to_set.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ClauseSetSplitClauses(ClauseSet_p from_set, ClauseSet_p to_set,
			  SplitType how)
{
   long res=0, tmp;
   Clause_p handle;

   assert(from_set&&to_set);

   while(!ClauseSetEmpty(from_set))
   {
      handle = ClauseSetExtractFirst(from_set);
      tmp = ClauseSplit(handle, to_set, how);
      if(!tmp)
      {
	 ClauseSetInsert(to_set, handle);
      }
      res += tmp;
   }
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: ClauseSetSplitClausesGeneral()
//
//   Split all clauses in from_set and put the result into to_set.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ClauseSetSplitClausesGeneral(ClauseSet_p from_set, ClauseSet_p
				 to_set, long tries)
{
   long res=0, tmp;
   Clause_p handle;

   assert(from_set&&to_set);

   while(!ClauseSetEmpty(from_set))
   {
      handle = ClauseSetExtractFirst(from_set);
      tmp = ClauseSplitGeneral(handle, to_set, tries);
      if(!tmp)
      {
	 ClauseSetInsert(to_set, handle);
      }
      res += tmp;
   }
   return res;
}




/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


