/*-----------------------------------------------------------------------

File  : cte_termvars.c

Author: Stephan Schulz

Contents

  Functions for the management of shared variables.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed Feb 25 00:48:17 MET 1998
    new

-----------------------------------------------------------------------*/

#include "cte_termvars.h"



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
// Function: var_named_new
//  create a new VarBankNamed_p
//
//
// Global Variables: -
//
// Side Effects    : memory
//
/----------------------------------------------------------------------*/
VarBankNamed_p var_named_new(Term_p var, char* name)
{
   VarBankNamed_p res;

   res = VarBankNamedCellAlloc();
   res->name = SecureStrdup(name);
   res->var = var;

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: var_named_free
//  free a VarBankNamed_p
//
//
// Global Variables: -
//
// Side Effects    : memory
//
/----------------------------------------------------------------------*/
void var_named_free(VarBankNamed_p junk)
{
   FREE(junk->name);
   VarBankNamedCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: clear_env_stack
//  clear the env stack, removing all named cells
//
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/
void clear_env_stack(VarBank_p bank)
{
   VarBankNamed_p named;

   while(!PStackEmpty(bank->env))
   {
      named = PStackPopP(bank->env);
      if(named)
      {
         var_named_free(named);
      }
   }
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function:  VarBankAlloc()
//
//   Allocate an empty, initialized VarBank-Structure, return pointer
//   to it.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

VarBank_p VarBankAlloc(SortTable_p sort_table)
{
   VarBank_p handle;

   handle = VarBankCellAlloc();
   handle->v_count = 0;
   handle->sort_table = sort_table;
   handle->max_var = 0;
   handle->stacks = PDArrayAlloc(INITIAL_SORT_STACK_SIZE, 5);
   handle->ext_index = NULL;
   handle->env = PStackAlloc();
   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: VarBankFree()
//
//   Deallocate a VarBankCell.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void VarBankFree(VarBank_p junk)
{
   int i, j;
   VarBankStack_p stack;

   assert(junk);
   StrTreeFree(junk->ext_index);
   PStackFree(junk->env);

   for(i=0; i<PDArraySize(junk->stacks); i++)
   {
      stack = (VarBankStack_p) PDArrayElementP(junk->stacks, i);
      if (! stack)
      {
         continue;
      }

      // free stack
      for(j=0; j<PDArraySize(stack); j++)
      {
         if(PDArrayElementP(stack, j))
         {
            TermTopFree(PDArrayElementP(stack, j));
         }
      }
      PDArrayFree(stack);
   }
   PDArrayFree(junk->stacks);

   VarBankCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: VarBankCreateStack
//    Create a stack for the given sort
//
//
// Global Variables: -
//
// Side Effects    : memory operations
//
/----------------------------------------------------------------------*/
VarBankStack_p VarBankCreateStack(VarBank_p bank, SortType sort)
{
   VarBankStack_p res;

   if (sort >= PDArraySize(bank->stacks))
   {
      PDArrayEnlarge(bank->stacks, sort);
   }

   res = PDArrayAlloc(DEFAULT_VARBANK_SIZE, GROW_EXPONENTIAL);
   PDArrayAssignP(bank->stacks, sort, res);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: VarBankClearExtNames()
//
//   Reset the External name -> FunCode association state
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void VarBankClearExtNames(VarBank_p vars)
{
   VarBankClearExtNamesNoReset(vars);
   VarBankResetVCount(vars);
   clear_env_stack(vars);
}


/*-----------------------------------------------------------------------
//
// Function: VarBankClearExtNamesNoReset()
//
//   Reset the External name -> FunCode association state, but do not
//   reset the variable counter
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void VarBankClearExtNamesNoReset(VarBank_p vars)
{
   StrTreeFree(vars->ext_index);
   vars->ext_index = NULL;
   clear_env_stack(vars);
}

/*-----------------------------------------------------------------------
//
// Function: VarBankVarsSetProp()
//
//   Set the given properties in all variables.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void VarBankVarsSetProp(VarBank_p bank, TermProperties prop)
{
   VarBankStack_p stack;
   Term_p handle;
   int i, j;

   for(i=0; i<PDArraySize(bank->stacks); i++)
   {
      stack = PDArrayElementP(bank->stacks, i);
      if (!stack)
      {
         continue;
      }

      for(j=0; j < stack->size; j++)
      {
         handle = PDArrayElementP(stack, j);
         if(handle)
         {
            TermCellSetProp(handle, prop);
         }
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: VarBankVarsDelProp()
//
//   Delete the given properties in all variables.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void VarBankVarsDelProp(VarBank_p bank, TermProperties prop)
{
   VarBankStack_p stack;
   Term_p handle;
   int i, j;

   for(i=0; i<PDArraySize(bank->stacks); i++)
   {
      stack = PDArrayElementP(bank->stacks, i);
      if (!stack)
      {
         continue;
      }

      for(j=0; j < stack->size; j++)
      {
         handle = PDArrayElementP(stack, j);
         if(handle)
         {
            TermCellDelProp(handle, prop);
         }
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function:  VarBankFCodeFind()
//
//   Return the pointer to the variable associated with given f_code
//   if it exists in the VarBank, NULL otherwise.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p VarBankFCodeFind(VarBank_p bank, FunCode f_code, SortType sort)
{
   VarBankStack_p stack;

   assert(f_code<0);
   stack = VarBankGetStack(bank, sort);
   return PDArrayElementP(stack, -f_code);
}


/*-----------------------------------------------------------------------
//
// Function:  VarBankExtNameFind()
//
//   Return the pointer to the variable associated with given external
//   name if it exists in the VarBank, NULL otherwise.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p VarBankExtNameFind(VarBank_p bank, char* name)
{
   StrTree_p entry;

   entry = StrTreeFind(&(bank->ext_index), name);

   if(entry)
   {
      return entry->val1.p_val;
   }
   return NULL;
}


/*-----------------------------------------------------------------------
//
// Function: VarBankVarAlloc()
//
//   Return a pointer to the newly created variable
//   with the given f_code and sort in the variable bank.
//
// Global Variables: -
//
// Side Effects    : May change variable bank
//
/----------------------------------------------------------------------*/

Term_p VarBankVarAlloc(VarBank_p bank, FunCode f_code, SortType sort)
{
   Term_p var;

   var = TermDefaultCellAlloc();
   TermCellSetProp(var, TPIsShared);

   var->weight = DEFAULT_VWEIGHT;
   var->v_count = 1;
   var->f_count = 0;
   var->entry_no = f_code;
   var->f_code = f_code;
   var->sort = sort;

   PDArrayAssignP(VarBankGetStack(bank, sort), -f_code, var);
   bank->max_var = MAX(-f_code, bank->max_var);

   assert(var->sort != STNoSort);

   return var;
}

/*-----------------------------------------------------------------------
//
// Function: VarBankGetFreshVar()
//
//   Return a pointer to the next "fresh" variable. Freshness is
//   controlled by the v_count entry in the variable bank, which is
//   increased by this function. The variable is only guaranteed to be
//   fresh if VarBankVarAssertAlloc() calls are not mixed with
//   VarBankGetFreshVar() calls.
//
//   As of 2010-02-10 this will only return even numbered variables -
//   odd ones are reserved to create clause copies that are
//   guaranteed to be variable-disjoint.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

Term_p  VarBankGetFreshVar(VarBank_p bank, SortType sort)
{
   bank->v_count+=2;

   return VarBankVarAssertAlloc(bank, -(bank->v_count), sort);
}


/*-----------------------------------------------------------------------
//
// Function: VarBankExtNameAssertAlloc()
//
//   Return a pointer to the variable with the given external name in
//   the variable bank. Create a new variable if none with the given
//   name exists and assign it the next unused FunCode.
//
// Global Variables: -
//
// Side Effects    : May change variable bank
//
/----------------------------------------------------------------------*/

Term_p VarBankExtNameAssertAlloc(VarBank_p bank, char* name)
{
   Term_p    var;
   StrTree_p handle, test;

   if(Verbose>=5)
   {
      fprintf(stderr, "alloc no sort %s\n", name);
   }

   var = VarBankExtNameFind(bank, name);

   if(!var)
   {
      var = VarBankGetFreshVar(bank, bank->sort_table->default_type);
      handle = StrTreeCellAlloc();
      handle->key = SecureStrdup(name);
      handle->val1.p_val = var;
      handle->val2.i_val = var->f_code;
      test = StrTreeInsert(&(bank->ext_index), handle);
      UNUSED(test); assert(test == NULL);
   }

   return var;
}

/*-----------------------------------------------------------------------
//
// Function: VarBankExtNameAssertAllocSort()
//
//   Return a pointer to the variable with the given external name
//   and sort in the variable bank. Create a new variable if none with
//   the given name exists and assign it the next unused FunCode.
//
// Global Variables: -
//
// Side Effects    : May change variable bank
//
/----------------------------------------------------------------------*/

Term_p VarBankExtNameAssertAllocSort(VarBank_p bank, char* name, SortType sort)
{
   Term_p    var;
   StrTree_p handle, test;
   VarBankNamed_p named;

   if(Verbose>=5)
   {
      fprintf(stderr, "alloc sort %s with sort ", name);
      SortPrintTSTP(stderr, bank->sort_table, sort);
      fputc('\n', stderr);
   }

   handle = StrTreeFind(&(bank->ext_index), name);
   if(!handle)
   {
      var = VarBankGetFreshVar(bank, sort);
      handle = StrTreeCellAlloc();
      handle->key = SecureStrdup(name);
      handle->val1.p_val = var;
      handle->val2.i_val = var->f_code;
      test = StrTreeInsert(&(bank->ext_index), handle);
      UNUSED(test); assert(test == NULL);
   }
   else
   {
      var = handle->val1.p_val;
      if(var->sort != sort)
      {
         /* save old var name */
         named = var_named_new(var, name);
         PStackPushP(bank->env, named);

         /* replace by new variable (of given sort) */
         var = VarBankGetFreshVar(bank, sort);
         handle->val1.p_val = var;
         handle->val2.i_val = var->f_code;
      }
   }

   return var;
}

/*-----------------------------------------------------------------------
//
// Function: VarBankPushEnv
//  enter a new environment for variables. The next VarBankPopEnv will
//  forget about all ext variables defined in between. This is useful
//  when parsing, if several variables share the same name but
//  not the same type
//
//
// Global Variables: -
//
// Side Effects    : modifies ext_index and env
//
/----------------------------------------------------------------------*/
void VarBankPushEnv(VarBank_p bank)
{
   PStackPushP(bank->env, NULL);
}


/*-----------------------------------------------------------------------
//
// Function: VarBankPopEnv
//  pops env frames until the env stack is empty, or NULL is obtained;
//  for each such frame, associate the corresponding name with its variable
//
//
// Global Variables: -
//
// Side Effects    : Modifies bank->env
//
/----------------------------------------------------------------------*/
void VarBankPopEnv(VarBank_p bank)
{
   StrTree_p handle, test;
   VarBankNamed_p named;

   while(!PStackEmpty(bank->env) && (named = PStackPopP(bank->env)))
   {
      handle = StrTreeCellAlloc();
      handle->key = SecureStrdup(named->name);
      handle->val1.p_val = named->var;
      handle->val2.i_val = named->var->f_code;
      test = StrTreeInsert(&(bank->ext_index), handle);

      if(test)
      {
         StrTreeCellFree(handle);  /* already present */
      }
   }
}

/*-----------------------------------------------------------------------
//
// Function: VarBankCardinal
// Returns the number of variables in the whole var bank
//
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
long VarBankCardinal(VarBank_p bank)
{
   VarBankStack_p stack;
   long res = 0;
   int i;

   for (i=0; i < PDArraySize(bank->stacks); i++)
   {
      stack = PDArrayElementP(bank->stacks, i);
      if (stack)
      {
         res += PDArrayMembers(stack);
      }
   }

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: VarBankCollectVars
// Collect all the variables of the bank into the given stack. Returns
// the number of variables pushed in the bank.
//
//
// Global Variables: -
//
// Side Effects    : Modifies stack
//
/----------------------------------------------------------------------*/
long VarBankCollectVars(VarBank_p bank, PStack_p into)
{
   long res = 0;
   VarBankStack_p stack;
   int i;
   FunCode j;
   Term_p current_term;

   for (i=0; i < PDArraySize(bank->stacks); i++)
   {
      stack = PDArrayElementP(bank->stacks, i);
      if (stack)
      {
         for(j=0; j<PDArraySize(stack); j++)
         {
            current_term = PDArrayElementP(stack, j);
            PStackPushP(into, current_term);
            res ++;
         }
      }
   }

   return res;
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


