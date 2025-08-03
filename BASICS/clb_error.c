/*-----------------------------------------------------------------------

File  : clb_error.c

Author: Stephan Schulz

Contents

  Routines for handling errors, warnings, and system stuff.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sun Jul  6 23:33:59 MET DST 1997
    New

-----------------------------------------------------------------------*/

#include <sys/types.h>
#include <sys/uio.h>
#include "clb_error.h"
#include "clb_simple_stuff.h"


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

/* Space for building error messages */

char ErrStr[MAX_ERRMSG_LEN];

/* Saving errno from the system call originating it */

int TmpErrno;

/* The name of the program */

char* ProgName = "Unknown program";


/* The empty string as a global, external variable that cannot easily
 * be optimize away. See InitError() for more explanation. */

char* EmptyString = "";


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: InitError()
//
//   Initialize the error handling module. Currently only stores the
//   name under which the program has been called. Copies only the
//   pointer, not the pointed-to value!
//
// Global Variables: ProgName
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void InitError(char* progname)
{
   ProgName = progname;
}


/*-----------------------------------------------------------------------
//
// Function: Error()
//
//   Print an error message to stderr and exit the program with the
//   given return code.
//
// Global Variables: ProgName
//
// Side Effects    : Terminates the program.
//
/----------------------------------------------------------------------*/

__attribute__((noreturn))
void Error(char* message, ErrorCodes ret, ...)
{
   va_list ap;
   va_start(ap, ret);

   fprintf(stderr, "%s: ", ProgName);
   vfprintf(stderr, message, ap);
   fprintf(stderr, "\n");
   va_end(ap);

   exit(ret);
}


/*-----------------------------------------------------------------------
//
// Function: SysError()
//
//   Print a user error message and a system error message to stderr
//   and exit the program with an appropriate return code. The value
//   of errno is restored from TmpErrno.
//
// Global Variables: ProgName, TmpErrno
//
// Side Effects    : Terminates the program.
//
/----------------------------------------------------------------------*/

__attribute__((noreturn))
void SysError(char* message, ErrorCodes ret, ...)
{
   va_list ap;
   va_start(ap, ret);

   fprintf(stderr, "%s: ", ProgName);
   vfprintf(stderr, message, ap);
   fprintf(stderr, "\n");
   errno = TmpErrno;
   perror(ProgName);
   va_end(ap);

   exit(ret);
}


/*-----------------------------------------------------------------------
//
// Function: Warning()
//
//   Print a warning to stderr
//
// Global Variables: ProgName
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void Warning(char* message, ...)
{
   va_list ap;
   va_start(ap, message);

   fprintf(stderr, "%s: Warning: ", ProgName);
   vfprintf(stderr, message, ap);
   fprintf(stderr, "\n");

   va_end(ap);
}


/*-----------------------------------------------------------------------
//
// Function: SysWarning()
//
//   Print a user error message and a system error message to stderr
//   and exit the program with an appropriate return code. The value
//   of errno is restored from TmpErrno.
//
// Global Variables: ProgName, TmpErrno
//
// Side Effects    : Terminates the program.
//
/----------------------------------------------------------------------*/

void SysWarning(char* message, ...)
{
   va_list ap;
   va_start(ap, message);

   fprintf(stderr, "%s: Warning: ", ProgName);
   vfprintf(stderr, message, ap);
   fprintf(stderr, "\n");
   errno = TmpErrno;
   perror(ProgName);

   va_end(ap);
}

/*-----------------------------------------------------------------------
//
// Function: ELog()
//
//   Write a message to a logfile.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void ELog(char* message, ...)
{
   va_list ap;
   va_start(ap, message);

   FILE *logfile;
   sprintf(ErrStr, "elog%d.log", getpid());
   logfile = fopen(ErrStr, "a");

   fprintf(logfile, "%d: %4.9f: ", getpid(), GetTotalCPUTime());
   vfprintf(logfile, message, ap);
   fprintf(stderr, "\n");
   va_end(ap);
   fclose(logfile);
}




/*-----------------------------------------------------------------------
//
// Function: GetTotalCPUTime()
//
//   Return the total CPU time use by the process s far, in floating
//   point seconds - or -1.0 if this cannot be determined.
//
// Global Variables: -
//
// Side Effects    : May terminate with error.
//
/----------------------------------------------------------------------*/

