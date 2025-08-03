/*-----------------------------------------------------------------------

File  : clb_ddarrays.c

Author: Stephan Schulz

Contents

  Funktions realising the dynamic array type for doubles.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sun Aug  8 22:49:36 GMT 1999
    Copied from clb_pdarrays.c

-----------------------------------------------------------------------*/

#include "clb_ddarrays.h"



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
// Function: DDArrayAlloc()
//
//   Return an initialized dynamic array of size init_size where all
//   elements are interpreted as pointers and initialized to NULL.
//
// Global Variables: -
//
// Side Effects    : Memory Operations
//
/----------------------------------------------------------------------*/

DDArray_p DDArrayAlloc(long init_size, long grow)
{
   DDArray_p handle = DDArrayCellAlloc();
   long i;

   assert(init_size > 0);
   assert(grow > 0);

   handle->size  = init_size;
   handle->grow  = grow;
   handle->array = SizeMalloc(handle->size*sizeof(double));
   for(i=0; i<handle->size; i++)
   {
      handle->array[i] = 0.0;
   }
   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: DDArrayFree()
//
//   Free a DDArray. Leaves elements untouched.
//
// Global Variables: -
//
// Side Effects    : Memory Operations
//
/----------------------------------------------------------------------*/

void DDArrayFree(DDArray_p junk)
{
   assert(junk);
   assert(junk->size > 0);
   assert(junk->array);

   SizeFree(junk->array, junk->size*sizeof(double));
   DDArrayCellFree(junk);
}

/*-----------------------------------------------------------------------
//
// Function: DDArayEnlarge()
//
//   Enlarge array enough to accomodate idx.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void DDArayEnlarge(DDArray_p array, long idx)
{
   double *tmp;
   long   old_size, i;

   old_size = array->size;
   tmp      = array->array;
   array->size = ((idx/array->grow)+1)*array->grow;
   array->array = SizeMalloc(array->size * sizeof(double));
   memcpy(array->array, tmp, old_size*sizeof(double));
   SizeFree(tmp, old_size * sizeof(double));
   for(i=old_size; i<array->size; i++)
   {
      array->array[i] = 0.0;
   }
}

/*-----------------------------------------------------------------------
//
// Function: DDArrayDebugPrint()
//
//   Print the array, only for debugging.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void DDArrayDebugPrint(FILE* out, DDArray_p array, long size)
{
   long i;

   for(i = 0; i<size; i++)
   {
      fprintf(out, " %5.3f ", DDArrayElement(array, i));
      if(((i+1) % 10)==0)
      {
    fprintf(out, "\n");
      }
   }
   fprintf(out, "\n");
}


/*-----------------------------------------------------------------------
//
// Function: DDArrayAdd()
//
//   Add the first limit elements from new to the corresponding entries
//   in collect. All entries are interpreted as numerical.
//
// Global Variables: -
//
// Side Effects    : Changes collect.
//
/----------------------------------------------------------------------*/

void DDArrayAdd(DDArray_p collect, DDArray_p data, long limit)
{
   long i;
   double old, new;

   assert(collect);
   assert(data);

   for(i=0; i<limit; i++)
   {
      old = DDArrayElement(collect, i);
      new = DDArrayElement(data, i);
      DDArrayAssign(collect, i, old+new);
   }
}


/*-----------------------------------------------------------------------
//
// Function: DDArraySelectPart()
//
//   Find a value d with at least part*(last+1) values >= d and
//   (1-part)*(last+1) values <= d in array.
//
// Global Variables: -
//
// Side Effects    : Changes order in array.
//
/----------------------------------------------------------------------*/

double DDArraySelectPart(DDArray_p array, double part, long size)
{
   double pivot, tmp, *arr;
   long i,j, start, end, rank1, rank2;

   assert((0 <= part) && (part <= 1));
   assert(size>0);

   tmp = (size-1)*part; /* -1 due to C's indexing */

   rank1 = ((long)tmp);
   rank2 = ((long)(tmp+0.5));

   start   = 0;
   end     = size-1;

   /* printf(COMCHAR" Rank1,2: %ld, %ld\n", rank1, rank2); */

   assert(array->size >= size);
   arr  = array->array;
   while(start!=end)
   {
      /* Pick a good pseudo-pivot to make worst case-behaviour less
    likely */

      pivot = (arr[start]+arr[(start+end)/2]+arr[end])/3.0;

      /* printf("Pivot: %f\n", pivot); */
      i=start;
      j=end;

      while(i != j)
      {
    /* printf("%ld, %ld\n", i, j); */
    while((i<j) && (arr[i] <= pivot))
    {
       i++;
    }
    while((j>i) && (arr[j] > pivot))
    {
       j--;
    }
    tmp    = arr[i];
    arr[i] = arr[j];
    arr[j] = tmp;
      }
      /* printf("Hier - i= %ld\n",i);
      DDArrayDebugPrint(stdout, array, size); */
      if(i > rank1)
      {
    end = i-1;
      }
      else
      {
    start = i;
      }
   }

   if(rank2!=rank1)
   {
      /* Now find the second value. We know that all values with index
    > rank1 are >= arr[rank1] and that all values >
    arr[rank1] have index > rank1 -> we only need to search for
    the minimum of the second part of the array */
      assert(rank1!=size-1); /* Should be impossible, otherwise part =
              1.0 and rank1==rank2 */
      tmp = arr[start+1];
      for(i=start+1; i<size; i++)
      {
    tmp = MIN(tmp, arr[i]);
      }
   }
   else
   {
      tmp = arr[start];
   }
   return (arr[start]+tmp)/2;
}




/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
