/*-----------------------------------------------------------------------

File  : cco_paramodulation.h

Author: Stephan Schulz

Contents
 
  Functions for controling paramodulation inferences.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Sat Jul  5 02:28:25 MET DST 1997
    New

-----------------------------------------------------------------------*/

#ifndef CCO_PARAMODULATION

#define CCO_PARAMODULATION

#include <ccl_paramod.h>
#include <che_proofcontrol.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


long ComputeClauseClauseParamodulants(TB_p bank, OCB_p ocb, Clause_p
				      clause, Clause_p parent_alias,
				      Clause_p with, ClauseSet_p
				      store, VarBank_p freshvars,
                                      ParamodulationType pm_type);

long ComputeAllParamodulants(TB_p bank, OCB_p ocb, Clause_p clause,
			     Clause_p parent_alias, ClauseSet_p
			     with_set, ClauseSet_p store, VarBank_p
			     freshvars, ParamodulationType pm_type);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





