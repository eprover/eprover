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
   {"AutoSched4", AUTOSCHED4, "Auto", 0.0625 , 0},
   {"AutoSched3", AUTOSCHED3, "Auto", 0.0625 , 0},
   {"AutoSched2", AUTOSCHED2, "Auto", 0.125,   0},
   {"AutoSched1", AUTOSCHED1, "Auto", 0.25 ,   0},
   {"AutoSched0", AUTOSCHED0, "Auto", 0.5  ,   0},
   {NULL        , NoOrdering, NULL  , 0.0  ,   0}
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
   
   if(ScheduleTimeLimit)
   {
      limit = ScheduleTimeLimit-time_used;
   }
   else
   {
      limit = DEFAULT_SCHED_TIME_LIMIT-time_used;
   }
   if(limit<0)
   {
      limit = 0;
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
   int raw_status, status, i;
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
         if(strats[i].time_absolute!=RLIM_INFINITY)
         {
            SetSoftRlimit(RLIMIT_CPU, strats[i].time_absolute);
         }
         SilentTimeOut = true;
         break;
      }
      else
      {
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
   if(pid)
   {      
      if(strats[i].time_absolute!=RLIM_INFINITY)
      {
         SetSoftRlimit(RLIMIT_CPU, strats[i].time_absolute);
      }      
   }
   return pid;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


