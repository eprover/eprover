/*-----------------------------------------------------------------------

File  : cte_replace.c

Author: Stephan Schulz

Contents

  Functions for replacing and rewriting of terms.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Mon Jan 26 19:16:16 MET 1998
    new

-----------------------------------------------------------------------*/

#include "cte_replace.h"
#include <ccl_clauses.h>

extern TB_p bank;



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
// Function: TermAddRWLink()
//
//   Add a rewrite link from term to replace, induced by demod. Note:
//   If demod is REWRITE_AT_SUBTERM, actual rewriting happened at a
//   subterm.
//
// Global Variables: -
//
// Side Effects    : As described
//
/----------------------------------------------------------------------*/

void TermAddRWLink(Term_p term, Term_p replace, struct clause_cell *demod,
                   bool sos, RWResultType type)
{
   assert(term);
   assert(replace);
   assert(term!=replace);

   TermCellSetProp(term, TPIsRewritten);

   if(type==RWAlwaysRewritable)
   {
      TermCellSetProp(term, TPIsRRewritten);
   }

   TermRWReplaceField(term) = replace;
   TermRWDemodField(term)   = demod;

   if(sos)
   {
      TermCellSetProp(term, TPIsSOSRewritten);
   }
}


/*-----------------------------------------------------------------------
//
// Function: TermDeleteRWLink()
//
//   Delete rewrite link from term.
//
// Global Variables: -
//
// Side Effects    : As described
//
/----------------------------------------------------------------------*/

void TermDeleteRWLink(Term_p term)
{
   assert(term);

   TermCellDelProp(term, TPIsRewritten|TPIsRRewritten|TPIsSOSRewritten);

   TermRWReplaceField(term) = NULL;
   TermRWDemodField(term)   = 0;
}


/*-----------------------------------------------------------------------
//
// Function: TermFollowRWChain()
//
//   Return the last term in an existing rewrite link chain.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p TermFollowRWChain(Term_p term)
{
   assert(term);

   /* printf("Starting chain\n"); */
   while(TermIsRewritten(term))
   {
      term = TermRWReplaceField(term);
      /* printf("Following chain\n"); */
      assert(term);
   }
   return term;
}


/*-----------------------------------------------------------------------
//
// Function: TBTermPosReplace()
//
//   Create a new term by replacing the subterm designated by pos with
//   repl and insert it into the term bank. Return pointer to the new
//   term. The superterm is implicit in the position (or, in the case
//   of the empty position, is unnecessary). Does not free any terms -
//   if necessary, this is the responsibility of the calling
//   functions.
//
//   Note that this function may dereference terms more than once,
//   which will lead to strange behaviour if instantiated variables
//   appear in the term...---> This has been fixed. Each term in repl
//   and its superterms should now be dereferenced only once.
//
//   New in E/HO: If we have to replace top-level term and some arguments
//   are remaining in it, we need the pointer to the original term
//   since we are replacing a (prefix) subterm.
//
// Global Variables: -
//
// Side Effects    : Memory management, changes term bank
//
/----------------------------------------------------------------------*/

Term_p TBTermPosReplace(TB_p bank, Term_p repl, TermPos_p pos,
                        DerefType deref, int remains, Term_p old_into)
{
   Term_p        handle, old;
   int           subscript;
   PStackPointer i;
   PStack_p      store = PStackAlloc();


   assert(bank);
   assert(repl);
   assert(pos);

   i = PStackGetSP(pos);


   /* Note that we start inside-out here - the first term handled is
      the actual subterm replaced, at the end repl is the complete
      term generated.*/
   while(i)
   {
      i--;
      subscript = PStackElementInt(pos, i);
      assert(i);
      i--;

      old = PStackElementP(pos, i);
      handle = TermTopCopy(old);
      assert(handle->arity > subscript);
#ifdef ENABLE_LFHO
      if(remains != -1)
      {
         Term_p tmp_repl = MakeRewrittenTerm(TermDerefAlways(handle->args[subscript]),
                                             TermDerefAlways(repl), remains, bank);

         handle->args[subscript] = remains ? TBTermTopInsert(bank, tmp_repl) : tmp_repl;

         remains = -1;
      }
      else
      {
         handle->args[subscript] = repl;
      }
#else
      handle->args[subscript] = repl;
#endif

      PStackPushP(store, handle);
      repl = handle;
   }

   if(remains > 0)
   {
      Term_p repl_tmp = MakeRewrittenTerm(TermDerefAlways(old_into),
                                          TermDerefAlways(repl), remains, bank);
      repl_tmp = TBTermTopInsert(bank, repl_tmp);
      repl = TBInsertNoProps(bank, repl_tmp, deref);
   }
   else
   {
      repl  = TBInsertNoProps(bank, repl, deref);
   }

   while(!PStackEmpty(store))
   {
      handle = PStackPopP(store);
      TermTopFree(handle);
   }
   PStackFree(store);

   return repl;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
