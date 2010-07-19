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
// Function: Log2()
//
//   Return the logarithm dualis of value.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

#define LOG_2 0.693147180559945286226763983

double Log2(double value)
{
   return log(value)/LOG_2;
}

/*-----------------------------------------------------------------------
//
// Function: Log2Ceil()
//
//   Return the ceiling of the logarithm dualis of value.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long Log2Ceil(long value)
{
   long i = -1;

   if(!value)
   {
      return -1;
   }
   value--;
   while(value)
   {
      value>>=1;
      i++;
   }
   return i+1;
}


#ifdef NEED_MATH_EMULATION

/*-----------------------------------------------------------------------
//
// Function: sqrt()
//
//   Simple hack implementing the square root function, which seems
//   to be missing from (some version of) Solaris libm with profiling
//   support. Don't ask me... This is slow and inexact....
//
// Global Variables: -
//
// Side Effects    : Slow
//
/----------------------------------------------------------------------*/

double sqrt(double value)
{
   double lower, upper, guess, diff;

   assert(value>=0);
   
   lower = 0; 
   upper = value;
   guess = (lower+upper)/2;
   diff = (guess*guess)-value;

   while(ABS(diff)>0.00001)
   {
      if(diff>0)
      {
	 upper = guess;
      }
      else
      {
	 lower = guess;
      }
      guess = (lower+upper)/2;
      diff = (guess*guess)-value;      
   }
   return guess;
}

/*-----------------------------------------------------------------------
//
// Function: pow()
//
//   Compute x**y. See above. Result is _very_ inexact!
//
// Global Variables: -
//
// Side Effects    : Slow
//
/----------------------------------------------------------------------*/

double pow(double x, double y)
{   
   int i;
   double old = 1;
   
   for(i=0; i<=y; i++)
   {
      old = old*x;
   }
   return old;
}


#endif

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

char* IndentStr(unsigned level)
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


