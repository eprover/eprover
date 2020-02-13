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

static REWRITE_CONSTANT rc = -1;

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
Term_p RewriteConstantsOnTerm(Term_p source, VarBank_p vars, DerefType deref)
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
      handle = RewriteConstantsOnTermCell(source);

      for(i=0; i<handle->arity; i++)
      {
         handle->args[i] = RewriteConstantsOnTerm(source->args[i], vars,
                                                  CONVERT_DEREF(i, limit, deref));
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
Term_p RewriteConstantsOnTermCell(Term_p source) 
{
   Term_p handle = TermDefaultCellAlloc();

   handle->properties = (source->properties&(TPPredPos));
   TermCellDelProp(handle, TPOutputFlag);

   if(source->arity)
   {
      handle->arity = source->arity;
      handle->args  = TermArgArrayAlloc(source->arity);
   }

   if(source->arity==0 && !TermIsVar(source))
   {
      if(rc == -1)
      {
         rc = source->f_code;
         handle->f_code = source->f_code;
      }
      else 
      {
         handle->f_code = rc;
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
// Side Effects    : Changes clause ;-).
//
/----------------------------------------------------------------------*/
void RewriteConstants(Clause_p clause) 
{
   Eqn_p next;
   Eqn_p literals = clause->literals;
   while(literals)
   {
      next = literals->next;
      literals->lterm = RewriteConstantsOnTerm(literals->lterm, 
                                               literals->bank->vars, false);
      literals->rterm = RewriteConstantsOnTerm(literals->rterm, 
                                               literals->bank->vars, false);
      literals = next;
   }
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
