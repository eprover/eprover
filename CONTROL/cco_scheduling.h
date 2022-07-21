/*-----------------------------------------------------------------------

File  : cco_scheduling.h

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Some simple data types and code to implement quick-and-dirty
  strategy scheduling for E.

  Copyright 2013 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Wed May 22 22:33:40 CEST 2013
    New

-----------------------------------------------------------------------*/

#ifndef CCO_SCHEDULING

#define CCO_SCHEDULING

#include <sys/types.h>
#include <sys/wait.h>
#include <cio_signals.h>
#include <che_hcb.h>
#include <cco_gproc_ctrl.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct schedule_cell
{
   char*        heu_name;
   TermOrdering ordering;
   char*        sine;
   float        time_fraction;
   rlim_t       time_absolute;
   int          cores;
}ScheduleCell, *Schedule_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define DEFAULT_SCHED_TIME_LIMIT 300
#define SCHEDULE_DONE (-1)
#define RETRY_DEFAULT_SCHEDULE_THRESHOLD (2.0)


void ScheduleTimesInit(ScheduleCell sched[], double time_used);

void ScheduleTimesInitMultiCore(ScheduleCell sched[], double time_used,
                                double time_limit, bool preprocessing_schedule,
                                int* cores, bool serialize);

void InitializePlaceholderSearchSchedule(Schedule_p search_sched,
                                         Schedule_p preproc_sched,
                                         bool force_preproc);

int ExecuteScheduleMultiCore(ScheduleCell strats[],
                             HeuristicParms_p  h_parms,
                             bool print_rusage,
                             int wc_time_limit,
                             int compute_cores_per_schedule,
                             int max_cores,
                             bool serialize);

Schedule_p GetFilteredDefaultSchedule(Schedule_p exhausted_sched);



#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
