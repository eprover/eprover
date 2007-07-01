/*-----------------------------------------------------------------------

File  : clb_os_wrapper.c

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Wrapper functions for certain OS functionality. 

  Copyright 2007 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Sun Jul  1 13:45:15 CEST 2007
    New

-----------------------------------------------------------------------*/

#include "clb_os_wrapper.h"



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
// Function: SetSoftRlimit()
//
//   Set a soft limit to the given value (or as close as the hard
//   limit allows). Return result. If get- or setrlimit() fail, errno
//   will contain the corresponding cause.
//
// Global Variables: -
//
// Side Effects    : Changes the limits
//
/----------------------------------------------------------------------*/

RlimResult SetSoftRlimit(int resource, rlim_t limit)
{
   RlimResult retval = RLimSuccess;
   struct rlimit rlim;

   if(getrlimit(resource, &rlim)==-1)
   {
      return RLimFailed;
   }
   if(rlim.rlim_max < resource)
   {
      retval   = RLimReduced;
      resource = rlim.rlim_max;
   }
   rlim.rlim_cur = resource;
   if(setrlimit(resource, &rlim)==-1)
   {
      retval = RLimFailed;
   }
   return retval;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


