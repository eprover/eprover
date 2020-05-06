/*-----------------------------------------------------------------------

  File  : cte_signature.c

  Author: Stephan Schulz

  Contents

  Functions implementing the signature functionality.

  Copyright 1998-2017 by the author.

  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created:  Sun Sep 21 19:27:54 MET DST 1997

-----------------------------------------------------------------------*/

#include "cte_signature.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

bool      SigSupportLists = false;

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: sig_print_operator()
//
//   Print a single operator
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void sig_print_operator(FILE* out, Sig_p sig, FunCode op, bool comments)
{
   if(comments)
   {
      fprintf(out, "   %-13s : %2d    #  %2ld %2d \n",
              sig->f_info[op].name, sig->f_info[op].arity, op,
              sig->f_info[op].properties);
   }
   else
   {
      fprintf(out, "   %-13s : %2d\n",
              sig->f_info[op].name, sig->f_info[op].arity);
   }
}

/*-----------------------------------------------------------------------
//
// Function: sig_compute_alpha_ranks()
//
//   For all symbols in sig compute the alpha-rank of the symbol.
//
// Global Variables: -
//
// Side Effects    : Memory operations, changes the ranks in sig.
//
/----------------------------------------------------------------------*/

static void sig_compute_alpha_ranks(Sig_p sig)
{
   PStack_p stack;
   long count = 0;
   StrTree_p handle;

   stack = StrTreeTraverseInit(sig->f_index);

   while((handle = StrTreeTraverseNext(stack)))
   {
      sig->f_info[handle->val1.i_val].alpha_rank = count++;
   }
   StrTreeTraverseExit(stack);

   sig->alpha_ranks_valid = true;
}



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: SigAlloc()
//
//   Allocate a initialized signature cell. Also initializes a type table.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Sig_p SigAlloc(TypeBank_p bank)
{
   Sig_p handle;

   handle = SigCellAlloc();

   handle->alpha_ranks_valid = false;
   handle->size           = DEFAULT_SIGNATURE_SIZE;
   handle->f_count        = 0;
   handle->f_info         =
      SecureMalloc(sizeof(FuncCell)*DEFAULT_SIGNATURE_SIZE);
   handle->f_index = NULL;
   handle->ac_axioms = PStackAlloc();

   handle->type_bank = bank;

   SigInsertId(handle, "$true", 0, true);
   assert(SigFindFCode(handle, "$true")==SIG_TRUE_CODE);
   SigSetFuncProp(handle, SIG_TRUE_CODE, FPInterpreted);
   SigDeclareType(handle, SIG_TRUE_CODE, handle->type_bank->bool_type);
   SigInsertId(handle, "$false", 0, true);
   assert(SigFindFCode(handle, "$false")==SIG_FALSE_CODE);
   SigSetFuncProp(handle, SIG_FALSE_CODE, FPInterpreted);
   SigDeclareType(handle, SIG_FALSE_CODE, handle->type_bank->bool_type);

   if(SigSupportLists)
   {
      SigInsertId(handle, "$nil", 0, true);
      assert(SigFindFCode(handle, "$nil")==SIG_NIL_CODE);
      SigInsertId(handle, "$cons", 2, true);
      assert(SigFindFCode(handle, "$cons")==SIG_CONS_CODE);
   }


   handle->internal_symbols = handle->f_count;

   handle->eqn_code      = 0;
   handle->neqn_code     = 0;
   handle->cnil_code     = 0;
   handle->orn_codes     = NULL;

   handle->or_code           = 0;
   handle->not_code          = 0;
   handle->qex_code          = 0;
   handle->qall_code         = 0;
   handle->and_code          = 0;
   handle->or_code           = 0;
   handle->impl_code         = 0;
   handle->equiv_code        = 0;
   handle->nand_code         = 0;
   handle->nor_code          = 0;
   handle->bimpl_code        = 0;
   handle->xor_code          = 0;
   handle->answer_code       = 0;

   handle->skolem_count      = 0;
   handle->newpred_count     = 0;

   handle->distinct_props = FPDistinctProp;
   return handle;
}




/*-----------------------------------------------------------------------
//
// Function: SigInitInternalCodes()
//
//   Put all the FOF operators as function symbols into sig. Sig
//   should be empty, so that sig->internal symbols can be properly
//   initialized.  Note that this will be used for plain term
//   signatures. It reuses some equivalent fields of signatures used
//   for patterns, but morphs the f_codes into internal symbols.
//
// Global Variables: -
//
// Side Effects    : Changes sig...
//
/----------------------------------------------------------------------*/

