/*-----------------------------------------------------------------------

  File  : clb_defines.h

  Author: Stephan Schulz

  Contents

  Basic definition useful (very nearly) everywhere.

  Copyright 1998-2020 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Sat Jul  5 02:28:25 MET DST 1997

-----------------------------------------------------------------------*/

#ifndef CLB_DEFINES

#define CLB_DEFINES

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/param.h>
#include <unistd.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <inttypes.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* Trick the stupid type concept for polymorphic indices (hashes,
   trees) with int/pointer type. */

typedef union int_or_p
{
   long i_val;
   void *p_val;
}IntOrP;

#ifdef CONSTANT_MEM_ESTIMATE
#define INTORP_MEM 4
#define LONG_MEM 4
#else
#define INTORP_MEM sizeof(IntOrP)
#define LONG_MEM sizeof(long)
#endif

/* Generic cleanup function for pseudo-objects - the function has to
   know how to get rid of the passed data. */

typedef void (*GenericExitFun)(void* data);

/* Type of a comparison function for <stdlib>'s qsort */

typedef int (*ComparisonFunctionType)(const void*, const void*);

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

// Character used to introduce comments

#ifndef UNIX_COMMENTS
// Doubled for printf...
#define COMCHAR "%%"
#define COMCHARRAW "%"
#else
#define COMCHAR "#"
#define COMCHARRAW "#"
#endif



#undef MAX
#define MAX(x,y) ({ __typeof__ (x) _x = (x);    \
         __typeof__ (y) _y = (y);               \
         _x > _y ? _x : _y; })

#undef MIN
#define MIN(x,y) ({ __typeof__ (x) _x = (x);    \
         __typeof__ (y) _y = (y);               \
         _x < _y ? _x : _y; })

#define CMP(x,y) ({ __typeof__ (x) _x = (x);    \
         __typeof__ (y) _y = (y);               \
         (_x > _y) - (_x < _y); })

#undef ABS
#define ABS(x) ((x)>0?(x):-(x))

#undef XOR
#define XOR(x,y) (!(x)!=!(y))

#undef EQUIV
#define EQUIV(x,y) (!(x)==!(y))

#undef SWAP
#define SWAP(x,y) do{ __typeof__ (x) tmp =(x); (x)=(y); (y)=(tmp);}while(0)


#define LIKELY(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)

#define UNUSED(x) (void)(x)

#define KILO 1024
#define MEGA (1024*1024)

#ifdef ENABLE_LFHO
#define LFHO(x) x
#else
#define LFHO(x)
#endif

/* Convenience function */
static inline size_t WriteStr(int fd, const char* msg);


#ifdef PRINT_TSTP_STATUS
#define TSTPOUT(file,msg) fprintf(file, COMCHAR" SZS status %s\n", msg); fflush(file)
#define TSTPOUTFD(fd,msg) do{                                   \
      WriteStr(fd, COMCHAR" SZS status ");                            \
      WriteStr(fd, msg);                                        \
      WriteStr(fd, "\n");                                       \
   }while(0)
#else
#define TSTPOUT(file,msg)
#define TSTPOUTFD(fd,msg)
#endif


#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)
#define GCC_DIAGNOSTIC_POP  _Pragma("GCC diagnostic pop")
#define GCC_DIAGNOSTIC_PUSH _Pragma("GCC diagnostic push")
#else
#define GCC_DIAGNOSTIC_POP
#define GCC_DIAGNOSTIC_PUSH
#endif

#define BOOL2STR(val) (val)?"true":"false"

/*-----------------------------------------------------------------------
//
// Function: WriteStr()
//
//   Computes the length of msg and writes msg to the file descriptor.
//   WriteStr is used for output instead of the print functions in low
//   memory situations since the later may try to allocate memory which
//   is likely to fail. WriteStr is defined as a function instead of a
//   macro to silence warnings in case the return value of write is
//   unused. The function write may be defined with warn_unused_result
//   in the system header files.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static inline size_t WriteStr(int fd, const char* msg){
   return write(fd, msg, strlen(msg));
}


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
