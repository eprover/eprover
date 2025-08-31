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
#include <che_new_autoschedule.h>
#include <unistd.h>
#include <sys/mman.h>



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: name_in_schedule()
//
//   Is the heuristic name in the schedule?
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

static inline bool name_in_schedule(const char* name, Schedule_p sched)
{
   bool ans = false;
   for(int i=0; sched[i].heu_name && !ans; i++)
   {
      ans = !strcmp(name, sched[i].heu_name);
   }
   return ans;
}



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
// Function: ScheduleTimesInitMultiCore()
//
//    If preprocessing_schedule is true (used for scheduling preprocessing)
//    based on the time fraction the number of cores allocated to the preprocessor
//    will be computed and stored in cores. Cores must be initialized to the
//    prefered maximal number of cores and if this number is smaller than
//    the number of preprocessors, then it is going to be set to the
//    number of preprocessors.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void ScheduleTimesInitMultiCore(ScheduleCell sched[], double time_used,
                                double time_limit, bool preprocessing_schedule,
                                int* cores, bool serialize)
{
   int i;
   rlim_t sum=0, tmp, limit, total_limit;
   int allocated_cores = 0;

   //printf(COMCHAR" ScheduleTimesInitMultiCore(X, time_used=%f, time_limit=%f, "
   //"preprocessing_schedule=%d, *cores=%d, serialize=%d)\n",
   //time_used, time_limit, preprocessing_schedule, *cores, serialize);

   int sched_size = 0;
   while(sched[sched_size].heu_name)
   {
      sched_size++;
   }
   if (preprocessing_schedule && sched_size > *cores)
   {
      // shortening schedule to the num of cores.
      sched_size = *cores;
      sched[sched_size].heu_name = NULL;

      // rescaling time fractions for lower number of cores.
      double total_ratio = 0;
      for(int i=0; sched[i].heu_name; i++)
      {
         total_ratio += sched[i].time_fraction;
      }
      double factor = 1 / total_ratio;
      for(int i=0; sched[i].heu_name; i++)
      {
         sched[i].time_fraction *= factor;
      }
   }

   limit = ceil(time_limit-time_used);
   total_limit = limit;
   if(preprocessing_schedule)
   {
      if(serialize)
      {
         for(i=0; sched[i].heu_name; i++)
         {
            // error in each step is at most 1 -- so we can
            // add at most sched_size extra cores
            sched[i].cores = 1;
         }
         allocated_cores = *cores = 1;
      }
      else
      {
         for(i=0; sched[i].heu_name; i++)
         {
            // error in each step is at most 1 -- so we can
            // add at most sched_size extra cores
            sched[i].cores = MAX(ceil(sched[i].time_fraction*(*cores)), 1.0);
            allocated_cores += sched[i].cores;
         }
         int error = allocated_cores-(*cores);
         assert(error <= sched_size);
         for(i=sched_size-1; error && i >=0; i--)
         {
            // fixing the error in allocating cores (starting)
            // from the least important schedule
            // IMPORTANT: we leave at least one core per schedule
            int to_take = MIN(sched[i].cores-1, error);
            sched[i].cores -=  to_take;
            error -= to_take;
         }
         assert(!error);
      }
   }

   for(i=0; sched[i].heu_name; i++)
   {
      if (!preprocessing_schedule && (sum >= total_limit))
      {
         // limiting search schedules from going crazy.
         sched[i].heu_name = NULL;
         break;
      }

      tmp = ceil((preprocessing_schedule && !serialize ? 1.0 : sched[i].time_fraction)
            *sched[i].cores*total_limit);
      sched[i].time_absolute = preprocessing_schedule ? tmp :  MIN(tmp, limit-sum);
      sum = sum+sched[i].time_absolute;
   }
   fprintf(GlobalOut,
           COMCHAR" Scheduled %d strats onto %d cores with %ju seconds (%ju total)\n",
           i, *cores, (uintmax_t)limit, (uintmax_t)sum);
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

int ExecuteScheduleMultiCore(ScheduleCell strats[],
                             HeuristicParms_p  h_parms,
                             bool print_rusage,
                             int wc_time_limit,
                             int preproc_schedule,
                             int max_cores,
                             bool serialize)
{
   int i;
   double run_time = GetTotalCPUTime();
   EGPCtrl_p handle;
   EGPCtrlSet_p procs = EGPCtrlSetAlloc();


   ScheduleTimesInitMultiCore(strats, run_time, wc_time_limit,
                              preproc_schedule, &max_cores, serialize);

   i=0;

   signal(SIGTERM, ESigTermSchedHandler);
   do
   {
      while(strats[i].heu_name &&
            ((max_cores-EGPCtrlSetCoresReserved(procs)) >= strats[i].cores))
      {
         handle = EGPCtrlCreate(strats[i].heu_name,
                                strats[i].cores,
                                strats[i].time_absolute);
         if(!handle)
         { /* Child - get out, do work! */
            h_parms->heuristic_name = strats[i].heu_name;
            h_parms->order_params.ordertype = strats[i].ordering;
            SilentTimeOut = true;
            EGPCtrlSetFree(procs, false);
            signal(SIGTERM, SIG_DFL);
            if(SigTermCaught)
            {
               exit(PARENT_REQUEST);
            }
            return i; // tells the other scheduling call what is the parent
         }
         else
         {
            EGPCtrlSetAddProc(procs, handle);
            // fprintf(stderr, "Will run %s(%d) for %ld\n",
            // handle->name, handle->pid, strats[i].time_absolute);
         }
         i++;
      }
      handle = EGPCtrlSetGetResult(procs);
      if(handle)
      {
         fprintf(GlobalOut, COMCHAR" Result found by %s\n", handle->name);
         fputs(DStrView(handle->output), GlobalOut);
         fflush(GlobalOut);
         if(print_rusage)
         {
            PrintRusage(GlobalOut);
         }
         EGPCtrlSetFree(procs, true);
         exit(handle->exit_status);
      }
      if(SigTermCaught)
      {
         EGPCtrlSetFree(procs, true);
         exit(PARENT_REQUEST);
      }
   }while(EGPCtrlSetCardinality(procs) || strats[i].heu_name);
   signal(SIGTERM, SIG_DFL);

   EGPCtrlSetFree(procs, true);

   fprintf(GlobalOut, COMCHAR" Schedule exhausted\n");
   if(print_rusage)
   {
      PrintRusage(GlobalOut);
   }
   return SCHEDULE_DONE;
}

/*-----------------------------------------------------------------------
//
// Function:  InitializePlaceholderSearchSchedule()
//
//   Find the placeholder position in search sched and replace
//   it with NULL (terminate the schedule array) if we do not
//   need to insert preprocessing schedule into search schedule,
//   otherwise replace it with the name of preprocessing schedule.
//
// Global Variables: SilentTimeOut
//
// Side Effects    : Forks, the child runs the proof search, re-sets
//                   time limits, sets heuristic parameters
//
/----------------------------------------------------------------------*/

void InitializePlaceholderSearchSchedule(Schedule_p search_sched,
                                         Schedule_p preproc_sched,
                                         bool force_preproc)
{
   const char* PLACEHOLDER = "<placeholder>";
   const double PREPROC_RATIO = 0.1;
   int i;
   for(i=0; strcmp(search_sched[i].heu_name, PLACEHOLDER); i++)
   {
      if(force_preproc)
      {
         if(!strcmp(search_sched[i].heu_name, preproc_sched->heu_name))
         {
            force_preproc = false; // already found
         }
      }
   }

   if(!force_preproc)
   {
      search_sched[i].heu_name = NULL;
   }
   else
   {
      search_sched[i].heu_name = preproc_sched->heu_name;
      search_sched[i].time_fraction = PREPROC_RATIO;
      double rest_multipler = 1-PREPROC_RATIO;
      for(int j=0; j<i; j++)
      {
         search_sched[j].time_fraction *= rest_multipler;
      }
      SWAP(search_sched[i], search_sched[1]);
   }
}

/*-----------------------------------------------------------------------
//
// Function:  GetDefaultSchedule()
//
//   After a schedule is exhausted, then turn back to the default schedule
//   and filter the configurations that have not be run from it.
//
// Global Variables: SilentTimeOut
//
// Side Effects    : Forks, the child runs the proof search, re-sets
//                   time limits, sets heuristic parameters
//
/----------------------------------------------------------------------*/

Schedule_p GetFilteredDefaultSchedule(Schedule_p exhausted_sched)
{
   Schedule_p default_sch = GetDefaultSchedule();
   int last_filtered = -1;
   for(int i=0; default_sch[i].heu_name; i++)
   {
      if(!name_in_schedule(default_sch[i].heu_name, exhausted_sched))
      {
         last_filtered++;
         SWAP(default_sch[last_filtered], default_sch[i]);
      }
   }

   const double ratio = 1.0 / (last_filtered+1);
   for(int i=0; i<last_filtered; i++)
   {
      default_sch[i].time_fraction = ratio;
   }
   default_sch[last_filtered+1].heu_name = NULL;

   return default_sch;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
