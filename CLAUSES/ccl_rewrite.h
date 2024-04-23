/*-----------------------------------------------------------------------

File  : ccl_rewrite.h

Author: Stephan Schulz

Contents

  Functions for rewriting terms and clauses with clause sets.

Copyright 1998-2011 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Tue May 26 19:47:52 MET DST 1998
    New

-----------------------------------------------------------------------*/

#ifndef CCL_REWRITE

#define CCL_REWRITE

#include <cte_replace.h>
#include <ccl_pdtrees.h>
#include <ccl_clausefunc.h>
#include <ccl_subterm_index.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* Collect all necessary information for rewriting into one structure */

typedef struct rw_desc_cell
{
   OCB_p        ocb;
   TB_p         bank;
   ClauseSet_p  *demods;
   SysDate      demod_date;
   RewriteLevel level;
   bool         prefer_general;
   bool         sos_rewritten; /* Return value! */
}RWDescCell, *RWDesc_p;




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


#define RWDescCellAlloc()    (RWDescCell*)SizeMalloc(sizeof(RWDescCell))
#define RWDescCellFree(junk) SizeFree(junk, sizeof(RWDescCell))


extern unsigned long RewriteAttempts;
extern unsigned long RewriteSuccesses;
extern unsigned long RewriteUnboundVarFails;
extern unsigned long RewriteUncached;
extern unsigned long BWRWMatchAttempts;
extern unsigned long BWRWMatchSuccesses;


Term_p TermComputeLINormalform(OCB_p ocb, TB_p bank, Term_p term,
                ClauseSet_p *demodulators,
                RewriteLevel level,
                               bool prefer_general,
                               bool restricted_rw,
                               bool lambda_demod);

long ClauseComputeLINormalform(OCB_p ocb, TB_p bank, Clause_p
                               clause, ClauseSet_p *demodulators,
                               RewriteLevel level, bool prefer_general,
                               bool lambda_demod);

long ClauseSetComputeLINormalform(OCB_p ocb, TB_p bank, ClauseSet_p
              set, ClauseSet_p *demodulators,
              RewriteLevel level, bool prefer_general,
              bool lambda_demod);

bool FindRewritableClauses(OCB_p ocb, ClauseSet_p set,
            PStack_p results, Clause_p
            new_demod, SysDate nf_date);

long FindRewritableClausesIndexed(OCB_p ocb, SubtermIndex_p index,
                                  PStack_p stack, Clause_p new_demod,
                                  SysDate nf_date);

bool ClauseLocalRW(OCB_p ocb, Clause_p clause);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
