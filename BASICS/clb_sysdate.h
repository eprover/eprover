/*-----------------------------------------------------------------------

File  : clb_sysdate.h

Author: Stephan Schulz

Contents
 
  Data types dealing with "dates" and "times". A "time" in this
  context is a data type with a defined starting point and a total
  ordering that monotonically increases during the run of the program
  and can be used to define an order of events. A "date" is a specific
  element from a "time".

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed Apr  8 16:49:44 MET DST 1998
    New

-----------------------------------------------------------------------*/

#ifndef CLB_SYSDATE

#define CLB_SYSDATE

#include <stdio.h>
#include <limits.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* Data type used both for "time" keeping and recording of
   "dates". */

typedef long SysDate;
   

typedef enum 
{
   DateEarlier = -1,
   DateEqual = 0,
   DateLater = 1
}DateRelation;

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define      SysDateCreationTime() 0L
#define      SysDateArmageddonTime() ULONG_MAX
#define      SysDateInc(sd) ((*(sd))++);assert(*(sd));
static __inline__ DateRelation SysDateCompare(SysDate date1, SysDate date2);
void         SysDatePrint(FILE* out, SysDate date);
#define      SysDateMaximum(date1, date2) \
             (SysDateCompare((date1), (date2))==DateEarlier?\
             (date2):(date1))
#define      SysDateIsCreationDate(date)\
             (SysDateCompare((date),SysDateCreationTime())==DateEqual)



/*-----------------------------------------------------------------------
//
// Function:  SysDateCompare()
//
//   Compare two times, return DateEqual, DateEarlier, DateLater,
//   if the first date is equal to, smaller or bigger than the second
//   one. 
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static __inline__ DateRelation SysDateCompare(SysDate date1, SysDate date2)
{
   if(date1 > date2)
   {
      return DateLater;
   }
   else if(date1 < date2)
   {
      return DateEarlier;
   }
   return DateEqual;
}

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





