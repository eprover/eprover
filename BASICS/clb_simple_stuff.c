/*-----------------------------------------------------------------------

File  : clb_simple_stuff.c

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

#include "clb_simple_stuff.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

static unsigned int xstate = 123456789,
   ystate = 987654321,
   zstate = 43219876,
   cstate = 6543217; /* State for KISS RNG*/

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

void JKISSSeed(int seed1, int seed2, int seed3)
{
   xstate = seed1;
   ystate = seed2;
   zstate = seed3;
}


/*-----------------------------------------------------------------------
//
// Function: JKISSRand()
//
//   Improved "Keep It Simple, Stupid" RNG generator, adapted from the
//   public domain version by David Jones (d.jones@cs.ucl.ac.uk).
//
// Global Variables: xstate, ystate, zstate, cstate
//
// Side Effects    : Permutes state
//
/----------------------------------------------------------------------*/

unsigned JKISSRand()
{
   unsigned long long t;
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

double JKISSRandDouble()
{
   return JKISSRand() / 4294967296.0;
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


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
