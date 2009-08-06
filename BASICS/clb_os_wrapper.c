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

#include <unistd.h>

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
         /* Nothing to do */
         break;
   default:
         assert(false && "Out of bounds return from SetSoftRlimit()");
         break;
   }   
}


/*-----------------------------------------------------------------------
//
// Function: SetMemoryLimit()
//
//   Set memory limit to the given limit (if any), or the largest
//   possible.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void SetMemoryLimit(rlim_t mem_limit)
{
   if(!mem_limit)
   {
      return;
   }  
   SetSoftRlimitErr(RLIMIT_DATA, mem_limit, "RLIMIT_DATA");
#ifdef RLIMIT_AS
   SetSoftRlimitErr(RLIMIT_DATA, mem_limit, "RLIMIT_AS");
#endif /* RLIMIT_AS */
}


/*-----------------------------------------------------------------------
//
// Function: GetSoftRlimit()
//
//   Return the soft limit for the given resource, or 0 on failure.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

rlim_t GetSoftRlimit(int resource)
{
   struct rlimit rlim;
 
   if(getrlimit(resource, &rlim)==-1)
   {
      return 0;      
   }
   return rlim.rlim_cur;
}




/*-----------------------------------------------------------------------
//
// Function: IncreaseMaxStackSize()
//
//   Try to increase the maximum stack size, then create a forked copy
//   of the process to work under the new limit. Wait for this process
//   to do the actual work, then propagate its exit
//   status/condition. At least on some UNIXES, maximum stack size
//   cannot increase after the process has started).
//
// Global Variables: -
//
// Side Effects    : Well, all, but really none ;-)
//
/----------------------------------------------------------------------*/

void IncreaseMaxStackSize(char *argv[], rlim_t stacksize)
{
   if(GetSoftRlimit(RLIMIT_STACK)>=stacksize*KILO)
   {
      return;
   }  
   if(SetSoftRlimit(RLIMIT_STACK, stacksize*KILO)!=RLimSuccess)
   {
      TmpErrno = errno;
      SysError("Cannot set stack size", SYS_ERROR);
   }
   if(execvp(argv[0], argv))
   {
      TmpErrno = errno;
      SysError("Cannot exec", SYS_ERROR);
   }
}




/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


