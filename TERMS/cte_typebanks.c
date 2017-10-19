#ifndef CTE_TYPEBANKS
#define CTE_TYPEBANKS

#define TYPEBANK_SIZE      4096
#define TYPEBANK_HASH_MASK TYPEBANK_SIZE-1

#define REALLOC_STEP       16

#define GetNameId(node)       ((node)->val1.i_val)
#define GetArity(node)        ((node)->val2.i_val)

#define type_a0hash(t) ((t)->f_code)
#define type_a1hash(t) (type_a0hash(t)^(((intptr_t)(t)->args[0])>>3))
#define type_aritynhash(t) (type_a1hash(t)^(((intptr_t)(t)->args[1])>>4))

extern bool ProblemIsHO;

typedef struct back_idx_info
{
   const char* name;
   int         arity;
} back_idx_info;

static __inline__ back_idx_info* bii_alloc(const char* name, int arity)
{
   back_idx_info* ptr = SizeMalloc(sizeof(back_idx_info));
   ptr->name  = SecureStrdup(name);
   ptr->arity = arity;
   return ptr;
}

#define hash_type(type)\
((((type)->arity == 0)?\
  type_a0hash(type):\
  (((type)->arity == 1)?type_a1hash(type):type_aritynhash(type)))\
   &TYPEBANK_HASH_MASK)

typedef struct typebank_cell {
   PDArray_p  back_idx;                   // Type constructor or simple type back index
   StrTree_p  name_idx;                   // Name to arity, type_identifier pair
                                          // for sorts arity is always 0
   long       names_count;                // Counter for different names inserted
   UniqueId   types_count;                // Counter for different types inserted -- Each type will
                                          // have unique ID.
   PObjTree_p hash_table[TYPEBANK_SIZE];  // 

   /* Some frequently accessed types */
   Type_p     bool_type;
   Type_p     i_type;
   Type_p     kind_type;
} TypeBank, *TypeBank_p;


int cmp_types(void* el1, void* el2)
{
   assert(el1 && el2);
   Type_p t1 = (Type_p)el1;
   Type_p t2 = (Type_p)el2;

   int res = t1->f_code - t2->f_code;
   if (!res)
   {     
      assert(t1->arity == t2->arity);
      for(int i=0; i<t1->arity && !res; i++)
      {
         res = PCmp(t1->args[i], t2->args[i]);
      }
   }

   return res;
}


static __inline __ type_arg_realloc(Type_p** args, int current, int new)
{
  Type_p* new_arr = SizeMalloc(new * sizeof(Type_p));

  int min_size = MIN(curent, new);
  for(int i=0; i<min_size; i++)
  {
    new_arr[i] = (*args)[i];
  }

  SizeFree(*args, current * sizeof(Type_p));
  *args = new_arr;
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

   handle->bool_type = InsertTypeShared(handle, AllocSimpleSort(STBool));
   handle->i_type    = InsertTypeShared(handle, AllocSimpleSort(STIndividuals));
   handle->kind_type = InsertTypeShared(handle, AllocSimpleSort(STKind));

   return handle;
}

Type_p TypeBankInsertTypeShared(TypeBank_p bank, Type_p t)
{
   assert(bank);
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


TypeConsCode TypeBankDefineTypeConstructor(TypeBank_p bank, const char* name, int arity)
{
   assert(bank);
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
      PDArrayStore(bank->back_idx, bii_alloc(name, arity));
      assert(PDArraySize(bank->back_idx) == bank->names_count);

      return id.i_val;
   }
}

SortType TypeBankDefineSimpleSort(TypeBank bank, const char* name)
{
   assert(bank);
   return DefineTypeConstructor(bank, name, 0);
}

TypeConsCode TypeBankFindTCCode(TypeBank_p bank, const char* name)
{
   assert(bank);
   StrTree_p node = StrTreeFind(&bank->name_idx, name);
   
   return node ? GetNameId(node) : NAME_NOT_FOUND;
}

int TypeBankFindTCArity(TypeBank_p bank, TypeConsCode tc_code)
{
   assert(tc_code != INVALID_ID && tc_code < PDArraySize(bank->back_idx));
   assert(PDArraySize(bank->back_idx) == bank->names_count);

   return ((back_idx_info*)PDArrayElementP(bank->back_idx, tc_code))->arity;
}

const char* TypeBankFindTCName(TypeBank_p bank, TypeConsCode tc_code)
{
   assert(tc_code != INVALID_ID && tc_code < PDArraySize(bank->back_idx));
   assert(PDArraySize(bank->back_idx) == bank->names_count);

   return ((back_idx_info*)PDArrayElementP(bank->back_idx, tc_code))->name;
}

