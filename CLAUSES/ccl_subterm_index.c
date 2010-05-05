/*-----------------------------------------------------------------------

File  : ccl_subterm_index.c

Author: Stephan Schulz (schulz@eprover.org)

Contents
 
  An index mapping subterms to occurances in clauses.

  Copyright 2010 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Wed Apr 14 09:27:17 CEST 2010
    New

-----------------------------------------------------------------------*/

#include "ccl_subterm_index.h"



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
// Function: term_collect_idx_subterms()
//
//   
//   Collect all non-variable subterms in term either into rest or
//   full (rest for "restricted rewriting" terms, full for the "full
//   rewriting" terms).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static long term_collect_idx_subterms(Term_p term, PTree_p *rest,
                                      PTree_p *full, bool restricted) 
{
   long res = 1;
   int i;

   if(TermIsVar(term))
   {
      return 0;
   }
   if(restricted)
   {
      PTreeStore(rest, term);
   }
   else
   {
      PTreeStore(full, term);
   }
   for(i=0; i<term->arity; i++)
   {
      res += term_collect_idx_subterms(term->args[i], rest, full, false);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: eqn_collect_idx_subterms()
//
//   Collect all non-variable subterms in eqn either into rest or
//   full (rest for "restricted rewriting" terms, full for the "full
//   rewriting" terms).
//
// Global Variables: -
//
// Side Effects    : Via term_collect_idx_subterms()
//
/----------------------------------------------------------------------*/

static long eqn_collect_idx_subterms(Eqn_p eqn, PTree_p *rest, PTree_p *full)
{
   long res = 0;
   bool restricted_rw = EqnIsMaximal(eqn) && EqnIsPositive(eqn) && EqnIsOriented(eqn);

   res += term_collect_idx_subterms(eqn->lterm, rest, full, restricted_rw);
   res += term_collect_idx_subterms(eqn->lterm, rest, full, false);

   return res;
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/





/*-----------------------------------------------------------------------
//
// Function: ClauseCollectIdxSubterms()
//
//   Collect all non-variable subterms in clause either into rest or
//   full (rest for "restricted rewriting" terms, full for the "full
//   rewriting" terms).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

long ClauseCollectIdxSubterms(Clause_p clause, 
                              PTree_p *rest, 
                              PTree_p *full)
{
   long res = 0;
   Eqn_p handle;

   for(handle = clause->literals; handle; handle = handle->next)
   {
      res += eqn_collect_idx_subterms(handle, rest, full);
   }
   return res;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


