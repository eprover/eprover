/*-----------------------------------------------------------------------

File  : ccl_subterm_index.h

Author: Stephan Schulz (schulz@eprover.org)

Contents
 
  A simple (hashed) index from terms to clauses in which this term
  appears as priviledged (rewriting restricted) or unpriviledged term. 

  Copyright 2010 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Wed May  5 10:19:14 CEST 2010
    New

-----------------------------------------------------------------------*/

#ifndef CCL_SUBTERM_INDEX

#define CCL_SUBTERM_INDEX

#include <cte_fp_index.h>
#include <ccl_clauses.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


long         ClauseCollectIdxSubterms(Clause_p clause, 
                                      PTree_p *rest, 
                                      PTree_p *full);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





