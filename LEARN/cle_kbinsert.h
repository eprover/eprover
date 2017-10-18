/*-----------------------------------------------------------------------

File  : cle_kbinsert.h

Author: Stephan Schulz

Contents

  Functions for implementing the kb-insert operation.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Tue Jul 27 22:10:34 GMT 1999
    New

-----------------------------------------------------------------------*/

#ifndef CLE_KBINSERT

#define CLE_KBINSERT

#include <cle_kbdesc.h>
#include <cle_annoterms.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

AnnoTerm_p ParseExampleClause(Scanner_p in, TB_p parse_terms, TB_p
               internal_terms, long ident);

long KBAxiomsInsert(ExampleSet_p set, ClauseSet_p axioms, Sig_p sig,
          char* name);


void KBParseExampleFile(Scanner_p in, char* name, ExampleSet_p set,
         AnnoSet_p examples, Sig_p res_sig);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





