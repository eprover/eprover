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
   TB_p    terms;
   PTree_p steps; /* Ordered by PCL-Id's */
}PCLProtCell, *PCLProt_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define PCLProtCellAlloc() (PCLProtCell*)SizeMalloc(sizeof(PCLProtCell))
#define PCLProtCellFree(junk)         SizeFree(junk, sizeof(PCLProtCell))

PCLProt_p PCLProtAlloc(void);
void      PCLProtFree(PCLProt_p junk);

#define PCLProtInsertStep(prot, step) \
        PTreeObjStore(&((prot)->steps), (step),\
        (ComparisonFunctionType)PCLStepIdCompare)

#define PCLProtExtractStep(prot, step) \
        PTreeObjExtractObject(&((prot)->steps), (step),\
        (ComparisonFunctionType)PCLStepIdCompare) 

bool      PCLProtDeleteStep(PCLProt_p prot, PCLStep_p step);

PCLStep_p PCLProtFindStep(PCLProt_p prot, PCLId_p id);

long      PCLProtParse(Scanner_p in, PCLProt_p prot);
void      PCLProtPrint(FILE* out, PCLProt_p prot);

void      PCLProtResetTreeData(PCLProt_p prot);

void      PCLExprCollectPreconds(PCLProt_p prot, PCLExpr_p expr,
				 PTree_p *tree);

long      PCLProtMarkProofClauses(PCLProt_p prot);
void      PCLProtPrintProofClauses(FILE* out, PCLProt_p prot);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





