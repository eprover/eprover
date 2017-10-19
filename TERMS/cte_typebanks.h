#ifndef CTE_TYPEBANKS
#define CTE_TYPEBANKS

#define TYPEBANK_SIZE 2048

#define NAME_NOT_FOUND          -1

#define GetNameId(node)       (node)->val1.i_val;
#define GetArity(node)        (node)->val2.i_val;

typedef struct typebank_cell {
   PDArray_p  back_idx;                   // Type constructor or simple type back index
   StrTree_p  name_idx;                   // Name to arity, type_identifier pair
                                          // for sorts arity is always 0
   long       names_count;                // Counter for different names inserted
   UniqueId   types_count;                // Counter for different types inserted -- Each type will
                                          // have unique ID.
   PObjTree_p hash_table[TYPEBANK_SIZE];  // Hash table for sharng
} TypeBank, *TypeBank_p;

#define TypeBankCellAlloc()   SizeMalloc(sizeof(TypeBank));

TypeBank_p TypeBankAlloc(void);

Type_p 		 InsertTypeShared(TypeBank_p bank, Type_p t);
TypeConsCode DefineTypeConstructor(TypeBank_p bank, const char* name, int arity);
SortType 	 DefineSimpleSort(TypeBank bank, const char* name);

TypeConsCode FindTCCode(TypeBank_p bank, const char* name);


#endif


