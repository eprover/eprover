/*-----------------------------------------------------------------------

File  : clb_defines.h

Author: Stephan Schulz

Contents
 
  Basic definition useful (very nearly) everywhere.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
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
#ifdef SPEC_CPU2004
#define MAXPATHLEN 256
#define RESTRICTED_FOR_WINDOWS
#else
#include <sys/param.h>
#include <unistd.h>
#endif
#include <stdlib.h>
#include <math.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/



#ifndef  __cplusplus

/* Booleans (if we don't already have them */

typedef enum
{
   false = 0,
   true = 1
}bool;

#define CPPCAST(type) /* Nothing */

#else

/* C++ casts */
#define CPPCAST(type) (type)

#endif


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


/* The NULL pointer */

#ifndef NULL
#ifndef  __cplusplus
#define NULL ((void*)0)
#else
#define NULL 0
#endif
#endif


#define E_URL "http://www.eprover.org"

#define STS_MAIL "schulz@eprover.org"
#define STS_SNAIL "Stephan Schulz (I4)\nTechnische Universitaet Muenchen\nInstitut fuer Informatik\nBoltzmannstrasse 3\n85748 Garching bei Muenchen\nGermany\n"

/* Allow for volatile functions if the compiler supports them - 
   gcc version egcs-2.90.23 980102 (egcs-1.0.1 release)
   apparently defines __GNUC__  but does not *sigh* */

#ifdef NEVER_DEFINED
#define VOLATILE volatile
#else
#define VOLATILE
#endif


/* Generic cleanup function for pseudo-objects - the function has to
   know how to get rid of the passed data. */

typedef void (*GenericExitFun)(void* data);

/* Type of a comparison function for <stdlib>'s qsort */

typedef int (*ComparisonFunctionType)(const void*, const void*);

typedef unsigned long ulong_c;


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

/* Macros for debugging: If debugging is enabled and DEBUGLEVEL
   matches the given level, do something arbitrary or print a
   message. DEBUGLEVEL is set in CLIB/Makefile.vars, the following
   values are predefined. */


#define PP_LOWDETAILS          1   /* Main loop stuff */
#define PP_HIGHDETAILS         2   /* Primary subroutines in
				      cco_proofproc.c */
#define PP_INSERTING_NEW     128  /*  Monitor new (and rewritten)
				      clauses */
#define CO_SIMPLIFICATION      4  /* Control of simplification */
#define RW_INTERFACE_WATCH1    8  /* Rewriting, Equations and up */
#define RW_INTERFACE_WATCH2   16  /* Rewriting, terms */
#define RW_MATCH_WATCH        32  /* Individual Matches */
#define RW_REWRITE_WATCH      64  /* Results of rewrite steps */
#define PDT_INTERFACE_WATCH  512  /* Discrimination trees input and
				     output */
#define PM_INPUT_PRINT       256  /* What goes into paramod? */
#define RPL_TERM_WATCH      2048  /* Term replacing in term banks */


#if defined(DEBUGLEVEL)
#define DEBUGCMD(level,op)       if((level)&DEBUGLEVEL){op}
#define DEBUGMARK(level,text) if((level)&DEBUGLEVEL)\
                                 {printf("# [%3d]"text,(level));}
#define DEBUGOUT(level,text)  if((level)&DEBUGLEVEL)\
                                 {printf(text);}
#else
#define DEBUGCMD(level,op)
#define DEBUGMARK(level,text)
#define DEBUGOUT(level,text)
#endif

#ifdef PRINT_TSTP_STATUS
#define TSTPOUT(file,msg) fprintf(file, "# TSTP exit status: %s\n", msg)
#else
#define TSTPOUT(file, msg)
#endif

#ifdef RESTRICTED_FOR_WINDOWS /* Windowss is broken */
#define rlim_t long
#endif

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





