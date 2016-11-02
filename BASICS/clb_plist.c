/*-----------------------------------------------------------------------

File  : clb_plist.c

Author: Stephan Schulz

Contents

  Doubly linked lists of pointers

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed Jul 22 04:17:45 MET DST 1998
    New

-----------------------------------------------------------------------*/

#include "clb_plist.h"



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
// Function: PListAlloc()
//
//   Allocate an empty PList.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

PList_p PListAlloc(void)
{
   PList_p handle = PListCellAlloc();

   handle->pred = handle->succ = handle;

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: PListFree()
//
//   Free a PList.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void PListFree(PList_p junk)
{
   while(!PListEmpty(junk))
   {
      PListDelete(junk->succ);
   }
   PListCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: PListInsert()
//
//   Insert a PListCell into a list after where.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void PListInsert(PList_p where, PList_p cell)
{
   cell->pred = where;
   cell->succ = where->succ;
   where->succ->pred = cell;
   where->succ = cell;
}


/*-----------------------------------------------------------------------
//
// Function: PListStore()
//
//   Store a given value in a PList.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void PListStore(PList_p where, IntOrP val)
{
   PList_p handle = PListCellAlloc();

   handle->key = val;
   PListInsert(where, handle);
}


/*-----------------------------------------------------------------------
//
// Function: PListStoreP()
//
//   Store a pointer in a PList
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void PListStoreP(PList_p where, void* val)
{
   IntOrP tmp;

   tmp.p_val = val;
   PListStore(where, tmp);
}


/*-----------------------------------------------------------------------
//
// Function: PListStoreInt()
//
//   Store an integer in a PList
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void PListStoreInt(PList_p where, long val)
{
   IntOrP tmp;

   tmp.i_val = val;
   PListStore(where, tmp);
}


/*-----------------------------------------------------------------------
//
// Function: PListExtract()
//
//   Extract a PListCell from a list
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

PList_p PListExtract(PList_p element)
{
   assert(element->pred);
   assert(element->succ);
   assert(element->pred!=element);
   assert(element->succ!=element);

   element->pred->succ = element->succ;
   element->succ->pred = element->pred;
   element->pred = element->succ = NULL;

   return element;
}


/*-----------------------------------------------------------------------
//
// Function: PListDelete()
//
//   Delete an entry from a PList.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void PListDelete(PList_p element)
{
   PListExtract(element);
   PListCellFree(element);
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


