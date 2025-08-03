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
sig_atomic_t TimeIsUp          = 0;
sig_atomic_t TimeLimitIsSoft   = 0;
sig_atomic_t SigTermCaught     = 0;
static sig_atomic_t fatal_error_in_progress = 0;
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
   struct rlimit limit;

   getrlimit(RLIMIT_CPU, &limit);
   SystemTimeLimit = limit.rlim_max;

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
         VERBOSE(WriteStr(GlobalOutFD, "SIGXCPU caught.\n"););
         if(TimeLimitIsSoft)
         {
            TimeIsUp = 1;
            TimeLimitIsSoft = false;
            limit.rlim_cur = MIN(HardTimeLimit, SystemTimeLimit);
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
            WriteStr(GlobalOutFD, "\n"COMCHAR" Failure: Resource limit exceeded (time)\n");
            TSTPOUTFD(GlobalOutFD, "ResourceOut");
            Error("CPU time limit exceeded, terminating", CPU_LIMIT_ERROR);
         }
         break;
   case SIGTERM:
   case SIGINT:
         VERBOSE(WriteStr(GlobalOutFD, "SIGTERM/SIGINT caught.\n"););
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
         WriteStr(STDERR_FILENO, "Warning: ");
         WriteStr(STDERR_FILENO, "Unexpected signal caught, continuing");
         break;
   }
}


/*-----------------------------------------------------------------------
//
// Function: ESigTermSchedHandler()
//
//   Record a caught SIGTERM.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void ESigTermSchedHandler(int mysignal)
{
   if(mysignal == SIGTERM)
   {
      SigTermCaught++;
      signal(SIGTERM, SIG_DFL);
   }
}




/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
