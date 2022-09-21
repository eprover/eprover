/*-----------------------------------------------------------------------

  File  : clb_simple_stuff.h

  Author: Stephan Schulz

  Contents

  Useful routines, usually pretty trivial.

  Copyright 1998-2017 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: Fri Oct 16 17:30:21 MET DST 1998

  -----------------------------------------------------------------------*/

#ifndef CLB_SIMPLE_STUFF

#define CLB_SIMPLE_STUFF

#include <string.h>
#include <clb_error.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* For sorting objects by a (double)float key: */

typedef struct
{
   double weight;
   IntOrP object;
}WeightedObjectCell, *WeightedObject_p;

typedef struct
{
   unsigned int xstate;
   unsigned int ystate;
   unsigned int zstate;
   unsigned int cstate;
}RandStateCell, *RandState_p;


typedef enum
{
   PRNoResult = 0,
   PRTheorem,
   PRUnsatisfiable,
   PRSatisfiable,
   PRCounterSatisfiable,
   PRFailure,
   PRGaveUp
}ProverResult;

typedef enum {
   PROBLEM_NOT_INIT = -1,
   PROBLEM_FO,
   PROBLEM_HO
} ProblemType;

extern ProblemType problemType;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define MAXINDENTSPACES 72


int StrDistance(const char* a, const char* b);

#define WeightedObjectArrayAlloc(number) \
        SecureMalloc(number * sizeof(WeightedObjectCell))
#define WeightedObjectArrayFree(array) FREE(array)

int     WeightedObjectCompareFun(WeightedObject_p o1, WeightedObject_p o2);

#define WeightedObjectArraySort(array, size) \
        qsort(array, size, sizeof(WeightedObjectCell),\
         (ComparisonFunctionType)WeightedObjectCompareFun)

void     JKISSSeed(RandState_p state, int seed1, int seed2, int seed3);
unsigned JKISSRand(RandState_p state);
double   JKISSRandDouble(RandState_p state);

char*  IndentStr(int level);
bool   StringStartsWith(const char* pattern, const char* prefix);

int    StringIndex(char* key, char* list[]);
long   StringArrayCardinality(char *array[]);

long   ComputeGCD(long a, long b);

void   SetProblemType(ProblemType t);

#define DBG_PRINT(out, prefix, main, suffix) \
   fprintf(out, "%s", prefix);\
   main;\
   fprintf(out, "%s", suffix)

#define DBG_TPRINT(out, prefix, term, suffix) \
   fprintf(out, "%s", prefix);\
   TermPrintDbg(out, term, TermGetBank(term)->sig, DEREF_NEVER);\
   fprintf(out, "%s", suffix)



#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
