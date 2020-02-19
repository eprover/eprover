/*-----------------------------------------------------------------------

File  : ccl_clause_abstraction.c

Author: Constantin Ruhdorfer

Contents

  Interface for indexing clauses for subsumption.

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
// Function: RewriteConstantsOnTerm()
//
//   Recursively traves a term and rewrites its constants to rc.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
Term_p RewriteConstantsOnTerm(Term_p source, VarBank_p vars, DerefType deref,
                              PDArray_p constant_sorts)
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
      handle = RewriteConstantsOnTermCell(source, constant_sorts);

      for(i=0; i<handle->arity; i++)
      {
         handle->args[i] = RewriteConstantsOnTerm(source->args[i], vars,
                                                  CONVERT_DEREF(i, limit, deref),
                                                  constant_sorts);
      }
   }

   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: RewriteConstantsOnTermCell()
//
//   Allocates a new Term that is either a copy of source or if term is
//   a constant inserts rc when it exists.
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
      sort     = GetReturnSort(source->type);
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

   TermSetBank(handle, TermGetBank(source));

   return handle;
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: RewriteConstants()
//
//   Takes a clause and rewrites als constants to rc where rc is the
//   first constant met during the proof.
//
// Global Variables: -
//
// Side Effects    : Rewrites constants in clause ;-).
//
/----------------------------------------------------------------------*/
void RewriteConstants(Clause_p clause, TB_p target, PDArray_p constant_sorts) 
{
   Eqn_p next;
   Eqn_p literals = clause->literals;
   while(literals)
   {
      next = literals->next;
      literals->lterm = RewriteConstantsOnTerm(literals->lterm, 
                                               literals->bank->vars, 
                                               false,
                                               constant_sorts);
      literals->lterm = TBInsert(target, literals->lterm, DEREF_ALWAYS);
      literals->rterm = RewriteConstantsOnTerm(literals->rterm, 
                                               literals->bank->vars, 
                                               false,
                                               constant_sorts);
      literals->rterm = TBInsert(target, literals->rterm, DEREF_ALWAYS);
      literals       = next;
   }
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
