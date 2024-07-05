/*-----------------------------------------------------------------------

  File  : cio_filevars.h

  Author: Stephan Schulz

  Contents

  Functions for managing file-stored "variable = value;" pairs.

  Copyright 1998, 1999, 2024 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: Thu Apr  8 16:00:49 MET DST 1999

-----------------------------------------------------------------------*/

#ifndef CIO_FILEVARS

#define CIO_FILEVARS

#include <clb_stringtrees.h>
#include <clb_pstacks.h>
#include <cio_basicparser.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct filevarscell
{
   PStack_p  names; /* Of sources, for error messages */
   StrTree_p vars;  /* Storing (ident, value) pairs */
}FileVarsCell, *FileVars_p;



/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


#define FileVarsCellAlloc() (FileVarsCell*)SizeMalloc(sizeof(FileVarsCell))
#define FileVarsCellFree(junk)        SizeFree(junk, sizeof(FileVarsCell))

FileVars_p FileVarsAlloc(void);
void       FileVarsFree(FileVars_p handle);
long       FileVarsParse(Scanner_p in, FileVars_p vars);
long       FileVarsReadFromFile(char* file, FileVars_p vars);
bool       FileVarsGetBool(FileVars_p vars, char* name, bool *value);
bool       FileVarsGetInt(FileVars_p vars, char* name,  long *value);
bool       FileVarsGetStr(FileVars_p vars, char* name,  char **value);
bool       FileVarsGetIdentifier(FileVars_p vars, char* name,  char
             **value);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
