#include "che_hcb.h"
#include <cco_scheduling.h>

#define DEFAULT_MASK "aaaa-aaaaaa-aa--aa"
#define RAW_DEFAULT_MASK "aaaaaaaaaaa"

extern ScheduleCell* ho_schedule;

int  GetAttemptIdx(const char* strategy_name);

void AutoHeuristicForRawCategory(const char* raw_category, HeuristicParms_p parms);
void AutoHeuristicForCategory(const char* category, HeuristicParms_p parms);
void ScheduleForRawCategory(const char* raw_category, int attempt_idx, HeuristicParms_p parms);
void ScheduleForCategory(const char* category, int attempt_idx, HeuristicParms_p parms);