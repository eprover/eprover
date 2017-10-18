/*-----------------------------------------------------------------------

File  : pcl_analysis.h

Author: Stephan Schulz

Contents

  Code for analysing PCL protocols, replacing (much of) what used to
  be in ANALYSIS for old E style proofs.

  Copyright 2004 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Tue Feb  3 23:26:44 CET 2004
    New

-----------------------------------------------------------------------*/

#ifndef PCL_ANALYSIS

#define PCL_ANALYSIS

#include <pcl_protocol.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/



/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


long PCLExprProofDistance(PCLProt_p prot, PCLExpr_p expr);
long PCLStepProofDistance(PCLProt_p prot, PCLStep_p step);
void PCLProtProofDistance(PCLProt_p prot);

void PCLExprUpdateGRefs(PCLProt_p prot, PCLExpr_p expr, bool proofstep);
#define PCLStepUpdateGRefs(prot, step) PCLExprUpdateGRefs((prot),(step)->just, PCLStepQueryProp((step),PCLIsProofStep))
void PCLProtUpdateGRefs(PCLProt_p prot);

long PCLProtSelectExamples(PCLProt_p prot, long neg_examples);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





