/*-----------------------------------------------------------------------

File  : cpr_varset.c

Author: Stephan Schulz

Contents

  Implementation of the propositional atom multiset datatype.

  Copyright 2003 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Tue May 13 22:06:43 CEST 2003
    New

-----------------------------------------------------------------------*/

#include "cpr_varset.h"



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
// Function: AtomSetAlloc()
//
//   Allocate an empty atom set.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

AtomSet_p AtomSetAlloc(void)
{
   AtomSet_p handle = AtomSetCellAlloc();

   handle->prev = handle->succ = handle;
   handle->atom = PLiteralNoLit;

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: AtomSetFree()
//
//   Free an atom set. Not extremely efficient (but I doubt it has to
//   be).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void AtomSetFree(AtomSet_p set)
{
   while(!AtomSetEmpty(set))
   {
      AtomSetExtract(set->succ);
   }
   AtomSetCellFree(set);
}


/*-----------------------------------------------------------------------
//
// Function: AtomSetExtract()
//
//   Extract the atom of the cell pointed to, and return it.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

PLiteralCode AtomSetExtract(AtomSet_p var)
{
   PLiteralCode atom;

   var->succ->prev = var->prev;
   var->prev->succ = var->succ;
   atom = var->atom;
   assert(atom!=PLiteralNoLit);
   AtomSetCellFree(var);

   return atom;
}


/*-----------------------------------------------------------------------
//
// Function: AtomSetInsert()
//
//   Insert an atom into the atom set.
//
// Global Variables: -
//
// Side Effects    : Memory operations, changes set.
//
/----------------------------------------------------------------------*/

void AtomSetInsert(AtomSet_p set, PLiteralCode atom)
{
   AtomSet_p handle = AtomSetCellAlloc();

   handle->atom = atom;
   handle->prev = set;
   handle->succ = set->succ;
   set->succ->prev = handle;
   set->succ = handle;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


