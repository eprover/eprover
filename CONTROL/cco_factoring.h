/*-----------------------------------------------------------------------

File  : cco_factoring.h

Author: Stephan Schulz

Contents

  Routines for the control of factoring

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Mon Jun  8 17:10:03 MET DST 1998
    New

-----------------------------------------------------------------------*/

#ifndef CCO_FACTORING

#define CCO_FACTORING

#include <ccl_factor.h>
#include <che_proofcontrol.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


long ComputeAllOrderedFactors(TB_p bank, OCB_p ocb,
               Clause_p clause, ClauseSet_p store,
               VarBank_p freshvars);

long ComputeAllEqualityFactors(TB_p bank, OCB_p ocb,
                Clause_p clause, ClauseSet_p store,
                VarBank_p freshvars);



#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





