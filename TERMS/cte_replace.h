/*-----------------------------------------------------------------------

File  : cte_replace.h

Author: Stephan Schulz

Contents

  Functions for replacing and rewriting of terms.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
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


void   TermAddRWLink(Term_p term, Term_p replace,
                     struct clause_cell *demod,
                     bool sos, RWResultType type);

void   TermDeleteRWLink(Term_p term);
Term_p TermFollowRWChain(Term_p term);

Term_p TBTermPosReplace(TB_p bank, Term_p repl, TermPos_p pos,
                        DerefType deref, int remains, Term_p orig);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





