/*-----------------------------------------------------------------------

File  : pcl_ministeps.h

Author: Stephan Schulz

Contents

  Maximally compact PCL steps, only for special purpose applications.

  Copyright 2002 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed Jul 10 20:44:47 MEST 2002
    New

-----------------------------------------------------------------------*/

#ifndef PCL_MINISTEPS

#define PCL_MINISTEPS

#include <pcl_miniclauses.h>
#include <pcl_expressions.h>
#include <pcl_steps.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct pclministepcell
{
   TB_p             bank;
   long id;
   union
   {
      MiniClause_p  clause;
      TFormula_p    formula;
   }logic;
   PCLStepProperties properties;
   PCLExpr_p just;
   char*     extra;
}PCLMiniStepCell, *PCLMiniStep_p;



/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


#define PCLMiniStepCellAlloc() (PCLMiniStepCell*)SizeMalloc(sizeof(PCLMiniStepCell))
#define PCLMiniStepCellFree(junk)         SizeFree(junk, sizeof(PCLMiniStepCell))

void          PCLMiniStepFree(PCLMiniStep_p junk);

PCLMiniStep_p PCLMiniStepParse(Scanner_p in, TB_p bank);
void          PCLMiniStepPrint(FILE* out, PCLMiniStep_p step, TB_p bank);
void          PCLMiniStepPrintFormat(FILE* out, PCLMiniStep_p step,
                 TB_p bank, OutputFormatType format);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





