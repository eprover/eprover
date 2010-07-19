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

VarBank_p VarBankAlloc(void)
{
   VarBank_p handle;

   handle = VarBankCellAlloc();
   handle->v_count = 0;
   handle->max_var = 0;
   handle->ext_index = NULL;
   handle->f_code_index = PDIntArrayAlloc(DEFAULT_VARBANK_SIZE,
					  GROW_EXPONENTIAL);

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
   int i;

   assert(junk);
   StrTreeFree(junk->ext_index);
   
   for(i=0; i<junk->f_code_index->size; i++)
   {
      if(PDArrayElementP(junk->f_code_index, i))
      {
	 TermTopFree(PDArrayElementP(junk->f_code_index, i));
      }
   }
   PDArrayFree(junk->f_code_index);
   VarBankCellFree(junk);
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
   vars->v_count = 0;
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
   Term_p handle;
   int i;

   for(i=0; i<bank->f_code_index->size; i++)
   {
      handle = PDArrayElementP(bank->f_code_index, i);
      if(handle)
      {         
	 TermCellSetProp(handle, prop);
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
   Term_p handle;
   int i;

   for(i=0; i<bank->f_code_index->size; i++)
   {
      handle = PDArrayElementP(bank->f_code_index, i);
      if(handle)
      {         
	 TermCellDelProp(handle, prop);
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

Term_p VarBankFCodeFind(VarBank_p bank, FunCode f_code)
{   
   assert(f_code<0); 
   return PDArrayElementP(bank->f_code_index, -f_code);
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

Term_p VarBankFCodeAssertAlloc(VarBank_p bank, FunCode f_code)
{
   Term_p    var;
   
   assert(f_code < 0);
   var = VarBankFCodeFind(bank, f_code);
   if(!var)
   {
      var = TermDefaultCellAlloc();
      var->entry_no = f_code;
      var->f_code = f_code;
      TermCellSetProp(var, TPIsShared);
      PDArrayAssignP(bank->f_code_index, -f_code, var);
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

Term_p  VarBankGetFreshVar(VarBank_p bank)
{
   Term_p var;

   bank->v_count+=2;
   
   var = VarBankFCodeAssertAlloc(bank, -bank->v_count);   
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

Term_p VarBankExtNameAssertAlloc(VarBank_p bank, char* name)
{
   Term_p    var;
   StrTree_p handle, test;

   var = VarBankExtNameFind(bank, name);
   if(!var)
   {
      var = VarBankGetFreshVar(bank);
      handle = StrTreeCellAlloc();
      handle->key = SecureStrdup(name);
      handle->val1.p_val = var;
      handle->val2.i_val = var->f_code;
      test = StrTreeInsert(&(bank->ext_index), handle);
      assert(test == NULL);
   }
   return var;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