double GetTotalCPUTime(void)
{
   double res = -1;

   struct rusage usage;

   if(!getrusage(RUSAGE_SELF, &usage))
   {
      res = (usage.ru_utime.tv_sec+usage.ru_stime.tv_sec)+
         ((usage.ru_utime.tv_usec+usage.ru_stime.tv_usec)/1000000.0);
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: GetTotalCPUTimeIncludingChildren()
//
//   Return the total CPU time use by the process s far, in floating
//   point seconds - or -1.0 if this cannot be determined. Compared to
//   GetTotalCPUTime
//
//
// Global Variables: -
//
// Side Effects    : May terminate with error.
//
/----------------------------------------------------------------------*/

double GetTotalCPUTimeIncludingChildren(void)
{
   double res = GetTotalCPUTime();

   struct rusage usage;

   if((int)res != -1 && !getrusage(RUSAGE_CHILDREN, &usage))
   {

      res += (usage.ru_utime.tv_sec+usage.ru_stime.tv_sec)+
              ((usage.ru_utime.tv_usec+usage.ru_stime.tv_usec)/1000000.0);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: PrintRusage()
//
//   Print resource usage to given stream.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void PrintRusage(FILE* out)
{
   struct rusage usage, cusage;

   if(getrusage(RUSAGE_SELF, &usage))
   {
      TmpErrno = errno;
      SysError("Unable to get resource usage information",
          SYS_ERROR);
   }
   if(getrusage(RUSAGE_CHILDREN, &cusage))
   {
      TmpErrno = errno;
      SysError("Unable to get resource usage information",
          SYS_ERROR);
   }
   usage.ru_utime.tv_sec  += cusage.ru_utime.tv_sec;
   usage.ru_utime.tv_usec += cusage.ru_utime.tv_usec;
   usage.ru_stime.tv_sec  += cusage.ru_stime.tv_sec;
   usage.ru_stime.tv_usec += cusage.ru_stime.tv_usec;

   fprintf(out,
      "\n"COMCHAR" -------------------------------------------------\n");
   fprintf(out,
      COMCHAR" User time                : %.3f s\n",
      (usage.ru_utime.tv_sec)+(usage.ru_utime.tv_usec)/1000000.0);
   fprintf(out,
      COMCHAR" System time              : %.3f s\n",
      (usage.ru_stime.tv_sec)+(usage.ru_stime.tv_usec)/1000000.0);
   fprintf(out,
      COMCHAR" Total time               : %.3f s\n",
      (usage.ru_utime.tv_sec+usage.ru_stime.tv_sec)+
      ((usage.ru_utime.tv_usec+usage.ru_stime.tv_usec)/1000000.0));
   fprintf(out,
      COMCHAR" Maximum resident set size: %ld pages\n",
      usage.ru_maxrss);
}


/*-----------------------------------------------------------------------
//
// Function: TestLetterString()
//
//   Return true if all letters in to_check also appear in options,
//   false otherwise.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool TestLetterString(char* to_check, char* options)
{
   char *current, *control;
   bool found;

   for(current = to_check; *current; current++)
   {
      found = false;
      for(control = options; *control; control++)
      {
         if(*current == *control)
         {
            found = true;
            break;
         }
      }
      if(!found)
      {
         return false;
      }
   }
   return true;
}


/*-----------------------------------------------------------------------
//
// Function: CheckOptionLetterString()
//
//   Check if all the letters in to_check appear in options. If not,
//   terminate with an error message.
//
// Global Variables: -
//
// Side Effects    : May terminates program
//
/----------------------------------------------------------------------*/

void CheckOptionLetterString(char* to_check, char* options, char *option)
{
   if(!TestLetterString(to_check, options))
   {
      char msg[1024];
      sprintf(msg, "Illegal argument to option %s", option);
      Error(msg, USAGE_ERROR);
   }
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
