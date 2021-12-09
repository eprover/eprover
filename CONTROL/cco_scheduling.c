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
// Function:  activate_child()
//
//   Fork a process correspoding to schedule cell with idx i and
//   set its parameters accordingly. Returns parent id either of child
//   or parent.
//
// Global Variables: SilentTimeOut
//
// Side Effects    : Forks, the child runs the proof search, re-sets
//                   time limits, sets heuristic parameters
//
/----------------------------------------------------------------------*/

pid_t activate_child(int child_idx, ScheduleCell strats[],
                     HeuristicParms_p h_parms)
{
   pid_t pid = fork();
   if(pid == 0)
   {
      //child
      SilentTimeOut = true;
      h_parms->heuristic_name         = strats[child_idx].heu_name;
      h_parms->order_params.ordertype = strats[child_idx].ordering;
      if(strats[child_idx].time_absolute!=RLIM_INFINITY)
      {
         locked_fprintf(stderr, "scheduling %s for %ld seconds.\n",
                        h_parms->heuristic_name,
                        (long)strats[child_idx].time_absolute);
         if(SetSoftRlimit(RLIMIT_CPU, strats[child_idx].time_absolute) 
               != RLimSuccess)
         {
            locked_fprintf(stderr, "softrlimit call failed.\n");
            exit(-1);
         }
      }
   }
   return pid;
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
                      bool print_rusage, int num_cpus, 
                      bool* proof_found_flag, sem_t* proof_found_sem)
{
   int raw_status, status = OTHER_ERROR;
   double run_time = GetTotalCPUTime();
   
   if(num_cpus <= 0)
   {
      // using all available CPUs
      num_cpus = sysconf(_SC_NPROCESSORS_ONLN);
      locked_fprintf(stderr, "# Will run on %d CPU cores.\n", num_cpus);
   }

   ScheduleTimesInit(strats, run_time, num_cpus);
   int sched_size = 0;
   for(int i=0; strats[i].heu_name; i++)
   {
      sched_size++;
   }

   int activated = 0;
   int finished = 0;
   pid_t child_pids[sched_size];
   pid_t pid = 1;

   for(; pid && activated < MIN(num_cpus,sched_size); activated++)
   {
      pid = activate_child(activated, strats, h_parms);
      if(pid != 0)
      {
         child_pids[activated] = pid;
      }
   }

   bool done = pid == 0;
   // if we are parent then we wait for any child to finish
   // and if it did not prove the problem schedule a new child
   while(!done)
   {
      pid_t child_id = -1;
      while(child_id == -1)
      {
         child_id = wait(&raw_status);
      }
      finished++;

      if(print_rusage)
      {
         PrintRusage(GlobalOut);
      }

      int sched_idx = -1;
      for(int i=0; sched_idx==-1 && i<activated; i++)
      {
         if(child_pids[i] == child_id)
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
            done = true;
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

      if(finished == sched_size)
      {
         done = true;
      }
      else if(!done && activated < sched_size)
      {
         // activating another schild
         activated++;
         pid = activate_child(activated, strats, h_parms);
         done = pid == 0;  // child is automatically done
      }
   }

   return pid;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
