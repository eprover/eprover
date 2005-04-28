/*-----------------------------------------------------------------------

File  : cte_signature.c

Author: Stephan Schulz

Contents

  Functions implementing the signature functionality.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Sun Sep 21 19:27:54 MET DST 1997
    New

-----------------------------------------------------------------------*/

#include "cte_signature.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

bool      SigSupportLists = false; 
TokenType SigIdentToken   = Identifier | PosInt | SemIdent | SQString | String;

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
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: SigAlloc()
//
//   Allocate a initialized signature cell.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Sig_p SigAlloc(void)
{
   Sig_p handle;

   handle = SigCellAlloc();

   handle->size    = DEFAULT_SIGNATURE_SIZE;
   handle->f_count = 0;
   handle->f_info  =
      SecureMalloc(sizeof(FuncCell)*DEFAULT_SIGNATURE_SIZE); 
   handle->f_index = NULL;
   handle->ac_axioms = PStackAlloc();

   SigInsertId(handle, "$true", 0, true);
   assert(SigFindFCode(handle, "$true")==SIG_TRUE_CODE);  
   SigSetFuncProp(handle, SIG_TRUE_CODE, FPPredSymbol|FPInterpreted);
   SigInsertId(handle, "$false", 0, true);
   assert(SigFindFCode(handle, "$false")==SIG_FALSE_CODE);  
   SigSetFuncProp(handle, SIG_FALSE_CODE, FPPredSymbol|FPInterpreted);
   
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
   handle->or_code       = 0;
   handle->cnil_code     = 0;
   handle->orn_codes     = NULL;
   handle->skolem_count  = 0;
   handle->newpred_count = 0;
   handle->null_code     = 0;
   handle->succ_code     = 0;

   handle->distinct_props = FPDistinctProp;
   return handle;
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
// Function: SigSetPredicate()
//
//   Set the value of the predicate field for a function symbol.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void SigSetPredicate(Sig_p sig, FunCode f_code, bool value)
{
   assert(f_code > 0);
   assert(f_code <= sig->f_count);

   if(value)
   {
      FuncSetProp(&(sig->f_info[f_code]),FPPredSymbol);
   }
   else
   {
      FuncDelProp(&(sig->f_info[f_code]),FPPredSymbol);
   }
}

/*-----------------------------------------------------------------------
//
// Function: SigIsPredicate()
//
//   Return the value of the predicate field for a function symbol.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool SigIsPredicate(Sig_p sig, FunCode f_code)
{
   assert(f_code > 0);
   assert(f_code <= sig->f_count);

   return FuncQueryProp(&(sig->f_info[f_code]), FPPredSymbol);
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
      sig->size+= DEFAULT_SIGNATURE_SIZE;
      sig->f_info  = SecureRealloc(sig->f_info,
				   sizeof(FuncCell)*sig->size);
   }
   
   /* Insert the element in f_index and f_info */
   sig->f_count++;
   sig->f_info[sig->f_count].name 
      = SecureStrdup(name); 
   sig->f_info[sig->f_count].arity = arity;
   sig->f_info[sig->f_count].properties = FPIgnoreProps;
   new = StrTreeCellAllocEmpty();
   new->key = sig->f_info[sig->f_count].name;
   new->val1.i_val = sig->f_count;

   test = StrTreeInsert(&(sig->f_index), new);
   assert(test == NULL);   
   SigSetSpecial(sig,sig->f_count,special_id);
   sig_compute_alpha_ranks(sig);
   return sig->f_count;
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
// Function: SigParseOperator()
//
//   Parse an operator (i.e. an optional $, followed by an
//   identifier), store the representation into id
//
// Global Variables: SigIdentToken
//
// Side Effects    : Reads input, changes id
//
/----------------------------------------------------------------------*/

void SigParseOperator(Scanner_p in, DStr_p id)
{
   DStrAppendStr(id, DStrView(AktToken(in)->literal));
   AcceptInpTok(in, SigIdentToken);
}


/*-----------------------------------------------------------------------
//
// Function: SigParseKnownOperator()
//
//   Parse an operator, return it's FunCode. Error, if operator is not
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
   SigParseOperator(in, id);

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
   
   SigParseOperator(in, id);
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

   while(TestInpTok(in, SigIdentToken) &&
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
      if(SigQueryFuncProp(sig, i, FPPredSymbol) &&
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
      if(SigQueryFuncProp(sig, i, FPPredSymbol) &&
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
      if(!SigIsAnyFuncPropSet(sig, i, FPPredSymbol|FPSpecial))
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
      if(!SigIsAnyFuncPropSet(sig, i, FPPredSymbol|FPSpecial))
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
      if(EQUIV(SigIsPredicate(sig, i), predicates)
	 &&(!SigIsSpecial(sig,i)))
      {
	 res++;
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
   char    new_symbol[24];
   
   sig->skolem_count++;
   sprintf(new_symbol,"esk%ld_%d",sig->skolem_count,arity);
   while(SigFindFCode(sig,new_symbol))
   {
      sig->skolem_count++;
      sprintf(new_symbol,"esk%ld_%d",sig->skolem_count,arity);
   }
   res = SigInsertId(sig, new_symbol, arity, false);
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
   char    new_symbol[26];
   
   sig->newpred_count++;
   sprintf(new_symbol,"epred%ld_%d",sig->newpred_count,arity);
   while(SigFindFCode(sig,new_symbol))
   {
      sig->newpred_count++;
      sprintf(new_symbol,"epred%ld_%d",sig->newpred_count,arity);
   }
   res = SigInsertId(sig, new_symbol, arity, false);
   SigSetFuncProp(sig, res, FPPredSymbol);
   
   return res;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/









