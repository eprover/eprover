/*-----------------------------------------------------------------------

File  : clb_sysdate.c

Author: Stephan Schulz

Contents

  See clb_sysdate.h...this is extremely simple ;-)

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed Apr  8 19:51:08 MET DST 1998
    New

-----------------------------------------------------------------------*/

#include "clb_sysdate.h"
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
// Function: SysDatePrint()
//
//   Print  representation of a system time to the given channel.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void SysDatePrint(FILE* out, SysDate date)
{
   fprintf(out, "%5lu", date);
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
