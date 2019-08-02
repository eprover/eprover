/*-----------------------------------------------------------------------

  File  : clb_pdarrays.h

  Author: Stephan Schulz

  Contents

  Dynamic arrays of pointers and long integers. You can define the
  growth behaviour by specifying a value. If it is GROW_EXPONENTIAL,
  arrays will always grow by a factor that is the lowest power of two
  that will make the array big enough. Otherwise it will grow by the
  smallest multiple of the value specified that creates the requested
  position.

  Copyright 1998, 1999, 2004 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: Wed Jul 22 21:34:41 MET DST 1998

  -----------------------------------------------------------------------*/

#ifndef CLB_PDARRAYS

#define CLB_PDARRAYS

#include <clb_memory.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct pdarraycell
{
   bool   integer;
   long   size;
   long   grow;
   IntOrP *array;
}PDArrayCell, *PDArray_p;

#define GROW_EXPONENTIAL 0

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define PDArrayCellAlloc() (PDArrayCell*)SizeMalloc(sizeof(PDArrayCell))
#define PDArrayCellFree(junk) SizeFree(junk, sizeof(PDArrayCell))

#ifdef CONSTANT_MEM_ESTIMATE
#define PDARRAYCELL_MEM 20
#else
#define PDARRAYCELL_MEM MEMSIZE(PDArrayCell)
#endif
#define PDArrayStorage(arr) (PDARRAYCELL_MEM+INTORP_MEM+((arr)->size*INTORP_MEM))

PDArray_p PDArrayAlloc(long init_size, long grow);
PDArray_p PDIntArrayAlloc(long init_size, long grow);
void      PDArrayFree(PDArray_p junk);
#define   PDArraySize(array) ((array)->size)

PDArray_p PDArrayCopy(PDArray_p array);

void      PDArrayEnlarge(PDArray_p array, long idx);
static inline IntOrP*   PDArrayElementRef(PDArray_p array, long idx);

void      PDArrayElementDeleteP(PDArray_p array, long idx);
void      PDArrayElementDeleteInt(PDArray_p array, long idx);

#define   PDArrayElementClear(arr, idx) ((arr)->array[(idx)].p_val = NULL)
#define   PDArrayAssign(array, idx, value)              \
   *PDArrayElementRef((array), (idx)) = (value)
#define   PDArrayAssignP(array, idx, value)                     \
   PDArrayElementRef((array), (idx))->p_val = (value)
#define   PDArrayAssignInt(array, idx, value)                   \
   PDArrayElementRef((array), (idx))->i_val = (value)

#define   PDArrayElement(array, idx)            \
   *PDArrayElementRef((array), (idx))
#define   PDArrayElementP(array, idx)           \
   (PDArrayElementRef((array), (idx))->p_val)
#define   PDArrayElementInt(array, idx)         \
   (PDArrayElementRef((array), (idx))->i_val)

long      PDArrayMembers(PDArray_p array);
long      PDArrayFirstUnused(PDArray_p array);
long      PDArrayStore(PDArray_p array, IntOrP value);
long      PDArrayStoreP(PDArray_p array, void* value);
long      PDArrayStoreInt(PDArray_p array, long value);
void      PDArrayAdd(PDArray_p collect, PDArray_p data, long limit);

long      PDArrayElementIncInt(PDArray_p array, long idx, long value);

/*---------------------------------------------------------------------*/
/*                     Inline functions                                */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: PDArrayElementRef()
//
//   Return a reference to an element in a dynamic array. This
//   reference is only good until the next call to this function! User
//   programs are expected to use this function only extremely rarely
//   and with special care. Use PDArrayElement()/PDArrayAssign()
//   instead.
//
// Global Variables: -
//
// Side Effects    : May enlarge and move array.
//
/----------------------------------------------------------------------*/

static inline IntOrP* PDArrayElementRef(PDArray_p array, long idx)
{
   assert(array);
   assert(idx >= 0);

   if(UNLIKELY(idx >= array->size))
   {
      PDArrayEnlarge(array, idx);
   }
   return &(array->array[idx]);
}

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
