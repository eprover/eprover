/*-----------------------------------------------------------------------

File  : ccl_garbage_coll.h

Author: Stephan Schulz (schulz@eprover.org)

Contents

  High-level support for the term cell garbage collection. This
  integrates a term bank and all clause- and formulasets which use
  terms from this bank.

  Copyright 2010 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sat Mar 20 09:26:51 CET 2010
    New

-----------------------------------------------------------------------*/

#ifndef CCL_GARBAGE_COLL

#define CCL_GARBAGE_COLL


#include <ccl_formulasets.h>
#include <ccl_clausesets.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


typedef struct gc_admin_cell
{
   TB_p    bank;
   PTree_p clause_sets;
   PTree_p formula_sets;
}GCAdminCell, *GCAdmin_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define GCAdminCellAlloc()    (GCAdminCell*)SizeMalloc(sizeof(GCAdminCell))
#define GCAdminCellFree(junk) SizeFree(junk, sizeof(GCAdminCell))

GCAdmin_p GCAdminAlloc(TB_p bank);
void      GCAdminFree(GCAdmin_p junk);
void      GCRegisterFormulaSet(GCAdmin_p gc, FormulaSet_p set);
void      GCRegisterClauseSet(GCAdmin_p gc, ClauseSet_p set);
void      GCDeregisterFormulaSet(GCAdmin_p gc, FormulaSet_p set);
void      GCDeregisterClauseSet(GCAdmin_p gc, ClauseSet_p set);

long      GCCollect(GCAdmin_p gc);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





