/*-----------------------------------------------------------------------

File  : cco_signals.h

Author: Stephan Schulz

Contents

  Signal handler for limit signals...not really necessary, but may
  work around some Solaris bugs. Also some support infrastructure...

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Fri Nov  6 14:50:28 MET 1998
    New

-----------------------------------------------------------------------*/

#ifndef CCO_SIGNALS

#define CCO_SIGNALS

#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <cio_tempfile.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

extern rlim_t                ScheduleTimeLimit; /* Used for
                                                 * determining how
                                                 * much time the
                                                 * scheduler can use
                                                 * in total */
extern rlim_t                SystemTimeLimit; /* Returned by the
                   initial getrlimit
                   call as general hard
                   time limit */
extern rlim_t                SoftTimeLimit;   /* Try to exit
                   gracefully */
extern rlim_t                HardTimeLimit;   /* Exit now! */
extern sig_atomic_t TimeIsUp;
extern sig_atomic_t TimeLimitIsSoft; /* Have we hit hard or
                   soft? */
extern sig_atomic_t SigTermCaught;
extern bool         SilentTimeOut;

void ESignalSetup(int mysignal);
void ESignalHandler(int mysignal);

void ESigTermSchedHandler(int mysignal);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
