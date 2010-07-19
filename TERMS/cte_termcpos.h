/*-----------------------------------------------------------------------

File  : cte_termcpos.h

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Functions dealing with compact term positions represented by simple
  integers. Subterms are numbered in standard left-right pre-order,
  with the root position at 0.

  Copyright 2006 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sun Nov 12 14:18:49 ICT 2006
    New

-----------------------------------------------------------------------*/

#ifndef CTE_TERMCPOS

#define CTE_TERMCPOS

#include <cte_termpos.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef long TermCPos;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define  TermCPosIsTopPos(pos) ((pos)==0)
Term_p   TermCPosGetSubterm(Term_p term, TermCPos pos);
TermCPos TermCPosFromTermPos(TermPos_p termpos);
bool     TermPosFromTermCPos(Term_p term, TermCPos pos);
void     TermPrintAllCPos(FILE* out, TB_p bank, Term_p term);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





