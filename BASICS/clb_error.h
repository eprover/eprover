/*-----------------------------------------------------------------------

  File  : clb_error.h

  Author: Stephan Schulz

  Contents

  Functions and datatypes for handling and reporting errors, warnings,
  and dealing with simple system stuff.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details.
  Run "eprover -h" for contact information.

  Created: Sat Jul  5 02:20:53 MET DST 1997 - New

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

typedef int ErrorCodes;

#define NO_ERROR              0
#define PROOF_FOUND           0
#define SATISFIABLE           1
#define OUT_OF_MEMORY         2
#define SYNTAX_ERROR          3
#define TYPE_ERROR            4
#define USAGE_ERROR           5
#define FILE_ERROR            6
#define SYS_ERROR             7
#define CPU_LIMIT_ERROR       8
#define RESOURCE_OUT          9
#define INCOMPLETE_PROOFSTATE 10
#define OTHER_ERROR           11
#define INPUT_SEMANTIC_ERROR  12
#define INTERFACE_ERROR       13
#define PARENT_REQUEST        14


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define MAX_ERRMSG_ADD   512
#define MAX_ERRMSG_LEN   MAX_ERRMSG_ADD+MAXPATHLEN

#ifdef ENABLE_LFHO
#define LFHO_ASSERT(check) assert(check)
#else
#define LFHO_ASSERT(check) assert(true)
#endif

extern char  ErrStr[];
extern int   TmpErrno;
extern char* ProgName;

void          InitError(char* progname);
void          Error(char* message, ErrorCodes ret, ...);
void          SysError(char* message, ErrorCodes ret, ...);
void          Warning(char* message, ...);
void          SysWarning(char* message, ...);
void          ELog(char* message, ...);
double        GetTotalCPUTime(void);
double        GetTotalCPUTimeIncludingChildren(void);
void          PrintRusage(FILE* out);

bool          TestLetterString(char* to_check, char* options);
void          CheckOptionLetterString(char* to_check, char* options,
                                      char *option);



#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
