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


/* Given a variable (which is a counted reference to a formula),
 * rewrite the formula pointed to by the variable with the named
 * function, making sure that 
 * unused parts get freed */

#define FormulaRefRewrite(var, terms, func, polarity) \
{\
   Formula_p frs_internal_tmp,frs_internal_var;\
   frs_internal_var    = var;\
   frs_internal_tmp    = func(frs_internal_var, (terms), (polarity));\
   var = FormulaGetRef(frs_internal_tmp);\
   FormulaRelRef(frs_internal_var);\
   FormulaFree(frs_internal_var);\
   assert(var->ref_count >= 1);\
}

#define FormulaRefSimplify(var, terms,polarity)\
        FormulaRefRewrite((var),(terms),FormulaSimplify,0)
Formula_p FormulaSimplify(Formula_p form, TB_p terms, int polarity);


bool FormulaNNF(Formula_p *form, TB_p terms, int polarity);


#define FormulaRefCNF(var, terms,polarity)\
        FormulaRefRewrite((var),(terms),FormulaCNF,polarity)
Formula_p FormulaCNF(Formula_p form, TB_p terms, int polarity);




#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





