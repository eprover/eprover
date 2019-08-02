/*-----------------------------------------------------------------------

File  : cte_termpos.h

Author: Stephan Schulz

Contents

  Positions in terms.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sun May 10 17:37:08 MET DST 1998
    Lifted from cte_rewrite.h (now moved to cte_replace.h)

-----------------------------------------------------------------------*/

#ifndef CTE_TERMPOS

#define CTE_TERMPOS


#include <cte_termbanks.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* Positions in terms are described by a stack containing term
   pointers and indices of principal subterms, to be read from the
   lowest stack address (address of the top term) to the highest
   address (index of the selected subterm in its direct
   superterm). The selected subterm is not contained directly in the
   stack. The empty stack corresponds to the empty position.

   Example: t = f(a,g(b))

   The position of b is represented by the following stack (where @s =
   address of s, stacks grow downwards):

   @f(a,g(b))
   2
   @g(b)
   1

   Term positions are really PStacks, they are only redefined for
   better readability. In particular, functions dealing with TermPos
   types are allowed and expected to use PStack-Operations on them. */


typedef PStackCell TermPosCell;
typedef PStack_p   TermPos_p;

/* How many stack elements does a single position component  take */

#define TERM_POS_ELEMENT_SIZE 2

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


#define TermPosAlloc()    PStackAlloc()
#define TermPosFree(junk) PStackFree(junk)

#define TermPosIsTopPos(pos) PStackEmpty(pos)
static inline Term_p TermPosGetSubterm(Term_p term, TermPos_p pos);

static inline Term_p TermPosFirstLIPosition(Term_p term, TermPos_p pos);
Term_p TermPosNextLIPosition(TermPos_p pos);

void   TermPosPrint(FILE* out, TermPos_p pos);
void   TermPosDebugPrint(FILE* out, Sig_p sig, TermPos_p pos);


/*---------------------------------------------------------------------*/
/*                  Inline Functions                                   */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: TermPosGetSubterm()
//
//   Given a term and a position, return the denoted subterm.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static inline Term_p TermPosGetSubterm(Term_p term, TermPos_p pos)
{
   Term_p super;
   int    idx;

   assert(term);

   if(PStackEmpty(pos))
   {
      return term;
   }
   assert(PStackGetSP(pos)>=2);

   idx = PStackTopInt(pos);
   super = PStackBelowTopP(pos);

   assert(super->arity>idx);

   return super->args[idx];
}


/*-----------------------------------------------------------------------
//
// Function: TermPosFirstLIPosition()
//
//   Return the first subterm of term in leftmost-innermost order
//   and make pos the corresponding position.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static inline Term_p TermPosFirstLIPosition(Term_p term, TermPos_p pos)
{
   PStackReset(pos);

   while(term->arity)
   {
      PStackPushP(pos, term);
      PStackPushInt(pos, 0);
      term = term->args[0];
   }

   return term;
}


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





