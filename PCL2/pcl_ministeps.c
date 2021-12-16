/*-----------------------------------------------------------------------

File  : pcl_ministeps.c

Author: Stephan Schulz

Contents

  Individual PCL steps and related stuff.

  Copyright 1998, 1999, 2002 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
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
   assert(junk && junk->id && junk->just);

   if(PCLStepIsFOF(junk))
   {
      /* Formua is garbage collected */
   }
   else
   {
      if(junk->logic.clause)
      {
         MiniClauseFree(junk->logic.clause);
      }
   }
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

   handle->bank = bank;
   handle->id = ParseInt(in);
   if(TestInpTok(in, Fullstop))
   {
      AktTokenError(in,
          "No compound PCL identifiers allowed in this mode",
          false);
   }
   AcceptInpTok(in, Colon);
   handle->properties = PCLParseExternalType(in);
   AcceptInpTok(in, Colon);

   if(SupportShellPCL && TestInpTok(in, Colon))
   {
      handle->logic.clause = NULL;
      PCLStepSetProp(handle, PCLIsShellStep);
   }
   else if(TestInpTok(in, OpenSquare))
   {
      handle->logic.clause = MinifyClause(ClausePCLParse(in, bank));
      PCLStepDelProp(handle, PCLIsFOFStep);
   }
   else
   {
      handle->logic.formula = TFormulaTPTPParse(in, bank);
      PCLStepSetProp(handle, PCLIsFOFStep);
   }
   AcceptInpTok(in, Colon);
   handle->just = PCLMiniExprParse(in);
   if(TestInpTok(in, Colon))
   {
      NextToken(in);
      CheckInpTok(in, SQString);
      handle->extra = DStrCopy(AktToken(in)->literal);
      NextToken(in);
   }
   else
   {
      handle->extra = NULL;
   }
   PCLStepDelProp(handle, PCLIsProofStep);
   if(handle->just->op == PCLOpInitial)
   {
      PCLStepSetProp(handle, PCLIsInitial);
   }
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
   PCLPrintExternalType(out, step->properties);
   fputs(" : ", out);
   if(!PCLStepIsShell(step))
   {
      if(PCLStepIsFOF(step))
      {
         TFormulaTPTPPrint(out, step->bank, step->logic.formula, true, true);
      }
      else
      {
         MiniClausePCLPrint(out, step->logic.clause, bank);
      }
   }
   fputs(" : ", out);
   PCLMiniExprPrint(out, step->just);
   if(step->extra)
   {
      fprintf(out, " : %s", step->extra);
   }
}


/*-----------------------------------------------------------------------
//
// Function: PCLMiniStepPrintTSTP()
//
//   Print a PCLMini step in TSTP format.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void PCLMiniStepPrintTSTP(FILE* out, PCLMiniStep_p step, TB_p bank)
{
   assert(step);

   if(PCLStepIsClausal(step))
   {
      fprintf(out, "cnf(%ld,%s,",step->id,
              PCLPropToTSTPType(step->properties));
      if(PCLStepIsShell(step))
      {
      }
      else
      {
         MiniClauseTSTPCorePrint(out, step->logic.clause, bank);
      }
   }
   else
   {
      fprintf(out, "fof(%ld, %s,", step->id,
              PCLPropToTSTPType(step->properties));
      if(PCLStepIsShell(step))
      {
      }
      else
      {
         TFormulaTPTPPrint(out, step->bank, step->logic.formula, true, true);
      }
   }
   fputc(',', out);
   PCLExprPrintTSTP(out, step->just, true);
   if(step->extra)
   {
      fprintf(out, ",[%s]", step->extra);
   }
   fputs(").", out);
}


/*-----------------------------------------------------------------------
//
// Function: PCLMiniStepPrintFormat()
//
//   Print a PCL step in the requested format.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void PCLMiniStepPrintFormat(FILE* out, PCLMiniStep_p step, TB_p bank,
                            OutputFormatType format)
{
   switch(format)
   {
   case pcl_format:
    PCLMiniStepPrint(out, step, bank);
    break;
   case tstp_format:
    PCLMiniStepPrintTSTP(out, step, bank);
    break;
   default:
    assert(false);
    break;
   }
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/