Type_p TypeBankParseType(Scanner_p in, TB_p bank)
{
   Type_p leftArg    = NULL;
   Type_p rightArg   = NULL;
   Type_p res        = NULL;


   if (!ProblemIsHO)
   {
      // We support only (A1 * A2 * ... * An) > B
      // or C type (constructors)

      if (!TestInpTok(in, OpenBracket))
      {
         leftArg = parse_single_type(in, bank);
         if (TestInpTok(in, GreaterSign))
         {
            AcceptInpTok(in, GreaterSign);

            rightArg = parse_single_type(in, bank);

            Type_p* args = TypeArgArrayAlloc(2);
            args[0] = leftArg;
            args[1] = rightArg;

            res = AllocArrowType(arity, args);

            if (TestInpTok(in, GreaterSign))
            {
               AktTokenError(in,"Mixing of first order and higher "
                                "order syntax is forbidden." );
            }
         }
         else 
         {
            res = leftArg;
         }
      }
      else
      {
         AcceptInpTok(in, OpenBracket);

         int allocated = REALLOC_STEP;
         Type_p* args = TypeArgArrayAlloc(allocated);
         args[0] = parse_single_type(in, bank);

         int arity = 1;

         do
         {
            if (TestInpTok(in, GreaterSign))
            {
               AktTokenError(in,"Mixing of first order and higher "
                                "order syntax is forbidden." );
            }
            AcceptInpTok(in, Mult);

            rightArg = parse_single_type(in, bank);
            if (arity == allocated)
            {
               type_arg_realloc(&args, allocated, allocated + REALLOC_STEP);
               allocated += REALLOC_STEP;
            }

            args[arity++] = rightArg;
         } while (!TestInpTok(in, CloseBracket));

         AcceptInpTok(in, CloseBracket);
         AcceptInpTok(in, GreaterSign);

         rightArg = parse_single_type(in, bank);
         if (arity == allocated)
         {
            type_arg_realloc(&args, allocated, allocated + REALLOC_STEP);
            allocated += REALLOC_STEP;
         }

         args[arity++] = rightArg;

         type_arg_realloc(&args, allocated, arity); // cut array to right size

         res = AllocArrowType(arity, args);
      }      
   }
   else 
   {
      // parsing HO types
      if (TestInpTok(in, OpenBracket))
      {
         AcceptInpTok(in, OpenBracket);
         leftArg = TypeBankParseType(in, bank);
         AcceptInpTok(in, CloseBracket); 
      }
      else
      {
         leftArg = parse_single_type(in, bank);
      }

      if (TestInpTok(in, CloseBracket | Fullstop | CloseSquare | Comma 
                         | EqualSign | NegEqualSign | FOFBinOp))
      {
         res = leftArg;
      }
      else
      {
         int allocated = REALLOC_STEP;
         Type_p* args = TypeArgArrayAlloc(allocated);
         args[0] = leftArg;

         int arity = 1;

         do
         {
            if (TestInpTok(in, Mult))
            {
               AktTokenError(in,"Mixing of first order and higher "
                                "order syntax is forbidden");
            }
            AcceptInpTok(in, GreaterSign);
            if (TestInpTok(in, OpenBracket))
            {
               /* We can have nested arrow type */
               AcceptInpTok(in, OpenBracket);
               rightArg = TypeBankParseType(in, bank);
               AcceptInpTok(in, CloseBracket);
            }
            else
            {
               rightArg = TypeBankParseType(in, bank);
            }


            if (arity == allocated)
            {
               type_arg_realloc(&args, allocated, allocated + REALLOC_STEP);
               allocated += REALLOC_STEP;
            }

            args[arity++] = rightArg;
         } while (!(TestInpTok(in, CloseBracket | Fullstop | CloseSquare | Comma)));

         if (IsArrowType(rightArg))
         {
            // we have to flatten out the rightArg
            args[arity-1] = rightArg->args[0];

            int i;
            for(i=0; i < rightArg->arity-1; i++)
            {
               if (arity == allocated)
               {
                  type_arg_realloc(&args, allocated, allocated + REALLOC_STEP);
                  allocated += REALLOC_STEP;
               }
               args[arity++] = rightArg->args[i+1];
            }
         }

         type_arg_realloc(&args, allocated, arity); // cut array to right size

         res = AllocArrowType(arity, args);
      }
   }

   return TypeBankInsertTypeShared(res);
}


static void __inline__ ensure_not_kind(Type_p arg, Scanner_p in)
{
   if (TypeIsKind(arg))
   {
      AkTokenErrorError(in, "Only ground types supported.");
   }
}


