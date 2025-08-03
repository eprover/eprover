/*-----------------------------------------------------------------------

File  : clb_os_wrapper.h

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Functions wrapping some OS functions in a convenient manner.

  Copyright 2007 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1>     New

-----------------------------------------------------------------------*/

#ifndef CLB_OS_WRAPPERS

#define CLB_OS_WRAPPERS

#include <assert.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "clb_error.h"


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef enum
{
   RLimFailed,
   RLimReduced,
   RLimSuccess
}RLimResult;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#ifdef PROFILE_WALL_CLOCK
#define GETTIME GetUSecTime
#else
#define GETTIME GetUSecClock
#endif

#define INCREASE_STACK_SIZE \
   {\
      struct rlimit rlim;\
      int res;\
      rlim.rlim_cur = 0;\
      rlim.rlim_max = 0;\
      \
      if(getrlimit(RLIMIT_STACK, &rlim)==-1)\
      {\
         TmpErrno = errno;\
         fprintf(stderr, "%s: Warning: Cannot get stack limit", argv[0]);\
      }\
      else\
      {\
         rlim.rlim_cur=rlim.rlim_max;\
         res = setrlimit(RLIMIT_STACK, &rlim);\
         if(res ==-1)\
         {\
            fprintf(stderr, "%s: Warning: Cannot set stack limit", argv[0]);\
            fprintf(stderr, "%s: Warning: Continuing with default stack size", argv[0]);\
         }\
      }\
   }



#ifdef INSTRUMENT_PERF_CTR
#define PERF_CTR_DEFINE(name)  long long name = 0; long long name##_store = 0
#define PERF_CTR_DECL(name)    extern long long name; extern long long name##_store
#define PERF_CTR_RESET(name)   name = 0
#define PERF_CTR_ENTRY(name)   name##_store = GETTIME()
#define PERF_CTR_EXIT(name)    name+=(GETTIME()-(name##_store));name##_store=0
#define PERF_CTR_PRINT(out, name) fprintf((out), COMCHAR" PC%-34s : %f\n", "(" #name ")", ((float)name)/1000000.0)
#else
#define PERF_CTR_DEFINE(name)  enum { name##_store } // Used to silence compiler warnings
#define PERF_CTR_DECL(name)    enum { name }         // about extra semicolons.
#define PERF_CTR_RESET(name)
#define PERF_CTR_ENTRY(name)
#define PERF_CTR_EXIT(name)
#define PERF_CTR_PRINT(out, name)
#endif

RLimResult SetSoftRlimit(int resource, rlim_t limit);
void       SetSoftRlimitErr(int resource, rlim_t limit, char* desc);
void       SetMemoryLimit(rlim_t mem_limit);
rlim_t     GetSoftRlimit(int resource);
// void       IncreaseMaxStackSize(char *argv[], rlim_t stacksize);
long long  GetUSecTime(void);
long long  GetUSecClock(void);
#define    GetMSecTime() (GetUSecTime()/1000)
#define    GetSecTime() (GetUSecTime()/1000000)
#define    GetSecTimeMod() (GetSecTime()%1000)
int        GetCoreNumber(void);
long       GetSystemPageSize(void);
long long  GetSystemPhysMemory(void);
void       StrideMemory(char* mem, long size);

FILE*      SecureFOpen(char* name, char* mode);
void       SecureFClose(FILE* fp);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
