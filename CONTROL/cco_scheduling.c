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
#include <unistd.h>
#include <sys/mman.h>



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

ScheduleCell _CASC_SCHEDULE[] =
{
   {"AutoSched0",  AUTOSCHED0,  "Auto", 0.5   , 0},
   {"AutoSched1",  AUTOSCHED1,  "Auto", 0.22  , 0},
   {"AutoSched2",  AUTOSCHED2,  "Auto", 0.0733, 0},
   {"AutoSched3",  AUTOSCHED3,  "Auto", 0.06  , 0},
   {"AutoSched4",  AUTOSCHED4,  "Auto", 0.0433, 0},
   {"AutoSched5",  AUTOSCHED5,  "Auto", 0.03  , 0},
   {"AutoSched6",  AUTOSCHED6,  "Auto", 0.0166, 0},
   {"AutoSched7",  AUTOSCHED7,  "Auto", 0.0167, 0},
   {"AutoSched8",  AUTOSCHED8,  "Auto", 0.0167, 0},
   {"AutoSched9",  AUTOSCHED9,  "Auto", 0.0167, 0},
   {NULL,          NoOrdering,  NULL  , 0.0   , 0}
};

ScheduleCell _CASC_SH_SCHEDULE[] =
{
   {"AutoSched0",  AUTOSCHED0,  "Auto", 0.34   , 0},
   {"AutoSched1",  AUTOSCHED1,  "Auto", 0.09  , 0},
   {"AutoSched2",  AUTOSCHED2,  "Auto", 0.09, 0},
   {"AutoSched3",  AUTOSCHED3,  "Auto", 0.09  , 0},
   {"AutoSched4",  AUTOSCHED4,  "Auto", 0.09, 0},
   {"AutoSched5",  AUTOSCHED5,  "Auto", 0.08  , 0},
   {"AutoSched6",  AUTOSCHED6,  "Auto", 0.08, 0},
   {"AutoSched7",  AUTOSCHED7,  "Auto", 0.08, 0},
   {"AutoSched8",  AUTOSCHED8,  "Auto", 0.08, 0},
   {"AutoSched9",  AUTOSCHED9,  "Auto", 0.08, 0},
   {NULL,          NoOrdering,  NULL  , 0.0   , 0}
};

const ScheduleCell* CASC_SCHEDULE = _CASC_SCHEDULE;
const ScheduleCell* CASC_SH_SCHEDULE = _CASC_SH_SCHEDULE;
ScheduleCell* chosen_schedule = _CASC_SCHEDULE;
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

void ScheduleTimesInit(ScheduleCell sched[], double time_used, int num_cpus)
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

   // limit is a wallclock limit
   limit = limit * num_cpus;

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

void _resume(int _)
{
   // dummy function to make sure signal is not ignored
   return;   
}

pid_t ExecuteSchedule(ScheduleCell strats[],
                      HeuristicParms_p  h_parms,
                      bool print_rusage, int num_cpus, 
                      bool* proof_found_flag, sem_t* proof_found_sem)
{
   int raw_status, status = OTHER_ERROR, i;
   pid_t pid       = 0, respid;
   double run_time = GetTotalCPUTime();
   
   if(num_cpus <= 0)
   {
      // using all available CPUs
      num_cpus = sysconf(_SC_NPROCESSORS_ONLN);
      locked_fprintf(stderr, "# Will run on %d CPU cores.\n", num_cpus);
   }

   ScheduleTimesInit(strats, run_time, num_cpus);
   PStack_p child_pids = PStackAlloc();

   for(i=0; strats[i].heu_name; i++)
   {
      h_parms->heuristic_name         = strats[i].heu_name;
      h_parms->order_params.ordertype = strats[i].ordering;
      locked_fprintf(GlobalOut, "# Will try %s for %ld seconds\n",
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
            if(SetSoftRlimit(RLIMIT_CPU, strats[i].time_absolute) != RLimSuccess)
            {
               locked_fprintf(stderr, "softrlimit call failed.\n");
               exit(-1);
            }
         }
         signal(SIGUSR1, _resume);
         pause();
         break;
      }
      else
      {
         /* Parent */
         PStackPushInt(child_pids, pid);
      }
   }
   if(print_rusage)
   {
      PrintRusage(GlobalOut);
   }
   if(pid==0)
   {
      return pid; // child continues on
   }
   else
   {
      PStackPointer activated = 0;
      PStackPointer finished = 0;
      PStackPointer sched_size = PStackGetSP(child_pids);
      for(; activated < num_cpus && activated < sched_size; activated++)
      {
         pid_t child_pid = PStackElementInt(child_pids, activated);
         kill(child_pid, SIGUSR1); // activating the child.
      }

      while(finished < sched_size)
      {
         respid = -1;
         while(respid == -1)
         {
            respid = wait(&raw_status);
         }
         finished++;

         int sched_idx = -1;
         for(PStackPointer i=0; sched_idx==-1 &&i<PStackGetSP(child_pids); i++)
         {
            if(PStackElementInt(child_pids, i) == respid)
            {
               sched_idx = i;
            }
         }
         assert(sched_idx!=-1);

         if(WIFEXITED(raw_status))
         {
            status = WEXITSTATUS(raw_status);
            if((status == SATISFIABLE) || (status == PROOF_FOUND))
            {
               if(print_rusage)
               {
                  PrintRusage(GlobalOut);
               }
               locked_fprintf(GlobalOut, "# Proof found by %s\n", strats[sched_idx].heu_name);
               kill(0, SIGKILL); // killing all the children
               break;
            }
            else
            {
               locked_fprintf(GlobalOut, "# No success with %s\n",
                        strats[sched_idx].heu_name);
            }
         }
         else
         {
            locked_fprintf(GlobalOut, "# Abnormal termination for %s\n",
                     strats[sched_idx].heu_name);
         }

         if(activated < sched_size)
         {
            // activating another schild
            kill(PStackElementInt(child_pids, activated++), SIGUSR1); 
         }
      }
      // all children died
      // munmap(proof_found_flag, sizeof(bool));
      // sem_destroy(proof_found_sem);
      // sem_destroy(print_mutex);
      // print_mutex = NULL;
   }
   return pid;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
