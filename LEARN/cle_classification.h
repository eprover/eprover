/*-----------------------------------------------------------------------

File  : cle_classification.h

Author: Stephan Schulz

Contents

  Functions for using TSM's as classification tools on terms.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Fri Aug 13 20:26:50 MET DST 1999
    New

-----------------------------------------------------------------------*/

#ifndef CLE_CLASSIFICATION

#define CLE_CLASSIFICATION

#include <cle_tsm.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


double TSMTermClassify(TSMAdmin_p admin, Term_p term, PatternSubst_p
             subst);

bool   TSMClassifiedTermCheck(TSMAdmin_p admin, FlatAnnoTerm_p term);

long   TSMClassifySet(TSMAdmin_p admin, FlatAnnoSet_p set);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





