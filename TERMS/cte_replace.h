/*-----------------------------------------------------------------------

File  : cte_replace.h

Author: Stephan Schulz

Contents
 
  Functions for replacing and rewriting of terms.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Mon Jan 12 17:50:21 MET 1998
    New

-----------------------------------------------------------------------*/

#ifndef CTE_REPLACE

#define CTE_REPLACE

#include <clb_pqueue.h>
#include <cte_termcpos.h>
#include <cio_output.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/** Can a term be rewritten, rewritten if not protected, or always
 ** rewritten? */

typedef enum 
{
   RWNotRewritable     = 0,
   RWLimitedRewritable = 1,
   RWAlwaysRewritable  = 2,
}RWResultType;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


void   TermAddRWLink(Term_p term, Term_p replace, long demod,
		     bool sos, RWResultType type);

void   TermDeleteRWLink(Term_p term);
Term_p TermFollowRWChain(Term_p term);

bool   TermComputeRWSequence(PStack_p stack, Term_p from, Term_p to);

Term_p TBTermPosReplace(TB_p bank, Term_p repl, TermPos_p pos,
			DerefType deref);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