void SigInsertInternalCodes(Sig_p sig)
{
   assert((SigSupportLists && sig->internal_symbols == SIG_CONS_CODE) ||
          (!SigSupportLists && sig->internal_symbols == SIG_FALSE_CODE));

   sig->eqn_code    = SigInsertId(sig, "$eq",   2, true);
   SigSetPolymorphic(sig, sig->eqn_code, true);

   sig->neqn_code   = SigInsertId(sig, "$neq",   2, true);
   SigSetPolymorphic(sig, sig->eqn_code, true);

   sig->qex_code   = SigInsertId(sig, "$qex",   2, true);
   sig->qall_code  = SigInsertId(sig, "$qall",  2, true);
   SigSetPolymorphic(sig, sig->qex_code, true);
   SigSetPolymorphic(sig, sig->qall_code, true);

   sig->not_code   = SigInsertFOFOp(sig, "$not",   1);
   sig->and_code   = SigInsertFOFOp(sig, "$and",   2);
   sig->or_code    = SigInsertFOFOp(sig, "$or",    2);
   sig->impl_code  = SigInsertFOFOp(sig, "$impl",  2);
   sig->equiv_code = SigInsertFOFOp(sig, "$equiv", 2);
   sig->nand_code  = SigInsertFOFOp(sig, "$nand",  2);
   sig->nor_code   = SigInsertFOFOp(sig, "$nor",   2);
   sig->bimpl_code = SigInsertFOFOp(sig, "$bimpl", 2);
   sig->xor_code   = SigInsertFOFOp(sig, "$xor",   2);

   sig->answer_code =  SigInsertId(sig, "$answer", 1, true);
   SigSetFuncProp(sig, sig->answer_code, FPInterpreted|FPPseudoPred);

#ifdef ENABLE_LFHO
   #ifndef NDEBUG
      // surpressing compiler warning
      FunCode app_var_code =
   #endif
      SigInsertId(sig, "$@_var", 1, true);
      assert(app_var_code == SIG_APP_VAR_CODE); //for future code changes
#endif

   Type_p* args = TypeArgArrayAlloc(2);
   args[1] = sig->type_bank->bool_type;
   args[0] = sig->type_bank->i_type;

   Type_p answer_type =
      TypeBankInsertTypeShared(sig->type_bank, AllocArrowType(2, args));

   SigDeclareFinalType(sig, sig->answer_code, answer_type);

   sig->internal_symbols = sig->f_count;
}



/*-----------------------------------------------------------------------
//
// Function: SigFree()
//
//   Free signature.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void SigFree(Sig_p junk)
{
   assert(junk);
   assert(junk->f_info);

   /* names are shared with junk->f_index and are free()ed by the
      StrTreeFree() call below! */
   FREE(junk->f_info);
   StrTreeFree(junk->f_index);
   PStackFree(junk->ac_axioms);
   if(junk->orn_codes)
   {
      PDArrayFree(junk->orn_codes);
   }

   SigCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: SigFindFCode()
//
//   Return the index of the entry name in sig, or 0 if name is not in
//   sig.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

FunCode SigFindFCode(Sig_p sig, const char* name)
{
   StrTree_p entry;

   entry = StrTreeFind(&(sig->f_index), name);

   if(entry)
   {
      return entry->val1.i_val;
   }
   return 0;
}


/*-----------------------------------------------------------------------
//
// Function: SigIsPredicate()
//
//  Returns true if the symbol is known to be a predicate
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool SigIsPredicate(Sig_p sig, FunCode f_code)
{
   Type_p type;

   assert(f_code > 0);
   assert(f_code <= sig->f_count);

   if(FuncQueryProp(&(sig->f_info[f_code]), FPTypePoly))
   {
      return true;
   }
   type = SigGetType(sig, f_code);
   return type && TypeIsPredicate(type);
}


/*-----------------------------------------------------------------------
//
// Function: SigIsFunction()
//
//   Return the value of the Function field for a function symbol.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool SigIsFunction(Sig_p sig, FunCode f_code)
{
   Type_p type;

   assert(f_code > 0);
   assert(f_code <= sig->f_count);

   if(!FuncQueryProp(&(sig->f_info[f_code]), FPTypeFixed))
   {
      return false;
   }

   type = SigGetType(sig, f_code);
   return type && !TypeIsPredicate(type);
}


/*-----------------------------------------------------------------------
//
// Function: SigIsFixedType
//
//   check whether we are sure of the type of this function
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
bool SigIsFixedType(Sig_p sig, FunCode f_code)
{
   assert(f_code > 0);
   assert(f_code <= sig->f_count);

   return FuncQueryProp(&(sig->f_info[f_code]), FPTypeFixed);
}


/*-----------------------------------------------------------------------
//
// Function: SigFixType
//
//   Fix the type of the function, because we are sure the type is the
//   right one (no ambiguity).
//
// Global Variables: -
//
// Side Effects    : Modifiy signature
//
/----------------------------------------------------------------------*/
void SigFixType(Sig_p sig, FunCode f_code)
{
   assert(f_code > 0);
   assert(f_code <= sig->f_count);

   FuncSetProp(&(sig->f_info[f_code]), FPTypeFixed);
}

/*-----------------------------------------------------------------------
//
// Function: SigIsPolymorphic
//
//   Checks whether the symbol is Ad-hoc polymorphic
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
bool SigIsPolymorphic(Sig_p sig, FunCode f_code)
{
   assert(f_code > 0);
   assert(f_code <= sig->f_count);

   return FuncQueryProp(&(sig->f_info[f_code]), FPTypePoly);
}

/*-----------------------------------------------------------------------
//
// Function: SigSetPolymorphic
//
//   Declare that this symbol is polymorphic
//
// Global Variables: -
//
// Side Effects    : modifies the signature
//
/----------------------------------------------------------------------*/
void SigSetPolymorphic(Sig_p sig, FunCode f_code, bool value)
{
   assert(f_code > 0);
   assert(f_code <= sig->f_count);

   FuncSetProp(&(sig->f_info[f_code]), FPTypePoly);
}


