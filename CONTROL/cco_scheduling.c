/*-----------------------------------------------------------------------

File  : cco_scheduling.c

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Stuff for scheduling.

  Copyright 2013 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1>     New

-----------------------------------------------------------------------*/

#include "cco_scheduling.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

ScheduleCell StratSchedule[] =
{
   {"AutoSched0", AUTOSCHED0, "Auto", 0.5066, 0},
   {"AutoSched1", AUTOSCHED1, "Auto", 0.2466, 0},
   {"AutoSched2", AUTOSCHED2, "Auto", 0.08  , 0},
   {"AutoSched3", AUTOSCHED3, "Auto", 0.06  , 0},
   {"AutoSched4", AUTOSCHED4, "Auto", 0.0433, 0},
   {"AutoSched5", AUTOSCHED5, "Auto", 0.03  , 0},
   {"AutoSched6", AUTOSCHED6, "Auto", 0.0166, 0},
   {"AutoSched7", AUTOSCHED7, "Auto", 0.0166, 0},
   {NULL        , NoOrdering, NULL  , 0.0   , 0}
};


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
// Function: ScheduleTimesInit()
//
//
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void ScheduleTimesInit(ScheduleCell sched[], double time_used)
{
   int i;
   rlim_t sum = 0, tmp, limit;

   limit = 0;
   if(ScheduleTimeLimit)
   {
      if(ScheduleTimeLimit>time_used)
      {
         limit = ScheduleTimeLimit-time_used;
      }
   }
   else
   {
      if(DEFAULT_SCHED_TIME_LIMIT > time_used)
      {
         limit = DEFAULT_SCHED_TIME_LIMIT-time_used;
      }
   }

   for(i=0; sched[i+1].heu_name; i++)
   {
      tmp = sched[i].time_fraction*limit;
      sched[i].time_absolute = tmp;
      sum += tmp;
   }
   if(ScheduleTimeLimit)
   {
      tmp = limit - sum;
      sched[i].time_absolute = tmp;
   }
   else
   {
      sched[i].time_absolute = RLIM_INFINITY;
   }
}


/*-----------------------------------------------------------------------
//
// Function:  ExecuteSchedule()
//
//   Execute the hard-coded strategy schedule.
//
// Global Variables: SilentTimeOut
//
// Side Effects    : Forks, the child runs the proof search, re-sets
//                   time limits, sets heuristic parameters
//
/----------------------------------------------------------------------*/

pid_t ExecuteSchedule(ScheduleCell strats[],
                      HeuristicParms_p  h_parms,
                      bool print_rusage)
{
   int raw_status, status = OTHER_ERROR, i;
   pid_t pid       = 0, respid;
   double run_time = GetTotalCPUTime();

   ScheduleTimesInit(strats, run_time);

   for(i=0; strats[i].heu_name; i++)
   {
      h_parms->heuristic_name = strats[i].heu_name;
      h_parms->ordertype      = strats[i].ordering;
      fprintf(GlobalOut, "# Trying %s for %ld seconds\n",
              strats[i].heu_name,
              (long)strats[i].time_absolute);
      fflush(GlobalOut);
      pid = fork();
      if(pid == 0)
      {
         /* Child */
         SilentTimeOut = true;
         if(strats[i].time_absolute!=RLIM_INFINITY)
         {
            SetSoftRlimit(RLIMIT_CPU, strats[i].time_absolute);
         }
         return pid;
      }
      else
      {
         /* Parent */
         respid = -1;
         while(respid == -1)
         {
            respid = waitpid(pid, &raw_status, 0);
         }
         if(WIFEXITED(raw_status))
         {
            status = WEXITSTATUS(raw_status);
            if((status == SATISFIABLE) || (status == PROOF_FOUND))
            {
               if(print_rusage)
               {
                  PrintRusage(GlobalOut);
               }
               exit(status);
            }
            else
            {
               fprintf(GlobalOut, "# No success with %s\n",
                       strats[i].heu_name);
            }
         }
         else
         {
            fprintf(GlobalOut, "# Abnormal termination for %s\n",
                    strats[i].heu_name);
         }
      }
   }
   if(print_rusage)
   {
      PrintRusage(GlobalOut);
   }
   /* The following is ugly: Because the individual strategies can
      fail, but the whole schedule can succeed, we cannot let the
      strategies report failure to dtandard out (that might confuse
      badly-written meta-tools (and there are such ;-)). Hence, the
      TSPT status in the failure case is suppressed and needs to be
      added here. This is ony partially possible - we take the exit
      status of the last strategy of the schedule. */
   switch(status)
   {
   case PROOF_FOUND:
   case SATISFIABLE:
         /* Nothing to do, success reported by the child */
         break;
   case OUT_OF_MEMORY:
    TSTPOUT(stdout, "ResourceOut");
         break;
   case SYNTAX_ERROR:
         /* Should never be possible here */
         TSTPOUT(stdout, "SyntaxError");
         break;
   case USAGE_ERROR:
         /* Should never be possible here */
         TSTPOUT(stdout, "UsageError");
         break;
   case FILE_ERROR:
         /* Should never be possible here */
         TSTPOUT(stdout, "OSError");
         break;
   case SYS_ERROR:
         TSTPOUT(stdout, "OSError");
         break;
   case CPU_LIMIT_ERROR:
         WriteStr(GlobalOutFD, "\n# Failure: Resource limit exceeded (time)\n");
         TSTPOUTFD(GlobalOutFD, "ResourceOut");
         Error("CPU time limit exceeded, terminating", CPU_LIMIT_ERROR);
         break;
   case RESOURCE_OUT:
    TSTPOUT(stdout, "ResourceOut");
         break;
   case INCOMPLETE_PROOFSTATE:
         TSTPOUT(GlobalOut, "GaveUp");
         break;
   case OTHER_ERROR:
         TSTPOUT(stdout, "Error");
         break;
   case INPUT_SEMANTIC_ERROR:
         TSTPOUT(stdout, "SemanticError");
         break;
   default:
         break;
   }
   exit(status);
   return pid;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


