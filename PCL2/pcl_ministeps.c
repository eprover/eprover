/*-----------------------------------------------------------------------

File  : pcl_ministeps.c

Author: Stephan Schulz

Contents

  Individual PCL steps and related stuff. 

  Copyright 1998, 1999, 2002 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Wed Jul 10 21:14:29 MEST 2002
    Ripped from pcl_steps.c

-----------------------------------------------------------------------*/

#include "pcl_ministeps.h"


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: PCLMiniStepFree()
//
//   Free a PCLMini step.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void PCLMiniStepFree(PCLMiniStep_p junk)
{
   assert(junk && junk->id && junk->clause && junk->just);
   
   MiniClauseFree(junk->clause);
   PCLMiniExprFree(junk->just);
   if(junk->extra)
   {
      FREE(junk->extra);
   }
   PCLMiniStepCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: PCLMiniStepParse()
//
//   Parse a PCLMini step.
//
// Global Variables: -
//
// Side Effects    : Input, memory operations
//
/----------------------------------------------------------------------*/

PCLMiniStep_p PCLMiniStepParse(Scanner_p in, TB_p bank)
{
   PCLMiniStep_p handle = PCLMiniStepCellAlloc();
   
   assert(in);
   assert(bank);
   
   handle->id = ParseInt(in);
   if(TestInpTok(in, Fullstop))
   {
      AktTokenError(in, 
		    "No compound PCL identifiers allowed in this mode",
		    false); 
   }
   AcceptInpTok(in, Colon);
   handle->clause = MinifyClause(ClausePCLParse(in, bank));
   AcceptInpTok(in, Colon);
   handle->just = PCLMiniExprParse(in);
   if(TestInpTok(in, Colon))
   {
      NextToken(in);
      CheckInpTok(in, String);
      handle->extra = DStrCopy(AktToken(in)->literal);
      NextToken(in);
   }    
   else
   {
      handle->extra = NULL;
   }
   ClauseDelProp(handle->clause, CPIsProofClause);
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: PCLMiniStepPrint()
//
//   Print a PCLMini step.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void PCLMiniStepPrint(FILE* out, PCLMiniStep_p step, TB_p bank)
{
   assert(step);

   fprintf(out, "%6ld : ", step->id);
   MiniClausePCLPrint(out, step->clause, bank);
   fputs(" : ", out);
   PCLMiniExprPrint(out, step->just);
   if(step->extra)
   {
      fprintf(out, " : \"%s\"", step->extra);
   }
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


