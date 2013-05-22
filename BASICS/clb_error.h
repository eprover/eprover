/*-----------------------------------------------------------------------

File  : clb_error.h

Author: Stephan Schulz

Contents
 
  Functions and datatypes for handling and reporting errors, warnings,
  and dealing with simple system stuff.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sat Jul  5 02:20:53 MET DST 1997
    New
<2> Wed Nov  3 13:30:39 CET 2004
    Added real memory code.

-----------------------------------------------------------------------*/

#ifndef CLB_ERROR

#define CLB_ERROR

#include <clb_defines.h>
#include <string.h>
#include <stdarg.h>
#include <sys/time.h>
#include <sys/resource.h>

#ifdef HP_UX
#include <syscall.h>
#define getrusage(a, b)  syscall(SYS_GETRUSAGE, a, b)
#endif



/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef enum
{
   NO_ERROR = EXIT_SUCCESS,
   PROOF_FOUND = EXIT_SUCCESS,
   SATISFIABLE,
   OUT_OF_MEMORY,
   SYNTAX_ERROR,
   USAGE_ERROR,
   FILE_ERROR,
   SYS_ERROR,
   CPU_LIMIT_ERROR,
   RESOURCE_OUT,
   INCOMPLETE_PROOFSTATE,
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

long          GetSystemPageSize(void);
long          GetSystemPhysMemory(void);

void          InitError(char* progname);
VOLATILE void Error(char* message, ErrorCodes ret, ...);
VOLATILE void SysError(char* message, ErrorCodes ret, ...);
void          Warning(char* message, ...);
void          SysWarning(char* message, ...);
double        GetTotalCPUTime(void);
void          PrintRusage(FILE* out);
void          StrideMemory(char* mem, long size);

bool          CheckLetterString(char* to_check, char* options);



#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





