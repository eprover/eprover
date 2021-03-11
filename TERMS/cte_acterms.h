/*-----------------------------------------------------------------------

  File  : cte_acterms.h

  Author: Stephan Schulz

  Contents

  Functions and data types for terms in AC normal form (flattened,
  subterms sorted alphabetically).

  Copyright 2000, 2021 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: Fri Nov 17 20:02:15 MET 2000

-----------------------------------------------------------------------*/

#ifndef CTE_ACTERMS

#define CTE_ACTERMS

#include <clb_objtrees.h>
#include <cte_termfunc.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* AC-Terms are a lot simpler than normal terms, as we only need a
   small number of them, and then only temporary */

typedef struct actermopcell
{
   FunCode          f_code;        /* Top symbol of term */
   PDArray_p        args;         /* Pointer to array of arguments */
}ACTermCell, *ACTerm_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define ACTermCellAlloc() (ACTermCell*)SizeMalloc(sizeof(ACTermCell))
#define ACTermCellFree(junk)         SizeFree(junk, sizeof(ACTermCell))


ACTerm_p ACTermAlloc(FunCode f);
void     ACTermFree(ACTerm_p term);
int      ACTermCompare(ACTerm_p t1, ACTerm_p t2);
ACTerm_p ACTermNormalize(Sig_p sig, Term_p term);
void     ACTermPrint(FILE* out, ACTerm_p term, Sig_p sig);
bool     TermACEqual(Sig_p sig, Term_p t1, Term_p t2);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
