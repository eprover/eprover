/*-----------------------------------------------------------------------

File  : clb_simple_stuff.h

Author: Stephan Schulz

Contents

  Useful routines, usually pretty trivial.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Fri Oct 16 17:30:21 MET DST 1998
    New

-----------------------------------------------------------------------*/

#ifndef CLB_SIMPLE_STUFF

#define CLB_SIMPLE_STUFF

#include <string.h>
#include <clb_error.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* For sorting objects by a float key: */

typedef struct
{
   double weight;
   IntOrP object;
}WeightedObjectCell, *WeightedObject_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define MAXINDENTSPACES 72

#define WeightedObjectArrayAlloc(number) \
        SecureMalloc(number * sizeof(WeightedObjectCell))
#define WeightedObjectArrayFree(array) FREE(array)

int     WeightedObjectCompareFun(WeightedObject_p o1, WeightedObject_p o2);

#define WeightedObjectArraySort(array, size) \
        qsort(array, size, sizeof(WeightedObjectCell),\
         (ComparisonFunctionType)WeightedObjectCompareFun)

void     JKISSSeed(int seed1, int seed2, int seed3);
unsigned JKISSRand();
double   JKISSRandDouble();

int    StringIndex(char* key, char* list[]);

char*  IndentStr(int level);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
