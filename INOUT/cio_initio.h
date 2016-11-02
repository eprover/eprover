/*-----------------------------------------------------------------------

File  : cio_initio.h

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Rather trivial code for initializing all I/O related stuff once and
  in one go.

  Copyright 2005 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Thu Mar 17 11:20:32 UYT 2005
    New

-----------------------------------------------------------------------*/

#ifndef CIO_INITIO

#define CIO_INITIO

#include <cio_output.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

extern char* TPTP_dir;

void InitIO(char* progname);

void ExitIO(void);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





