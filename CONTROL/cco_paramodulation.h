/*-----------------------------------------------------------------------

  File  : cco_paramodulation.h

  Author: Stephan Schulz

  Contents

  Functions for controling paramodulation inferences.

  Copyright 1998-2020 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Sat Jul  5 02:28:25 MET DST 1997

  -----------------------------------------------------------------------*/

#ifndef CCO_PARAMODULATION

#define CCO_PARAMODULATION

#include <cte_idx_fp.h>
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



long ComputeIntoParamodulants(ParamodInfo_p pminfo,
                              ParamodulationType type,
                              Clause_p clause,
                              OverlapIndex_p into_index,
                              OverlapIndex_p negp_index,
                              ClauseSet_p store);

long ComputeFromParamodulants(ParamodInfo_p pminfo,
                              ParamodulationType type,
                              Clause_p clause,
                              OverlapIndex_p from_index,
                              ClauseSet_p store);

long ComputeFromSimParamodulants(ParamodInfo_p pminfo,
                                 ParamodulationType type,
                                 Clause_p clause,
                                 OverlapIndex_p from_index,
                                 ClauseSet_p store);

long ComputeAllParamodulantsIndexed(TB_p bank, OCB_p ocb,
                                    VarBank_p freshvars,
                                    Clause_p clause,
                                    Clause_p parent_alias,
                                    OverlapIndex_p into_index,
                                    OverlapIndex_p negp_index,
                                    OverlapIndex_p from_index,
                                    ClauseSet_p store,
                                    ParamodulationType pm_type);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
