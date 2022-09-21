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
         if(resource != RLIMIT_DATA) // Mask MacOS bug on ARM
         {
            snprintf(message, 300, "Could not set limit %s to %lld (%s)",
                     ldesc, (long long)limit, strerror(TmpErrno));
            Warning(message);
         }
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
// Function: GetCoreNumber()
//
//   Return the number of cores (via
//   sysconf(_SC_NPROCESSORS_ONLN). Returns 1 on failure (and prints a
//   warning), for safe continuation.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int GetCoreNumber(void)
{
   long res = -1;

   errno = 0;
   res = sysconf(_SC_NPROCESSORS_ONLN);
   if(errno)
   {
      assert(res==-1);
      Warning("sysconf(_NPROCESSORS_ONLN) call to get core number failed, assuming 1 core!\n");
      res = 1;
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: GetSystemPageSize()
//
//   Find and return the system page size (in bytes), if
//   possible. Return -1 otherwise.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long GetSystemPageSize(void)
{
   long res = -1;

   errno = 0;
   res = sysconf(_SC_PAGESIZE);
   if(errno)
   {
      assert(res==-1);
      Warning("sysconf() call to get page size failed!\n");
      res = -1;
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: GetSystemPhysMemory()
//
//   Try to find the phyical memory installed in the machine. Return
//   it (in MB) or -1 if no information can be obtained.
//
// Global Variables: -
//
// Side Effects    : No persistent ones
//
/----------------------------------------------------------------------*/

#define MEM_PHRASE "Primary memory available: "

long long GetSystemPhysMemory(void)
{
   long long res = -1;

   long long tmpres = 0, pages=-1, pagesize;

   pagesize = GetSystemPageSize();
#ifdef _SC_PHYS_PAGES
   pages = sysconf(_SC_PHYS_PAGES);
#endif
   if((pagesize !=-1) && (pages != -1))
   {
      tmpres = pagesize * pages;
      res = tmpres / MEGA;
   }
   if(res==-1)
   {
      FILE* pipe;
      char line[220];
      int limit = strlen(MEM_PHRASE);
      char *convert;
      double resmult = 0.0;

      pipe = popen("hostinfo", "r");
      if(pipe)
      {
         while(fgets(line, 220, pipe))
         {
            if(strncmp(MEM_PHRASE, line, limit)==0)
            {
               resmult = strtod(line+limit, &convert);
               if(strstr(convert, "kilobyte"))
               {  /* Past-proof, of course */
                  res = resmult/1024;
               }
               else if(strstr(convert, "megabyte"))
               {
                  res = resmult;
               }
               else if(strstr(convert, "gigabyte"))
               {
                  res = resmult*1024;
               }
               else if(strstr(convert, "terabyte"))
               { /* Future-proof */
                  res = resmult*(1024*1024);
               }
               else
               {
                  res = -1;
               }
               break;
            }
         }

         pclose(pipe);
      }
   }
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: StrideMemory()
//
//   Write an arbitrary value into memory each E_PAGE_SIZE
//   bytes. It's used for preallocated memory reserves. Normally,
//   allocated pages need not really be available unless written to if
//   overallocation is being used. This should ensure that allocated
//   pages are backed by real memory in such (broken!) cases.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void StrideMemory(char* mem, long size)
{
   char* stride;
   static long e_page_size = 0;

   if(!e_page_size)
   {
      e_page_size = GetSystemPageSize();
   }
   if(e_page_size==-1)
   {
      Warning("Could not determine page size, guessing 4096!");
      e_page_size=4096;
   }

   for(stride = mem; stride < mem+size; stride+=e_page_size)
   {
      *stride = 'S'; /* Arbitrary value*/
   }
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
