/*-----------------------------------------------------------------------

File  : pcl_protocol.h

Author: Stephan Schulz

Contents

  Lists of PCL steps

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
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
   long     number;
   PTree_p  steps;      /* Ordered by PCL-Id's */
   PStack_p in_order;   /* Steps in increasing order of ids. */
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
         ((prot)->number++),\
        (PTreeObjStore(&((prot)->steps), (step),\
        (ComparisonFunctionType)PCLStepIdCompare)))

PCLStep_p PCLProtExtractStep(PCLProt_p prot, PCLStep_p step);
bool      PCLProtDeleteStep(PCLProt_p prot, PCLStep_p step);

#define   PCLProtStepNo(prot) ((prot->number))
PCLStep_p PCLProtFindStep(PCLProt_p prot, PCLId_p id);

void      PCLProtSerialize(PCLProt_p prot);

long      PCLProtParse(Scanner_p in, PCLProt_p prot);
void      PCLProtPrintExtra(FILE* out, PCLProt_p prot, bool data,
             OutputFormatType format);
#define   PCLProtPrint(out, prot, format) PCLProtPrintExtra((out),\
                         (prot),\
                         false, \
                         (format))
bool      PCLStepHasFOFParent(PCLProt_p prot, PCLStep_p step);
long      PCLProtStripFOF(PCLProt_p prot);

void      PCLProtResetTreeData(PCLProt_p prot, bool just_weights);

void      PCLExprCollectPreconds(PCLProt_p prot, PCLExpr_p expr,
             PTree_p *tree);
#define   PCLStepCollectPreconds(prot, step, tree)\
          PCLExprCollectPreconds((prot), (step)->just, (tree))
PCLStep_p PCLExprGetQuotedArg(PCLProt_p prot, PCLExpr_p expr, int arg);

bool      PCLProtMarkProofClauses(PCLProt_p prot);
void      PCLProtSetProp(PCLProt_p prot, PCLStepProperties props);
void      PCLProtDelProp(PCLProt_p prot, PCLStepProperties props);
long      PCLProtCountProp(PCLProt_p prot, PCLStepProperties props);
long      PCLProtCollectPropSteps(PCLProt_p prot, PCLStepProperties props,
                                  PStack_p steps);
void      PCLProtPrintPropClauses(FILE* out, PCLProt_p prot,
              PCLStepProperties prop,
              OutputFormatType format);

#define PCLProtPrintProofClauses(out, prot, format)\
        PCLProtPrintPropClauses((out), (prot), PCLIsProofStep, format)
void    PCLProtPrintExamples(FILE* out, PCLProt_p prot);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





