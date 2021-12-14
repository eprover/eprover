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
   {"AutoSched0",  AUTOSCHED0,  "Auto", 0.5   , 0, 1},
   {"AutoSched1",  AUTOSCHED1,  "Auto", 0.22  , 0, 1},
   {"AutoSched2",  AUTOSCHED2,  "Auto", 0.0733, 0, 1},
   {"AutoSched3",  AUTOSCHED3,  "Auto", 0.06  , 0, 1},
   {"AutoSched4",  AUTOSCHED4,  "Auto", 0.0433, 0, 1},
   {"AutoSched5",  AUTOSCHED5,  "Auto", 0.03  , 0, 1},
   {"AutoSched6",  AUTOSCHED6,  "Auto", 0.0166, 0, 1},
   {"AutoSched7",  AUTOSCHED7,  "Auto", 0.0167, 0, 1},
   {"AutoSched8",  AUTOSCHED8,  "Auto", 0.0167, 0, 1},
   {"AutoSched9",  AUTOSCHED9,  "Auto", 0.0167, 0, 1},
   {NULL,          NoOrdering,  NULL  , 0.0   , 0, 1}
};

ScheduleCell _CASC_SH_SCHEDULE[] =
{
   {"AutoSched0",  AUTOSCHED0,  "Auto", 0.34, 0, 1},
   {"AutoSched1",  AUTOSCHED1,  "Auto", 0.09, 0, 1},
   {"AutoSched2",  AUTOSCHED2,  "Auto", 0.09, 0, 1},
   {"AutoSched3",  AUTOSCHED3,  "Auto", 0.09, 0, 1},
   {"AutoSched4",  AUTOSCHED4,  "Auto", 0.09, 0, 1},
   {"AutoSched5",  AUTOSCHED5,  "Auto", 0.08, 0, 1},
   {"AutoSched6",  AUTOSCHED6,  "Auto", 0.08, 0, 1},
   {"AutoSched7",  AUTOSCHED7,  "Auto", 0.08, 0, 1},
   {"AutoSched8",  AUTOSCHED8,  "Auto", 0.08, 0, 1},
   {"AutoSched9",  AUTOSCHED9,  "Auto", 0.08, 0, 1},
   {NULL,          NoOrdering,  NULL  , 0.0 , 0, 1}
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
         locked_fprintf(stderr, "# Scheduling %s for %ld seconds.\n",
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
   int raw_status, status = OTHER_ERROR, i;
   pid_t pid       = 0, respid;
   double run_time = GetTotalCPUTime();

   ScheduleTimesInitMultiCore(strats, run_time, 4);

   ScheduleTimesInit(strats, run_time);

   for(i=0; strats[i].heu_name; i++)
   {
      h_parms->heuristic_name         = strats[i].heu_name;
      h_parms->order_params.ordertype = strats[i].ordering;
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
      strategies report failure to standard out (that might confuse
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



/*-----------------------------------------------------------------------
//
// Function: ScheduleTimesInitMultiCore()
//
//    Create timings for multi-core-scheduling. This is very naive,
//    and probably only useful for testing.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void ScheduleTimesInitMultiCore(ScheduleCell sched[], double time_used, int cores)
{
   int i;
   rlim_t sum=0, tmp, limit, total_limit;


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
   total_limit = limit*cores;

   for(i=0; sched[i+1].heu_name; i++)
   {
      tmp = sched[i].time_fraction*total_limit;
      if(tmp>limit)
      {
         total_limit+= (tmp-limit);
         tmp = limit;
      }
      sched[i].time_absolute = tmp;
      sum = sum+tmp;
   }
   fprintf(GlobalOut,
           "# Scheduled %d strats onto %d cores with %ju seconds (%ju total)\n",
           i, cores, (uintmax_t)limit, (uintmax_t)sum);
}


/*-----------------------------------------------------------------------
//
// Function:  ExecuteScheduleMultiCore()
//
//   Execute the hard-coded strategy schedule.
//
// Global Variables: SilentTimeOut
//
// Side Effects    : Forks, the child runs the proof search, re-sets
//                   time limits, sets heuristic parameters
//
/----------------------------------------------------------------------*/

void ExecuteScheduleMultiCore(ScheduleCell strats[],
                              HeuristicParms_p  h_parms,
                              bool print_rusage,
                              int cores)
{
   int i;
   double run_time = GetTotalCPUTime();
   EGPCtrl_p handle;
   EGPCtrlSet_p procs = EGPCtrlSetAlloc();

   ScheduleTimesInitMultiCore(strats, run_time, cores);

   i=0;
   do
   {
      while(strats[i].heu_name &&
            ((cores-EGPCtrlSetCoresReserved(procs)) >= strats[i].cores))
      {
         handle = EGPCtrlCreate(strats[i].heu_name,
                                strats[i].cores,
                                strats[i].time_absolute);
         if(!handle)
         { /* Child - get out, do work! */
            h_parms->heuristic_name         = strats[i].heu_name;
            h_parms->order_params.ordertype = strats[i].ordering;
            SilentTimeOut = true;
            return;
         }
         else
         {
            EGPCtrlSetAddProc(procs, handle);
         }
         i++;
      }
      handle = EGPCtrlSetGetResult(procs);
      if(handle)
      {
         fprintf(GlobalOut, "# Result found by %s\n", handle->name);
         fputs(DStrView(handle->output), GlobalOut);
         fflush(GlobalOut);
         exit(handle->exit_status);
      }
   }while(EGPCtrlSetCardinality(procs));

   EGPCtrlSetFree(procs);

   fprintf(GlobalOut, "# Schedule exhausted\n");
   TSTPOUT(GlobalOut, "GaveUp");
   exit(RESOURCE_OUT);
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
