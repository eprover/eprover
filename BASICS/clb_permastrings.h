/*-----------------------------------------------------------------------

  File  : clb_permastrings.h

  Author: Stephan Schulz (schulz@eprover.org)

  Contents

  Simple registry for (potentially shared) permanent strings to
  simplify memory mangement.

  Copyright 2023 by the authors.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

  Created: Fri Nov 24 15:01:29 CET 2023

-----------------------------------------------------------------------*/

#ifndef CLB_PERMASTRINGS

#define CLB_PERMASTRINGS

#include <clb_stringtrees.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

char* PermaString(char *str);
char* PermaStringStore(char *str);
void  PermaStringsFree(void);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
