/*-----------------------------------------------------------------------

  File  : cte_termvars.h

  Author: Stephan Schulz

  Contents

  Functions for the management of shared variables.

  Copyright 1998, 1999, 2018 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: Tue Feb 24 15:52:12 MET 1998 (Rehacked with parts from the
  now obsolete cte_vartrans.h)

-----------------------------------------------------------------------*/

#ifndef CTE_TERMVARS

#define CTE_TERMVARS

#include <clb_pdarrays.h>
#include <clb_pstacks.h>
#include <cte_termtypes.h>
#include <cte_typebanks.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* Type for storing variables - typically used for storing all
 * variables of one sort in a defined order. */
typedef PStack_p VarBankStack_p;

/* Variable banks store information about variables. They contain two
   indices, one associating an external variable name with an internal
   term cell (and f_code, just because a StrTree can store two data
   items...), and one associating an f_code with a term cell. The first
   index is used for parsing and may be incomplete (i.e. not all
   variable cells will be indexed by a string), the second index
   should be complete (i.e. all variable cells have an entry in the
   array).

   "Normal variables" have even f_codes -2, -4, -6, ... and are
   printed as X1, X2, X3. For each "normal" variable, there
   potentially is a matching "alternative" variable of the same type,
   but with odd f_code (-1, -3, -5, ...). These are printed as Y1,
   Y2, Y3. respectively. All long-term stored clauses use normal
   variables. Alternative variables are used for the variable-disjoint
   copy of the given clause, and (if I can make it work) for
   term/clause patterns.

*/

typedef struct varbankcell
{
   char*       id;
   long        var_count;
   FunCode     fresh_count; /* FunCode counter for new variables */
   TypeBank_p sort_table;  /* Sorts that are used for variables */
   FunCode     max_var;     /* Largest variable ever created */
   PDArray_p   varstacks;   /* Maps each sort to a bank of variables
                             * of these sort (these are e.g. used
                             * in-order to represent external
                             * variables. */
   PDArray_p   v_counts;   /* Number of fresh variables of a given
                            * sort already used. */
   PDArray_p   variables;  /* Array of all variables, indexed by
                              -f_code */
   StrTree_p   ext_index;  /* Associate names and cells */
   PStack_p    env;        /* Scoping environment for quantified
                            * external variables */
   struct varbankcell *shadow; /* Alternative varbank that needs the
                                * same id/type associations */
   struct tbcell *term_bank;   /* Assoziated term bank, if any */
}VarBankCell, *VarBank_p;


/* Remembers the association between a variable and a name */
typedef struct varbanknamedcell
{
   Term_p   var;
   char*    name;
}VarBankNamedCell, *VarBankNamed_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define INITIAL_SORT_STACK_SIZE  10
#define DEFAULT_VARBANK_SIZE   30

#define VarBankCellAlloc() (VarBankCell*)SizeMalloc(sizeof(VarBankCell))
#define VarBankCellFree(junk)    SizeFree(junk, sizeof(VarBankCell))
#define VarBankNamedCellAlloc() (VarBankNamedCell*)SizeMalloc(sizeof(VarBankNamedCell))
#define VarBankNamedCellFree(junk)  SizeFree(junk, sizeof(VarBankNamedCell))


/* Access the stack corresponding to this sort */
static inline VarBankStack_p  VarBankGetStack(VarBank_p bank, TypeUniqueID sort);

VarBank_p  VarBankAlloc(TypeBank_p sort_table);
void       VarBankFree(VarBank_p junk);
void       VarBankPairShadow(VarBank_p primary, VarBank_p secondary);
void       VarBankResetVCounts(VarBank_p bank);
void       VarBankSetVCountsToUsed(VarBank_p bank);
void       VarBankClearExtNames(VarBank_p bank);
void       VarBankClearExtNamesNoReset(VarBank_p bank);
void       VarBankVarsSetProp(VarBank_p bank, TermProperties prop);
void       VarBankVarsDelProp(VarBank_p bank, TermProperties prop);

VarBankStack_p  VarBankCreateStack(VarBank_p bank, TypeUniqueID sort);
Term_p VarBankFCodeFind(VarBank_p bank, FunCode f_code);
Term_p VarBankExtNameFind(VarBank_p bank, char* name);
static inline Term_p VarBankVarAssertAlloc(VarBank_p bank, FunCode f_code, Type_p sort);
Term_p VarBankVarAlloc(VarBank_p bank, FunCode f_code, Type_p sort);
Term_p VarBankGetFreshVar(VarBank_p bank, Type_p sort);
Term_p VarBankExtNameAssertAlloc(VarBank_p bank, char* name);
Term_p VarBankExtNameAssertAllocSort(VarBank_p bank, char* name, Type_p sort);
void   VarBankPushEnv(VarBank_p bank);
void   VarBankPopEnv(VarBank_p bank);
long   VarBankCardinality(VarBank_p bank);    /* Number of existing variables */
long   VarBankCollectVars(VarBank_p bank, PStack_p stack);
#define VarFCodeIsAltCode(f_code) ((f_code)%2)
#define VarIsAltVar(var) (VarFCodeIsAltCode((var)->f_code))

#define VarBankGetAltFreshVar(bank, sort) VarBankGetAltVar((bank), VarBankGetFreshVar((bank), (sort)))



/*---------------------------------------------------------------------*/
/*                         Inline Functions                            */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: VarBankGetStack()
//
//   Obtain a pointer to the stack that stores variables of a given
//   sort.
//
// Global Variables: -
//
// Side Effects    : May modify the varbank, Memory operations
//
/----------------------------------------------------------------------*/

static inline VarBankStack_p  VarBankGetStack(VarBank_p bank, TypeUniqueID sort)
{
   VarBankStack_p res;

   res = (VarBankStack_p) PDArrayElementP(bank->varstacks, sort);
   if (!res)
   {
      res = VarBankCreateStack(bank, sort);
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: VarBankVarAssertAlloc()
//
//   Return a pointer to the variable with the given f_code and sort in the
//   variable bank. Create the variable if it does not exist.
//
// Global Variables: -
//
// Side Effects    : May change variable bank
//
/----------------------------------------------------------------------*/

static inline Term_p VarBankVarAssertAlloc(VarBank_p bank, FunCode f_code,
                                           Type_p type)
{
   Term_p var;

   assert(f_code < 0);
   var = PDArrayElementP(bank->variables, -f_code);
   if(UNLIKELY(!var))
   {
      var = VarBankVarAlloc(bank, f_code, type);
      TermSetBank(var, bank->term_bank);
   }
   assert(var->v_count==1);
   assert(var->type);
   assert(var->type == type);

   return var;
}

/*-----------------------------------------------------------------------
//
// Function: VarBankGetAltVar()
//
//   Given variable X_n, return Y_n (i.e. the one with f_code
//   increased by one - -1 goes to -2).
//
// Global Variables: -
//
// Side Effects    : May change variable bank
//
/----------------------------------------------------------------------*/

static inline Term_p VarBankGetAltVar(VarBank_p bank, Term_p term)
{
   Term_p var;

   assert(term->f_code < 0);
   assert(!VarIsAltVar(term));
   var = VarBankVarAssertAlloc(bank, term->f_code+1, term->type);
   return var;
}



#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
