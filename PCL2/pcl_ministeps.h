/*-----------------------------------------------------------------------

File  : pcl_ministeps.h

Author: Stephan Schulz

Contents
 
  Maximally compact PCL steps, only for special purpose applications.

  Copyright 2002 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Wed Jul 10 20:44:47 MEST 2002
    New

-----------------------------------------------------------------------*/

#ifndef PCL_MINISTEPS

#define PCL_MINISTEPS

#include <pcl_miniclauses.h>
#include <pcl_expressions.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct pclministepcell
{
   unsigned long id;
   MiniClause_p  clause;
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


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





