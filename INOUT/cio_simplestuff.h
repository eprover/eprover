/*-----------------------------------------------------------------------

File  : cio_simplestuff.h

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Simple functions for simple operations that don't quite fit
  elsewhere.

  Copyright 2012 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Fri Jul 27 01:33:21 CEST 2012
    New

-----------------------------------------------------------------------*/

#ifndef CIO_SIMPLESTUFF

#define CIO_SIMPLESTUFF

#include <cio_output.h>
#include <cio_network.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

bool ReadTextBlock(DStr_p result, FILE* fp, char* terminator);
bool TCPReadTextBlock(DStr_p result, int fd, char* terminator);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





