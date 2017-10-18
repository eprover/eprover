/*-----------------------------------------------------------------------

File  : cte_varsets.h

Author: Stephan Schulz

Contents

  Data structures for representing sets of variables. This is similar
  in concept to cte_varhash.c, but for a different application and
  hence with different access characteristics (extremely fast
  lookup).

  Copyright 2014 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sun Jul  6 08:55:57 CEST 2014
    New

-----------------------------------------------------------------------*/

#ifndef CTE_VARSETS

#define CTE_VARSETS


#include <clb_objtrees.h>
#include <cte_termvars.h>
#include <cte_termfunc.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct var_set_cell
{
   Term_p  t;           /* Term or formula covered by this structure */
   bool    valid;       /* Is it up-to-date? */
   PTree_p vars;
}VarSetCell, *VarSet_p;


typedef PObjTree_p VarSetStore_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define VarSetCellAlloc()    (VarSetCell*)SizeMalloc(sizeof(VarSetCell))
#define VarSetCellFree(junk) SizeFree(junk, sizeof(VarSetCell))

VarSet_p VarSetAlloc(Term_p term);
void     VarSetReset(VarSet_p set);
void     VarSetFree(VarSet_p set);

bool VarSetInsert(VarSet_p set, Term_p var);
void VarSetInsertVarSet(VarSet_p set, VarSet_p vars);
bool VarSetDeleteVar(VarSet_p set, Term_p var);
bool VarSetContains(VarSet_p set, Term_p var);
void VarSetCollectVars(VarSet_p set);
void VarSetUnion(VarSet_p set, VarSet_p set1, VarSet_p set2);
void VarSetMerge(VarSet_p set, VarSet_p set1);

void     VarSetStoreFree(VarSetStore_p store);
VarSet_p VarSetStoreFindVarSet(VarSetStore_p *store, Term_p key);
VarSet_p VarSetStoreGetVarSet(VarSetStore_p *store, Term_p key);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





