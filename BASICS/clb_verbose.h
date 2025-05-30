/*-----------------------------------------------------------------------

File  : clb_verbose.h

Author: Stephan Schulz

Contents

  Declarations for the Verbose variable and macros for verbose
  reporting on certain operations.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Mon Sep 15 14:41:33 MET DST 1997
    New

-----------------------------------------------------------------------*/

#ifndef CLB_VERBOSE

#define CLB_VERBOSE

#include <clb_error.h>
#include <clb_simple_stuff.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

extern int Verbose;

#define VERBOSE(arg) {if(Verbose){arg}}
#define VERBOUT(arg) VERBOSE(\
        fprintf(stderr, "%s: %s", ProgName, (arg)); fflush(stderr);)
#define VERBOUTARG(arg1,arg2) VERBOSE(\
        fprintf(stderr, "%s: %s%s\n", ProgName, (arg1), (arg2));\
   fflush(stderr);)

#define VERBOSE2(arg) {if(Verbose>=2){arg}}
#define VERBOUT2(arg) VERBOSE2(\
        fprintf(stderr, "%s: %s", ProgName, (arg)); fflush(stderr);)

#define VERBOUTARG2(arg1,arg2) VERBOSE2(\
        fprintf(stderr, "%s: %s%s\n", ProgName, (arg1), (arg2));\
   fflush(stderr);)

#define VERBOSE10(arg) {if(Verbose>=10){arg}}
#define VERBOUT10(arg) VERBOSE10(\
        fprintf(stderr, "%s: %s", ProgName, (arg)); fflush(stderr);)


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
