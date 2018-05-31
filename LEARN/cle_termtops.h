/*-----------------------------------------------------------------------

  File  : cle_termtops.h

  Author: Stephan Schulz

  Contents

  Compute the various term tops for given (shared!) terms.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Tue Aug  3 17:14:11 MET DST 1999

  -----------------------------------------------------------------------*/

#ifndef CLE_TERMTOPS

#define CLE_TERMTOPS

#include <cte_termbanks.h>
#include <cle_patterns.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

Term_p TermTop(Term_p term, int depth, VarBank_p freshvars);
Term_p AltTermTop(Term_p term, int depth, VarBank_p freshvars);
Term_p CSTermTop(Term_p term, int depth, VarBank_p freshvars);
Term_p ESTermTop(Term_p term, int depth, VarBank_p freshvars);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
