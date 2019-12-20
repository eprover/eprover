/*-----------------------------------------------------------------------

File  : clb_ddarrays.h

Author: Stephan Schulz

Contents

  Dynamic arrays of large data types - at the moment doubles only.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sun Aug  8 22:45:29 GMT 1999
    Copied from clb_pdarrays.h

-----------------------------------------------------------------------*/

#ifndef CLB_DDARRAYS

#define CLB_DDARRAYS

#include <clb_memory.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct ddarraycell
{
   long   size;
   long   grow;
   double *array;
}DDArrayCell, *DDArray_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define DDArrayCellAlloc() (DDArrayCell*)SizeMalloc(sizeof(DDArrayCell))
#define DDArrayCellFree(junk) SizeFree(junk, sizeof(DDArrayCell))

DDArray_p DDArrayAlloc(long init_size, long grow);
void      DDArrayFree(DDArray_p junk);

void      DDArayEnlarge(DDArray_p array, long idx);
static inline double* DDArrayElementRef(DDArray_p array, long idx);


#define   DDArrayAssign(array, idx, value) \
          *DDArrayElementRef((array), (idx)) = (value)

#define   DDArrayElement(array, idx) \
     *DDArrayElementRef((array), (idx))

void      DDArrayAdd(DDArray_p collect, DDArray_p data, long limit);

double    DDArraySelectPart(DDArray_p array, double part, long size);


/*---------------------------------------------------------------------*/
/*                     Inline functions                                */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: DDArrayElementRef()
//
//   Return a reference to an element in a dynamic array. This
//   reference is only good until the next call to this function! User
//   programs are expected to use this function only extremely rarely
//   and with special care. Use DDArrayElement()/DDArrayAssign()
//   instead.
//
// Global Variables: -
//
// Side Effects    : May enlarge and move array.
//
/----------------------------------------------------------------------*/

static inline double* DDArrayElementRef(DDArray_p array, long idx)
{
   assert(array);
   assert(idx >= 0);

   if(!(idx < array->size))
   {
      DDArayEnlarge(array, idx);
   }
   return &(array->array[idx]);
}

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





