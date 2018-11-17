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

FixedDArray_p FixedDArrayCopy(FixedDArray_p array);

void          FixedDArrayPrint(FILE* out, FixedDArray_p array);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
