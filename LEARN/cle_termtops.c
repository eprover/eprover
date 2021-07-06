/*-----------------------------------------------------------------------

  File  : cle_termtops.c

  Author: Stephan Schulz

  Contents

  Functions for computing various kinds of top terms.

  Copyright 1998, 1999, 2018 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Tue Aug  3 17:14:06 MET DST 1999

  -----------------------------------------------------------------------*/

#include "cle_termtops.h"



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
// Function: term_del_prop_level()
//
//   Clear prop in all terms reachable by a path of length <= depth
//   from the root node.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

static void term_del_prop_level(Term_p term, int depth, TermProperties prop)
{
   PStack_p stack = PStackAlloc();
   int      i;

   PStackPushP(stack, term);
   PStackPushInt(stack, depth);

   if(depth>=0) /* Special case depth = -1 can happen -> Do nothing */
   {
      while(!PStackEmpty(stack))
      {
         depth = PStackPopInt(stack);
         term =  PStackPopP(stack);
         TermCellDelProp(term, prop);
         if(depth!=0)
         {
            for(i=0; i<term->arity; i++)
            {
               assert(!TermIsFreeVar(term));
               PStackPushP(stack, term->args[i]);
               PStackPushInt(stack, depth-1);
            }
         }
      }
   }
   PStackFree(stack);
}


/*-----------------------------------------------------------------------
//
// Function: term_set_prop_at_level()
//
//   Set prop in all terms reachable by a path of length == depth from
//   the root node.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

static void term_set_prop_at_level(Term_p term, int depth,
                                   TermProperties prop)
{
   PStack_p stack = PStackAlloc();
   int      i;

   PStackPushP(stack, term);
   PStackPushInt(stack, depth);

   while(!PStackEmpty(stack))
   {
      depth = PStackPopInt(stack);
      term =  PStackPopP(stack);
      if(depth!=0)
      {
         for(i=0; i<term->arity; i++)
         {
            assert(!TermIsFreeVar(term));
            PStackPushP(stack, term->args[i]);
            PStackPushInt(stack, depth-1);
         }
      }
      else
      {
         TermCellSetProp(term, prop);
      }
   }
   PStackFree(stack);
}


/*-----------------------------------------------------------------------
//
// Function: rek_term_top()
//
//   Return the term top of term at level i.
//
// Global Variables: -
//
// Side Effects    : Changes freshvars
//
/----------------------------------------------------------------------*/

