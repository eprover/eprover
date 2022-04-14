/*-----------------------------------------------------------------------

  File  : cte_garbage_coll.h

  Author: Stephan Schulz (schulz@eprover.org)

  Contents

  Support for the termcell garbage collection. This allows the
  association of all clause- and formulasets with a term bank.

  Copyright 2010, 2022 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Sat Jan 29 02:58:46 CET 2022 (from old ccl_garbage_coll.h)

    New

-----------------------------------------------------------------------*/

#ifndef CTE_GARBAGE_COLL

#define CTE_GARBAGE_COLL


#include <clb_ptrees.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


typedef struct gc_admin_cell
{
   PTree_p clause_sets;
   PTree_p formula_sets;
}GCAdminCell, *GCAdmin_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define GCAdminCellAlloc()    (GCAdminCell*)SizeMalloc(sizeof(GCAdminCell))
#define GCAdminCellFree(junk) SizeFree(junk, sizeof(GCAdminCell))



GCAdmin_p GCAdminAlloc();
void      GCAdminFree(GCAdmin_p junk);
void      GCRegisterFormulaSet(GCAdmin_p gc, void* set);
void      GCRegisterClauseSet(GCAdmin_p gc, void* set);
void      GCDeregisterFormulaSet(GCAdmin_p gc, void *set);
void      GCDeregisterClauseSet(GCAdmin_p gc, void* set);

//long      GCCollect(GCAdmin_p gc);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
