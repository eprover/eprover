/*-----------------------------------------------------------------------

File  : cte_varsets.c

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Implementation of variable sets.

  Copyright 2014 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Mon Jul  7 10:45:49 CEST 2014
    New

-----------------------------------------------------------------------*/

#include "cte_varsets.h"

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
// Function: varset_free_fun()
//
//   Wrapper of type ObjDelFun
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void varset_free_fun(void* junk)
{
   VarSetFree((VarSet_p)junk);
}


/*-----------------------------------------------------------------------
//
// Function: varset_cmp_fun()
//
//   Compare variable sets (by value of set->t).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int varset_cmp_fun(const void* set1, const void* set2)
{
   VarSet_p s1 = (VarSet_p)set1;
   VarSet_p s2 = (VarSet_p)set2;

   return PCmp(s1->t, s2->t);
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: VarSetAlloc()
//
//   Allocate a variable set.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

VarSet_p VarSetAlloc(Term_p term)
{
   VarSet_p handle = VarSetCellAlloc();
   handle->t = term;
   handle->valid = false;
   handle->vars = NULL;

   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: VarSetReset()
//
//   Remove all variables from set (and mark it invalid).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void VarSetReset(VarSet_p set)
{
   PTreeFree(set->vars);
   set->vars = NULL;
}


/*-----------------------------------------------------------------------
//
// Function: VarSetFree()
//
//   Free a variable set.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void VarSetFree(VarSet_p set)
{
   VarSetReset(set);
   VarSetCellFree(set);
}




/*-----------------------------------------------------------------------
//
// Function: VarSetInsert()
//
//   Insert a variable into a set. Nominally a NOP if variable is
//   already in the set, but may reorganise the underlying
//   tree. Returns false if variable is already in the set, true
//   otherwise.
//
// Global Variables: -
//
// Side Effects    : Reorganises splay tree, memory operations
//
/----------------------------------------------------------------------*/

bool VarSetInsert(VarSet_p set, Term_p var)
{
   return PTreeStore(&set->vars, var);
}

/*-----------------------------------------------------------------------
//
// Function: VarSetInsertVarSet()
//
//   Insert all vaiables in the second set into the first set.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void VarSetInsertVarSet(VarSet_p set, VarSet_p vars)
{
   PTreeInsertTree(&set->vars, vars->vars);
}


/*-----------------------------------------------------------------------
//
// Function: VarSetDeleteVar()
//
//   Delete variable from set. A NOP if var is not present. Returns
//   true if the key was present.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool VarSetDeleteVar(VarSet_p set, Term_p var)
{
   return PTreeExtractKey(&set->vars, var) != NULL;
}


/*-----------------------------------------------------------------------
//
// Function: VarSetContains()
//
//   Return true iff var is in set.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

bool VarSetContains(VarSet_p set, Term_p var)
{
   return PTreeFind(&set->vars, var) != NULL;
}

/*-----------------------------------------------------------------------
//
// Function: VarSetCollectVars()
//
//   Make sure that set contains all variables in set->t.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void VarSetCollectVars(VarSet_p set)
{
   assert(set->t);
   VarSetReset(set);
   TermCollectVariables(set->t, &set->vars);
}



/*-----------------------------------------------------------------------
//
// Function: VarSetUnion()
//
//   Make set the union of set1 and set2.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void VarSetUnion(VarSet_p set, VarSet_p set1, VarSet_p set2)
{
   VarSetReset(set);
   set->vars = PTreeCopy(set1->vars);
   PTreeMerge(&set->vars, set2->vars);
}


/*-----------------------------------------------------------------------
//
// Function: VarSetMerge()
//
//   Merge the second varset into the first, destroying the former.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void VarSetMerge(VarSet_p set, VarSet_p set1)
{
   PTreeMerge(&(set->vars), set1->vars);
   VarSetFree(set1);
}



/*-----------------------------------------------------------------------
//
// Function: VarSetStoreFree()
//
//   Free a VarSetStore.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/
void VarSetStoreFree(VarSetStore_p store)
{
   PObjTreeFree(store, varset_free_fun);
}


/*-----------------------------------------------------------------------
//
// Function: VarSetStoreFindVarSet()
//
//   Find the varset associated with key and return it. Return
//   NULL if it does not exist.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

VarSet_p VarSetStoreFindVarSet(VarSetStore_p *store, Term_p key)
{
   VarSet_p dummy = VarSetAlloc(key), res;

   res =  PTreeObjFindObj(store, key, varset_cmp_fun);

   VarSetFree(dummy);

   return res;
}



/*-----------------------------------------------------------------------
//
// Function: VarSetStoreGetVarSet()
//
//   Find varset for key in the store. If none, create an insert new
//   empty varset.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

VarSet_p VarSetStoreGetVarSet(VarSetStore_p *store, Term_p key)
{
   VarSet_p dummy = VarSetAlloc(key), res;

   res = PTreeObjStore(store, dummy, varset_cmp_fun);
   if(res)
   {
      VarSetFree(dummy);
   }
   else
   {
      res = dummy;
   }
   return res;
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


