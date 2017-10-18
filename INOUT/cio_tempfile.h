/*-----------------------------------------------------------------------

File  : cio_tempfile.h

Author: Stephan Schulz

Contents

  Functions dealing with temporary files.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sat Jul 24 02:25:20 MET DST 1999
    New

-----------------------------------------------------------------------*/

#ifndef CIO_TEMPFILE

#define CIO_TEMPFILE

#include <stdlib.h>
#include <clb_memory.h>
#include <cio_fileops.h>
#include <cio_commandline.h>
#include <clb_stringtrees.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

void     TempFileCleanup(void);
void     TempFileRegister(char *name);
char*    TempFileName(void);
char*    TempFileCreate(FILE* source);
void     TempFileRemove(char* name);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





