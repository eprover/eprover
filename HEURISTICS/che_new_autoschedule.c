/*-----------------------------------------------------------------------

  File  : che_new_autoschedule.c

  Author: Petar Vukmirovic, Stephan Schulz

  Contents

  Code implementing the new, symbolic auto scheduling mode.

  Copyright 2022 the authors.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main E directory for details.
  Run "eprover -h" for contact information.

  Changes

  Created: Tue May  2 22:07:17 GMT 2000

  -----------------------------------------------------------------------*/

#include "che_new_autoschedule.h"

#include "schedule.vars"


/*-----------------------------------------------------------------------
//
// Function: class_to_schedule()
//
//   Given a class name, return a schedule. If there is an exact
//   match for the class, use the associated schedule, otherwise use
//   the schedule associated to the largest of the classes with
//   minimal string distance.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

ScheduleCell* class_to_schedule(const char* problem_category,
                                StrSchedPair* schedules)
{
   int min_idx = -1;
   int min_dist = INT_MAX;
   int max_class_size = INT_MIN;
   for(int i=0; min_dist && schedules[i].key; i++)
   {
      int dist = StrDistance(schedules[i].key, problem_category);
      if(dist == 0)
      {
         min_idx = i;
         min_dist = 0;
         break;
      }
      else if (dist < min_dist)
      {
         min_dist = dist;
         min_idx = i;
         max_class_size = schedules[i].class_size;
      }
      else if (dist == min_dist && schedules[i].class_size > max_class_size)
      {
         min_idx = i;
         max_class_size = schedules[i].class_size;
      }
   }
   assert(min_idx >= 0);
   if(min_dist)
   {
      fprintf(GlobalOut, COMCHAR" partial match(%d): %s\n",
              min_dist, schedules[min_idx].key);
   }
   return schedules[min_idx].value;
}


/*-----------------------------------------------------------------------
//
// Function: StrategiesPrintPredefined()
//
//   Print all predefined strategies.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void StrategiesPrintPredefined(FILE* out, bool names_only)
{
   int i;
   for(i=0; conf_map[i].key; i++)
   {
      if(names_only)
      {
         fprintf(out, "%s\n", conf_map[i].key);
      }
      else
      {
         fprintf(out, "%s = \n%s\n", conf_map[i].key, conf_map[i].value);
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: GetPreprocessingSchedule()
//
//   Get preprocessing schedule for a class.
//
// Global Variables: preproc_sched_map (from schedule.vars)
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

ScheduleCell* GetPreprocessingSchedule(const char* problem_category)
{
   return class_to_schedule(problem_category, preproc_sched_map);
}


/*-----------------------------------------------------------------------
//
// Function: GetSearchSchedule()
//
//   Get search schedule for a class.
//
// Global Variables: search_sched_map (from schedule.vars)
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

ScheduleCell* GetSearchSchedule(const char* problem_category)
{
   return class_to_schedule(problem_category, search_sched_map);
}


/*-----------------------------------------------------------------------
//
// Function: GetHeuristicWithName()
//
//   Given a name, find and parse a heuristic into the provided cell.
//
// Global Variables: conf_map (from schedule.vars)
//
// Side Effects    : Memory management
//
/----------------------------------------------------------------------*/

void GetHeuristicWithName(const char* name, HeuristicParms_p target)
{
   for(int i=0; conf_map[i].key; i++)
   {
      if(!strcmp(name, conf_map[i].key))
      {
         Scanner_p in = CreateScanner(StreamTypeInternalString,
                                      (char*)conf_map[i].value,
                                      true, NULL, true);
         HeuristicParmsParseInto(in, target, false);
         DestroyScanner(in);
         return;
      }
   }
   Error("Error: Configuration name %s not found.",
         OTHER_ERROR, name);
}




/*-----------------------------------------------------------------------
//
// Function: GetDefaultSchedule()
//
//   Return the default (fallback) schedule.
//
// Global Variables: _DEFAULT_SCHEDULE (needs renaming)
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

ScheduleCell* GetDefaultSchedule()
{
   return _DEFAULT_SCHEDULE;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
