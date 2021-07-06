/*-----------------------------------------------------------------------

File  : cte_varhash.c

Author: Stephan Schulz

Contents

  Functions for the management of variable hashs and hashed lists.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed Jul 22 04:55:59 MET DST 1998
    new

-----------------------------------------------------------------------*/

#include "cte_varhash.h"


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
// Function: VarHashEntryListFree()
//
//   Free a linear list of var hash entries.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void VarHashEntryListFree(VarHashEntry_p list)
{
   VarHashEntry_p handle;

   while(list)
   {
      handle = list->next;
      VarHashEntryCellFree(list);
      list = handle;
   }
}



/*-----------------------------------------------------------------------
//
// Function: VarHashFree()
//
//   Free a variable hash.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void VarHashFree(VarHash_p junk)
{
   int i;

   for(i=0; i<VAR_HASH_SIZE; i++)
   {
      VarHashEntryListFree(junk->hash[i]);
   }
   VarHashCellFree(junk);
}

/*-----------------------------------------------------------------------
//
// Function: VarHashFunction()
//
//   Hash function, map term cell with f_code onto an index.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int VarHashFunction(Term_p var)
{
   assert(var->f_code < 0);

   return (-(var->f_code) & VAR_HASH_MASK);
}

/*-----------------------------------------------------------------------
//
// Function: VarHashListFind()
//
//   Find an entry in the linear list of hash entries. Return NULL on
//   failure.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

VarHashEntry_p VarHashListFind(VarHashEntry_p list, Term_p var)
{
   while(list)
   {
      if(list->key == var)
      {
    break;
      }
      list = list->next;
   }
   return list;
}


/*-----------------------------------------------------------------------
//
// Function: VarHashAddValue()
//
//  If var is stored in hash, add value to its entries value,
//  otherwise create an entry and set its value to value. Return the
//  stored value.
//
// Global Variables: -
//
// Side Effects    : Changes value, memory operation
//
/----------------------------------------------------------------------*/

long VarHashAddValue(VarHash_p hash, Term_p var, long value)
{
   int index            = VarHashFunction(var);
   VarHashEntry_p entry = VarHashListFind(hash->hash[index], var);

   if(entry)
   {
      entry->val+=value;
   }
   else
   {
      entry = VarHashEntryAlloc(var, value);
      entry->next = hash->hash[index];
      hash->hash[index] = entry;
   }
   return entry->val;
}


/*-----------------------------------------------------------------------
//
// Function: VarHashAddVarDistrib()
//
//   Scans a term and adds the variable occurences to the hash, with
//   each occurence being counted with the "add" value.
//   NB: Derefs not changed, function called only with FOL arguments.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void VarHashAddVarDistrib(VarHash_p hash, Term_p term, DerefType
           deref, long add)
{
   assert(problemType != PROBLEM_HO);
   PStack_p stack = PStackAlloc();
   int      i;

   PStackPushP(stack, term);
   PStackPushInt(stack, deref);

   while(!PStackEmpty(stack))
   {
      deref = PStackPopInt(stack);
      term = PStackPopP(stack);
      term = TermDeref(term, &deref);

      if(TermIsFreeVar(term))
      {
    VarHashAddValue(hash, term, add);
      }
      else
      {
    for(i=0; i<term->arity; i++)
    {
       PStackPushP(stack, term->args[i]);
       PStackPushInt(stack, deref);
    }
      }
   }
   PStackFree(stack);
}

/*-----------------------------------------------------------------------
//
// Function: PDArrayAddVarDistrib()
//
//   Scans a term and adds the variable occurences to the array, with
//   each occurence being counted with the "add" value.
//   NB: Derefs not changed, function called only with FOL arguments.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void PDArrayAddVarDistrib(PDArray_p array, Term_p term, DerefType
           deref, long add)
{
   assert(problemType != PROBLEM_HO);
   PStack_p stack = PStackAlloc();
   int      i;
   long     tmp;

   PStackPushP(stack, term);
   PStackPushInt(stack, deref);

   while(!PStackEmpty(stack))
   {
      deref = PStackPopInt(stack);
      term = PStackPopP(stack);
      term = TermDeref(term, &deref);

      if(TermIsFreeVar(term))
      {
    tmp = PDArrayElementInt(array, (-(term->f_code)));
    PDArrayAssignInt(array, (-(term->f_code)), tmp+add);
      }
      else
      {
    for(i=0; i<term->arity; i++)
    {
       PStackPushP(stack, term->args[i]);
       PStackPushInt(stack, deref);
    }
      }
   }
   PStackFree(stack);
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/




