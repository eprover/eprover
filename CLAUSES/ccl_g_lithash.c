/*-----------------------------------------------------------------------

File  : ccl_g_lithash.c

Author: Stephan Schulz

Contents

  Algorithms and data structures implementing a simple literal
  indexing structure for implementing local unification constraints
  for the grounding procedure.

Copyright 1998-2011 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Thu Jun 21 11:38:40 CEST 2001
    New

-----------------------------------------------------------------------*/

#include "ccl_g_lithash.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: lit_tree_free()
//
//   Free the memory taken up by a PObjTree containing LitDescCells.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void lit_tree_free(PTree_p tree)
{
   LitDesc_p handle;
   PStack_p stack = PStackAlloc();

   PStackPushP(stack, tree);

   while(!PStackEmpty(stack))
   {
      tree = PStackPopP(stack);
      if(tree)
      {
    handle = tree->key;
    assert(handle);
    assert(handle->lit);
    LitDescCellFree(handle);
    PStackPushP(stack, tree->lson);
    PStackPushP(stack, tree->rson);
      }
   }
   PStackFree(stack);
   PTreeFree(tree);
}



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: LitDescCompare()
//
//   Compare two literal occurrence description cells. They are equal
//   if the literal terms are equal, the clause is not used!
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int LitDescCompare(const void* lit1, const void* lit2)
{
   const LitDesc_p l1 = (const LitDesc_p) lit1;
   const LitDesc_p l2 = (const LitDesc_p) lit2;

   return PCmp(l1->lit, l2->lit);
}


/*-----------------------------------------------------------------------
//
// Function: LitHashAlloc()
//
//   Allocate a literal hash suitable for the given signature.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

LitHash_p LitHashAlloc(Sig_p sig)
{
   long i;

   LitHash_p handle = LitHashCellAlloc();

   handle->sig_size = sig->f_count+1;
   handle->pos_lits = SizeMalloc(handle->sig_size*sizeof(PTree_p));
   handle->neg_lits = SizeMalloc(handle->sig_size*sizeof(PTree_p));

   for(i=0; i<handle->sig_size; i++)
   {
      handle->pos_lits[i] = NULL;
      handle->neg_lits[i] = NULL;
   }
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: LitHashFree()
//
//   Free the memory occupied by a lithashtable.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void LitHashFree(LitHash_p junk)
{
   long i;

   for(i=0; i<junk->sig_size; i++)
   {
      lit_tree_free(junk->pos_lits[i]);
      lit_tree_free(junk->neg_lits[i]);
   }
   SizeFree(junk->pos_lits, junk->sig_size*sizeof(PTree_p));
   SizeFree(junk->neg_lits, junk->sig_size*sizeof(PTree_p));
   LitHashCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: LitHashInsertEqn()
//
//   Insert a literal (the left hand side of the equation) into the
//   literal hash.
//
// Global Variables: -
//
// Side Effects    : Changes the hash
//
/----------------------------------------------------------------------*/

void LitHashInsertEqn(LitHash_p hash, Eqn_p eqn, Clause_p clause)
{
   LitDesc_p handle = LitDescCellAlloc(), old;
   PTree_p *root;

   assert(hash);
   assert(eqn);
   assert(!EqnIsEquLit(eqn));
   assert(eqn->lterm->f_code > 0);
   assert(eqn->lterm->f_code < hash->sig_size);

   handle->lit = eqn->lterm;
   handle->clause = clause;

   if(EqnIsPositive(eqn))
   {
      root = &(hash->pos_lits[eqn->lterm->f_code]);
   }
   else
   {
      root = &(hash->neg_lits[eqn->lterm->f_code]);
   }
   old = PTreeObjStore(root, handle, LitDescCompare);
   if(old) /* Literal already known */
   {
      LitDescCellFree(handle);
      old->clause = NULL; /* It does not belong to a unique clause */
   }
}


/*-----------------------------------------------------------------------
//
// Function: LitHashInsertClause()
//
//   Insert all literals in clause into the loteral hash.
//
// Global Variables: -
//
// Side Effects    : Changes the hash
//
/----------------------------------------------------------------------*/

void LitHashInsertClause(LitHash_p hash, Clause_p clause)
{
   Eqn_p handle;

   for(handle=clause->literals; handle; handle = handle->next)
   {
      LitHashInsertEqn(hash, handle, clause);
   }
}


/*-----------------------------------------------------------------------
//
// Function: LitHashInsertClauseSet()
//
//   Insert all literals in all clauses in the set into the hash.
//
// Global Variables: -
//
// Side Effects    : Changes the hash
//
/----------------------------------------------------------------------*/

void LitHashInsertClauseSet(LitHash_p hash, ClauseSet_p set)
{
   Clause_p handle;

   for(handle = set->anchor->succ; handle!= set->anchor; handle =
     handle->succ)
   {
      LitHashInsertClause(hash, handle);
   }
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





















