/*-----------------------------------------------------------------------

File  : che_new_autoschedule.c

Author: Stephan Schulz

Contents

  Code that is automatically generated by 

Changes

<1> Tue May  2 22:07:17 GMT 2000
    Extracted from che_heuristics.c

-----------------------------------------------------------------------*/

#include "che_hcb.h"

#include "autoschedule_gen.vars"

void class_to_heuristic(const char* problem_category, const char** categories,
                        const char** configurations, int num_categories, 
                        HeuristicParms_p params)
{
  int i=0;
  for(; i<num_categories && strcmp(categories[i], problem_category); i++)
      {}
  const char* configuration = i == num_categories ? best_conf : confs[i];
  Scanner_p in = CreateScanner(StreamTypeInternalString, (char*)configuration, true, NULL, true);
  HeuristicParmsParseInto(in, params, true); 
  DestroyScanner(in);
}

void HeuristicForRawCategory(const char* raw_category, HeuristicParms_p parms)
{
  class_to_heuristic(raw_category, raw_categories, raw_confs, num_raw_categories, parms);
}

void HeuristicForCategory(const char* category, HeuristicParms_p parms)
{
  class_to_heuristic(category, categories, confs, num_raw_categories, parms);
}
