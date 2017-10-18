#ifndef CTE_TYPEBANKS
#define CTE_TYPEBANKS

#define TYPEBANK_SIZE 2048

#define GetNameId(node)       (node)->val1.i_val;
#define GetArity(node)        (node)->val2.i_val;

typedef struct typebank_cell {
   PDArray_p  back_idx;                   // Type constructor or simple type back index
   StrTree_p  name_idx;                   // Name to arity, type_identifier pair
                                          // for sorts arity is always 0
   long       names_count;                // Counter for different names inserted
   UniqueId   types_count;                // Counter for different types inserted -- Each type will
                                          // have unique ID.
   PObjTree_p hash_table[TYPEBANK_SIZE];  // 
} TypeBank, *TypeBank_p;


int cmp_types(void* el1, void* el2)
{
   Type_p t1 = (Type_p)el1;
   Type_p t2 = (Type_p)el2;

   int res = t1->which_type - t2->which_type;
   if (!res)
   {
      switch (t1->which_type)
      {
         case SIMPLE_SORT:
            res = GetSimpleSort(t1) - GetSimpleSort(t2);
            break;
         case COMPLEX_SORT:
            ComplexSort_p c_sort1 = GetComplexSort(t1);
            ComplexSort_p c_sort2 = GetComplexSort(t2);
            
            res = c_sort1->tc_code - c_sort2->tc_code;
            if (!res)
            {
               assert(c_sort1.arity == c_sort2.arity);
               for(int i=0; i<c_sort1->arity && !res; i++)
               {
                  res = PCmp(c_sort1->args[i], c_sort2->args[i]);
               }
            }
            break;
         default:
            assert(t1->which_type == COMPLEX_TYPE);

            ComplexType_p c_type1 = GetComplexType(t1);
            ComplexType_p c_type2 = GetComplexType(t2);

            assert(c_type1.arity == c_type2.arity);
            for(int i=0; i<c_type1->arity && !res; i++)
            {
               res = PCmp(c_type1->args[i], c_type2->args[i]);
            }
            break;
      }
   }

   return res;
}


static unsigned int __inline__ hash_type(Type_p t)
{
   unsigned int res;
   switch(t->which_type)
   {
      case SIMPLE_SORT:
         res = (unsigned int) (GetSimpleSort(t));
         break;
      case COMPLEX_SORT:
         ComplexSort_p c_sort = GetComplexSort(t);
         res = (unsigned int) (c_sort->c_code ^ ((intptr_t)(c_sort->args[0])>>3));
         if (c_sort->arity > 1)
         {
            res = res ^ ((intptr_t)(c_sort->args[0])<<3);
         }
         break;
      default:
         assert(t1->which_type == COMPLEX_TYPE);

         ComplexSort_p c_type = GetComplexType(t);
         res = (unsigned int) (((intptr_t)(c_type->args[0])>>3));
         if (c_type->arity > 1)
         {
            res = res ^ ((intptr_t)(c_type->args[c_type->arity>>1])<<3);
         }
         break;
   }
   return res % TYPEBANK_SIZE;
}


TypeBank_p TypeBankAlloc()
{
   TypeBank_p handle = TypeBankCellAlloc();

   handle->back_idx = PDArrayAlloc();
   handle->name_idx = NULL;

   handle->names_count = 0;
   handle->types_count = 0;

   for(int i = 0; i < TYPEBANK_SIZE; i++)
   {
      hash_table[i] = NULL;
   }

   SortType res = DefineSimpleSort(handle, "$no_type");
   UNUSED(res); assert(res == STNoSort);
   res = DefineSimpleSort(table, "$o");
   UNUSED(res); assert(res == STBool);
   res = DefineSimpleSort(table, "$i");
   UNUSED(res); assert(res == STIndividuals);
   res = DefineSimpleSort(table, "$tType");
   UNUSED(res); assert(res == STKind);
   res = DefineSimpleSort(table, "$int");
   UNUSED(res); assert(res == STInteger);
   res = DefineSimpleSort(table, "$rat");
   UNUSED(res); assert(res == STRational);
   res = DefineSimpleSort(table, "$real");
   UNUSED(res); assert(res == STReal);

   return handle;
}

Type_p InsertTypeShared(TypeBank_p bank, Type_p t)
{
   Type_p res;
   if (t->unique_id == INVALID_ID)
   {
      // if unique_id is set -- then the object must be present.
      res = (Type_p) PTreeObjStore(bank + hash_type(t), t, cmp_types);
      if (res == NULL)
      {
         res = t;
      }
   }
   else
   {
      res = t;
   }

   return res;
}


TypeConsCode DefineTypeConstructor(TypeBank_p bank, const char* name, int arity)
{
   StrTree_p node = StrTreeFind(&bank->name_idx, name);
   if (node)
   {
      if (GetArity(node) != arity)
      {
         return GetNameId(node);
      }
      else
      {
         DStr_p err_msg = DStrAlloc();
         DStrAppendStr(err_msg, "Redefinition of type constructor ");
         DStrAppendStr(err_msg, name);

         Error(DStrView(err_msg), SYNTAX_ERROR);
      }
   }
   else 
   {
      IntOrP id    = {.i_val = bank->names_count++};
      IntOrP arity = {.i_val = arity};
      StrTreeInsert(&bank->name_idx, name, id, arity);

      return id.i_val;
   }
}


SortType DefineSimpleSort(TypeBank bank, const char* name)
{
   return DefineTypeConstructor(bank, name, 0);
}


#endif


