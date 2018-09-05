/*-----------------------------------------------------------------------

File  : ccl_splitting.c

Author: Stephan Schulz

Contents

  Implements functions for destructive splitting of clauses with at
  least two non-propositional variable disjoint subsets of literals.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
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
// Function: initialize_lit_table()
//
//   Initialize the literal table. For each literal, mark them as
//   unassigned to any art and collect the variables that are marked
//   by var_filter. If ground literals are not split off individually,
//   assign them to partition 1.
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
   int i, lit_no = ClauseLiteralNumber(clause);
   Eqn_p handle = clause->literals;

   for(i=0; i<lit_no; i++)
   {
      assert(handle);
      lit_table[i].literal = handle;
      lit_table[i].part    = 0;
      lit_table[i].varset  = NULL;
      (void)EqnCollectPropVariables(handle, &(lit_table[i].varset),
              var_filter);
      /* We are only calling EqnCollectVariables for the side
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
// Function: cond_init_lit_table()
//
//   Initialize the literal table. For each literal, mark them as
//   unassigned to any art and collect the variables that are marked
//   by var_filter. If ground literals are not split off individually,
//   assign them to partition 1.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

static int cond_init_lit_table(LitSplitDesc_p lit_table,
                                Clause_p clause, SplitType how,
                                PStack_p split_vars)
{
   int split_var_no;

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
   return split_var_no;
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
// Function: assemble_part_literals()
//
//   Given a partition number, assemble and return all literals
//   belonging to that partition.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Eqn_p assemble_part_literals(LitSplitDesc_p lit_table, int lit_no, int part)
{
   Eqn_p handle = NULL, tmp;
   int j;

   for(j=0; j<lit_no; j++)
   {
      if(lit_table[j].part == part)
      {
         tmp = lit_table[j].literal;
         tmp->next = handle;
         handle = tmp;
      }
   }
   return handle;
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
//   If fresh_defs is false (and split_vars is empty), try if an
//   existing definition already covers any of the parts and reuse it,
//   otherwise always use fresh definitions.
//
// Global Variables: -
//
// Side Effects    : Memory allocation, if successful, destroys
//                   clause!
//
/----------------------------------------------------------------------*/

int clause_split_general(DefStore_p store, Clause_p clause,
                         ClauseSet_p set, SplitType how,
                         bool fresh_defs, PStack_p split_vars)
{
   int               res = 0, part = 0,i,size, lit_no, split_var_no;
   LitSplitDesc_p    lit_table;
   Eqn_p             handle,tmp, join;
   FunCode           new_pred;
   long              def_id;
   TB_p              bank;
   Clause_p          new_clause;
   FormulaProperties props;

   assert(clause);
   assert(!clause->set);
   assert(set);

   props = ClauseGiveProps(clause, CPTypeMask|CPIsSOS);
   lit_no = ClauseLiteralNumber(clause);

   if(lit_no<=1 || ClauseHasSplitLiteral(clause))
   {
      return 0;
   }
   bank = clause->literals->bank;
   size = lit_no*sizeof(LitSplitDescCell);
   lit_table = SizeMalloc(size);

   split_var_no = cond_init_lit_table(lit_table, clause, how, split_vars);

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
      PStack_p def_stack = PStackAlloc();

      /* Build split clauses from original literals */
      join = NULL;
      clause->literals = NULL; /* Literals are stored in lit_table and
              are recycled in new clauses, clause
              skeleton is refilled below. */
      for(i=1; i<=part; i++)
      {
         if(split_var_no)
         {
            /* Get new predicate code */
            new_pred = SigGetNewPredicateCode(bank->sig,
                                              split_var_no);
            SigSetFuncProp(bank->sig, new_pred, FPClSplitDef);

            /* Create definition clause (for maintaining completeness) */
            handle = GenDefLit(bank, new_pred, true, split_vars);
            assert(!handle->next);

            handle->next = assemble_part_literals(lit_table, lit_no, i);

            new_clause = ClauseAlloc(handle);
            assert(new_clause);

            def_id = new_clause->ident;
            assert(def_id);
            assert(new_pred);
         }
         else
         {
            WFormula_p new_form;
            /* Create definition clause (for maintaining completeness) */
            handle = assemble_part_literals(lit_table, lit_no, i);

            new_pred = GetDefinitions(store, handle,
                                      &new_form, &new_clause, fresh_defs);
            def_id = 0;
            if(new_form)
            {
               def_id = new_form->ident;
               ClausePushDerivation(clause, DCApplyDef, new_form, NULL);
            }
         }
         PStackPushInt(def_stack, def_id);

         if(new_clause)
         {
            /* Fix clause properties */

            new_clause->properties = props;
            /* Note: Potentially recycled definitions have no real
               parents, as they are conceptually introduced ad-hoc and
               can be reused for many other clauses. */

            /* Insert result clause */
            ClauseSetInsert(set, new_clause);

            /* Document creation -> Now done in ccl_def_handling.c*/
            /* DocClauseCreationDefault(new_clause, inf_split, clause,
               NULL); */
         }
         /* Extend remainder clause (after application of definition) */
         tmp       = GenDefLit(bank, new_pred, false, split_vars);
         tmp->next = join;
         join = tmp;
      }
      clause->literals = join;
      ClauseRecomputeLitCounts(clause);

      ClauseSetInsert(set, clause);
      DocClauseApplyDefsDefault(clause, clause->ident, def_stack);

      res = part+1;
      PStackFree(def_stack);
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

int ClauseSplit(DefStore_p store, Clause_p clause, ClauseSet_p set,
                SplitType how, bool fresh_defs)
{
   int res ;
   PStack_p dummy = PStackAlloc();

   res = clause_split_general(store, clause, set, how, fresh_defs, dummy);
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

int ClauseSplitGeneral(DefStore_p store, bool fresh_defs,
                       Clause_p clause, ClauseSet_p set, long tries)
{
   int res, var_no, set_size;
   PStackPointer i;
   PStack_p vars, split_vars, permute_stack;
   PTree_p  vars_tree = NULL;

   res =  ClauseSplit(store, clause, set, SplitGroundOne, fresh_defs);
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
      res = clause_split_general(store, clause, set, SplitGroundNone,
                                 fresh_defs, split_vars);
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

long ClauseSetSplitClauses(DefStore_p store, ClauseSet_p from_set,
                           ClauseSet_p to_set, SplitType how, bool fresh_defs)
{
   long res=0, tmp;
   Clause_p handle;

   assert(from_set&&to_set);

   while(!ClauseSetEmpty(from_set))
   {
      handle = ClauseSetExtractFirst(from_set);
      tmp = ClauseSplit(store, handle, to_set, how, fresh_defs);
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

long ClauseSetSplitClausesGeneral(DefStore_p store, bool fresh_defs,
                                  ClauseSet_p from_set,
                                  ClauseSet_p to_set, long tries)
{
   long res=0, tmp;
   Clause_p handle;

   assert(from_set&&to_set);

   while(!ClauseSetEmpty(from_set))
   {
      handle = ClauseSetExtractFirst(from_set);
      tmp = ClauseSplitGeneral(store, fresh_defs, handle, to_set, tries);
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
