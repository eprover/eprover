/*-----------------------------------------------------------------------

File  : pcl_propanalysis.h

Author: Stephan Schulz

Contents

  Functions for computing various properties of the clauses in a PCL
  protocol.

  Copyright 2002 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Thu Feb 28 16:27:34 MET 2002
    New

-----------------------------------------------------------------------*/

#ifndef PCL_PROPANALYIS

#define PCL_PROPANALYIS

#include <che_clausefeatures.h>
#include <pcl_protocol.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct pcl_propdata_cell
{
   long      fof_formulae;
   long      pos_clauses;
   long      neg_clauses;
   long      mix_clauses;
   long      pos_clause_literals;
   long      neg_clause_literals;
   long      mix_clause_literals;
   long      pos_literals;
   long      neg_literals;
   long      const_count;
   long      func_count;
   long      pred_count;
   long      var_count;
   PCLStep_p longest_clause;
   PCLStep_p max_symbol_clause;
   PCLStep_p max_standard_weight_clause;
   PCLStep_p max_depth_clause;
}PCLPropDataCell, *PCLPropData_p;



/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

typedef int (*PCLCmpFunType)(const PCLStep_p, const PCLStep_p);

PCLStep_p PCLProtFindMaxStep(PCLProt_p prot, PCLCmpFunType cmp);

void      PCLProtPropAnalyse(PCLProt_p prot, PCLPropData_p data);

void      PCLProtPropDataPrint(FILE* out, PCLPropData_p data);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/






