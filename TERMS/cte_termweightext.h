#ifndef CTE_TERMWEIGHTEXT

#define CTE_TERMWEIGHTEXT

#include "cte_termtypes.h"

typedef enum
{
   TWESimple = 0,
   TWESubtermsSum = 1,
   TWESubtermsMax = 2
}TermWeightExtenstionStyle;

typedef double (*TermWeightFun)(Term_p term, void* data);

typedef struct termweightextensioncell 
{
   double max_term_multiplier;
   double max_literal_multiplier;
   double pos_eq_multiplier;

   TermWeightExtenstionStyle ext_style;
   TermWeightFun term_weight_fun;
   void* data;

} TermWeightExtensionCell, *TermWeightExtension_p;

#define TermWeightExtensionCellAlloc() (TermWeightExtensionCell*)SizeMalloc(sizeof(TermWeightExtensionCell))
#define TermWeightExtensionCellFree(junk)         SizeFree(junk, sizeof(TermWeightExtensionCell))

TermWeightExtension_p TermWeightExtensionAlloc(
   double max_term_multiplier,
   double max_literal_multiplier,
   double pos_eq_multiplier,
   TermWeightExtenstionStyle ext_style,
   TermWeightFun term_weight_fun,
   void* data);

void TermWeightExtensionFree(TermWeightExtension_p junk);

double TermExtWeight(Term_p term, TermWeightExtension_p twe);


#endif

