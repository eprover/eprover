/*-----------------------------------------------------------------------

File  : pcl_steps.h

Author: Stephan Schulz

Contents
 
  PCL steps.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Thu Mar 30 17:52:53 MET DST 2000
    New

-----------------------------------------------------------------------*/

#ifndef PCL_STEPS

#define PCL_STEPS

#include <pcl_expressions.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef enum 
{
   PCLNoProp        =  0,
   PCLIsLemma       =  1,
   PCLIsInitial     =  2,
   PCLIsFinal       =  4,
   PCLIsMarked      =  8,
   PCLIsProofStep   = 16
}PCLStepProperties;


typedef struct pclstepcell
{
   PCLId_p           id;
   Clause_p          clause;
   PCLExpr_p         just;
   char*             extra;
   PCLStepProperties properties;
   long              proof_dag_size;
   long              proof_tree_size;
   long              active_pm_refs;
   long              other_generating_refs;
   long              active_simpl_refs;
   long              passive_simpl_refs;
   long              pure_quote_refs;
   long              proof_distance;
   float             lemma_quality;
}PCLStepCell, *PCLStep_p;

#define PCLNoWeight -1


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


#define PCLStepCellAlloc() (PCLStepCell*)SizeMalloc(sizeof(PCLStepCell))
#define PCLStepCellFree(junk)         SizeFree(junk, sizeof(PCLStepCell))

#define PCLStepSetProp(clause, prop) SetProp((clause), (prop))
#define PCLStepDelProp(clause, prop) DelProp((clause), (prop))
#define PCLStepGiveProps(clause, prop) GiveProps((clause), (prop))
#define PCLStepQueryProp(clause, prop) QueryProp((clause), (prop))
#define PCLStepIsAnyPropSet(clause, prop) IsAnyPropSet((clause), (prop))

void      PCLStepFree(PCLStep_p junk);

PCLStep_p PCLStepParse(Scanner_p in, TB_p bank);
void      PCLStepPrintExtra(FILE* out, PCLStep_p step, bool data);
#define   PCLStepPrint(out, step) PCLStepPrintExtra((out),(step),false)
void      PCLStepPrintTSTP(FILE* out, PCLStep_p step);
void      PCLStepPrintFormat(FILE* out, PCLStep_p step, bool data, 
			     OutputFormatType format);

int       PCLStepIdCompare(PCLStep_p step1, PCLStep_p step2);
void      PCLStepResetTreeData(PCLStep_p step, bool just_weights);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





