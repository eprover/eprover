/*-----------------------------------------------------------------------

File  : pcl_protocol.h

Author: Stephan Schulz

Contents

  Lists of PCL steps 

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Sat Apr  1 22:17:54 GMT 2000
    New

-----------------------------------------------------------------------*/

#ifndef PCL_PROTOCOL

#define PCL_PROTOCOL

#include <pcl_steps.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct pclprotcell
{
   TB_p     terms;
   PTree_p  steps; /* Ordered by PCL-Id's */
   PStack_p in_order; /* Steps in increasing order of ids. */
   bool     is_ordered; /* True if previous is true ;-) */
}PCLProtCell, *PCLProt_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define PCLProtCellAlloc() (PCLProtCell*)SizeMalloc(sizeof(PCLProtCell))
#define PCLProtCellFree(junk)         SizeFree(junk, sizeof(PCLProtCell))

PCLProt_p PCLProtAlloc(void);
void      PCLProtFree(PCLProt_p junk);

#define PCLProtInsertStep(prot, step) \
        (((prot)->is_ordered = false),\
        (PTreeObjStore(&((prot)->steps), (step),\
        (ComparisonFunctionType)PCLStepIdCompare)))

#define PCLProtExtractStep(prot, step) \
        PTreeObjExtractObject(&((prot)->steps), (step),\
        (ComparisonFunctionType)PCLStepIdCompare) 

bool      PCLProtDeleteStep(PCLProt_p prot, PCLStep_p step);

PCLStep_p PCLProtFindStep(PCLProt_p prot, PCLId_p id);
void      PCLProtSerialize(PCLProt_p prot);

long      PCLProtParse(Scanner_p in, PCLProt_p prot);
void      PCLProtPrintExtra(FILE* out, PCLProt_p prot, bool data);
#define   PCLProtPrint(out, prot) PCLProtPrintExtra((out), (prot), false)

void      PCLProtResetTreeData(PCLProt_p prot, bool just_weights);

void      PCLExprCollectPreconds(PCLProt_p prot, PCLExpr_p expr,
				 PTree_p *tree);

bool      PCLProtMarkProofClauses(PCLProt_p prot);
void      PCLProtPrintPropClauses(FILE* out, PCLProt_p prot, 
				  PCLStepProperties prop, bool just_clauses);
#define PCLProtPrintProofClauses(out, prot)\
        PCLProtPrintPropClauses((out), (prot), PCLIsProofStep, false)


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





