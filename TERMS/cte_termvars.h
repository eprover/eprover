/*-----------------------------------------------------------------------

File  : cte_termvars.h

Author: Stephan Schulz

Contents
 
  Functions for the management of shared variables.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Tue Feb 24 15:52:12 MET 1998
    Rehacked with parts from the now obsolete cte_vartrans.h

-----------------------------------------------------------------------*/

#ifndef CTE_TERMVARS

#define CTE_TERMVARS

#include <clb_pdarrays.h>
#include <cte_termtypes.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* Variable banks store information about variables. They contain two
   indices, one associating an external variable name with an internal
   term cell (and f_code, just because a StrTree can store two data
   items...), and one associating an f_code with a term cell. The first
   index is used for parsing and may be incomplete (i.e. not all
   variable cells will be indexed by a string), the second index
   should be complete (i.e. all variable cells have an entry in the
   array). */

typedef struct varbankcell
{
   FunCode   v_count;      /* FunCode counter for new variables */
   FunCode   max_var;      /* Largest variable ever created */
   StrTree_p ext_index;    /* Associate names and cells */
   PDArray_p f_code_index; /* Associate FunCodes and cells */   
}VarBankCell, *VarBank_p;




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define DEFAULT_VARBANK_SIZE   30

/* Variables greater than this are reserved for fresh variables. At
   the moment this is only used for term pattern generation and term
   top computing in the learning modules */
#define FRESH_VAR_LIMIT      1024 

#define VarBankCellAlloc() (VarBankCell*)SizeMalloc(sizeof(VarBankCell))
#define VarBankCellFree(junk)         SizeFree(junk, sizeof(VarBankCell))

VarBank_p  VarBankAlloc(void);
void       VarBankFree(VarBank_p junk);
void       VarBankClearExtNames(VarBank_p bank);
void       VarBankClearExtNamesNoReset(VarBank_p bank);
void       VarBankVarsSetProp(VarBank_p bank, TermProperties prop);
void       VarBankVarsDelProp(VarBank_p bank, TermProperties prop);

Term_p VarBankFCodeFind(VarBank_p bank, FunCode f_code);
Term_p VarBankExtNameFind(VarBank_p bank, char* name);
Term_p VarBankFCodeAssertAlloc(VarBank_p bank, FunCode f_code);
Term_p VarBankGetFreshVar(VarBank_p bank);
Term_p VarBankExtNameAssertAlloc(VarBank_p bank, char* name);
#define VarBankGetVCount(bank) ((bank)->v_count)
#define VarBankSetVCount(bank,count) ((bank)->v_count = (count))
#define VarBankResetVCount(bank) ((bank)->v_count = 0)
#define VarIsFreshVar(var) ((var)->f_code <= -FRESH_VAR_LIMIT)
#define VarFCodeIsFresh(f_code) ((f_code) <= -FRESH_VAR_LIMIT)

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
























