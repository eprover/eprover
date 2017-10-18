#include "cte_complexsorts.h"

TypeConsCode RegisterTypeCons(Sig_p sig, const char* typeConsName, int arity)
{
   TypeConsCode tc_code = SigInsertId(sig, typeConsName, arity, true);

   if (!tc_code)
   {
      DStr_p error_msg = DStrAlloc();
      DStrAppendStr(error_msg, "Redefinition of type ");
      DStrAppendStr(error_msg, typeConsName);
      DStrAppendStr(error_msg, ".");

      Error(DStrView(error_msg), SYNTAX_ERROR);
   }

   return tc_code;
}

ComplexSort_p CreateTypeCons(TypeConsCode code, int arity, Type_p args)
{
   ComplexSort_p handle = ComplexSortCellAlloc();
   handle->tc_code   = code;
   handle->arity     = arity;
   handle->args      = args;

   return handle;
}

