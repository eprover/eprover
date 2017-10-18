#ifndef CTE_COMPLEXSORTS
#define CTE_COMPLEXSORTS

typedef struct complextype {
   int      arity;
   Type_p*  args;
} ComplexTypeCell, *ComplexType_p;

#define ComplexTypeCellAlloc()      SizeMalloc(sizeof(ComplexTypeCell))
#define ComplexTypeCellFree(junk)   SizeFree(sizeof(ComplexTypeCell)); junk = NULL

ComplexSort_p CreateComplexType(int arity, Type_p* args);

#endif