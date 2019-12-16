/*-----------------------------------------------------------------------

  File  : cle_clauseenc.h

  Author: Stephan Schulz

  Contents

  Functions for dealing with term representations of clauses.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Tue Jul 13 12:36:03 MET DST 1999

  -----------------------------------------------------------------------*/

#ifndef CLE_CLAUSEENC

#define CLE_CLAUSEENC

#include <cle_patterns.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/



/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

Term_p   FlatEncodeClauseListRep(TB_p bank, PStack_p list);
Term_p   RecEncodeClauseListRep(TB_p bank, PStack_p list);
Term_p   TermEncodeEqnList(TB_p bank, Eqn_p list, bool flat);
Term_p   FlatRecodeRecClauseRep(TB_p bank,Term_p clauserep);
Term_p   ParseClauseTermRep(Scanner_p in, TB_p bank, bool flat);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
