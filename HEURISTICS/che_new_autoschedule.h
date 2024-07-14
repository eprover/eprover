/*-----------------------------------------------------------------------

  File  : che_new_autoschedule.h

  Author: Stephan Schulz (schulz@eprover.org), Petar Vukmirovic

  Contents

  Code for new (symbolic) autoschedule.

  Copyright 2022 by the authors.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

  Created: Wed Sep 21 02:49:50 CEST 2022

-----------------------------------------------------------------------*/

#ifndef CHE_NEW_AUTOSCHEDULE

#define CHE_NEW_AUTOSCHEDULE

#include "che_hcb.h"
#include <cco_scheduling.h>

#define DEFAULT_MASK "aaaaa-aaaaaa-aaaaaaaaa"
#define RAW_DEFAULT_MASK "aaaaaaaaaaaaa"


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct
{
  char* key;
  char* value;
} StrStrPair;

typedef struct
{
  char* key;
  ScheduleCell* value;
  int class_size; // how many problems were originally in the class used for scheduling
                  // (will be used for tie-breaking when choosing the class)
} StrSchedPair;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

void          StrategiesPrintPredefined(FILE* out, bool name_only);
ScheduleCell* GetPreprocessingSchedule(const char* problem_category);
ScheduleCell* GetSearchSchedule(const char* problem_category);
ScheduleCell* GetDefaultSchedule();
void          GetHeuristicWithName(const char* name,
                                   HeuristicParms_p target);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
