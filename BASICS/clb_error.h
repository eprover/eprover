/*-----------------------------------------------------------------------

File  : clb_error.h

Author: Stephan Schulz

Contents
 
  Functions and datatypes for handling and reporting errors. 

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Sat Jul  5 02:20:53 MET DST 1997
    New

-----------------------------------------------------------------------*/

#ifndef CLB_ERROR

#define CLB_ERROR

#include <clb_defines.h>

#ifndef RESTRICTED_FOR_WINDOWS
#include <sys/time.h>
#include <sys/resource.h>
#endif

#ifdef HP_UX
#include <syscall.h>
#define getrusage(a, b)  syscall(SYS_GETRUSAGE, a, b)
#endif


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef enum
{
   NO_ERROR,
   OUT_OF_MEMORY,
   SYNTAX_ERROR,
   USAGE_ERROR,
   FILE_ERROR,
   SYS_ERROR,
   CPU_LIMIT_ERROR,
   OTHER_ERROR,
   INPUT_SEMANTIC_ERROR
}ErrorCodes;

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define MAX_ERRMSG_ADD   512
#define MAX_ERRMSG_LEN   MAX_ERRMSG_ADD+MAXPATHLEN

extern char  ErrStr[];
extern int   TmpErrno;
extern char* ProgName;

void          InitError(char* progname);
void          ReleaseErrorReserve(void);
VOLATILE void Error(char* message, ErrorCodes ret);
VOLATILE void SysError(char* message, ErrorCodes ret);
void          Warning(char* message);
void          PrintRusage(FILE* out);
void          StrideMemory(char* mem, long size);

bool          CheckLetterString(char* to_check, char* options);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





