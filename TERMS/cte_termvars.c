/*-----------------------------------------------------------------------

  File  : cte_termvars.c

  Author: Stephan Schulz

  Contents

  Functions for the management of shared variables.

  Copyright 1998, 1999, 2008 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Wed Feb 25 00:48:17 MET 1998

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
// Function: var_named_new()
//
//  Create a new VarBankNamedCell associating name and variable.
//
// Global Variables: -
//
// Side Effects    : memory
//
/----------------------------------------------------------------------*/

VarBankNamed_p var_named_new(Term_p var, char* name)
{
   VarBankNamed_p res;

   //printf("var_named_new()\n");

   res = VarBankNamedCellAlloc();
   res->name = SecureStrdup(name);
   res->var = var;

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: var_named_free()
//
//  free a VarBankNamed structure.
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
// Function: clear_env_stack()
//
//  clear the env stack, removing all named cells
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

VarBank_p VarBankAlloc(TypeBank_p type_bank)
{
   VarBank_p handle;

   handle              = VarBankCellAlloc();
   handle->id          = "Unpaired";
   handle->var_count   = 0;
   handle->fresh_count = 0;
   handle->sort_table  = type_bank;
   handle->max_var     = 0;
   handle->varstacks   = PDArrayAlloc(INITIAL_SORT_STACK_SIZE, 5);
   handle->v_counts    = PDIntArrayAlloc(INITIAL_SORT_STACK_SIZE, 5);
   handle->variables   = PDArrayAlloc(DEFAULT_VARBANK_SIZE, GROW_EXPONENTIAL);
   handle->ext_index   = NULL;
   handle->env         = PStackAlloc();
   handle->shadow      = NULL;
   handle->term_bank   = NULL;
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
   VarBankStack_p stack;
   Term_p var;

   assert(junk);
   StrTreeFree(junk->ext_index);
   PStackFree(junk->env);

   for(i=0; i<PDArraySize(junk->varstacks); i++)
   {
      stack = (VarBankStack_p)PDArrayElementP(junk->varstacks, i);
      if(stack)
      {
         PStackFree(stack);
      }
   }
   PDArrayFree(junk->varstacks);
   PDArrayFree(junk->v_counts);

   for(i=0; i<=junk->max_var; i++)
   {
      if((var = PDArrayElementP(junk->variables, i)))
      {
         TermTopFree(var);
      }
   }
   PDArrayFree(junk->variables);

   if(junk->shadow)
   {
      junk->shadow->shadow = NULL;
   }
   VarBankCellFree(junk);
}

/*-----------------------------------------------------------------------
//
// Function: VarBankPairShadow()
//
//   Pair two variable banks to ensure that they ave consistent
//   id/variable mappings. Primary may contain variables, secondary
//   should be empty.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void VarBankPairShadow(VarBank_p primary, VarBank_p secondary)
{
   long sort;
   PStackPointer i;
   PStack_p varstack;
   Term_p var;

   assert(primary);
   assert(secondary);
   assert(secondary->var_count == 0);

   primary->shadow      = secondary;
   secondary->shadow    = primary;
   primary->id          = "Primary";
   secondary->id        = "Secondary";
   secondary->term_bank = primary->term_bank;
   assert(primary->term_bank);

   // Create primary vars in secondary
   for(sort=0; sort < PDArraySize(primary->varstacks); sort++)
   {
      varstack = PDArrayElementP(primary->varstacks, sort);
      if(varstack)
      {
         for(i=0; i<PStackGetSP(varstack); i++)
         {
            var = PStackElementP(varstack, i);
            assert(!VarIsAltVar(var));
            assert(var->type->type_uid == sort);
            VarBankVarAlloc(secondary, var->f_code, var->type);
         }
      }
   }
   secondary->fresh_count = primary->fresh_count;
}


/*-----------------------------------------------------------------------
//
// Function: VarBankCreateStack()
//
//    Create a stack for variables of the given sort.
//
// Global Variables: -
//
// Side Effects    : memory operations
//
/----------------------------------------------------------------------*/

VarBankStack_p VarBankCreateStack(VarBank_p bank, TypeUniqueID sort)
{
   VarBankStack_p res;

   res = PStackAlloc();
   PDArrayAssignP(bank->varstacks, sort, res);

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: VarBankResetVCounts()
//
//   Reset all the fresh variable counters for the different sorts.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void VarBankResetVCounts(VarBank_p bank)
{
   int i;

   for(i=0; i<PDArraySize(bank->v_counts); i++)
   {
      PDArrayAssignInt(bank->v_counts, i, 0);
   }
}


/*-----------------------------------------------------------------------
//
// Function: VarBankSetVCountsToUsed()
//
//   Set all the fresh variable counters for the different sorts to
//   the maximum number of variables allocated for that sort.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void VarBankSetVCountsToUsed(VarBank_p bank)
{
   int i;
   VarBankStack_p stack;

   for(i=0; i<PDArraySize(bank->v_counts); i++)
   {
      stack = VarBankGetStack(bank,i);
      if(stack)
      {
         PDArrayAssignInt(bank->v_counts, i, PStackGetSP(stack));
      }
      else
      {
         PDArrayAssignInt(bank->v_counts, i, 0);
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
   VarBankResetVCounts(vars);
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
   Term_p handle;
   int i;

   for(i=0; i<PDArraySize(bank->variables); i++)
   {
      handle = PDArrayElementP(bank->variables, i);
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

   for(i=0; i<PDArraySize(bank->variables); i++)
   {
      handle = PDArrayElementP(bank->variables, i);
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
   return PDArrayElementP(bank->variables, -f_code);
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
// Function:var_bank_var_alloc()
//
//   Return a pointer to the newly created variable
//   with the given f_code and sort in the variable bank.
//
// Global Variables: -
//
// Side Effects    : May change variable bank
//
/----------------------------------------------------------------------*/

Term_p var_bank_var_alloc(VarBank_p bank, FunCode f_code, Type_p type)
{
   Term_p var;
   VarBankStack_p stack = VarBankGetStack(bank, type->type_uid);

   assert(!PDArrayElementP(bank->variables, -f_code));

   var = TermDefaultCellAlloc();
   TermCellSetProp(var, TPIsShared);
   if(TypeIsArrow(type))
   {
      TermCellSetProp(var, TPHasEtaExpandableSubterm);
   }

   var->weight = DEFAULT_VWEIGHT;
   var->v_count = 1;
   var->f_count = 0;
   var->entry_no = f_code;
   var->f_code = f_code;
   var->type = type;

   PDArrayAssignP(bank->variables, -f_code, var);
   if(!VarIsAltVar(var))
   {
      PStackPushP(stack, var);
   }
   bank->max_var = MAX(-f_code, bank->max_var);
   bank->var_count++;
   TermSetBank(var, bank->term_bank);

   assert(var->type);
   return var;
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
// Side Effects    : May change variable bank and its shadow
//
/----------------------------------------------------------------------*/

Term_p VarBankVarAlloc(VarBank_p bank, FunCode f_code, Type_p type)
{
   Term_p var = var_bank_var_alloc(bank, f_code, type);
   if(bank->shadow)
   {
      var_bank_var_alloc(bank->shadow, f_code, type);
   }
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

Term_p  VarBankGetFreshVar(VarBank_p bank, Type_p t)
{
   Term_p res;

   int v_count = PDArrayElementInt(bank->v_counts, t->type_uid);
   VarBankStack_p stack = VarBankGetStack(bank, t->type_uid);
   assert(stack);
   if(UNLIKELY(PStackGetSP(stack)<=v_count))
   {
      //printf(COMCHAR" XXX %s %ld %d  ", bank->id, PStackGetSP(stack), v_count);
      bank->fresh_count+=2;
      res = VarBankVarAssertAlloc(bank, -(bank->fresh_count), t);
      //printf("=> %ld %d\n", PStackGetSP(stack), v_count);
      assert(PStackGetSP(stack)>v_count);
      if(bank->shadow)
      {
         bank->shadow->fresh_count = bank->fresh_count;
      }
   }
   else
   {
      res = PStackElementP(stack, v_count);
   }
   v_count++;
   PDArrayAssignInt(bank->v_counts, t->type_uid, v_count);

   return res;
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
      fprintf(stderr, "Alloc no sort %s\n", name);
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

Term_p VarBankExtNameAssertAllocSort(VarBank_p bank, char* name, Type_p type)
{
   Term_p    var;
   StrTree_p handle, test;
   VarBankNamed_p named;

   if(Verbose>=5)
   {
      fprintf(stderr, COMCHAR" Alloc variable %s with sort ", name);
      TypePrintTSTP(stderr, bank->sort_table, type);
      fputc('\n', stderr);
   }

   handle = StrTreeFind(&(bank->ext_index), name);
   if(!handle)
   {
      var = VarBankGetFreshVar(bank, type);
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
      if(var->type != type)
      {
         /* save old var name */
         named = var_named_new(var, name);
         PStackPushP(bank->env, named);

         /* replace by new variable (of given sort) */
         var = VarBankGetFreshVar(bank, type);
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
      var_named_free(named);

      if(test)
      {
         // StrTree insert compares only by the key:
         // if varable with the same name but with the different
         // type exists, then type will not be overwritten.
         test->val1.p_val = handle->val1.p_val;
         test->val2.i_val = handle->val2.i_val;
         FREE(handle->key);
         StrTreeCellFree(handle);
      }
   }
}

/*-----------------------------------------------------------------------
//
// Function: VarBankCardinality()
//
//   Returns the number of variables in the whole var bank
//
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
long VarBankCardinality(VarBank_p bank)
{
   return bank->var_count;
}


/*-----------------------------------------------------------------------
//
// Function: VarBankCollectVars()
//
//   Collect all the variables of the bank onto the given
//   stack. Returns the total number of variables pushed onto the stack.
//
// Global Variables: -
//
// Side Effects    : Modifies stack
//
/----------------------------------------------------------------------*/

long VarBankCollectVars(VarBank_p bank, PStack_p into)
{
   long res = 0;
   FunCode i;
   Term_p t;


   printf("VarBankCollectVars()...\n");
   for (i=0; i < bank->max_var; i++)
   {
      t = PDArrayElementP(bank->variables, i);
      if(t)
      {
         PStackPushP(into, t);
         res ++;
      }
   }
   printf("...VarBankCollectVars()\n");
   return res;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
