/*-----------------------------------------------------------------------

File  : clb_error.c

Author: Stephan Schulz

Contents
 
  Routines for handling errors and warnings.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Sun Jul  6 23:33:59 MET DST 1997
    New

-----------------------------------------------------------------------*/

#include "clb_error.h"


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

/* Space for building error messages */

char ErrStr[MAX_ERRMSG_LEN];

/* Saving errno from the system call originating it */

int TmpErrno;

/* The name of the program */

char* ProgName = "Unknown program";

/* Memory we can use for error processing */
static char* reserve_memory = NULL;
#define ERROR_MEM_RESERVE (128*KILO)

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
   reserve_memory = malloc(ERROR_MEM_RESERVE);
   if(!reserve_memory)
   {
      Error("Cannot allocate memory for error handling!",
	    OUT_OF_MEMORY);
   }
#ifdef MEMORY_RESERVE_PARANOID
   StrideMemory(reserve_memory, ERROR_MEM_RESERVE);
#endif
   /* If we try to print to stderr for the first time in an
    * out-of-memory situation, fprintf() occaqsionally fails because
    * the necessary buffers cannot be allocated. This is an attempt to
    * fix this behaviour. Note that I know of no standard that
    * guarantees this to work (in fact, I don't know how to reliably
    * print in out-of-memory cases. */

   fprintf(stderr, EmptyString);
   fprintf(stdout, EmptyString);
}


/*-----------------------------------------------------------------------
//
// Function: ReleaseErrorReserve()
//
//   Release the memory reserved for error handling, so that functions
//   higher up in the hierarchy can use it.
//
// Global Variables: reserve_memory
//
// Side Effects    : Memory release
//
/----------------------------------------------------------------------*/

void ReleaseErrorReserve(void)
{
   if(reserve_memory)
   {
      free(reserve_memory);
      reserve_memory=NULL;
   }
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

VOLATILE void Error(char* message, ErrorCodes ret)
{
   ReleaseErrorReserve();
   fprintf(stderr, "%s: %s\n", ProgName, message);
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

VOLATILE void SysError(char* message, ErrorCodes ret)
{
   ReleaseErrorReserve();
   fprintf(stderr, "%s: %s\n", ProgName, message);
   errno = TmpErrno;
   perror(ProgName);
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

void Warning(char* message)
{
   fprintf(stderr, "%s: Warning: %s\n", ProgName, message);
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
   struct rusage usage;
   
   if(getrusage(RUSAGE_SELF, &usage))
   {
      TmpErrno = errno;
      SysError("Unable to get resource usage information",
	       SYS_ERROR);
   }
   fprintf(out, 
	   "\n# -------------------------------------------------\n");
   fprintf(out, 
	   "# User time                : %.3f s\n",
	   (usage.ru_utime.tv_sec)+(usage.ru_utime.tv_usec)/1000000.0);
   fprintf(out, 
	   "# System time              : %.3f s\n",
	   (usage.ru_stime.tv_sec)+(usage.ru_stime.tv_usec)/1000000.0);
   fprintf(out, 
	   "# Total time               : %.3f s\n",
	   (usage.ru_utime.tv_sec+usage.ru_stime.tv_sec)+
	   ((usage.ru_utime.tv_usec+usage.ru_stime.tv_usec)/1000000.0));
   fprintf(out, 
	   "# Maximum resident set size: %ld pages\n",
	   usage.ru_maxrss);
}


/*-----------------------------------------------------------------------
//
// Function: StrideMemory()
//
//   Write an arbitrary value into memory each E_PAGE_SIZE
//   bytes. It's used for preallocated memory reserves. Normally,
//   allocated pages need not really be available unless written to if
//   overallocation is being used. This should ensure that allocated
//   pages are backed by real memory in such (broken!) cases.  
//
// Global Variables: 
//
// Side Effects    : 
//
/----------------------------------------------------------------------*/

void StrideMemory(char* mem, long size)
{
   char* stride;
   
   for(stride = mem; stride < mem+size; stride+=E_PAGE_SIZE) 
   {
      *stride = 'S'; /* Arbitrary value*/
   }
}

/*-----------------------------------------------------------------------
//
// Function: CheckLetterString()
//
//   Return true if all letters in to_check also appear in options,
//   false otherwise.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
  
bool CheckLetterString(char* to_check, char* options)
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



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