/*-----------------------------------------------------------------------
//
// Function: SigQueryProp
//  Checks whether a symbol has all the given properties
//
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
bool SigQueryProp(Sig_p sig, FunCode f_code, FunctionProperties prop)
{
   assert(f_code > 0);
   assert(f_code <= sig->f_count);

   return FuncQueryProp(&(sig->f_info[f_code]), prop);
}

/*-----------------------------------------------------------------------
//
// Function: SigSetSpecial()
//
//   Set the value of the special field for a function symbol.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void SigSetSpecial(Sig_p sig, FunCode f_code, bool value)
{
   assert(f_code > 0);
   assert(f_code <= sig->f_count);

   if(value)
   {
      SigSetFuncProp(sig,f_code,FPSpecial);
   }
   else
   {
      SigDelFuncProp(sig,f_code,FPSpecial);
   }
}



/*-----------------------------------------------------------------------
//
// Function: SigIsSpecial()
//
//   Return the value of the special field for a function symbol.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool SigIsSpecial(Sig_p sig, FunCode f_code)
{
   assert(f_code > 0);
   assert(f_code <= sig->f_count);

   return FuncQueryProp(&(sig->f_info[f_code]), FPSpecial);
}


/*-----------------------------------------------------------------------
//
// Function: SigGetAlphaRank()
//
//   Given a signature and an function symbol code, return the symbols
//   alpha-rank.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int SigGetAlphaRank(Sig_p sig, FunCode f_code)
{
   assert(f_code > 0);
   assert(f_code <= sig->f_count);

   if(!sig->alpha_ranks_valid)
   {
      sig_compute_alpha_ranks(sig);
   }
   assert(sig->alpha_ranks_valid);

   return (sig->f_info[f_code]).alpha_rank;
}


/*-----------------------------------------------------------------------
//
// Function: SigSetAllSpecial()
//
//   Set the special value of all symbols in sig.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void SigSetAllSpecial(Sig_p sig, bool value)
{
   FunCode i;

   for(i=1; i<=sig->f_count; i++)
   {
      SigSetSpecial(sig, i, value);
   }
}

/*-----------------------------------------------------------------------
//
// Function: SigInsertId()
//
//   Insert the symbol name with arity into the signature. Return the
//   f_code assigned to the name or 0 if the same name has already
//   been used with a different arity.
//
// Global Variables: -
//
// Side Effects    : Potential memory operations.
//
/----------------------------------------------------------------------*/

FunCode SigInsertId(Sig_p sig, const char* name, int arity, bool special_id)
{
   long      pos;
   StrTree_p new, test;

   pos = SigFindFCode(sig, name);

   if(pos) /* name is already known */
   {
      if(sig->f_info[pos].arity != arity)
      {
         printf("Problem: %s %d != %d\n", name, arity, sig->f_info[pos].arity);
         return 0; /* ...but incompatible */
      }
      if(special_id)
      {
         SigSetSpecial(sig, pos, true);
      }
      return pos; /* all is fine... */
   }
   /* Now insert the new name...ensure that there is space */
   if(sig->f_count == sig->size-1)
   {
      /* sig->size+= DEFAULT_SIGNATURE_SIZE; */
      sig->size *= DEFAULT_SIGNATURE_GROW;
      sig->f_info  = SecureRealloc(sig->f_info,
                                   sizeof(FuncCell)*sig->size);
   }

   /* Insert the element in f_index and f_info */
   sig->f_count++;
   sig->f_info[sig->f_count].name
      = SecureStrdup(name);
   sig->f_info[sig->f_count].arity = arity;
   sig->f_info[sig->f_count].properties = FPIgnoreProps;
   sig->f_info[sig->f_count].type = NULL;
   sig->f_info[sig->f_count].feature_offset = -1;
   new = StrTreeCellAllocEmpty();
   new->key = sig->f_info[sig->f_count].name;
   new->val1.i_val = sig->f_count;

   test = StrTreeInsert(&(sig->f_index), new);
   UNUSED(test); assert(test == NULL);
   SigSetSpecial(sig,sig->f_count,special_id);
   sig->alpha_ranks_valid = false;

   return sig->f_count;
}

