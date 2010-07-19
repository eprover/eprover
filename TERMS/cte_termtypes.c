/*-----------------------------------------------------------------------

File  : cte_termtypes.c

Author: Stephan Schulz

Contents
 
  Declarations for the basic term type and primitive functions, mainly
  on single term cells. This module mostly provides only
  infrastructure for higher level modules.


  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Tue Feb 24 02:17:11 MET 1998
    New, from cte_terms.c
<2> Thu Mar 28 21:40:52 CEST 2002
    Started to implement new rewriting

-----------------------------------------------------------------------*/

#include "cte_termtypes.h"


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
// Function: TermDefaultCellAlloc()
//
//   Allocate a term cell with default values.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Term_p TermDefaultCellAlloc(void)
{
   Term_p handle;
   
   handle = TermCellAlloc();

   handle->properties = TPIgnoreProps;
   handle->arity = 0;
   handle->binding = NULL;
   handle->args = NULL;
   handle->weight = DEFAULT_VWEIGHT; /* If no variable, will be
					changed automagically later
					on */
   handle->rw_data.nf_date[0] = SysDateCreationTime(); 
   handle->rw_data.nf_date[1] = SysDateCreationTime(); 
   
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: TermConstCellAlloc()
//
//   Allocate a term cell for the constant term with symbol symbol.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Term_p TermConstCellAlloc(FunCode symbol)
{
   Term_p handle = TermDefaultCellAlloc();
   
   handle->weight = DEFAULT_FWEIGHT;
   handle->f_code = symbol;

   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: TermTopAlloc()
//
//   Allocate a term top with given f_code and (uninitialized)
//   argument array. 
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p  TermTopAlloc(FunCode f_code, int arity)
{
   Term_p res = TermDefaultCellAlloc();

   res->f_code = f_code;
   res->arity  = arity;
   if(arity)
   {
      res->args = TermArgArrayAlloc(arity);
   }
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: TermTopFree()
//
//   Return term cell and arg array (if it exists).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void TermTopFree(Term_p junk)
{
   if(junk->arity)
   {
      assert(junk->args);
      TermArgArrayFree(junk->args, junk->arity);
   }
   else
   {
      assert(!junk->args);
   }
   TermCellFree(junk);
}

/*-----------------------------------------------------------------------
//
// Function:  TermFree()
//
//   Return the memory taken by an (unshared) term. Does not free the
//   variable cells, which belong to a VarBank.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void TermFree(Term_p junk)
{
   assert(junk);
   if(!TermIsVar(junk))
   {
      assert(!TermCellQueryProp(junk, TPIsShared));
      if(junk->arity)
      {
	 int i;
	 
	 assert(junk->args);
	 for(i=0; i<junk->arity; i++)
	 {
	    TermFree(junk->args[i]);
	 }
      }
      else
      {
	 assert(!junk->args);
      }
      TermTopFree(junk);
   }
}


/*-----------------------------------------------------------------------
//
// Function: TermNewSkolemTerm()
//
//   Create a new Skolem term (or renaming atom) with the named
//   variables as arguments.
//
// Global Variables: -
//
// Side Effects    : Memory operations, creates new Skolem function in
//                   sig. 
//
/----------------------------------------------------------------------*/

Term_p TermAllocNewSkolem(Sig_p sig, PStack_p variables, bool atom)
{
   Term_p handle = TermDefaultCellAlloc();
   PStackPointer arity = PStackGetSP(variables), i;

   if(!atom)
   {
      handle->f_code = SigGetNewSkolemCode(sig, arity);
   }
   else
   {
      handle->f_code = SigGetNewPredicateCode(sig, arity);
   }
   if(arity)
   {
      handle->arity = arity;
      handle->args = TermArgArrayAlloc(arity);
      for(i=0; i<arity; i++)
      {
         handle->args[i] = PStackElementP(variables, i);
      }
   }
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: TermSetProp()
//
//   Set the properties in all term cells belonging to term.
//
// Global Variables: -
//
// Side Effects    : Changes properties (even in shared terms! Beware!)
//
/----------------------------------------------------------------------*/

void TermSetProp(Term_p term, DerefType deref, TermProperties prop)
{
   PStack_p stack = PStackAlloc();
   int i;

   PStackPushP(stack, term);
   PStackPushInt(stack, deref);
   
   while(!PStackEmpty(stack))
   {
      deref = PStackPopInt(stack);
      term  = PStackPopP(stack);
      term  = TermDeref(term, &deref);
      TermCellSetProp(term, prop);
      for(i=0; i<term->arity; i++)
      {
	 PStackPushP(stack, term->args[i]);
	 PStackPushInt(stack, deref);
      }
   }
   PStackFree(stack);
}


/*-----------------------------------------------------------------------
//
// Function: TermSearchProp()
//
//   If prop is set in any subterm of term, return true, otherwise
//   false. 
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool TermSearchProp(Term_p term, DerefType deref, TermProperties prop)
{
   PStack_p stack = PStackAlloc();
   int i;
   bool res = false;

   PStackPushP(stack, term);
   PStackPushInt(stack, deref);
   
   while(!PStackEmpty(stack))
   {
      deref = PStackPopInt(stack);
      term  = PStackPopP(stack);
      term  = TermDeref(term, &deref);
      if(TermCellQueryProp(term, prop))
      {
	 res = true;
	 break;
      }
      for(i=0; i<term->arity; i++)
      {
	 PStackPushP(stack, term->args[i]);
	 PStackPushInt(stack, deref);
      }
   }
   PStackFree(stack);
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: TermDelProp()
//
//   Delete the properties in all term cells belonging to term.
//
// Global Variables: -
//
// Side Effects    : Changes properties (even in shared terms! Beware!)
//
/----------------------------------------------------------------------*/

void TermDelProp(Term_p term, DerefType deref, TermProperties prop)
{
   PStack_p stack = PStackAlloc();
   int i;

   PStackPushP(stack, term);
   PStackPushInt(stack, deref);
   
   while(!PStackEmpty(stack))
   {
      deref = PStackPopInt(stack);
      term  = PStackPopP(stack);
      term  = TermDeref(term, &deref);
      TermCellDelProp(term, prop);
      for(i=0; i<term->arity; i++)
      {
	 PStackPushP(stack, term->args[i]);
	 PStackPushInt(stack, deref);
      }
   }
   PStackFree(stack);
}



/*-----------------------------------------------------------------------
//
// Function: TermVarSetProp()
//
//   Set the properties in all variable cells belonging to term.
//
// Global Variables: -
//
// Side Effects    : Changes properties (even in shared terms! Beware!)
//
/----------------------------------------------------------------------*/

void TermVarSetProp(Term_p term, DerefType deref, TermProperties prop)
{
   PStack_p stack = PStackAlloc();
   int i;

   PStackPushP(stack, term);
   PStackPushInt(stack, deref);
   
   while(!PStackEmpty(stack))
   {
      deref = PStackPopInt(stack);
      term  = PStackPopP(stack);
      term  = TermDeref(term, &deref);
      if(TermIsVar(term))
      {
	 TermCellSetProp(term, prop);
      }
      for(i=0; i<term->arity; i++)
      {
	 PStackPushP(stack, term->args[i]);
	 PStackPushInt(stack, deref);
      }
   }
   PStackFree(stack);
}



/*-----------------------------------------------------------------------
//
// Function: TermVarSearchProp()
//
//   If prop is set in any variable cell in term, return true, otherwise
//   false. 
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool TermVarSearchProp(Term_p term, DerefType deref, TermProperties prop)
{
   PStack_p stack = PStackAlloc();
   int i;
   bool res = false;

   PStackPushP(stack, term);
   PStackPushInt(stack, deref);
   
   while(!PStackEmpty(stack))
   {
      deref = PStackPopInt(stack);
      term  = PStackPopP(stack);
      term  = TermDeref(term, &deref);
      if(TermIsVar(term) && TermCellQueryProp(term, prop))
      {
	 res = true;
	 break;
      }
      for(i=0; i<term->arity; i++)
      {
	 PStackPushP(stack, term->args[i]);
	 PStackPushInt(stack, deref);
      }
   }
   PStackFree(stack);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: TermVarDelProp()
//
//   Delete the properties in all variable cells belonging to term.
//
// Global Variables: -
//
// Side Effects    : Changes properties (even in shared terms! Beware!)
//
/----------------------------------------------------------------------*/

void TermVarDelProp(Term_p term, DerefType deref, TermProperties prop)
{
   PStack_p stack = PStackAlloc();
   int i;

   PStackPushP(stack, term);
   PStackPushInt(stack, deref);
   
   while(!PStackEmpty(stack))
   {
      deref = PStackPopInt(stack);
      term  = PStackPopP(stack);
      term  = TermDeref(term, &deref);
      if(TermIsVar(term))
      {
	 TermCellDelProp(term, prop);
      }
      for(i=0; i<term->arity; i++)
      {
	 PStackPushP(stack, term->args[i]);
	 PStackPushInt(stack, deref);
      }
   }
   PStackFree(stack);
}


/*-----------------------------------------------------------------------
//
// Function: TermStackSetProps()
//
//   Set the given properties in all term cells on the stack.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void TermStackSetProps(PStack_p stack, TermProperties prop)
{
   PStackPointer i;
   Term_p term;

   for(i=0; i<PStackGetSP(stack); i++)
   {
      term = PStackElementP(stack, i);
      TermCellSetProp(term, prop);
   }
}


/*-----------------------------------------------------------------------
//
// Function: TermStackDelProps()
//
//   Delete the given properties in all term cells on the stack.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void TermStackDelProps(PStack_p stack, TermProperties prop)
{
   PStackPointer i;
   Term_p term;

   for(i=0; i<PStackGetSP(stack); i++)
   {
      term = PStackElementP(stack, i);
      TermCellDelProp(term, prop);
   }
}






/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/




