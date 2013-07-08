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
   handle->sort_table = sort_table;
   handle->max_var = 0;
   handle->stacks = PDArrayAlloc(INITIAL_SORT_STACK_SIZE, 5);
   handle->ext_index = NULL;

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

   for(i=0; i<PDArraySize(junk->stacks); ++i)
   {
      stack = (VarBankStack_p) PDArrayElementP(junk->stacks, i);
      if (! stack)
      {
         continue;
      }
   
      // free stack
      for(j=0; j<stack->f_code_index->size; ++j)
      {
         if(PDArrayElementP(stack->f_code_index, j))
         {
            TermTopFree(PDArrayElementP(stack->f_code_index, j));
         }
      }
      PDArrayFree(stack->f_code_index);
      VarBankStackCellFree(stack);
   }

   VarBankCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: VarBankCreateStack
// Create a stack for the given sort
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

   res = PDArrayElementP(bank->stacks, sort);
   assert(res == NULL);

   res = VarBankStackCellAlloc();
   res->v_count = 0;
   res->f_code_index = PDIntArrayAlloc(DEFAULT_VARBANK_SIZE, GROW_EXPONENTIAL);
   PDArrayAssignP(bank->stacks, sort, (void*)res);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: VarBankSetAllVCount
// sets the v_count of all var stacks to the given f_code
//   
//
// Global Variables: -
//
// Side Effects    : Memory operations on the var bank
//
/----------------------------------------------------------------------*/
void VarBankSetAllVCount(VarBank_p bank, FunCode f_code)
{
   VarBankStack_p stack;
   int i;

   for (i=0; i < PDArraySize(bank->stacks); ++i)
   {
      stack = PDArrayElementP(bank->stacks, i);

      if (stack)
      {
         stack->v_count = f_code;
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: VarBankResetAllVCount
// Resets the v_count of all stacks
//   
//
// Global Variables: -
//
// Side Effects    : memory operations
//
/----------------------------------------------------------------------*/
void VarBankResetAllVCount(VarBank_p bank)
{
   int i;
   VarBankStack_p stack;

   // reset all v_count
   for (i = 0; i < PDArraySize(bank->stacks); ++i)
   {
      stack = (VarBankStack_p) PDArrayElementP(bank->stacks, i);
      if (stack)
      {
         stack->v_count = 0;
      }
   }
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
   VarBankResetAllVCount(vars);
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

   for(i=0; i<PDArraySize(bank->stacks); ++i)
   {
      stack = PDArrayElementP(bank->stacks, i);
      if (!stack)
      {
         continue;
      }

      for(j=0; j<stack->f_code_index->size; j++)
      {
         handle = PDArrayElementP(stack->f_code_index, j);
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

   for(i=0; i<PDArraySize(bank->stacks); ++i)
   {
      stack = PDArrayElementP(bank->stacks, i);
      if (!stack)
      {
         continue;
      }

      for(j=0; j<stack->f_code_index->size; j++)
      {
         handle = PDArrayElementP(stack->f_code_index, j);
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
   return PDArrayElementP(stack->f_code_index, -f_code);
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
// Function: VarBankFCodeAssertAlloc()
//
//   Return a pointer to the variable with the given f_code in the
//   variable bank. Create the variable if it does not exist.
//
// Global Variables: -
//
// Side Effects    : May change variable bank
//
/----------------------------------------------------------------------*/

Term_p VarBankFCodeAssertAlloc(VarBank_p bank, FunCode f_code, SortType sort)
{
   VarBankStack_p stack;
   Term_p    var;
   
   assert(f_code < 0);
   stack = VarBankGetStack(bank, sort);
   var = PDArrayElementP(stack->f_code_index, -f_code);
   if(!var)
   {
      var = TermDefaultCellAlloc();
      var->entry_no = f_code;
      var->f_code = f_code;
      TermCellSetProp(var, TPIsShared);
      PDArrayAssignP(stack->f_code_index, -f_code, var);
      bank->max_var = MAX(-f_code, bank->max_var);
   }
   assert(!TermCellQueryProp(var, TPIsGround));
   return var;
}

/*-----------------------------------------------------------------------
//
// Function: VarBankGetFreshVar()
//
//   Return a pointer to the next "fresh" variable. Freshness is
//   controlled by the v_count entry in the variable bank, which is
//   increased by this function. The variable is only guaranteed to be
//   fresh if VarBankFCodeAssertAlloc() calls are not mixed with
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
   Term_p var;
   VarBankStack_p stack;

   stack = VarBankGetStack(bank, sort);

   stack->v_count+=2;
   
   var = VarBankFCodeAssertAlloc(bank, -stack->v_count, sort);   
   assert(var);
   return var;
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

Term_p VarBankExtNameAssertAlloc(VarBank_p bank, char* name, SortType sort)
{
   Term_p    var;
   StrTree_p handle, test;

   var = VarBankExtNameFind(bank, name);
   if(!var)
   {
      var = VarBankGetFreshVar(bank, sort);
      handle = StrTreeCellAlloc();
      handle->key = SecureStrdup(name);
      handle->val1.p_val = var;
      handle->val2.i_val = var->f_code;
      test = StrTreeInsert(&(bank->ext_index), handle);
      assert(test == NULL);
   }
   return var;
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

   for (i=0; i<PDArraySize(bank->stacks); ++i)
   {
      stack = (VarBankStack_p) PDArrayElementP(bank->stacks, i);
      if (stack)
      {
         res += PDArrayMembers(stack->f_code_index);
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

   for (i=0; i < PDArraySize(bank->stacks); ++i)
   {
      stack = PDArrayElementP(bank->stacks, i);
      if (stack)
      {
         for(j=0; j<PDArraySize(stack->f_code_index); ++j)
         {
            current_term = PDArrayElementP(stack->f_code_index, j);
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


