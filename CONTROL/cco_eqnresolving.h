/*-----------------------------------------------------------------------

File  : cco_eqnresolving.h

Author: Stephan Schulz

Contents

  Routines for the control of equality resolution.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Mon Jun  8 17:10:03 MET DST 1998
    New

-----------------------------------------------------------------------*/

#ifndef CCO_EQNRESOLVING

#define CCO_EQNRESOLVING

#include <ccl_eqnresolution.h>
#include <che_proofcontrol.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

long ComputeAllEqnResolvents(TB_p bank, Clause_p clause, ClauseSet_p
              store, VarBank_p freshvars);

long ClauseERNormalizeVar(TB_p bank, Clause_p clause, ClauseSet_p
           store, VarBank_p freshvars, bool strong);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





