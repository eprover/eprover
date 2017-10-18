/*-----------------------------------------------------------------------

File  : cio_commandline.h

Author: Stephan Schulz

Contents

  Definitions for handling options and recognising non-option
  arguments.

  "Why don't you use getopt()?"

  - Implementations of getopt() seem to differ significantly between
    UNIX implementations. Finding out what the differences are and
    coding around them seems to be more work than writing this version
    from scratch.
  - This implementation comes with more support for the handling of
    numerical arguments for options.
  - Finally, this implementation allows (well, forces) the programmer
    to document an option _immediately_, and automates the process of
    presenting this information to the user.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sun Sep  7 00:38:12 MET DST 1997
    New

-----------------------------------------------------------------------*/

#ifndef CIO_COMMANDLINE

#define CIO_COMMANDLINE

#include <clb_dstrings.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* Supported option types: */

typedef enum
{
   NoArg,
   OptArg,
   ReqArg
}OptArgType;

typedef struct optcell
{
   int         option_code;
   char        shortopt;    /* Single Character options */
   char*       longopt;     /* Double dash, GNU-Style */
   OptArgType  type;        /* What about Arguments? */
   char*       arg_default; /* Default for optional argument (long
                style only */
   char*       desc;        /* Put the documentation in immediately! */
}OptCell, *Opt_p;


typedef struct clstatecell
{
   int     sc_opt_c;  /* Which character of the current element of
          argv has to be read next? */
   int     argi;      /* Which element of argv[] ? */
   int     argsize;   /* How large is the argv array really? */
   int     argc;      /* How many elements in argv[]? */
   char**  argv;      /* Vector of arguments. Processed options and
          option args will be removed */
}CLStateCell, *CLState_p;

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define CLStateCellAlloc() (CLStateCell*)SizeMalloc(sizeof(CLStateCell))
#define CLStateCellFree(junk)        SizeFree(junk, sizeof(CLStateCell))


#define FORMAT_WIDTH 78

CLState_p CLStateAlloc(int argc, char* argv[]);
void      CLStateFree(CLState_p junk);

Opt_p  CLStateGetOpt(CLState_p state, char** arg, OptCell options[]);
int    CLStateInsertArg(CLState_p state, char* arg);

double CLStateGetFloatArg(Opt_p option, char* arg);
long   CLStateGetIntArg(Opt_p option, char* arg);
long   CLStateGetIntArgCheckRange(Opt_p option, char* arg, long lower, long upper);
bool   CLStateGetBoolArg(Opt_p option, char* arg);

void PrintOption(FILE* out, Opt_p option);
void PrintOptions(FILE* out, OptCell option[], char* header);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
