/*-----------------------------------------------------------------------

  File  : clb_simple_stuff.c

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

#include "clb_simple_stuff.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

static unsigned int xstate = 123456789,
   ystate = 987654321,
   zstate = 43219876,
   cstate = 6543217; /* State for KISS RNG*/


static RandStateCell rand_state = {123456789,987654321,43219876,6543217};

ProblemType problemType = PROBLEM_NOT_INIT;


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
// Function: StrDistance()
//
//   Compute distance of two strings (number of different characters,
//   plus difference in length.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int StrDistance(const char* a, const char* b)
{
   int dist = 0;
   while(*a && *b)
   {
      dist += *a == *b ? 0 : 1;
      a++;
      b++;
   }
   while(*a)
   {
      a++;
      dist++;
   }
   while(*b)
   {
      b++;
      dist++;
   }
   return dist;
}


/*-----------------------------------------------------------------------
//
// Function: WeightedObjectCompareFun()
//
//   Compare the weight of two weighted objects.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int WeightedObjectCompareFun(WeightedObject_p o1, WeightedObject_p o2)
{
   int res = 0;

   if(o1->weight < o2->weight)
   {
      res = -1;
   }
   else if(o1->weight > o2->weight)
   {
      res = 1;
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: JKISSSeed()
//
//   Initialize the portable KISS random number generator.
//
// Global Variables: xstate, ystate, zstate, cstate
//
// Side Effects    : Sets xstate, ystate, zstate
//
/----------------------------------------------------------------------*/

void JKISSSeed(RandState_p state, int seed1, int seed2, int seed3)
{
   if(!state)
   {
      state = &rand_state;
   }
   state->xstate = seed1;
   state->ystate = seed2;
   state->zstate = seed3;
}


/*-----------------------------------------------------------------------
//
// Function: JKISSRand()
//
//   Improved "Keep It Simple, Stupid" RNG generator, adapted from the
//   public domain version by Davida Jones (d.jones@cs.ucl.ac.uk).
//
// Global Variables: xstate, ystate, zstate, cstate
//
// Side Effects    : Permutes state
//
/----------------------------------------------------------------------*/

unsigned JKISSRand(RandState_p state)
{
   unsigned long long t;

   if(!state)
   {
      state = &rand_state;
   }
   xstate = 314527869 * xstate + 1234567;
   ystate ^= ystate << 5;
   ystate ^= ystate >> 7;
   ystate ^= ystate << 22;
   t = 4294584393ULL * zstate + cstate;
   cstate = t >> 32;
   zstate = t;
   return xstate + ystate + zstate;
}

/*-----------------------------------------------------------------------
//
// Function: JKISSRandDouble()
//
//    Returns a pseudo-random number r with 0 <= r < 1.
//
// Global Variables: As JKISSRand()
//
// Side Effects    : Per JKISSRand()
//
/----------------------------------------------------------------------*/

double JKISSRandDouble(RandState_p state)
{
   return JKISSRand(state) / 4294967296.0;
}


/*-----------------------------------------------------------------------
//
// Function: IndentStr()
//
//   Return a pointer to a string of level spaces, or MAXINDENTSPACES
//   if this is smaller. Not reentrant.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

char* IndentStr(int level)
{
   static char spaces[MAXINDENTSPACES+1];
   int i;

   for(i=0; i<MIN(MAXINDENTSPACES, level); i++)
   {
      spaces[i] = ' ';
   }
   spaces[i] = '\0';

   return spaces;
}

/*-----------------------------------------------------------------------
//
// Function: StringStartsWith()
//
//   Determines if string pattern starts with string prefix.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool StringStartsWith(const char* pattern, const char* prefix)
{
   while(*prefix)
   {
      if(!*pattern || *prefix++ != *pattern++)
      {
         return false;
      }
   }
   return true;
}


/*-----------------------------------------------------------------------
//
// Function: StringIndex()
//
//   Given a NULL-Terminated array of strings, return the index of key
//   (or -1 if key does not occur in the array).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int StringIndex(char* key, char* list[])
{
   int i;
   {
      for(i=0; list[i]; i++)
      {
         if(strcmp(key, list[i])==0)
         {
            return i;
         }
      }
   }
   return -1;
}


/*-----------------------------------------------------------------------
//
// Function: StringArrayCardinality()
//
//   Return the number of initial non-NULL entries in a
//   NULL-terminated string array.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long StringArrayCardinality(char *array[])
{
   long i;

   for(i=0; array[i]; i++)
   {
   }
   return i;
}


/*-----------------------------------------------------------------------
//
// Function: ComputeGCD()
//
//   Compute the Greatest Common Divisor of two (positive)
//   longs. Returns 0 if both are 0 or one is negative.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ComputeGCD(long a, long b)
{
   if((a<0) || (b<0))
   {
      return 0;
   }
   while(true)
   {
      if(a==0)
      {
         return b;
      }
      if(b==0)
      {
         return a;
      }
      if(a>b)
      {
         a = a%b;
      }
      else
      {
         b = b%a;
      }
   }
   return a;
}



/*-----------------------------------------------------------------------
//
// Function: SetProblemType()
//
//   Changes problem type to t if the problem type was not initialized.
//   If user tries to overried the problem type the error is reported.
//
// Global Variables: problemType
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void SetProblemType(ProblemType t)
{
  if(problemType == PROBLEM_NOT_INIT || problemType == t)
  {
     problemType = t;
  }
  else
  {
     Error("Mixing of first order and higher order syntax is not allowed.",
           SYNTAX_ERROR);
  }
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
