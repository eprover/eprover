typedef enum TypeDiscriminator {
   SIMPLE_SORT    = 0,
   COMPLEX_SORT,
   COMPLEX_TYPE
};

typedef union type_choice_union {
   SortType      s_sort;
   ComplexSort_p c_sort;
   ComplexType_p c_type;
} TypeChoice;

typedef long UniqueId;

typedef struct typecell {
   TypeDiscriminator   which_type;
   TypeChoice          type;
} TypeCell, *Type_p;

#define GetSimpleSort(t)       ((t)->type.s_sort)
#define GetComplexSort(t)      ((t)->type.c_sort)
#define GetComplexType(t)      ((t)->type.c_type)



