/*-----------------------------------------------------------------------

  File  : clb_fixdarrays.h

  Author: Stephan Schulz

  Contents

  Rather trivial datatype for arrays of long integers with a known,
  fixed and and queryable size.

  Copyright 2003-2018 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Wed Jul  9 23:35:01 CEST 2003

  -----------------------------------------------------------------------*/

#ifndef CLB_FIXDARRAYS

#define CLB_FIXDARRAYS

#include <clb_memory.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


typedef struct fixd_array_cell
{
   long size;
   long array[];
}FixedDArrayCell, *FixedDArray_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


FixedDArray_p FixedDArrayAlloc(long size);
void          FixedDArrayFree(FixedDArray_p junk);

#define FixedDArraySize(array) ((array)->size)

void FixedDArrayInitialize(FixedDArray_p array, long value);

void FixedDArrayAdd(FixedDArray_p dest, FixedDArray_p s1, FixedDArray_p s2);
void FixedDArrayMulAdd(FixedDArray_p dest, FixedDArray_p s1, long f1,
                       FixedDArray_p s2, long f2);
#define FixedDArraySub(dest, s1, s2) FixedDArrayMulAdd((dest),(s1), 1, (s2), -1)
void FixedDArrayMax(FixedDArray_p dest, FixedDArray_p s1, FixedDArray_p s2);
void FixedDArrayMin(FixedDArray_p dest, FixedDArray_p s1, FixedDArray_p s2);

void FixedDArrayPrint(FILE* out, FixedDArray_p array);

FixedDArray_p FixedDArrayCopy(FixedDArray_p array);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
