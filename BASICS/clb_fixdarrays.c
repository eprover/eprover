/*-----------------------------------------------------------------------

File  : clb_fixdarrays.c

Author: Stephan Schulz

Contents

  Functions for handling fixed size arrays. 

  Copyright 1998-2003 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Wed Jul  9 23:40:23 CEST 2003
    New

-----------------------------------------------------------------------*/

#include "clb_fixdarrays.h"



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
// Function: FixedDArrayAlloc()
//
//   Allocate an array of known size.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

FixedDArray_p FixedDArrayAlloc(long size)
{
   FixedDArray_p handle = FixedDArrayCellAlloc();

   handle->size = size;
   handle->array = SizeMalloc(sizeof(long)*size);
   
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: FixedDArrayFree()
//
//   Free an array. Handles NULL silently.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

void FixedDArrayFree(FixedDArray_p junk)
{
   if(junk)
   {
      SizeFree(junk->array, sizeof(long)*junk->size);
      FixedDArrayCellFree(junk);
   }
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


