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
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
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
   "dates". Uses a struct because it may become necessary to extend
   this (2 to the power of 32 is not _really_ big...) */

typedef struct sysdate
{
   unsigned long date;
}SysDate, *SysDate_p;
   

typedef enum 
{
   DateEarlier = -1,
   DateEqual = 0,
   DateLater = 1
}DateRelation;

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

static __inline__ SysDate      SysDateCreationTime(void);
static __inline__ SysDate      SysDateArmageddonTime(void);
#define      SysDateInc(sd) (((sd)->date)++);assert((sd)->date);
static __inline__ DateRelation SysDateCompare(SysDate date1, SysDate date2);
void         SysDatePrint(FILE* out, SysDate date);
#define      SysDateMaximum(date1, date2) \
             (SysDateCompare((date1), (date2))==DateEarlier?\
             (date2):(date1))
#define      SysDateIsCreationDate(date)\
             (SysDateCompare((date),SysDateCreationTime())==DateEqual)


/*-----------------------------------------------------------------------
//
// Function:  SysDateCreationTime()
//
//   Return a representation of the earliest possible time.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static __inline__ SysDate SysDateCreationTime(void)
{
   SysDate date;
   
   date.date = 0;
   
   return date;
}


/*-----------------------------------------------------------------------
//
// Function:  SysDateArmageddonTime()
//
//   Return a representation of the latest possible time.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static __inline__ SysDate SysDateArmageddonTime(void)
{
   SysDate date;
   
   date.date = ULONG_MAX;
   
   return date;
}


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
   if(date1.date > date2.date)
   {
      return DateLater;
   }
   else if(date1.date < date2.date)
   {
      return DateEarlier;
   }
   return DateEqual;
}

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





