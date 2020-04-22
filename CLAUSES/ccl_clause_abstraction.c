/*-----------------------------------------------------------------------

File  : ccl_clause_abstraction.c

Author: Constantin Ruhdorfer

Contents

  Functions for clause abstraction.

Copyright 2019-2020 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

-----------------------------------------------------------------------*/

#include <ccl_clause_abstraction.h>

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: RewriteSymbolsOnTerm()
//
//   Recursively traves a term and rewrites its constants to the appropiate
//   constant symbol given its sort.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
Term_p RewriteSymbolsOnTerm(Term_p source, VarBank_p vars, DerefType deref,
                            PDArray_p look_up, AbstractionMode mode, Sig_p sig)
{
   Term_p handle;
   int    i;

   assert(source);

   const int limit = DEREF_LIMIT(source, deref);
   source          = TermDeref(source, &deref);

   if(TermIsVar(source))
   {
      handle = VarBankVarAssertAlloc(vars, source->f_code, source->type);
   }
   else
   {
      switch (mode)
      {
      case CONSTANT:
         handle = RewriteConstantsOnTermCell(source, look_up);
         break;
      case SKOLEM:
         handle = RewriteSkolemsOnTermCell(source, look_up, sig);
         break;
      default:
         break;
      }

      for(i=0; i<handle->arity; i++)
      {
         handle->args[i] = RewriteSymbolsOnTerm(source->args[i], vars,
                                                CONVERT_DEREF(i, limit, deref),
                                                look_up, mode, sig);
      }
   }

   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: RewriteConstantsOnTermCell()
//
//   Allocates a new Term that is either a copy of source or if term is
//   a constant inserts the appropiate constant symbol given its sort 
//   if it exists.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
Term_p RewriteConstantsOnTermCell(Term_p source, PDArray_p constant_sorts) 
{
   Term_p handle = TermDefaultCellAlloc();
   Type_p sort   = NULL;

   handle->properties = (source->properties&(TPPredPos));
   TermCellDelProp(handle, TPOutputFlag);

   if(source->arity)
   {
      handle->arity = source->arity;
      handle->args  = TermArgArrayAlloc(source->arity);
   }

   if(source->arity==0 && !TermIsVar(source))
   {
      sort     = source->type;
      long res = PDArrayElementInt(constant_sorts, sort->f_code);

      if (res == constant_sorts->default_int)
      {
         PDArrayAssignInt(constant_sorts, sort->f_code, source->f_code);
         handle->f_code = source->f_code;
      }
      else 
      {
         handle->f_code = res;
      }
   }
   else
   {
      handle->f_code = source->f_code;
   }

   handle->type = source->type;
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: RewriteSkolemsOnTermCell()
//
//   Allocates a new Term that is either a copy of source or if term is
//   a skolem symbol inserts the appropiate symbol given its arity 
//   if it exists.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
Term_p RewriteSkolemsOnTermCell(Term_p source, PDArray_p skolem_types, 
                                Sig_p sig)
{
   long index_pos = 0;
   long res       = 0;
   Term_p handle  = TermDefaultCellAlloc();

   handle->properties = (source->properties&(TPPredPos));
   TermCellDelProp(handle, TPOutputFlag);

   if(source->arity)
   {
      handle->arity = source->arity;
      handle->args  = TermArgArrayAlloc(source->arity);
   }

   if(SigQueryFuncProp(sig, source->f_code, FPIsSkolemSymbol))
   {
      index_pos = SigGetType(sig, source->f_code)->type_uid;
      res       = PDArrayElementInt(skolem_types, index_pos);

      if (res == skolem_types->default_int)
      {
         PDArrayAssignInt(skolem_types, index_pos, source->f_code);
         handle->f_code = source->f_code;
      }
      else 
      {
         handle->f_code = res;
      }
   }
   else
   {
      handle->f_code = source->f_code;
   }

   handle->type = source->type;
   return handle;
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: RewriteConstants()
//
//   Takes a clause and rewrites all constants to rc where rc is the
//   first constant met during the proof.
//
// Global Variables: -
//
// Side Effects    : Rewrites constants in clause ;-).
//
/----------------------------------------------------------------------*/
void RewriteConstants(Clause_p clause, TB_p target, PDArray_p constant_sorts) 
{
   Eqn_p  next;
   Eqn_p  literals = clause->literals;
   Term_p newL;
   Term_p newR;

   while(literals)
   {
      next = literals->next;
      newL = RewriteSymbolsOnTerm(literals->lterm, 
                                  literals->bank->vars, 
                                  false,
                                  constant_sorts,
                                  CONSTANT,
                                  NULL);
      literals->lterm = TBInsert(target, newL, DEREF_ALWAYS);
      TermFree(newL);
      newR = RewriteSymbolsOnTerm(literals->rterm, 
                                  literals->bank->vars, 
                                  false,
                                  constant_sorts,
                                  CONSTANT,
                                  NULL);
      literals->rterm = TBInsert(target, newR, DEREF_ALWAYS);
      TermFree(newR);
      literals        = next;
   }
}


/*-----------------------------------------------------------------------
//
// Function: RewriteSkolemSymbols()
//
//   Takes a clause and rewrites all Skolem symbols to rc where rc is the
//   first constant met during the proof.
//
// Global Variables: -
//
// Side Effects    : Rewrites skolem symbols in clause ;-).
//
/----------------------------------------------------------------------*/
void RewriteSkolemSymbols(Clause_p clause, TB_p target,
                          PDArray_p skolem_sym_lookup, Sig_p sig)
{
   Eqn_p  next;
   Eqn_p  literals = clause->literals;
   Term_p newL;
   Term_p newR;

   while(literals)
   {
      next = literals->next;
      newL = RewriteSymbolsOnTerm(literals->lterm, 
                                  literals->bank->vars, 
                                  false,
                                  skolem_sym_lookup,
                                  SKOLEM,
                                  sig);
      literals->lterm = TBInsert(target, newL, DEREF_ALWAYS);
      TermFree(newL);
      newR = RewriteSymbolsOnTerm(literals->rterm, 
                                  literals->bank->vars, 
                                  false,
                                  skolem_sym_lookup,
                                  SKOLEM,
                                  sig);
      literals->rterm = TBInsert(target, newR, DEREF_ALWAYS);
      TermFree(newR);
      literals        = next;
   }
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
