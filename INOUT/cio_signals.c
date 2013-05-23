/*-----------------------------------------------------------------------

File  : cco_signals.c

Author: Stephan Schulz

Contents
 
  Signal handler for limit signals...not really necessary, but may
  work around some Solaris bugs.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Fri Nov  6 14:50:28 MET 1998
    New

-----------------------------------------------------------------------*/

#include "cio_signals.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

rlim_t                ScheduleTimeLimit = 0;
rlim_t                SystemTimeLimit   = RLIM_INFINITY;
rlim_t                SoftTimeLimit     = RLIM_INFINITY;
rlim_t                HardTimeLimit     = RLIM_INFINITY;
VOLATILE sig_atomic_t TimeIsUp          = 0;
VOLATILE sig_atomic_t TimeLimitIsSoft   = 0;
static VOLATILE sig_atomic_t fatal_error_in_progress = 0;
bool                  SilentTimeOut     = false;

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
// Function: ESignalSetup()
//
//   Set up ESignalHandler() as handle for mysignal, check for
//   errors. 
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void ESignalSetup(int mysignal)
{
   if(signal(mysignal, ESignalHandler) == SIG_ERR)
   {
      TmpErrno = errno;
      SysError("Unable to set up signal handler", SYS_ERROR);
   }
}


/*-----------------------------------------------------------------------
//
// Function: ESignalHandler()
//
//   Handle signals...print message and exit or continue, depending on
//   the signal.
//
// Global Variables: -
//
// Side Effects    : May terminate program or print warning
//
/----------------------------------------------------------------------*/

void ESignalHandler(int mysignal)
{
   struct rlimit     limit;

   switch(mysignal)
   {
   case SIGXCPU:
	 limit.rlim_max = SystemTimeLimit;
	 limit.rlim_cur = SystemTimeLimit;
	 if(setrlimit(RLIMIT_CPU, &limit))
	 {
	    TmpErrno = errno;
	    SysError("Unable to reset cpu time limit", SYS_ERROR);
	 }
         VERBOSE(WRITE_STR(GlobalOutFD, "SIGXCPU caught.\n"));
	 if(TimeLimitIsSoft)
	 {	  	    
	    TimeIsUp = 1;
	    TimeLimitIsSoft = false;
	    limit.rlim_cur = HardTimeLimit;
	    if(setrlimit(RLIMIT_CPU, &limit))
	    {
	       TmpErrno = errno;
	       SysError("Unable to set cpu time limit to hard limit",
			SYS_ERROR);
	    }
	    ESignalSetup(SIGXCPU); /* Reenable signal handler */
	    return;
	 }	 
         if(SilentTimeOut)
         {
            exit(CPU_LIMIT_ERROR);
         }
         else
         {
            WRITE_STR(GlobalOutFD, "\n# Failure: Resource limit exceeded (time)\n");	 
            TSTPOUTFD(GlobalOutFD, "ResourceOut");
            Error("CPU time limit exceeded, terminating", CPU_LIMIT_ERROR);
         }
	 break;
   case SIGTERM:
   case SIGINT:
	 VERBOSE(WRITE_STR(GlobalOutFD, "SIGTERM/SIGINT caught.\n"));
	 if(fatal_error_in_progress)
	 {
	    signal(mysignal, SIG_DFL);
	    raise(mysignal);
	 }
	 fatal_error_in_progress = 1;
	 TempFileCleanup();
	 raise(mysignal); 
	 break;
   default:
         WRITE_STR(STDERR_FILENO, "Warning: ");
	 WRITE_STR(STDERR_FILENO, "Unexpected signal caught, continuing");
	 break;
   }
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


