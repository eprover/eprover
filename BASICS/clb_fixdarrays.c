/*-----------------------------------------------------------------------

File  : clb_fixdarrays.c

Author: Stephan Schulz

Contents

  Functions for handling fixed size arrays.

Copyright 1998-2011 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
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


/*-----------------------------------------------------------------------
//
// Function: FixedDArrayCopy()
//
//   Copy an array, return pointer to new copy.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

FixedDArray_p FixedDArrayCopy(FixedDArray_p array)
{
   if(!array)
   {
      return NULL;
   }
   else
   {
      FixedDArray_p handle = FixedDArrayAlloc(array->size);
      long i;

      for(i=0; i<array->size; i++)
      {
    handle->array[i] = array->array[i];
      }
      return handle;
   }
}


/*-----------------------------------------------------------------------
//
// Function: FixedDArrayPrint()
//
//   Print an array (useful for debugging, I suspect).
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void FixedDArrayPrint(FILE* out, FixedDArray_p array)
{
   long i;

   fprintf(out, "# Size %ld:", array->size);
   for(i=0; i<array->size; i++)
   {
      fprintf(out, " %4ld", array->array[i]);
   }
   fputc('\n', out);
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


