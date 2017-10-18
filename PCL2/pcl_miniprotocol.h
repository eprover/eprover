/*-----------------------------------------------------------------------

File  : pcl_miniprotocol.h

Author: Stephan Schulz

Contents

  Lists of MiniPCL steps

  Copyright 1998, 1999, 2002 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Thu Jul 11 17:37:03 MEST 2002
    New (from pcl_rotocol.h

-----------------------------------------------------------------------*/

#ifndef PCL_MINIPROTOCOL

#define PCL_MINIPROTOCOL

#include <pcl_ministeps.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct pclminiprotcell
{
   TB_p      terms;
   long      max_ident;
   PDArray_p steps; /* indexed by simple-ids */
}PCLMiniProtCell, *PCLMiniProt_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define PCLMiniProtCellAlloc() (PCLMiniProtCell*)SizeMalloc(sizeof(PCLMiniProtCell))
#define PCLMiniProtCellFree(junk)         SizeFree(junk, sizeof(PCLMiniProtCell))

PCLMiniProt_p PCLMiniProtAlloc(void);
void          PCLMiniProtFree(PCLMiniProt_p junk);

bool          PCLMiniProtInsertStep(PCLMiniProt_p prot, PCLMiniStep_p
                step);
PCLMiniStep_p PCLMiniProtFindStep(PCLMiniProt_p prot, unsigned long id);
PCLMiniStep_p PCLMiniProtExtractStep(PCLMiniProt_p prot, PCLMiniStep_p
                 step);
bool          PCLMiniProtDeleteStep(PCLMiniProt_p prot, PCLMiniStep_p
                step);


long      PCLMiniProtParse(Scanner_p in, PCLMiniProt_p prot);
void      PCLMiniProtPrint(FILE* out, PCLMiniProt_p prot,
            OutputFormatType format);

void      PCLMiniExprCollectPreconds(PCLMiniProt_p prot, PCLExpr_p expr,
                 PTree_p *tree);

bool      PCLMiniProtMarkProofClauses(PCLMiniProt_p prot, bool fast);
void      PCLMiniProtSetClauseProp(PCLMiniProt_p prot, PCLStepProperties props);
void      PCLMiniProtDelClauseProp(PCLMiniProt_p prot, PCLStepProperties props);

void      PCLMiniProtPrintProofClauses(FILE* out, PCLMiniProt_p prot,
                   OutputFormatType format);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/








