/*-----------------------------------------------------------------------

  File  : clb_pdarrays.c

  Author: Stephan Schulz

  Contents

  Funktions realising the dynamic array type.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: Thu Jul 23 17:40:32 MEST 1998

  -----------------------------------------------------------------------*/

#include "clb_pdarrays.h"



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
// Function: PDArrayAlloc()
//
//   Return an initialized dynamic array of size init_size where all
//   elements are interpreted as pointers and initialized to NULL.
//
// Global Variables: -
//
// Side Effects    : Memory Operations
//
/----------------------------------------------------------------------*/

PDArray_p PDArrayAlloc(long init_size, long grow)
{
   PDArray_p handle = PDArrayCellAlloc();
   long i;

   assert(init_size > 0);
   assert(grow >= 0);

   handle->integer = false;
   handle->size  = init_size;
   handle->grow  = grow;
   handle->array = SizeMalloc(handle->size*sizeof(IntOrP));
   for(i=0; i<handle->size; i++)
   {
      handle->array[i].p_val = NULL;
   }
   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: PDIntArrayAlloc()
//
//   Return an initialized dynamic array of size init_size where all
//   elements are interpreted as (long) integers and initialized to 0.
//
// Global Variables: -
//
// Side Effects    : Memory Operations
//
/----------------------------------------------------------------------*/

PDArray_p PDIntArrayAlloc(long init_size, long grow)
{
   PDArray_p handle = PDArrayCellAlloc();
   long i;

   assert(init_size > 0);
   assert(grow >= 0);

   handle->integer = true;
   handle->size  = init_size;
   handle->grow  = grow;
   handle->array = SizeMalloc(handle->size*sizeof(IntOrP));
   for(i=0; i<handle->size; i++)
   {
      handle->array[i].i_val = 0;
   }
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: PDArrayFree()
//
//   Free a PDArray. Leaves elements untouched.
//
// Global Variables: -
//
// Side Effects    : Memory Operations
//
/----------------------------------------------------------------------*/

void PDArrayFree(PDArray_p junk)
{
   assert(junk);
   assert(junk->size > 0);
   assert(junk->array);

   SizeFree(junk->array, junk->size*sizeof(IntOrP));
   PDArrayCellFree(junk);
}

/*-----------------------------------------------------------------------
//
// Function: PDArrayEnlarge()
//
//   Enlarge array enough to accomodate index.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void PDArrayEnlarge(PDArray_p array, long idx)
{
   IntOrP *tmp;
   long   old_size, i;

   old_size = array->size;
   tmp      = array->array;
   if(array->grow)
   {
      array->size = ((idx/array->grow)+1)*array->grow;
   }
   else
   {
      while(array->size <= idx)
      {
         array->size = array->size*2;
      }
   }
   array->array = SizeMalloc(array->size * sizeof(IntOrP));
   memcpy(array->array, tmp, old_size*sizeof(IntOrP));
   SizeFree(tmp, old_size * sizeof(IntOrP));
   for(i=old_size; i<array->size; i++)
   {
      if(array->integer)
      {
         array->array[i].i_val = 0;
      }
      else
      {
         array->array[i].p_val = NULL;
      }
   }
}

/*-----------------------------------------------------------------------
//
// Function: PDArrayCopy()
//
//   Copy a PDArray with contents. Use with care, as some data
//   structures may not be copyable very well (e.g. pointers to the
//   same array, registered references, ...)
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

PDArray_p PDArrayCopy(PDArray_p array)
{
   PDArray_p handle = PDArrayAlloc(array->size, array->grow);
   long i;

   for(i=0; i<array->size; i++)
   {
      handle->array[i] = array->array[i];
   }
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: PDArrayElementDeleteP()
//
//   If idx is within the currently allocated array, set the value
//   to NULL. Otherwise do nothing.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void PDArrayElementDeleteP(PDArray_p array, long idx)
{
   if(idx < array->size)
   {
      PDArrayAssignP(array, idx, NULL);
   }
}

/*-----------------------------------------------------------------------
//
// Function: PDArrayElementDeleteInt()
//
//   If idx is within the currently allocated array, set the value
//   to 0. Otherwise do nothing.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void PDArrayElementDeleteInt(PDArray_p array, long idx)
{
   if(idx < array->size)
   {
      PDArrayAssignInt(array, idx, 0);
   }
}


/*-----------------------------------------------------------------------
//
// Function: PDArrayMembers()
//
//   Return number of non-NULL elements in the array.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long PDArrayMembers(PDArray_p array)
{
   long i, res =0;

   assert(array);

   for(i=0; i<array->size; i++)
   {
      if(PDArrayElementP(array, i))
      {
         res++;
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: PDArrayFirstUnused()
//
//   Return 1 + the index of the largest element != NULL in array (0
//   if the array is empty).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long PDArrayFirstUnused(PDArray_p array)
{
   long i;

   assert(array);

   for(i=array->size; i; i--)
   {
      if(PDArrayElementP(array, i-1))
      {
         break;
      }
   }
   return i;
}


/*-----------------------------------------------------------------------
//
// Function: PDArrayStore()
//
//   Store the given value after the end of the used part of the
//   array. This is similar to PStackPush() for stacks, but a LOT less
//   efficient. Return value is the index assigned.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long PDArrayStore(PDArray_p array, IntOrP value)
{
   long idx;

   assert(array);

   idx = PDArrayFirstUnused(array);
   PDArrayAssign(array, idx, value);

   return idx;
}

/*-----------------------------------------------------------------------
//
// Function: PDArrayStoreP()
//
//   Store the given pointer value after the end of the used part of
/    the array. See PDArrayStore().
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long PDArrayStoreP(PDArray_p array, void* value)
{
   IntOrP tmp;

   tmp.p_val = value;

   return PDArrayStore(array, tmp);
}


/*-----------------------------------------------------------------------
//
// Function: PDArrayStoreInt()
//
//   Store the given long int value after the end of the used part of
/    the array. See PDArrayStore().
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long PDArrayStoreInt(PDArray_p array, long value)
{
   IntOrP tmp;

   tmp.i_val = value;

   return PDArrayStore(array, tmp);
}


/*-----------------------------------------------------------------------
//
// Function: PDArrayAdd()
//
//   Add the first limit elements from new to the corresponding entries
//   in collect. All entries are interpreted as numerical.
//
// Global Variables: -
//
// Side Effects    : Changes collect.
//
/----------------------------------------------------------------------*/

void PDArrayAdd(PDArray_p collect, PDArray_p data, long limit)
{
   long i, old, new;

   for(i=0; i<limit; i++)
   {
      old = PDArrayElementInt(collect, i);
      new = PDArrayElementInt(data, i);
      PDArrayAssignInt(collect, i, old+new);
   }
}


/*-----------------------------------------------------------------------
//
// Function: PDArrayElementIncInt()
//
//   Increment entry indexed in array by value. Return new value.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

long PDArrayElementIncInt(PDArray_p array, long idx, long value)
{
   IntOrP *ref = PDArrayElementRef(array, idx);

   ref->i_val += value;

   return ref->i_val;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
