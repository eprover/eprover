/*-----------------------------------------------------------------------

File  : cio_output.h

Author: Stephan Schulz

Contents

  Simple functions for secure opening of output files with -
  convention and error checking. Much simpler than the input, because
  much less can go wrong with output...

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Fri Nov 28 11:55:59 MET 1997
    New

-----------------------------------------------------------------------*/

#ifndef CIO_OUTPUT

#define CIO_OUTPUT

#include <stdio.h>
#include <clb_dstrings.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


#define OUTPRINT(level, message)\
    if(level<= OutputLevel){fprintf(GlobalOut, message);}


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

extern long  OutputLevel;
extern FILE* GlobalOut;
extern int   GlobalOutFD;

#define      InitOutput() GlobalOut=stdout;GlobalOutFD=STDOUT_FILENO
void         OpenGlobalOut(char* outname);
FILE*        OutOpen(char* name);
void         OutClose(FILE* file);
void         PrintDashedStatuses(FILE* out, char *stat1, char *stat2, char *fallback);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





