/*-----------------------------------------------------------------------

File  : clb_pdrangearrays.c

Author: Stephan Schulz

Contents

  Funktions realising the dynamic array type.

  Copyright 2010 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Thu May 27 18:33:01 CEST 2010
    New

-----------------------------------------------------------------------*/

#include "clb_pdrangearrays.h"



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
// Function: range_arr_size()
//
//   Given the current size, growths model, and minimal new size,
//   retunr the actual new size.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static long range_arr_size(long minsize, long size, long grow)
{
   if(grow)
   {
      size = ((minsize/grow)+1)*grow;
   }
   else
   {
      /* This is certainly correct and almost certainly faster than
         meddling with logs and exps. */
      while(size <= minsize)
      {
         size *= 2;
      }
   }
   return size;
}


/*-----------------------------------------------------------------------
//
// Function: range_arr_expand_down()
//
//   Expand a range array down until it is big enough to accomodate
//   idx.
//
// Global Variables: -
//
// Side Effects    : Memory reallocations
//
/----------------------------------------------------------------------*/

static void range_arr_expand_down(PDRangeArr_p array, long idx)
{
   IntOrP *tmp;
   long   old_size, old_offset, i;

   assert(array);
   assert(idx < array->offset+array->size);

   old_size   = array->size;
   old_offset = array->offset;
   tmp        = array->array;

   array->size   = range_arr_size(old_offset-idx+old_size,
                                  old_size, array->grow);
   array->offset = old_offset-(array->size-old_size);
   array->array  = SizeMalloc(array->size * sizeof(IntOrP));

   for(i=0; i< (old_offset-array->offset); i++)
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
   memcpy(array->array+(old_offset-array->offset), tmp, old_size*sizeof(IntOrP));
   SizeFree(tmp, old_size * sizeof(IntOrP));
}


/*-----------------------------------------------------------------------
//
// Function: range_arr_expand_up()
//
//   Expand a range array up until it is big enough to accomodate
//   idx.
//
// Global Variables: -
//
// Side Effects    : Memory reallocations.
//
/----------------------------------------------------------------------*/

static void range_arr_expand_up(PDRangeArr_p array, long idx)
{
   IntOrP *tmp;
   long   old_size, old_offset, i;

   assert(array);
   assert(idx >= (array->offset+array->size));

   old_size   = array->size;
   old_offset = array->offset;
   tmp        = array->array;

   array->size = range_arr_size(idx-old_offset+1,
                                old_size, array->grow);
   array->array = SizeMalloc(array->size * sizeof(IntOrP));

   memcpy(array->array, tmp, old_size*sizeof(IntOrP));
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
   SizeFree(tmp, old_size * sizeof(IntOrP));
}



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: PDRangeArrAlloc()
//
//   Return an initialized dynamic array of size init_size where all
//   elements are interpreted as pointers and initialized to NULL.
//
// Global Variables: -
//
// Side Effects    : Memory Operations
//
/----------------------------------------------------------------------*/

PDRangeArr_p PDRangeArrAlloc(long idx, long grow)
{
   PDRangeArr_p handle = PDRangeArrCellAlloc();
   long i;

   assert(grow >= 0);

   handle->integer = false;
   handle->offset = idx;

   handle->size  = grow?grow:1;
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
// Function: PDIntRangeArrAlloc()
//
//   Return an initialized dynamic array of size init_size where all
//   elements are interpreted as (long) integers and initialized to 0.
//
// Global Variables: -
//
// Side Effects    : Memory Operations
//
/----------------------------------------------------------------------*/

PDRangeArr_p PDIntRangeArrAlloc(long idx, long grow)
{
   PDRangeArr_p handle = PDRangeArrCellAlloc();
   long i;

   assert(grow >= 0);

   handle->integer = true;
   handle->offset = idx;

   handle->size  = grow?grow:1;
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
// Function: PDRangeArrFree()
//
//   Free a PDRangeArr. Leaves elements untouched.
//
// Global Variables: -
//
// Side Effects    : Memory Operations
//
/----------------------------------------------------------------------*/

void PDRangeArrFree(PDRangeArr_p junk)
{
   assert(junk);
   assert(junk->size > 0);
   assert(junk->array);

   SizeFree(junk->array, junk->size*sizeof(IntOrP));
   PDRangeArrCellFree(junk);
}

/*-----------------------------------------------------------------------
//
// Function: PDRangeArrEnlarge()
//
//   Enlarge array enough to accomodate index.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void PDRangeArrEnlarge(PDRangeArr_p array, long idx)
{
   if(idx < array->offset)
   {
      range_arr_expand_down(array, idx);
   }
   else
   {
      range_arr_expand_up(array, idx);
   }
   assert(idx>=array->offset);
   assert(idx<(array->offset+array->size));
}

/*-----------------------------------------------------------------------
//
// Function: PDRangeArrCopy()
//
//   Copy a PDRangeArr with contents. Use with care, as some data
//   structures may not be copyable very well (e.g. pointers to the
//   same array, registered references, ...)
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

PDRangeArr_p PDRangeArrCopy(PDRangeArr_p array)
{
   PDRangeArr_p handle = PDRangeArrCellAlloc();

   *handle = *array;
   handle->array =  SizeMalloc(handle->size*sizeof(IntOrP));
   memcpy(handle->array, array->array, handle->size*sizeof(IntOrP));

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: PDRangeArrElementDeleteP()
//
//   If idx is within the currently allocated array, set the value
//   to NULL. Otherwise do nothing.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void PDRangeArrElementDeleteP(PDRangeArr_p array, long idx)
{
   if(PDRangeArrIndexIsCovered(array, idx))
   {
      PDRangeArrAssignP(array, idx, NULL);
   }
}

/*-----------------------------------------------------------------------
//
// Function: PDRangeArrElementDeleteInt()
//
//   If idx is within the currently allocated array, set the value
//   to 0. Otherwise do nothing.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void PDRangeArrElementDeleteInt(PDRangeArr_p array, long idx)
{
   if(PDRangeArrIndexIsCovered(array, idx))
   {
      PDRangeArrAssignInt(array, idx, 0);
   }
}


/*-----------------------------------------------------------------------
//
// Function: PDRangeArrMembers()
//
//   Return number of non-NULL elements in the array.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long PDRangeArrMembers(PDRangeArr_p array)
{
   long i, res =0;

   assert(array);

   for(i=PDRangeArrLowKey(array); i<PDRangeArrLimitKey(array); i++)
   {
      if(PDRangeArrElementP(array, i))
      {
    res++;
      }
   }
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: PDRangeArrElementIncInt()
//
//   Increment entry indexed in array by value. Return new value.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

long PDRangeArrElementIncInt(PDRangeArr_p array, long idx, long value)
{
   IntOrP *ref = PDRangeArrElementRef(array, idx);

   ref->i_val += value;

   return ref->i_val;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
