/*-----------------------------------------------------------------------

  File  : cte_dbvars.h

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

#ifndef CTE_DBVARS

#define CTE_DBVARS

#include <clb_objtrees.h>
#include <clb_intmap.h>
#include <cte_termtypes.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* Type for storing DB variables - bank is a Curried mapping 
   index -> type -> DB variable object */
typedef IntMap_p DBVarBank_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define DBVarBankAlloc() IntMapAlloc()

// _ in the name means that this is a function that must be exported
// but that it should be seldom used -- only in contexts where you do
// not have access to TermBank. Otherwise use TBRequestDBVar.
Term_p  _RequestDBVar(DBVarBank_p db_bank, Type_p type, long db_index);
void    DBVarBankFree(DBVarBank_p db_bank);


#endif


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
