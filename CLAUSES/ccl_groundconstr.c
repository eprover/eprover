/*-----------------------------------------------------------------------

  File  : ccl_groundconstr.c

  Author: Stephan Schulz

  Contents

  Computing constraints on the possible instances of groundable
  clauses.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: Thu Jun  7 23:44:55 MEST 2001

  -----------------------------------------------------------------------*/

#include "ccl_groundconstr.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: LitOccTableAlloc()
//
//   Allocate a LitOccTable suitable for the signature. This wastes
//   some memory, but except for pathological cases, this should be
//   insignificant, and the time efficiency  of the operations should
//   be good.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

LitOccTable_p LitOccTableAlloc(Sig_p sig)
{
   int arity, i, j;

   LitOccTable_p handle = LitOccTableCellAlloc();

   handle->sig_size  = sig->f_count+1; /* To allow for element 0*/
   arity = SigFindMaxPredicateArity(sig);
   handle->maxarity = MAX(arity,2); /* To cover $eqn, which might be
                                       special */
   handle->matrix = SizeMalloc((handle->sig_size)
                               * (handle->maxarity+1)
                               * sizeof(LitConstrCell));
   for(i=0; i< handle->sig_size; i++)
   {
      for(j=0; j<handle->maxarity; j++)
      {
         LIT_OCC_TABLE_ENTRY(handle, i, j).constrained = true;
         LIT_OCC_TABLE_ENTRY(handle, i, j).constraints = NULL;
      }
   }
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: LitOccTableFree()
//
//   Free a LitOccTable.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void LitOccTableFree(LitOccTable_p junk)
{
   int i, j;

   assert(junk);
   assert(junk->matrix);

   for(i=0; i< junk->sig_size; i++)
   {
      for(j=0; j<junk->maxarity; j++)
      {
         PTreeFree(LIT_OCC_TABLE_ENTRY(junk, i, j).constraints);
      }
   }
   SizeFree(junk->matrix,((junk->sig_size)
                          * (junk->maxarity + 1)
                          * sizeof(LitConstrCell)));
   LitOccTableCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: LitPosGetConstrState()
//
//   Return true if the position described carries any constraints,
//   false otherwise.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool LitPosGetConstrState(LitOccTable_p table, FunCode pred, int pos)
{
   assert(table);
   assert(pred < table->sig_size);
   assert(pos < table->maxarity);

   return LIT_OCC_TABLE_ENTRY(table, pred, pos).constrained;
}


/*-----------------------------------------------------------------------
//
// Function: LitPosSetConstrState()
//
//   Return true if the position described carries any constraints,
//   false otherwise.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void LitPosSetConstrState(LitOccTable_p table, FunCode pred, int pos,
                          bool value)
{
   assert(table);
   assert(pred < table->sig_size);
   assert(pos < table->maxarity);

   LIT_OCC_TABLE_ENTRY(table, pred, pos).constrained = value;
}


/*-----------------------------------------------------------------------
//
// Function: LitPosGetConstraints()
//
//   Return the constraints carried at the position described. This
//   function can only be called on positions that carry constraints!
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

PTree_p LitPosGetConstraints(LitOccTable_p table, FunCode pred, int pos)
{
   assert(table);
   assert(pred < table->sig_size);
   assert(pos < table->maxarity);
   assert(LIT_OCC_TABLE_ENTRY(table, pred, pos).constrained);

   return LIT_OCC_TABLE_ENTRY(table, pred, pos).constraints;
}


/*-----------------------------------------------------------------------
//
// Function: LitPosAddConstraint()
//
//   Add the term to the set of disjunctive constraints at the
//   described position. Return true if this makes the position
//   unconstrained.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

bool LitPosAddConstraint(LitOccTable_p table, FunCode pred, int pos,
                         Term_p term)
{
   if(TermIsFreeVar(term))
   {
      LitPosSetConstrState(table, pred, pos, false);
      return true;
   }
   else
   {
      PTreeStore(&(LIT_OCC_TABLE_ENTRY(table, pred, pos).constraints),
                 term);
      return false;
   }
}


/*-----------------------------------------------------------------------
//
// Function: LitOccAddLitAlt()
//
//   Add the constraints induced by literal into the corresponding
//   table.
//
// Global Variables: -
//
// Side Effects    : Changes table.
//
/----------------------------------------------------------------------*/

void LitOccAddLitAlt(LitOccTable_p p_table, LitOccTable_p n_table,
                     Eqn_p eqn)
{
   int i;
   LitOccTable_p handle;
   Term_p lit;

   assert(!EqnIsEquLit(eqn));

   if(EqnIsPositive(eqn))
   {
      handle = p_table;
   }
   else
   {
      handle = n_table;
   }
   lit = eqn->lterm;

   for(i=0; i< lit->arity; i++)
   {
      LitPosAddConstraint(handle, lit->f_code, i, lit->args[i]);
   }
}


/*-----------------------------------------------------------------------
//
// Function: LitOccAddClauseAlt()
//
//   Add the constraints induced by clause to the constraint tables.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void LitOccAddClauseAlt(LitOccTable_p p_table, LitOccTable_p n_table,
                        Clause_p clause)
{
   Eqn_p handle;

   for(handle = clause->literals; handle; handle=handle->next)
   {
      LitOccAddLitAlt(p_table, n_table, handle);
   }
}


/*-----------------------------------------------------------------------
//
// Function: LitOccAddClauseSetAlt()
//
//   Add the constraints induced by the clause set to the constraint
//   tables.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void LitOccAddClauseSetAlt(LitOccTable_p p_table, LitOccTable_p
                           n_table, ClauseSet_p set)
{
   Clause_p handle;

   for(handle = set->anchor->succ; handle!= set->anchor; handle =
          handle->succ)
   {
      LitOccAddClauseAlt(p_table, n_table, handle);
   }
}



/*-----------------------------------------------------------------------
//
// Function: SigCollectConstantTerms()
//
//   Push terms corresponding to all constants in sig onto the
//   stack. If sig contains no constant, insert a new skolem
//   constant. If uniq is set, just push the one term corresponding to
//   uniq.
//
// Global Variables: -
//
// Side Effects    : Potentially changes sig and bank
//
/----------------------------------------------------------------------*/

long SigCollectConstantTerms(TB_p bank, PStack_p stack, FunCode uniq)
{
   FunCode i;
   long res = 0;
   Term_p tmp, found;

   if(uniq)
   {
      assert((uniq > 0) && (uniq <= bank->sig->f_count) &&
             (SigFindArity(bank->sig,uniq)==0));
      tmp = TermConstCellAlloc(uniq);
      found = TBTermTopInsert(bank, tmp);
      PStackPushP(stack, found);
      res=1;
   }
   else
   {
      for(i=bank->sig->internal_symbols+1; i<=bank->sig->f_count; i++)
      {
         if(!SigIsPredicate(bank->sig, i) &&
            !SigQueryProp(bank->sig, i, FPSpecial) &&
            SigFindArity(bank->sig,i)==0)
         {
            tmp = TermConstCellAlloc(i);
            found = TBTermTopInsert(bank, tmp);
            PStackPushP(stack, found);
            res++;
         }
      }
   }
   if(!res)
   {
      OUTPRINT(1, COMCHAR" No constant in specification, "
               "added new Skolem constant\n");
      i = SigGetNewSkolemCode(bank->sig, 0);
      tmp = TermConstCellAlloc(i);
      found = TBTermTopInsert(bank, tmp);
      PStackPushP(stack, found);
      res++;
   }
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: EqnCollectVarConstr()
//
//   For all variables occuring in eqn, remove the alternatives not
//   compatible with the constraints in the tables.
//
// Global Variables: -
//
// Side Effects    : Changes the variable constraints.
//
/----------------------------------------------------------------------*/

void EqnCollectVarConstr(LitOccTable_p p_table, LitOccTable_p n_table,
                         PDArray_p var_constr, Eqn_p eqn)
{
   int     i;
   PTree_p tree;
   LitOccTable_p constr;
   Term_p lit = eqn->lterm;


   constr = EqnIsPositive(eqn)?n_table:p_table; /* Contraints are
                                                   induced by literals
                                                   of the opposite
                                                   sign! */
   for(i=0; i<lit->arity; i++)
   {
      if(TermIsFreeVar(lit->args[i]))
      {
         if(LitPosGetConstrState(constr,lit->f_code,i))
         {
            tree = PDArrayElementP(var_constr,
                                   -(lit->args[i]->f_code));
            (void)PTreeDestrIntersection(&tree,
                                         LitPosGetConstraints(constr,
                                                              lit->f_code,
                                                              i));
            PDArrayAssignP(var_constr,
                           -(lit->args[i]->f_code),
                           tree);
            /* if(tmp)
               {
               printf("Constraints at work: %ld\n", tmp);
               }*/
         }
      }
   }
}




/*-----------------------------------------------------------------------
//
// Function: ClauseCollectVarConstr()
//
//   Apply all variable constraints for clause to the initialized
//   var_constr array return them.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void ClauseCollectVarConstr(LitOccTable_p p_table, LitOccTable_p
                            n_table, Clause_p clause, PTree_p
                            ground_terms, PDArray_p var_constr)
{
   Eqn_p handle;

   for(handle = clause->literals; handle; handle = handle->next)
   {
      EqnCollectVarConstr(p_table, n_table, var_constr, handle);
   }
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
