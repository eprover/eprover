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

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define SysDateCreationTime() ((SysDate)0)
#define SysDateInvalidTime() ((SysDate)-1)
#define SysDateIsInvalid(date) ((date) == SysDateInvalidTime())
#define SysDateInc(sd) ((*(sd))++);assert(*(sd));
#define SysDateIsEarlier(date1, date2) ((date1)<(date2))
#define SysDateEqual(date1, date2) ((date1)==(date2))
void    SysDatePrint(FILE* out, SysDate date);
#define SysDateMaximum(date1, date2) MAX(date1, date2)
#define SysDateIsCreationDate(date) ((date) == SysDateCreationTime())

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





