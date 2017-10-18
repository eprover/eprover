/*-----------------------------------------------------------------------

File  : clb_plist.h

Author: Stephan Schulz

Contents

  Doubly linked lists of pointers and integers.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Mon Jul 20 02:26:17 MET DST 1998
    New

-----------------------------------------------------------------------*/

#ifndef CLB_PLIST

#define CLB_PLIST


#include <clb_memory.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


typedef struct plistcell
{
   IntOrP key;
   struct plistcell *pred;
   struct plistcell *succ;
}PListCell, *PList_p;

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define PListCellAlloc() (PListCell*)SizeMalloc(sizeof(PListCell))
#define PListCellFree(junk)         SizeFree(junk, sizeof(PListCell))

PList_p PListAlloc(void);
void    PListFree(PList_p junk);
#define PListEmpty(anchor) ((anchor)->pred == anchor)

void    PListInsert(PList_p where, PList_p cell);
void    PListStore(PList_p where, IntOrP val);
void    PListStoreP(PList_p where, void* val);
void    PListStoreInt(PList_p where, long val);
PList_p PListExtract(PList_p element);
void    PListDelete(PList_p element);




#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





