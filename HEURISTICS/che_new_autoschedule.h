#include "che_hcb.h"
#include <cco_scheduling.h>

#define DEFAULT_MASK "aaaaa-aaaaaa-aaaaaaaaa"
#define RAW_DEFAULT_MASK "aaaaaaaaaaaaa"

ScheduleCell* GetPreprocessingSchedule(const char* problem_category);
ScheduleCell* GetSearchSchedule(const char* problem_category);
void GetHeuristicWithName(const char* name, HeuristicParms_p target);

