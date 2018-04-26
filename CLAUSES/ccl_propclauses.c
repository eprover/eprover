/*-----------------------------------------------------------------------

  File  : ccl_propclauses.c

  Author: Stephan Schulz

  Contents

  Functions for handling propositional clauses.

  Copyright 1998-2011 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: Sun Dec 16 17:20:36 CET 2001

  -----------------------------------------------------------------------*/

#include "ccl_propclauses.h"



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
// Function: PropClauseAlloc()
//
//   Allocate a propositional clause representing the same clause as
//   the normal one. Does some sanity checking, but only in
//   assertions.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

PropClause_p PropClauseAlloc(Clause_p clause)
{
   PropClause_p handle = PropClauseCellAlloc();
   Eqn_p lit;
   int   i;

   assert(clause);

   handle->next = NULL;
   handle->lit_no = ClauseLiteralNumber(clause);
   if(clause->literals)
   {
      handle->literals = SizeMalloc(handle->lit_no
                                    *sizeof(PropLitCell));
   }
   else
   {
      handle->literals = NULL;
   }


   lit = clause->literals;
   for(i=0; i<handle->lit_no; i++)
   {
      assert(lit);
      assert(lit->rterm->f_code == SIG_TRUE_CODE);

      handle->literals[i].properties = lit->properties;
      handle->literals[i].lit = lit->lterm;
      lit = lit->next;
   }
   assert(!lit);
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: PropClauseFree()
//
//   Free the memory taken up by a correctly build propositional
//   clause. Does not touch the terms/atoms!
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void PropClauseFree(PropClause_p clause)
{
   assert(clause);

   if(clause->lit_no)
   {
      assert(clause->literals);
      SizeFree(clause->literals, clause->lit_no * sizeof(PropLitCell));
   }
   else
   {
      assert(!clause->literals);
   }
   PropClauseCellFree(clause);
}


/*-----------------------------------------------------------------------
//
// Function: PropClauseToClause()
//
//   Generate a conventional clause from a propositional clause.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Clause_p PropClauseToClause(TB_p bank, PropClause_p clause)
{
   Eqn_p    list=NULL, *eqn;
   Clause_p handle;
   int i;

   eqn = &list;
   for(i=0; i<clause->lit_no; i++)
   {
      *eqn = EqnAlloc(clause->literals[i].lit,
                      bank->true_term,
                      bank, false);
      (*eqn)->properties = clause->literals[i].properties;
      eqn = &((*eqn)->next);
   }
   handle = ClauseAlloc(list);
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: PropClausePrint()
//
//   Print a propositional clause (by temporarily converting it to a
//   normal one (which will have an unpredictable identifier).
//
// Global Variables: -
//
// Side Effects    : Output, Memory operations
//
/----------------------------------------------------------------------*/

void PropClausePrint(FILE* out, TB_p bank, PropClause_p clause)
{
   Clause_p handle = PropClauseToClause(bank, clause);

   ClausePrint(out, handle, true);
   ClauseFree(handle);
}

/*-----------------------------------------------------------------------
//
// Function: PropClauseMaxVar()
//
//   Return the largest variable index in clause.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long PropClauseMaxVar(PropClause_p clause)
{
   int i;
   long res = 0, tmp;

   for(i=0; i<clause->lit_no; i++)
   {
      tmp = clause->literals[i].lit->entry_no;
      if(tmp > res)
      {
         res = tmp;
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: PropClauseSetAlloc()
//
//   Allocate an empty propositional clause set.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

PropClauseSet_p PropClauseSetAlloc(void)
{
   PropClauseSet_p set = PropClauseSetCellAlloc();

   set->members = 0;
   set->literals = 0;
   set->empty_clauses = 0;
   set->list    = NULL;
   set->inspos  = &(set->list);

   return set;
}


/*-----------------------------------------------------------------------
//
// Function: PropClauseSetFree()
//
//   Free a PropClauseSet and all its clauses.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void PropClauseSetFree(PropClauseSet_p set)
{
   PropClause_p handle,tmp;

   assert(set);

   for(handle = set->list; handle; handle = tmp)
   {
      tmp = handle->next;
      PropClauseFree(handle);
   }
   PropClauseSetCellFree(set);
}


/*-----------------------------------------------------------------------
//
// Function: PropClauseSetInsertPropClause()
//
//   Insert a propositional clause into the set. Return new number of
//   elements.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long PropClauseSetInsertPropClause(PropClauseSet_p set, PropClause_p
                                   clause)
{
   assert(set && clause && set->inspos);

   clause->next = NULL;
   *(set->inspos) = clause;
   set->inspos = &(clause->next);
   set->members++;
   set->literals+=clause->lit_no;
   if(clause->lit_no==0)
   {
      set->empty_clauses++;
   }
   return set->members;
}


/*-----------------------------------------------------------------------
//
// Function: PropClauseSetInsertClause()
//
//   Insert the (normal) clause into set as a propositional clause.
//
// Global Variables: -
//
// Side Effects    : Destroys clause
//
/----------------------------------------------------------------------*/

long PropClauseSetInsertClause(PropClauseSet_p set, Clause_p clause)
{
   PropClause_p handle = PropClauseAlloc(clause);

   ClauseFree(clause);

   return PropClauseSetInsertPropClause(set, handle);
}


/*-----------------------------------------------------------------------
//
// Function: PropClauseSetPrint()
//
//   Print a propositional clause set.
//
// Global Variables: -
//
// Side Effects    : Output, Memory operations
/
/----------------------------------------------------------------------*/

void PropClauseSetPrint(FILE* out, TB_p bank, PropClauseSet_p set)
{
   PropClause_p handle = set->list;

   while(handle)
   {
      PropClausePrint(out, bank, handle);
      fputc('\n', out);
      handle = handle->next;
   }
}


/*-----------------------------------------------------------------------
//
// Function: PropClauseSetMaxVar()
//
//   Return the largest used variable number in set.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long PropClauseSetMaxVar(PropClauseSet_p set)
{
   long res = 0, tmp;

   PropClause_p handle = set->list;

   while(handle)
   {
      tmp = PropClauseMaxVar(handle);
      if(tmp > res)
      {
         res = tmp;
      }
      handle = handle->next;
   }
   return res;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
