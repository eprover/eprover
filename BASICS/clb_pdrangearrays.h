/*-----------------------------------------------------------------------

File  : clb_pdrangearrays.h

Author: Stephan Schulz

Contents

  Dynamic arrays of pointers and long integers with an index range
  defined by upper and lower bound.

  You can define the growth behaviour by specifying a value. If it is
  GROW_EXPONENTIAL, arrays will always grow by a factor that is the
  lowest power of two that will make the array big enough. Otherwise
  it will grow by the smallest multiple of the value specified that
  creates the requested position.

  Copyright 2010 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Thu May 27 18:09:45 CEST 2010
    New

-----------------------------------------------------------------------*/

#ifndef CLB_PDRANGEARRAYS

#define CLB_PDRANGEARRAYS

#include <clb_pdarrays.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct pdrangearrcell
{
   bool   integer; /* true->it maps to integers, false it maps to
                      pointers */
   long   offset; /* Indices go from offset (inclusive) to offset+size
                     (exclusive) */
   long   size;
   long   grow;
   IntOrP *array;
}PDRangeArrCell, *PDRangeArr_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define PDRangeArrCellAlloc() (PDRangeArrCell*)SizeMalloc(sizeof(PDRangeArrCell))
#define PDRangeArrCellFree(junk) SizeFree(junk, sizeof(PDRangeArrCell))

#ifdef CONSTANT_MEM_ESTIMATE
#define PDRANGEARRELL_MEM 20
#else
#define PDRANGEARR_MEM MEMSIZE(PDRangeArrCell)
#endif
#define PDRangeArrStorage(arr) (PDRANGEARRL_MEM+INTORP_MEM+((arr)->size*INTORP_MEM))

PDRangeArr_p PDRangeArrAlloc(long idx, long grow);
PDRangeArr_p PDIntRangeArrAlloc(long idx, long grow);
void         PDRangeArrFree(PDRangeArr_p junk);
void         PDRangeArrEnlarge(PDRangeArr_p array, long idx);
PDRangeArr_p PDRangeArrCopy(PDRangeArr_p array);

static inline IntOrP*   PDRangeArrElementRef(PDRangeArr_p array, long idx);

void      PDRangeArrElementDeleteP(PDRangeArr_p array, long idx);
void      PDRangeArrElementDeleteInt(PDRangeArr_p array, long idx);

#define   PDRangeArrAssign(array, idx, value) \
         *PDRangeArrElementRef((array), (idx)) = (value)
#define   PDRangeArrAssignP(array, idx, value) \
          PDRangeArrElementRef((array), (idx))->p_val = (value)
#define   PDRangeArrAssignInt(array, idx, value) \
          PDRangeArrElementRef((array), (idx))->i_val = (value)

#define   PDRangeArrElement(array, idx) \
     *PDRangeArrElementRef((array), (idx))
#define   PDRangeArrElementP(array, idx) \
     (PDRangeArrElementRef((array), (idx))->p_val)
#define   PDRangeArrElementInt(array, idx) \
     (PDRangeArrElementRef((array), (idx))->i_val)

#define   PDRangeArrLowKey(array) ((array)->offset)
#define   PDRangeArrLimitKey(array) ((array)->offset+(array)->size)

#define   PDRangeArrIndexIsCovered(array, idx) \
   (((idx)>=PDRangeArrLowKey(array))&&(((idx)<PDRangeArrLimitKey(array))))

long      PDRangeArrElementIncInt(PDRangeArr_p array, long idx, long value);

/*---------------------------------------------------------------------*/
/*                     Inline functions                                */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: PDRangeArrElementRef()
//
//   Return a reference to an element in a dynamic array. This
//   reference is only good until the next call to this function! User
//   programs are expected to use this function only extremely rarely
//   and with special care. Use PDRangeArrElement()/PDRangeArrAssign()
//   instead.
//
// Global Variables: -
//
// Side Effects    : May enlarge and move array.
//
/----------------------------------------------------------------------*/

static inline IntOrP* PDRangeArrElementRef(PDRangeArr_p array, long idx)
{
   assert(array);

   if(!PDRangeArrIndexIsCovered(array, idx))
   {
      PDRangeArrEnlarge(array, idx);
   }
   assert(PDRangeArrIndexIsCovered(array, idx));
   assert(idx>=array->offset);
   assert((idx-array->offset)<array->size);
   return &(array->array[idx-(array->offset)]);
}



#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
