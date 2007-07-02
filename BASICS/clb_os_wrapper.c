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

RLimResult SetSoftRlimit(int resource, rlim_t limit)
{
   RLimResult retval = RLimSuccess;
   struct rlimit rlim;

   if(getrlimit(resource, &rlim)==-1)
   {
      return RLimFailed;
   }
   if(rlim.rlim_max < resource)
   {
      retval   = RLimReduced;
      limit = rlim.rlim_max;
   }
   rlim.rlim_cur = limit;
   if(setrlimit(resource, &rlim)==-1)
   {
      retval = RLimFailed;
   }

   return retval;
}


/*-----------------------------------------------------------------------
//
// Function: SetSoftRlimitErr()
//
//   Try to set a soft limit to the given value. Print a warning if it
//   has to be reduced, terminate with a proper system error if it
//   fails. If desc is provided, use it for messages.
//
// Global Variables: 
//
// Side Effects    : As described...
//
/----------------------------------------------------------------------*/

void SetSoftRlimitErr(int resource, rlim_t limit, char* desc)
{
   char* ldesc ="";
   char message[200];
   RLimResult res;

   if(desc)
   {
      assert(strlen(desc)<=100);
      ldesc = desc;
   }
   res = SetSoftRlimit(resource, limit);

   switch(res)
   {
   case RLimFailed:
         sprintf(message, "Could not set limit %s", ldesc);
         TmpErrno = errno;
         SysError(message, SYS_ERROR);
         break;
   case RLimReduced:
         sprintf(message, "Had to reduce limit %s", ldesc);
         Warning(message);
         break;
   case RLimSuccess:
         // Nothing to do 
         break;
   default:
         assert(false && "Out of bounds return from SetSoftRlimit()");
         break;
   }   
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