/*-----------------------------------------------------------------------
//
// Function: SigInsertFOFOp()
//
//   Insert a special function symbol used to encode a first-order
//   operator.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

FunCode SigInsertFOFOp(Sig_p sig, const char* name, int arity)
{
   FunCode res = SigInsertId(sig, name, arity, true);

   SigSetFuncProp(sig, res, FPFOFOp);
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: SigPrint()
//
//   Print the signature in external representation, with comments
//   showing internal structure.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void SigPrint(FILE* out, Sig_p sig)
{
   FunCode i;

   fprintf(out, "# Signature (%2ld symbols out of %2ld allocated):\n",
           sig->f_count, sig->size);
   fprintf(out, "#     -Symbol-    -Arity- -Encoding-\n");

   for(i=1; i<=sig->f_count; i++)
   {
      sig_print_operator(out, sig, i, true);
   }
}


/*-----------------------------------------------------------------------
//
// Function: SigPrintSpecial()
//
//   Print the external special symbols from sig.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void SigPrintSpecial(FILE* out, Sig_p sig)
{
   FunCode i;

   fputs("# Special symbols:\n", out);
   for(i=1; i<=sig->f_count; i++)
   {
      if(SigIsSpecial(sig, i))
      {
         sig_print_operator(out, sig, i, true);
      }
   }
}

/*-----------------------------------------------------------------------
//
// Function: SigPrintACStatus()
//
//   For each function symbol which is A, C, or AC, print its status
//   as a comment.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void SigPrintACStatus(FILE* out, Sig_p sig)
{
   FunCode i;

   for(i=1; i<=sig->f_count; i++)
   {
      if(SigQueryFuncProp(sig, i, FPIsAC))
      {
         fprintf(out, "# %s is AC\n", sig->f_info[i].name);
         continue;
      }
      if(SigQueryFuncProp(sig, i, FPAssociative))
      {
         fprintf(out, "# %s is associative\n", sig->f_info[i].name);
         continue;
      }
      if(SigQueryFuncProp(sig, i, FPCommutative))
      {
         fprintf(out, "# %s is commutative\n", sig->f_info[i].name);
         continue;
      }
   }
}



/*-----------------------------------------------------------------------
//
// Function: SigParseKnownOperator()
//
//   Parse an operator, return its FunCode. Error, if operator is not
//   in sig.
//
// Global Variables: -
//
// Side Effects    : Reads input, memory operations
//
/----------------------------------------------------------------------*/

