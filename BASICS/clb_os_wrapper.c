/*-----------------------------------------------------------------------

File  : clb_os_wrapper.c

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Wrapper functions for certain OS functionality.

  Copyright 2007 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sun Jul  1 13:45:15 CEST 2007
    New

-----------------------------------------------------------------------*/

#include <unistd.h>
#include <time.h>

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
      TmpErrno = errno;
      return RLimFailed;
   }
   if(rlim.rlim_max < limit)
   {
      retval   = RLimReduced;
      limit = rlim.rlim_max;
   }
   rlim.rlim_cur = limit;
   if(setrlimit(resource, &rlim)==-1)
   {
      TmpErrno = errno;
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
   static char message[300];
   RLimResult res;

   if(desc)
   {
      ldesc = desc;
   }
   res = SetSoftRlimit(resource, limit);

   switch(res)
   {
   case RLimFailed:
         snprintf(message, 300, "Could not set limit %s to %lld (%s)",
                  ldesc, (long long)limit, strerror(TmpErrno));
         Warning(message);
         break;
   case RLimReduced:
         snprintf(message, 300, "Had to reduce limit %s", ldesc);
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
//   Try to increase the maximum stack size, then reexec the process
//   to work under the new limit. At least on some UNIXES, maximum
//   stack size cannot increase after the process has started).
//
// Global Variables: -
//
// Side Effects    : Well, all, but really none ;-)
//
/----------------------------------------------------------------------*/

/* void IncreaseMaxStackSize(char *argv[], rlim_t stacksize) */
/* { */
/*    int   i, argc; */
/*    char* opt="Dummy"; */
/*    printf("Hallo\n"); */

/*    char **argv2; */
/*    for(i=1; argv[i]; i++) */
/*    { */
/*       printf("Hallo %s\n", argv[i]); */
/*       if(strcmp(argv[i], opt)==0) */
/*       { */
/*          return; */
/*       } */
/*    } */
/*    argc = i; */

/*    if(GetSoftRlimit(RLIMIT_STACK)>=stacksize*KILO) */
/*    { */
/*       return; */
/*    }   */
/*    if(SetSoftRlimit(RLIMIT_STACK, stacksize*KILO)!=RLimSuccess) */
/*    { */
/*       TmpErrno = errno; */
/*       Warning("Cannot set stack limit:"); */
/*       Warning(strerror(TmpErrno)); */
/*       Warning("Continuing with default stack limit"); */
/*       return; */
/*    } */

/*    argv2 = malloc(sizeof(char*) * (argc+2)); */
/*    for(i=0; argv[i]; i++) */
/*    { */
/*       argv2[i] = argv[i]; */
/*    } */
/*    argv2[i] = opt; */
/*    argv2[i+1] = NULL; */

/*    if(execvp(argv2[0], argv2)) */
/*    { */
/*       TmpErrno = errno; */
/*       SysError("Cannot exec", SYS_ERROR); */
/*    } */
/*    free(argv2); */
/* } */


/*-----------------------------------------------------------------------
//
// Function: GetUSecTime()
//
//   Return the time in microseconds since the epoch.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long long GetUSecTime(void)
{
   struct timeval tv;

   gettimeofday(&tv, NULL);

   return (long long)tv.tv_sec*1000000ll+tv.tv_usec;
}

/*-----------------------------------------------------------------------
//
// Function: GetUSecClock()
//
//   Return the process cpu time in microseconds.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long long GetUSecClock(void)
{
   long long res = (clock()*1000000ll)/CLOCKS_PER_SEC;

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: SecureFOpen()
//
//   As fopen(), but terminate with a useful error message on failure.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

FILE* SecureFOpen(char* name, char* mode)
{
   FILE* res;

   res = fopen(name, mode);
   if(!res)
   {
      TmpErrno = errno;
      SysError("Cannot open file %s",FILE_ERROR,name);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: SecureFClose()
//
//   As fclose(), but print a warning on error.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void SecureFClose(FILE* fp)
{
   if(fclose(fp))
   {
      TmpErrno = errno;
      SysWarning("Problem closing file");
   }
}





/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


