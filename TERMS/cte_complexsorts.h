#include <cte_signature.h>

#ifndef CTE_COMPLEXSORTS

#define CTE_COMPLEXSORTS
typedef long   TypeConsCode;
//typedef IntMap TypeConsStore;

typedef struct complexsorts {
   TypeConsCode    tc_code;
   int             arity;
   Type_p*         args;
} ComplexSortCell, *ComplexSort_p;

#define ComplexSortCellAlloc()		SizeMalloc(sizeof(ComplexSortCell))
#define ComplexSortCellFree(junk)	SizeFree(sizeof(ComplexSortCell)); junk = NULL

ComplexSort_p CreateTypeCons(TypeConsCode code, int arity, Type_p args);

#endif
