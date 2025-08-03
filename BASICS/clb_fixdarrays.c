/*-----------------------------------------------------------------------

  File  : clb_fixdarrays.c

  Author: Stephan Schulz

  Contents

  Functions for handling fixed size arrays.

  Copyright 1998-2018 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Wed Jul  9 23:40:23 CEST 2003

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
   FixedDArray_p handle = SizeMalloc(sizeof(FixedDArrayCell)+size*sizeof(long));
   handle->size = size;

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
      SizeFree(junk, sizeof(FixedDArrayCell)+(junk->size*sizeof(long)));
   }
}


/*-----------------------------------------------------------------------
//
// Function: FixedDArrayInitialize()
//
//   Set all values in the array to a given value.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void FixedDArrayInitialize(FixedDArray_p array, long value)
{
   long i;

   for(i=0; i<array->size; i++)
   {
      array->array[i] = value;
   }
}


/*-----------------------------------------------------------------------
//
// Function: FixedDArrayAdd()
//
//   Component-wise addition of both sources. Guaranteed to work if
//   dest is a source (but not maximally efficient - who cares). Yes,
//   it's worth mentioning it ;-)
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void FixedDArrayAdd(FixedDArray_p dest, FixedDArray_p s1, FixedDArray_p s2)
{
   long i;

   assert(s1 && s2 && dest);
   assert(s1->size == dest->size);
   assert(s2->size == dest->size);

   for(i=0; i<dest->size; i++)
   {
      dest->array[i] = s1->array[i]+s2->array[i];
   }
}


/*-----------------------------------------------------------------------
//
// Function: FixedDArrayMulAdd()
//
//   Component-wise addition of both weighted sources. Guaranteed to
//   work if dest is a source (but not maximally efficient - who
//   cares). Yes, it's worth mentioning it ;-)
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void FixedDArrayMulAdd(FixedDArray_p dest, FixedDArray_p s1, long f1,
                      FixedDArray_p s2, long f2)
{
   long i;

   assert(s1 && s2 && dest);
   assert(s1->size == dest->size);
   assert(s2->size == dest->size);

   for(i=0; i<dest->size; i++)
   {
      dest->array[i] = f1*s1->array[i]+f2*s2->array[i];
   }
}


/*-----------------------------------------------------------------------
//
// Function: FixedDArrayMax()
//
//   Compute componentwise  max of vectors. See above.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void FixedDArrayMax(FixedDArray_p dest, FixedDArray_p s1, FixedDArray_p s2)
{
   long i;

   assert(s1 && s2 && dest);
   assert(s1->size == dest->size);
   assert(s2->size == dest->size);

   for(i=0; i<dest->size; i++)
   {
      dest->array[i] = MAX(s1->array[i],s2->array[i]);
   }
}


/*-----------------------------------------------------------------------
//
// Function: FixedDArrayMin()
//
//   Compute componentwise  min of vectors. See above.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void FixedDArrayMin(FixedDArray_p dest, FixedDArray_p s1, FixedDArray_p s2)
{
   long i;

   assert(s1 && s2 && dest);
   assert(s1->size == dest->size);
   assert(s2->size == dest->size);

   for(i=0; i<dest->size; i++)
   {
      dest->array[i] = MIN(s1->array[i],s2->array[i]);
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

   fprintf(out, COMCHAR" Size %ld:", array->size);
   for(i=0; i<array->size; i++)
   {
      fprintf(out, " %4ld", array->array[i]);
   }
   fputc('\n', out);
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

      memcpy(handle, array,
             sizeof(FixedDArrayCell)+(array->size*sizeof(long)));
      return handle;
   }
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
