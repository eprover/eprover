/*-----------------------------------------------------------------------

File  : ccl_efficient_subsumption_index.c

Author: Constantin Ruhdorfer

Contents

  Interface for indexing clauses for subsumption.

Copyright 2019-2020 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

-----------------------------------------------------------------------*/

#include <ccl_efficient_subsumption_index.h>

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
      handle = RewriteConstantsAndCopyTerm(source);

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

/*-----------------------------------------------------------------------
//
// Function: EfficientSubsumptionIndexInsertClause()
//
//   Inserts a clause into the watchlists indexes.
//   This function determines the appropiate indexes for the clause.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
void EfficientSubsumptionIndexInsert(EfficientSubsumptionIndex_p index, 
                                    FVPackedClause_p newclause)
{
   if(index->unitclasue_index && ClauseIsUnit(newclause->clause))
   {
      UnitclauseIndexInsertClause(index->unitclasue_index, newclause->clause);
   } 
   else if (index->fvindex) 
   {
      FVIndexInsert(index->fvindex, newclause);
   }
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: EfficientSubsumptionIndexAlloc()
//
//   Allocate an empty EfficientSubsumptionIndex.
//
// Global Variables: -
//
// Side Effects    : Memory management
//
/----------------------------------------------------------------------*/
EfficientSubsumptionIndex_p EfficientSubsumptionIndexAlloc(FVCollect_p cspec, 
                                                         PermVector_p perm)
{
   EfficientSubsumptionIndex_p handle = EfficientSubsumptionIndexAllocRaw();
   handle->fvindex                   = FVIAnchorAlloc(cspec, perm);
   handle->unitclasue_index          = NULL;
   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: EfficientSubsumptionIndexFree()
//
//   Delete all index datastructures.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/
void EfficientSubsumptionIndexFree(EfficientSubsumptionIndex_p index)
{
   if (index->fvindex) 
   {
      FVIAnchorFree(index->fvindex);
   }
   if (index->unitclasue_index)
   {
      FPIndexFree(index->unitclasue_index);
   }
   EfficientSubsumptionIndexFreeRaw(index);
}

/*-----------------------------------------------------------------------
//
// Function: EfficientSubsumptionIndexUnitClauseIndexInit()
//
//   Initialize the unit clause index as required by the parameters.
//
// Global Variables: -
//
// Side Effects    : Memory management
//
/----------------------------------------------------------------------*/
void EfficientSubsumptionIndexUnitClauseIndexInit(EfficientSubsumptionIndex_p index,
                                                 Sig_p sig, 
                                                 char* unitclause_index_type)
{
   FPIndexFunction indexfun;
   indexfun                = GetFPIndexFunction(unitclause_index_type);
   index->unitclasue_index = FPIndexAlloc(indexfun, sig, UnitclauseIndexFreeWrapper);
}

/*-----------------------------------------------------------------------
//
// Function: EfficientSubsumptionIndexInsertClause()
//
//   Inserts a clause into the efficient subsumption index.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
void EfficientSubsumptionIndexInsertClause(EfficientSubsumptionIndex_p index, 
                                          Clause_p clause)
{
   FVPackedClause_p pclause = FVIndexPackClause(clause, index->fvindex);
   assert(clause->weight == ClauseStandardWeight(clause));
   EfficientSubsumptionIndexInsert(index, pclause);
   FVUnpackClause(pclause);
}

/*-----------------------------------------------------------------------
//
// Function: ClausesetIndexDeleteEntry()
//
//   Deletes a clause from the watchlists indexes inserted by 
//   ClausesetIndexInsertNewClause; E.g. This function determines
//   the appropiate indexes for the clause.
//
// Global Variables: -
//
// Side Effects    : Changes index
//
/----------------------------------------------------------------------*/
Clause_p ClausesetIndexDeleteEntry(EfficientSubsumptionIndex_p index, 
                                   Clause_p junk)
{
   if(index->unitclasue_index && ClauseIsUnit(junk))
   {
      UnitclauseIndexDeleteClause(index->unitclasue_index, 
                                  junk);
   }
   if (index->fvindex) 
   {
      FVIndexDelete(index->fvindex, junk);
   }
   return junk;
}

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
   // printf("Original: ");
   // ClausePrint(stdout, clause, true);
   // printf("\n");
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
   // printf("Rewritten: ");
   // ClausePrint(stdout, clause, true);
   // printf("\n");
}