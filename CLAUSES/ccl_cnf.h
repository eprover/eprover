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


#define FormulaRefSimplify(var, terms) \
{\
   Formula_p frs_internal_handle, frs_internal_tmp,frs_internal_var;\
   frs_internal_var    = var;\
   frs_internal_handle = FormulaRelRef(frs_internal_var); \
   frs_internal_tmp    = FormulaSimplify(frs_internal_handle, (terms));\
   var = FormulaGetRef(frs_internal_tmp);\
   FormulaFree(frs_internal_var);\
}
Formula_p FormulaSimplify(Formula_p form, TB_p terms);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