static Term_p rek_term_top(Term_p term, int depth, VarBank_p freshvars)
{
   Term_p handle;

   if(depth==0)
   {
      handle = VarBankGetAltFreshVar(freshvars, term->type);
   }
   else if(TermIsFreeVar(term))
   {
      handle = VarBankVarAssertAlloc(freshvars, term->f_code, term->type);
   }
   else
   {
      handle = TermDefaultCellArityAlloc(term->arity);
      handle->arity = term->arity;
      handle->f_code = term->f_code;
      if(handle->arity > 0)
      {
         int i;

         for(i=0; i<handle->arity; i++)
         {
            handle->args[i] = rek_term_top(term->args[i], depth-1,
                                           freshvars);
         }
      }
   }
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: alt_rek_term_top()
//
//   Return the alternate term top of term at level i.
//
// Global Variables: -
//
// Side Effects    : Changes freshvars
//
/----------------------------------------------------------------------*/

static Term_p alt_rek_term_top(Term_p term, int depth, VarBank_p
                               freshvars, PStack_p bindings)
{
   Term_p handle;

   if(depth==0)
   {
      if(!term->binding)
      {
         handle = VarBankGetAltFreshVar(freshvars, term->type);
         term->binding = handle;
         PStackPushP(bindings, term);
      }
      else
      {
         handle = term->binding;
      }
   }
   else if(TermIsFreeVar(term))
   {
      handle = VarBankVarAssertAlloc(freshvars, term->f_code, term->type);
   }
   else
   {
      handle = TermDefaultCellArityAlloc(term->arity);
      handle->arity = term->arity;
      handle->f_code = term->f_code;
      if(handle->arity > 0)
      {
         int i;

         for(i=0; i<handle->arity; i++)
         {
            handle->args[i] = alt_rek_term_top(term->args[i], depth-1,
                                               freshvars, bindings);
         }
      }
   }
   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: term_top_marked()
//
//   Copy the term top up to the nodes marked with TPOpFlag
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Term_p term_top_marked(Term_p term, VarBank_p freshvars, PStack_p
                       bindings)
{
   Term_p handle;

   if(TermCellQueryProp(term, TPOpFlag))
   {
      if(!term->binding)
      {
         handle = VarBankGetAltFreshVar(freshvars, term->type);
         term->binding = handle;
         PStackPushP(bindings, term);
      }
      else
      {
         handle = term->binding;
      }
   }
   else if(TermIsFreeVar(term))
   {
      handle = VarBankVarAssertAlloc(freshvars, term->f_code, term->type);
   }
   else
   {
      handle = TermDefaultCellArityAlloc(term->arity);
      handle->arity = term->arity;
      handle->f_code = term->f_code;
      if(handle->arity > 0)
      {
         int i;

         for(i=0; i<handle->arity; i++)
         {
            handle->args[i] = term_top_marked(term->args[i],
                                              freshvars, bindings);
         }
      }
   }
   return handle;
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: TermTop()
//
//   Compute top(term, depth).
//
// Global Variables: -
//
// Side Effects    : May allocate variables, changes freshvars
//
/----------------------------------------------------------------------*/

Term_p TermTop(Term_p term, int depth, VarBank_p freshvars)
{
   assert(TermIsShared(term));
   VarBankResetVCounts(freshvars);

   return rek_term_top(term, depth, freshvars);
}


/*-----------------------------------------------------------------------
//
// Function: AltTermTop()
//
//   Compute top'(term, depth). See above. Requires that bindings are
//   NULL in term.
//
// Global Variables: -
//
// Side Effects    : May allocate variables, changes freshvars
//
/----------------------------------------------------------------------*/

Term_p AltTermTop(Term_p term, int depth, VarBank_p freshvars)
{
   PStack_p bindings = PStackAlloc();
   Term_p   handle, res;

   assert(TermIsShared(term));
   VarBankResetVCounts(freshvars);

   res = alt_rek_term_top(term, depth, freshvars, bindings);

   while(!PStackEmpty(bindings))
   {
      handle = PStackPopP(bindings);
      assert(handle->binding);
      handle->binding = NULL;
   }
   PStackFree(bindings);
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: CSTermTop()
//
//   Return the compact shared top term of t at level depth.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p CSTermTop(Term_p term, int depth, VarBank_p freshvars)
{
   PStack_p bindings = PStackAlloc();
   Term_p   handle, res;

   assert(TermIsShared(term));
   VarBankResetVCounts(freshvars);

   term_del_prop_level(term, depth, TPOpFlag);
   term_set_prop_at_level(term, depth, TPOpFlag);

   res = term_top_marked(term, freshvars, bindings);

   while(!PStackEmpty(bindings))
   {
      handle = PStackPopP(bindings);
      assert(handle->binding);
      handle->binding = NULL;
   }

   PStackFree(bindings);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ESTermTop()
//
//   Return the extended shared top term of t at level depth.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p ESTermTop(Term_p term, int depth, VarBank_p freshvars)
{
   PStack_p bindings = PStackAlloc();
   Term_p   handle, res;

   assert(TermIsShared(term));
   VarBankResetVCounts(freshvars);

   term_set_prop_at_level(term, depth, TPOpFlag);
   term_del_prop_level(term, depth-1, TPOpFlag);

   res = term_top_marked(term, freshvars, bindings);

   while(!PStackEmpty(bindings))
   {
      handle = PStackPopP(bindings);
      assert(handle->binding);
      handle->binding = NULL;
   }
   PStackFree(bindings);
   return res;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


