/*-----------------------------------------------------------------------

File  : clb_defines.h

Author: Stephan Schulz

Contents
 
  Basic definition useful (very nearly) everywhere.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sat Jul  5 02:28:25 MET DST 1997
    New

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


/* Maximum and minimum, absolute values, exclusive or functions */

#ifdef MAX
#undef MAX
#endif
#define MAX(x,y) ((x)>(y)?(x):(y))

#ifdef MIN
#undef MIN
#endif
#define MIN(x,y) ((x)<(y)?(x):(y))

#ifdef ABS
#undef ABS
#endif
#define ABS(x)   ((x)>0? (x):-1*(x))

#ifdef XOR
#undef XOR
#endif
#define XOR(x,y) (((!(x))&&(y))||((x)&&(!(y))))

#ifdef EQUIV
#undef EQUIV
#endif
#define EQUIV(x,y) ((((x))&&(y))||(!(x)&&(!(y))))

#define SWAP(type, x,y) {type tmp =(x); (x)=(y); (y)=(tmp);}


/* I cannot keep things in my mind ;-) */

#define KILO 1024
#define MEGA (KILO*KILO)


/* Convenience function */
#define WRITE_STR(fd,msg) write(fd,msg,strlen(msg));


#ifdef PRINT_TSTP_STATUS
#define TSTPOUT(file,msg) fprintf(file, "# SZS status %s\n", msg)
#define TSTPOUTFD(fd,msg) \
        WRITE_STR(fd, "# SZS status ");WRITE_STR(fd, msg);WRITE_STR(fd, "\n")

#else
#define TSTPOUT(file, msg)
#define TSTPOUTFD(fd,msg)
#endif

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





