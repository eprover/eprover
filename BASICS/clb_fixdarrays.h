/*-----------------------------------------------------------------------

File  : clb_fixdarrays.h

Author: Stephan Schulz

Contents

  Rather trivial datatype for arrays of long integers with a known,
  fixed and and queryable size.

  Copyright 2003 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed Jul  9 23:35:01 CEST 2003
    New

-----------------------------------------------------------------------*/

#ifndef CLB_FIXDARRAYS

#define CLB_FIXDARRAYS

#include <clb_memory.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


typedef struct fixd_array_cell
{
   long *array;
   long size;
}FixedDArrayCell, *FixedDArray_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define FixedDArrayCellAlloc()    (FixedDArrayCell*)SizeMalloc(sizeof(FixedDArrayCell))
#define FixedDArrayCellFree(junk) SizeFree(junk, sizeof(FixedDArrayCell))

FixedDArray_p FixedDArrayAlloc(long size);
void          FixedDArrayFree(FixedDArray_p junk);

#define FixedDArraySize(array) ((array)->size)

FixedDArray_p FixedDArrayCopy(FixedDArray_p array);

void          FixedDArrayPrint(FILE* out, FixedDArray_p array);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





