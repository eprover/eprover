/*-----------------------------------------------------------------------

File  : ccl_cnf.h

Author: Stephan Schulz

Contents

  Functions implementing (eventually) the CNF conversion of first
  order formulae. 

  Copyright 1998-2003 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Wed Feb 25 23:50:31 CET 2004
    New

-----------------------------------------------------------------------*/

#ifndef CCL_CNF

#define CCL_CNF

#include <ccl_formulae.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

bool FormulaSimplify(Formula_p *form, TB_p terms);

bool FormulaNNF(Formula_p *form, TB_p terms, int polarity);

bool FormulaMiniScope(Formula_p *form);

Formula_p FormulaVarRename(Formula_p form, TB_p terms);

Formula_p FormulaSkolemizeOutermost(Formula_p form, TB_p terms);

Formula_p FormulaDistributeDisjunctions(Formula_p form);

bool FormulaCNF(Formula_p *form, TB_p terms);





#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





