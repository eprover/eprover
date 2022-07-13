/*-----------------------------------------------------------------------

  File  : cte_dbvars.c

  Author: Petar Vukmirovic

  Contents

  Functions for the management of shared De Bruijn variables.

  Copyright 1998, 1999, 2021 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: di 6 jul 2021 11:13:09 CEST

-----------------------------------------------------------------------*/

#include "cte_dbvars.h"


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

Term_p mk_db(FunCode db_idx, Type_p type)
{
   assert(db_idx >= 0);
   assert(type);

   Term_p var = TermDefaultCellAlloc();
   TermCellSetProp(var, TPIsShared|TPIsDBVar|TPHasDBSubterm
                        |(TypeIsArrow(type)?TPHasEtaExpandableSubterm:0));

   var->weight = DEFAULT_FWEIGHT;
   var->v_count = 0;
   var->f_count = 1;
   var->entry_no = db_idx;
   var->f_code = db_idx;
   var->type = type;

   return var;
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: _RequestDBVar()
//
//   Create (or find) a unique, shared term that represents 
//   a DB variable with the given type and index. Function always
//   returns the same results given the same arguments.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

Term_p _RequestDBVar(DBVarBank_p db_bank, Type_p type, long db_index)
{
   assert(db_index >= 0);
   assert(type->type_uid != INVALID_TYPE_UID);
   
   IntMap_p* typemap_ref = (IntMap_p*)IntMapGetRef(db_bank, db_index);
   if(!*typemap_ref)
   {
      *typemap_ref = IntMapAlloc();
   }

   IntMap_p typemap = *typemap_ref;
   Term_p *term_ref = (Term_p*)IntMapGetRef(typemap, type->type_uid);
   if(!*term_ref)
   {
      *term_ref = mk_db(db_index, type);
   }

   return *term_ref;
}


/*-----------------------------------------------------------------------
//
// Function: DBVarBankFree()
//
//   Release all memory used by de Bruijn variable bank.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void DBVarBankFree(DBVarBank_p db_bank)
{
   IntMapIter_p iter_typemaps = IntMapIterAlloc(db_bank, 0, LONG_MAX);
   IntMap_p typemap;
   long db_idx;
   while((typemap = IntMapIterNext(iter_typemaps, &db_idx)))
   {
      IntMapIter_p iter_vars = IntMapIterAlloc(typemap, 0, LONG_MAX);
      Term_p var;
      TypeUniqueID key;
      while((var = IntMapIterNext(iter_vars, &key)))
      {
         assert(TermIsDBVar(var));
         TermTopFree(var);
      }
      IntMapIterFree(iter_vars);
      IntMapFree(typemap);
   }
   IntMapIterFree(iter_typemaps);
   IntMapFree(db_bank);
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