FunCode SigParseKnownOperator(Scanner_p in, Sig_p sig)
{
   FunCode       res;
   int           line, column;
   DStr_p        id, source_name, errpos;
   StreamType    type;

   line = AktToken(in)->line;
   column = AktToken(in)->column;
   source_name = DStrGetRef(AktToken(in)->source);
   type        = AktToken(in)->stream_type;

   id = DStrAlloc();
   FuncSymbParse(in, id);

   res = SigFindFCode(sig, DStrView(id));

   if(!res)
   {
      errpos = DStrAlloc();

      DStrAppendStr(errpos, PosRep(type, source_name, line, column));
      DStrAppendChar(errpos, ' ');
      DStrAppendStr(errpos, DStrView(id));
      DStrAppendStr(errpos, " undeclared!");
      Error(DStrView(errpos), SYNTAX_ERROR);
      DStrFree(errpos);
   }
   DStrReleaseRef(source_name);
   DStrFree(id);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: SigParseSymbolDeclaration()
//
//   Parse a single symbol declaration (f:3) and insert it into sig.
//
// Global Variables: -
//
// Side Effects    : Changes sig, reads input, may cause error
//
/----------------------------------------------------------------------*/

FunCode SigParseSymbolDeclaration(Scanner_p in, Sig_p sig, bool special_id)
{
   int        arity, line, column;
   DStr_p     id = DStrAlloc(), source_name, errpos;
   FunCode    res;
   StreamType type;

   line = AktToken(in)->line;
   column = AktToken(in)->column;
   source_name = DStrGetRef(AktToken(in)->source);
   type        = AktToken(in)->stream_type;

   FuncSymbParse(in, id);
   AcceptInpTok(in, Colon);
   arity = AktToken(in)->numval;
   AcceptInpTok(in, PosInt);

   res = SigInsertId(sig, DStrView(id), arity, special_id);
   if(!res)
   {
      errpos = DStrAlloc();

      DStrAppendStr(errpos, PosRep(type, source_name, line, column));
      DStrAppendChar(errpos, ' ');
      DStrAppendStr(errpos, DStrView(id));
      DStrAppendStr(errpos, " declared with arity ");
      DStrAppendInt(errpos, (long)arity);
      DStrAppendStr(errpos, " but registered with arity ");
      DStrAppendInt(errpos,
                    (long)SigFindArity(sig, SigFindFCode(sig, DStrView(id))));
      Error(DStrView(errpos), SYNTAX_ERROR);
      DStrFree(errpos);
   }
   DStrReleaseRef(source_name);
   DStrFree(id);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: SigParse()
//
//   Parse a list of declarations into a signature.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

FunCode SigParse(Scanner_p in, Sig_p sig, bool special_ids)
{
   FunCode res = 0;

   /* FIXME: Cannot handle complex symbols here! */
   while(TestInpTok(in, FuncSymbToken) &&
         TestTok(LookToken(in, 1), Colon))
   {
      res = SigParseSymbolDeclaration(in, sig, special_ids);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: SigFindMaxUsedArity()
//
//   Return the largest arity of any function symbol used in the
//   signature.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int SigFindMaxUsedArity(Sig_p sig)
{
   int     res = 0;
   FunCode i;

   for(i=1; i<=sig->f_count; i++)
   {
      res = MAX(res, SigFindArity(sig, i));
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: SigFindMaxPredicateArity()
//
//   Return the largest arity of any predicate function symbol used in
//   the signature.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int SigFindMaxPredicateArity(Sig_p sig)
{
   FunCode i;
   int res=0, arity;

   for(i=sig->internal_symbols+1; i<=sig->f_count; i++)
   {
      if(SigIsPredicate(sig, i) &&
         !SigQueryFuncProp(sig, i, FPSpecial))
      {
         arity = SigFindArity(sig,i);
         res = MAX(res,arity);
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: SigFindMinPredicateArity()
//
//   Return the smallest arity of any predicate function symbol used in
//   the signature.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int SigFindMinPredicateArity(Sig_p sig)
{
   FunCode i;
   int res=INT_MAX, arity;

   for(i=sig->internal_symbols+1; i<=sig->f_count; i++)
   {
      if(SigIsPredicate(sig, i) &&
         !SigQueryFuncProp(sig, i, FPSpecial))
      {
         arity = SigFindArity(sig,i);
         res = MIN(res,arity);
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: SigFindMaxFunctionArity()
//
//   Return the largest arity of any real function symbol used in the
//   signature.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int SigFindMaxFunctionArity(Sig_p sig)
{
   FunCode i;
   int res=0, arity;

   for(i=sig->internal_symbols+1; i<=sig->f_count; i++)
   {
      if(!SigIsPredicate(sig, i) && !SigQueryFuncProp(sig, i, FPSpecial))
      {
         arity = SigFindArity(sig,i);
         res = MAX(res,arity);
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: SigFindMinFunctionArity()
//
//   Return the smallest arity of any real function symbol used in the
//   signature.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int SigFindMinFunctionArity(Sig_p sig)
{
   FunCode i;
   int res=INT_MAX, arity;

   for(i=sig->internal_symbols+1; i<=sig->f_count; i++)
   {
      if(!SigIsPredicate(sig, i) && !SigQueryFuncProp(sig, i, FPSpecial))
      {
         arity = SigFindArity(sig,i);
         res = MIN(res,arity);
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: SigCountAritySymbols()
//
//   Count number of symbols with a given arity. If predictates is
//   true, count predicates, otherwise count function symbols.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int SigCountAritySymbols(Sig_p sig, int arity, bool predicates)
{
   FunCode i;
   int res=0, tmp_arity;

   for(i=sig->internal_symbols+1; i<=sig->f_count; i++)
   {
      if(EQUIV(SigIsPredicate(sig, i), predicates)
         &&(!SigIsSpecial(sig,i)))
      {
         tmp_arity = SigFindArity(sig,i);
         if(tmp_arity==arity)
         {
            res++;
         }
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: SigCountSymbols()
//
//   Count number of symbols. If predictates is
//   true, count predicates, otherwise count function symbols.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int SigCountSymbols(Sig_p sig, bool predicates)
{
   FunCode i;
   int res=0;

   for(i=sig->internal_symbols+1; i<=sig->f_count; i++)
   {
      if(!SigIsSpecial(sig,i))
      {
         if(predicates && SigIsPredicate(sig, i))
         {
            res++;
         }
         else if(!predicates && SigIsFunction(sig, i))
         {
            res++;
         }
      }
   }
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: SigAddSymbolArities()
//
//   Count the occurences of symbols of a given arity (by adding one
//   for each symbol to the corresponding entry in distrib). If
//   predicates is true, count predicate symbols only, otherwise count
//   function symbols only. Only looks at symbols where select[symbol]
//   is true. Return maximal arity of relevant symbols.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int SigAddSymbolArities(Sig_p sig, PDArray_p distrib, bool predicates,
                        long selection[])
{
   FunCode i;
   int     max_arity = -1, arity;

   for(i = 1; i<=sig->f_count; i++)
   {
      if(EQUIV(SigIsPredicate(sig, i), predicates) &&
         selection[i])
      {
         arity = SigFindArity(sig, i);
         max_arity = MAX(arity, max_arity);
         PDArrayElementIncInt(distrib, arity, 1);
      }
   }
   return max_arity;
}


/*-----------------------------------------------------------------------
//
// Function: SigCollectSortConsts()
//
//   Collect all constant symbols with the given sort onto
//   res. Untyped symbols are assume to be type STIndividuals. Return
//   number of constants found.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long SigCollectSortConsts(Sig_p sig, Type_p sort, PStack_p res)
{
   FunCode   i;
   long      rescount = 0;
   Type_p    f_sort;

   for(i = sig->internal_symbols+1; i<=sig->f_count; i++)
   {
      if(SigFindArity((sig), i) == 0)
      {
         f_sort = sig->type_bank->i_type;
         if(sig->f_info[i].type)
         {
            f_sort = sig->f_info[i].type;
         }
         if(f_sort==sort)
         {
            /* printf("Collecting constant %ld (%s) for sort %d\n", */
            /*        i, SigFindName(sig, i), sort); */
            PStackPushInt(res, i);
            rescount++;
         }
      }
   }
   return rescount;
}


/*-----------------------------------------------------------------------
//
// Function: SigGetOrNCode()
//
//   Return FunCode for $orn, create them if non-existant.
//
// Global Variables: -
//
// Side Effects    : May change sig
//
/----------------------------------------------------------------------*/

FunCode SigGetOrNCode(Sig_p sig, int arity)
{
   FunCode res;

   assert(sig);

   if(!sig->orn_codes)
   {
      sig->orn_codes = PDArrayAlloc(10,10);
   }
   res = PDArrayElementInt(sig->orn_codes, arity);

   if(res)
   {
      return res;
   }
   {
      char tmp_str[16]; /* large enough for "or" + digits of INT_MAX */

      sprintf(tmp_str, "$or%d", arity);
      res = SigInsertId(sig, tmp_str, arity, true);
      assert(res);
      PDArrayAssignInt(sig->orn_codes, arity, res);
      return res;
   }
}


/*-----------------------------------------------------------------------
//
// Function: SigGetOtherEqnCode()
//
//   If eqn_code is passed in, return neqn_code, and vice
//   versa. Assumes FOF-initialized signature.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

FunCode SigGetOtherEqnCode(Sig_p sig, FunCode f_code)
{
   if(f_code == sig->eqn_code)
   {
      return sig->neqn_code;
   }
   assert(f_code == sig->neqn_code);
   return sig->eqn_code;
}



/*-----------------------------------------------------------------------
//
// Function: SigGetNewSkolemCode()
//
//   Return a new skolem symbol with arity n. The symbol will be of
//   the form esk<count>_<ar>, and is guaranteed to be new to sig.
//
// Global Variables: -
//
// Side Effects    : Extends signature
//
/----------------------------------------------------------------------*/

FunCode SigGetNewSkolemCode(Sig_p sig, int arity)
{
   FunCode res;
   char    new_symbol[48];

   sig->skolem_count++;
   sprintf(new_symbol,"esk%ld_%d",sig->skolem_count,arity);
   while(SigFindFCode(sig,new_symbol))
   {
      sig->skolem_count++;
      sprintf(new_symbol,"esk%ld_%d",sig->skolem_count,arity);
   }
   res = SigInsertId(sig, new_symbol, arity, false);
   SigSetFuncProp(sig, res, FPIsSkolemSymbol);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: SigGetNewPredicateCode()
//
//   Return a new predicate symbol with arity n. The symbol will be of
//   the form epred<count>_<ar>, and is guaranteed to be new to sig.
//
// Global Variables: -
//
// Side Effects    : Extends signature
//
/----------------------------------------------------------------------*/

FunCode SigGetNewPredicateCode(Sig_p sig, int arity)
{
   FunCode res;
   char    new_symbol[48];

   sig->newpred_count++;
   sprintf(new_symbol,"epred%ld_%d",sig->newpred_count,arity);
   while(SigFindFCode(sig,new_symbol))
   {
      sig->newpred_count++;
      sprintf(new_symbol,"epred%ld_%d",sig->newpred_count,arity);
   }
   res = SigInsertId(sig, new_symbol, arity, false);
   SigSetFuncProp(sig, res, FPDefPred);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: SigDeclareType()
//
//   Declare the type of the given function. Will fail (and crash) if
//   the type is already declared and is fixed.
//
// Global Variables: -
//
// Side Effects    : Modifies the signature
//
/----------------------------------------------------------------------*/

void SigDeclareType(Sig_p sig, FunCode f, Type_p type)
{
   Func_p fun;

   fun = &sig->f_info[f];
   if(fun->type)
   {
      /* type already declared, check it's the same */
      if(fun->type != type)
      {
         if(SigIsFixedType(sig, f))
         {
            if(Verbose>=3)
            {
               fprintf(stderr, "# Type conflict for %s between ",
                       SigFindName(sig, f));
               TypePrintTSTP(stderr, sig->type_bank, fun->type);
               fprintf(stderr, " and ");
               TypePrintTSTP(stderr, sig->type_bank, type);
               fprintf(stderr, "\n");
            }
            Error("type error", INPUT_SEMANTIC_ERROR);
         }
         else
         {
            if(Verbose >= 2)
            {
               fprintf(stderr, "# type re-declaration %s: ", SigFindName(sig, f));
               TypePrintTSTP(stderr, sig->type_bank, type);
               fprintf(stderr, "\n");
            }
            fun->type = type;
         }
      }
   }
   else
   {
      if(Verbose >= 2)
      {
         fprintf(stderr, "# type declaration %s: ", SigFindName(sig, f));
         TypePrintTSTP(stderr, sig->type_bank, type);
         fprintf(stderr, "\n");
      }
      fun->type = type;
   }
}


/*-----------------------------------------------------------------------
//
// Function: SigDeclareFinalType()
//
//   Declare the type of the symbol, and fix it (cannot be changed)
//
// Global Variables: -
//
// Side Effects    : Modifies the type table
//
/----------------------------------------------------------------------*/

void SigDeclareFinalType(Sig_p sig, FunCode f_code, Type_p type)
{
   SigDeclareType(sig, f_code, type);
   SigFixType(sig, f_code);
}


/*-----------------------------------------------------------------------
//
// Function: SigDeclareIsFunction()
//
//  This symbol occurs in a function position (in an equation, as
//  a subterm...).
//
// Global Variables: -
//
// Side Effects    : Modifies signature (may change the type)
//
/----------------------------------------------------------------------*/

void SigDeclareIsFunction(Sig_p sig, FunCode f_code)
{
   Type_p type, new_type;

   if(SigIsPolymorphic(sig, f_code))
   {
      return;
   }

   type = SigGetType(sig, f_code);
   assert(type);

   /* must update type */
   if(TypeIsBool(type))
   {
      new_type = TypeChangeReturnType(sig->type_bank, type, SigDefaultSort(sig));
      assert(new_type->type_uid != INVALID_TYPE_UID);

      SigDeclareFinalType(sig, f_code, new_type);
   }
   else
   {
      SigFixType(sig, f_code);
   }
}


/*-----------------------------------------------------------------------
//
// Function: SigDeclareIsPredicate()
//
//   This symbol occurs as a predicate, without ambiguity.
//
// Global Variables: -
//
// Side Effects    : Modifies signature
//
/----------------------------------------------------------------------*/
void SigDeclareIsPredicate(Sig_p sig, FunCode f_code)
{
   Type_p type, new_type;

   if(SigIsPolymorphic(sig, f_code))
   {
      return;
   }

   type = SigGetType(sig, f_code);
   assert(type);

   /* must update type */
   if(!TypeIsBool(type))
   {
      new_type = TypeChangeReturnType(sig->type_bank, type, sig->type_bank->bool_type);
      SigDeclareFinalType(sig, f_code, new_type);
   }
   else
   {
      SigFixType(sig, f_code);
   }
}


/*-----------------------------------------------------------------------
//
// Function: SigPrintTypes()
//
//   Prints symbols with their type to the given file descriptor.
//
//
// Global Variables: -
//
// Side Effects    : IO on the file descriptor
//
/----------------------------------------------------------------------*/

void SigPrintTypes(FILE* out, Sig_p sig)
{
   FunCode i;
   Func_p fun;

   for(i=1; i <= sig->f_count; i++)
   {
      if (i > 1)
      {
         fputs(", ", out);
      }

      fun = &sig->f_info[i];
      fprintf(out, "%s:", fun->name);
      if (!fun->type)
      {
         fputs("<no type>", out);
      }
      else
      {
         TypePrintTSTP(out, sig->type_bank, fun->type);
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: SigPrintTypeDeclsTSTP()
//
//   Print TPTP-3 type declarations for all real symbols.
//
//
// Global Variables: -
//
// Side Effects    : IO on the file descriptor
//
/----------------------------------------------------------------------*/

void SigPrintTypeDeclsTSTP(FILE* out, Sig_p sig)
{
   FunCode i;
   Func_p fun;
   const char* tag = problemType == PROBLEM_HO ? "thf" : "tff";

   for(i=sig->internal_symbols+1; i <= sig->f_count; i++)
   {
      fun = &sig->f_info[i];
      if (fun->type)
      {
         fprintf(out, "%s(decl_%ld, type, %s: ", tag, i, fun->name);
         TypePrintTSTP(out, sig->type_bank, fun->type);
         fprintf(out, ").\n");
      }
   }
}




/*-----------------------------------------------------------------------
//
// Function: SigParseTFFTypeDeclaration()
//
//    Parses a type declaration, and update the signature if it is a
//    symbol declaration (ignores type declarations)
//
// Global Variables: -
//
// Side Effects    : Reads from scanner, may raise an error
//
/----------------------------------------------------------------------*/

void SigParseTFFTypeDeclaration(Scanner_p in, Sig_p sig)
{
   DStr_p id;
   FuncSymbType symbtype;
   FunCode f;
   Type_p type;
   bool within_paren = false;

   id = DStrAlloc();

   if(TestInpTok(in, OpenBracket))
   {
      NextToken(in);
      within_paren = true;
   }

   /* parse symbol */
   symbtype = FuncSymbParse(in, id);
   if(symbtype == FSIdentVar || symbtype == FSNone)
   {
      Error("expected symbol in type declaration", OTHER_ERROR);
   }

   /* parse type */
   AcceptInpTok(in, Colon);
   type = TypeBankParseType(in, sig->type_bank);

   if(within_paren)
   {
      AcceptInpTok(in, CloseBracket);
   }

   /* we only keep declarations of symbols, not declaration of types */
   if(!TypeIsTypeConstructor(type))
   {
      int arity = TypeIsArrow(type) ? type->arity - 1 : 0;
      f = SigInsertId(sig, DStrView(id), arity, false);

      SigDeclareType(sig, f, type);
      SigFixType(sig, f);
   }
   else
   {
      if(TypeIsArrow(type))
      {
         TypeBankDefineTypeConstructor(sig->type_bank, DStrView(id), type->arity-1);
      }
      else
      {
         TypeBankDefineSimpleSort(sig->type_bank, DStrView(id));
      }
   }

   DStrFree(id);
}

/*-----------------------------------------------------------------------
//
// Function: SigHasUnimplementedInterpretedSymbols()
//
//   Return true if there are uninterpreted interpreted symbols in the
//   signature (in which case the prover is incomplete).
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

bool SigHasUnimplementedInterpretedSymbols(Sig_p sig)
{
   FunCode i;

   for(i=sig->internal_symbols+1; i<=sig->f_count; i++)
   {
      if(FuncQueryProp(&(sig->f_info[i]), FPInterpreted))
      {
         return true;
      }
   }
   return false;
}

/*-----------------------------------------------------------------------
//
// Function: SigUpdateFeatureOffset()
//
//    Update the feature index of the symbol.
//
//    Index is min(arity, SIG_FEATURE_ARITY_LIMIT-1) for predicate
//    symbols,
//    SIG_FEATURE_ARITY_LIMIT+min(arity, SIG_FEATURE_ARITY_LIMIT-1)
//    for  function symbols.
//
// Global Variables: -
//
// Side Effects    : As above
//
/----------------------------------------------------------------------*/

void SigUpdateFeatureOffset(Sig_p sig, FunCode f)
{
   int feature_arity = MIN(SIG_FEATURE_ARITY_LIMIT-1, SigFindArity(sig, f));

   if(SigIsPredicate(sig, f))
   {
      sig->f_info[f].feature_offset = feature_arity+SIG_FEATURE_ARITY_LIMIT;
   }
   else
   {
      sig->f_info[f].feature_offset = feature_arity;
   }
}

/*-----------------------------------------------------------------------
//
// Function: SigGetTypedApp()
//
//    Gets the symbol that corresponds term application of type
//    (arg1 * arg2) > ret. This roughly corresponds to higher-order
//    type (t1 -> t2) -> t1 -> t2, so the invariant is arg1 == t1 -> t2,
//    arg2 == t1, ret = t2.
//
// Global Variables: -
//
// Side Effects    : As above
//
/----------------------------------------------------------------------*/

FunCode SigGetTypedApp(Sig_p sig, Type_p arg1, Type_p arg2, Type_p ret)
{
   DStr_p typed_app_name = DStrAlloc();

   DStrAppendStr(typed_app_name, "app_");
   DStrAppendInt(typed_app_name, arg1->type_uid);
   DStrAppendChar(typed_app_name, '_');
   DStrAppendInt(typed_app_name, arg2->type_uid);
   DStrAppendChar(typed_app_name, '_');
   DStrAppendInt(typed_app_name, ret->type_uid);


   Type_p* arr = TypeArgArrayAlloc(3);
   arr[0] = arg1;
   arr[1] = arg2;
   arr[2] = ret;

   Type_p app_type = AllocArrowType(3, arr);

   FunCode ret_fcode = SigInsertId(sig, DStrView(typed_app_name), 2, false);
   if(!sig->f_info[ret_fcode].type)
   {
      sig->f_info[ret_fcode].type = app_type;
   }
   else
   {
      TypeFree(app_type);
   }
   SigSetFuncProp(sig, ret_fcode, FPTypedApplication);

   DStrFree(typed_app_name);

   return ret_fcode;
}

/*-----------------------------------------------------------------------
//
// Function: SigPrintAppEncodedDecls()
//
//    Prints type declarations that correspond to app-encoded terms.
//
// Global Variables: -
//
// Side Effects    : As above
//
/----------------------------------------------------------------------*/

void SigPrintAppEncodedDecls(FILE* out, Sig_p sig)
{
   for(FunCode i=sig->internal_symbols+1; i <= sig->f_count; i++)
   {
      fprintf(out, "tff(symboltypedecl%ld, type, %s: ",
                      (i+1)-sig->internal_symbols, SigFindName(sig, i));
      if (SigQueryFuncProp(sig, i, FPTypedApplication))
      {
         Type_p t = SigGetType(sig, i);
         DStr_p left = TypeAppEncodedName(t->args[0]);
         DStr_p right = TypeAppEncodedName(t->args[1]);
         DStr_p ret = TypeAppEncodedName(t->args[2]);


         fprintf(out, "(%s * %s) > %s",
                         DStrView(left), DStrView(right), DStrView(ret));

         DStrFree(left);
         DStrFree(right);
         DStrFree(ret);
      }
      else
      {
         Type_p t = SigGetType(sig, i);
         DStr_p typename = TypeAppEncodedName(t);

         fprintf(out, "%s", DStrView(typename));
      }
      fprintf(out, ").\n");
   }

}

/*-----------------------------------------------------------------------
//
// Function: SigSymbolUnifiesWithVar()
//
//   Checks whether f_code can be unified with a variable. In HO
//   case variable unifies with any function code; in FOOL case
//   variable unifies with $true, $false and non-predicate symbols.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool SigSymbolUnifiesWithVar(Sig_p sig, FunCode f_code)
{
   assert(sig);
   assert(f_code);

   return problemType == PROBLEM_HO ||
          f_code == SIG_TRUE_CODE || f_code == SIG_FALSE_CODE ||
          f_code <= 0 ||
          !SigIsPredicate(sig,f_code);
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
