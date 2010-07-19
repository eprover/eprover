/*-----------------------------------------------------------------------

File  : cto_kbodata.c

Author: Stephan Schulz (schulz@eprover.org)

Contents
 
  Basic code for the KBDLin data structure. Most code is in the KBO
  implementation file so that the trivial functions can be more easily
  inlined. 

  Copyright 2010 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sat Feb 13 23:43:56 EET 2010
    New

-----------------------------------------------------------------------*/

#include "cto_kbodata.h"



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
// Function: KBOLinAlloc()
//
//   Allocate an initialized KBOLinCell.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

KBOLin_p KBOLinAlloc()
{
   KBOLin_p handle = KBOLinCellAlloc();

   handle->wb = 0;
   handle->vb = PDIntArrayAlloc(20,50);
   handle->max_var = 0;
   handle->pos_bal = 0;
   handle->neg_bal = 0;
   handle->res     = to_equal;
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: KBOLinFree()
//
//   Free a KBOLin structure.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void KBOLinFree(KBOLin_p junk)
{
   PDArrayFree(junk->vb);
   KBOLinCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: 
//
//   
//
// Global Variables: 
//
// Side Effects    : 
//
/----------------------------------------------------------------------*/

void KBOLinReset(KBOLin_p kbobal)
{
   FunCode i;

   for(i=1; i<=kbobal->max_var; i++)
   {
      PDArrayAssignInt(kbobal->vb, i, 0);
   }   
   kbobal->wb = 0;
   kbobal->max_var = 0;
   kbobal->pos_bal = 0;
   kbobal->neg_bal = 0;
   kbobal->res     = to_equal;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


