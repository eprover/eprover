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

typedef struct pclstepcell
{
   PCLId_p   id;
   Clause_p  clause;
   PCLExpr_p just;
   bool      extra_string;
   char*     extra;
}PCLStepCell, *PCLStep_p;



/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


#define PCLStepCellAlloc() (PCLStepCell*)SizeMalloc(sizeof(PCLStepCell))
#define PCLStepCellFree(junk)         SizeFree(junk, sizeof(PCLStepCell))

void      PCLStepFree(PCLStep_p junk);

PCLStep_p PCLStepParse(Scanner_p in, TB_p bank);
void      PCLStepPrint(FILE* out, PCLStep_p step);

int       PCLStepIdCompare(PCLStep_p step1, PCLStep_p step2);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