Type_p parse_single_type(Scanner_p in, TB_p bank)
{
   DStr_p         id;
   FuncSymbType   id_type;
   Type_p         type;
   Type_p*        typeArgs = NULL;
   int            arity = 0;

   id = DStrAlloc();

   id_type = TermParseOperator(in, id);
   if (id_type != FSIdentFreeFun && id_type != FSIdentInterpreted)
   {
      AktTokenError(in, "Function identifier expected");
   }

   if (TestInpTok(in, OpenBracket))
   {
      AcceptInpTok(in, OpenBracket);

      int allocated = REALLOC_STEP;
      typeArgs = TypeArgArrayAlloc(allocated);

      Type_p arg = parse_single_type(in, bank);
      ensure_not_kind(arg, in);

      typeArgs[arity++] = arg;

      while(TestInpTok(in, Comma))
      {
         AcceptInpTok(in, Comma);

         if (arity == allocated)
         {
            TermArgArrayRealloc(&typeArgs, allocated,
                                allocated + REALLOC_STEP);
            allocated += REALLOC_STEP;
         }

         arg = parse_single_type(in, bank);
         ensure_not_kind(arg, in);

         typeArgs[arity++] = arg;
      }
      AcceptInpTok(in, CloseBracket);

      //If f_code did not exist previously, it is a type constructor
      TypeConsCode tc_code;
      if ((tc_code = TypeBankFindTCCode(bank, DStrView(id))) == NAME_NOT_FOUND)
      {
         tc_code = TypeBankDefineTypeConstructor(bank, DStrView(id), arity);
      }
      else if (TypeBankFindTCArity(bank, tc_code) != arity)
      {
         DStr_p msg = DStrAlloc();
         DStrAppendStr(msg, "Redefition of type constructor ");
         DStrAppendStr(msg, TypeBankFindTCName(bank, tc_code));
         DStrAppendStr(msg, ". Mismatch in number of arguments.")
         AkTokenError(in, DStrView(msg));
      }

      type_arg_realloc(&args, allocated, arity); // cut array to right size
      type = TypeAlloc(tc_code, arity, args);
   }
   else
   {
      TypeConsCode tc_code;
      if ((tc_code = TypeBankFindTCCode(bank, DStrView(id))) == NAME_NOT_FOUND)
      {
         tc_code = TypeBankDefineSimpleSort(bank, DStrView(id));
      }
      else if (TypeBankFindTCArity(bank, tc_code) != 0)
      {
         DStr_p msg = DStrAlloc();
         DStrAppendStr(msg, "Type constructor ");
         DStrAppendStr(msg, TypeBankFindTCName(bank, tc_code));
         DStrAppendStr(msg, "has not been declared as simple sort.")
         AkTokenError(in, DStrView(msg));
      }
      type = AllocSimpleSort(tc_code);
   }


   DStrFree(id);
   return TypeBankInsertTypeShared(bank, type);
}

void TypePrintTSTP(FILE* out, TypeBank_p bank, Type_p type)
{
   if (TypeIsArrow(type))
   {
      if (!ProblemIsHO)
      {
         int nr_of_args = type->arity -1;

         if (nr_of_args == 1)
         {
            TypePrintTSTP(out, bank, type->args[0]);
            fprintf(stderr, " > ");
         }
         else
         {
            fprintf(stderr, "(");
            for(int i=0; i<nr_of_args-1; i++)
            {
               TypePrintTSTP(out, bank, type->args[i]);
               fprintf(stderr, " * ");
            }
            TypePrintTSTP(out, bank, type->args[nr_of_args-1]);
            fprintf(stderr, ") > ");

            TypePrintTSTP(out, bank, type->args[arity-1])
         }
      }
      else
      {
         for(int i=0; i<type->arity-1; i++)
         {
            TypePrintTSTP(out, bank, type->args[i]);
            fprintf(stderr, " > ");
         }
         TypePrintTSTP(out, bank, type->args[type->arity-1]);
      }
   }
   else
   {
      fprintf(stderr, "%s", TypeBankFindTCName(bank, type->f_code));
      if (tc->arity)
      {
         fprintf(stderr, "(");
         for(int i=0; i<type->arity-1; i++)
         {
            TypePrintTSTP(out, bank, type->args[i]);
            fprintf(stderr, ", ");
         }
         TypePrintTSTP(out, bank, type->args[type->arity-1]);
         fprintf(stderr, ")");
      }
   }
}

Type_p TypeChangeReturnType(TypeBank_p bank, Type_p type, Type_p new_ret)
{
   assert(TypeIsArrow(type));
   
   Type_p copy = TypeCopy(type);
   type->args[type->arity-1] = new_ret;

   return TypeBankInsertTypeShared(bank, copy);
}


#endif


