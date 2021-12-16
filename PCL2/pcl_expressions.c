/*-----------------------------------------------------------------------

  File  : pcl_expressions.c

  Author: Stephan Schulz

  Contents

  PCL2 Expressions

  Copyright 1998, 1999, 2018 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Wed Mar 29 00:29:18 GMT 2000

  -----------------------------------------------------------------------*/

#include "pcl_expressions.h"



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
// Function: PCLExprAlloc()
//
//   Allocate an initialized PCL-expression-cell
//
// Global Variables: -
//
// Side Effects    : Memory allocation
//
/----------------------------------------------------------------------*/

PCLExpr_p PCLExprAlloc(void)
{
   PCLExpr_p handle = PCLExprCellAlloc();

   handle->op     = PCLOpNoOp;
   handle->arg_no = 0;
   handle->args   = PDArrayAlloc(2,2);

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: PCLExprFree()
//
//   Free a PCL-expr-cell.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void PCLExprFree(PCLExpr_p junk)
{
   long      i;
   PCLExpr_p expr;
   PCLId_p   ident;
   PCL2Pos_p pos;

   assert(junk);

   for(i=0; i<junk->arg_no; i++)
   {
      if(junk->op==PCLOpQuote)
      {
         ident = PCLExprArg(junk,i);
         PCLIdFree(ident);
      }
      else if(junk->op==PCLOpInitial)
      {
         assert(PCLExprArg(junk,i));
         ClauseInfoFree(PCLExprArg(junk,i));
      }
      else
      {
         expr = PCLExprArg(junk,i);
         PCLExprFree(expr);
      }
      pos = PCLExprArgPos(junk,i);
      if(pos)
      {
         PCL2PosFree(pos);
      }
   }
   PDArrayFree(junk->args);
   PCLExprCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: PCLMiniExprFree()
//
//   Free a PCL Mini-Expression.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void PCLMiniExprFree(PCLExpr_p junk)
{
   long      i;
   PCLExpr_p expr;
   PCL2Pos_p pos;

   assert(junk);

   for(i=0; i<junk->arg_no; i++)
   {
      if(junk->op==PCLOpQuote)
      {
         /* Do nothing - its' just a a long stored in the array */
      }
      else if(junk->op==PCLOpInitial)
      {
         assert(PCLExprArg(junk,i));
         ClauseInfoFree(PCLExprArg(junk,i));
      }
      else
      {
         expr = PCLExprArg(junk,i);
         PCLMiniExprFree(expr);
      }
      pos = PCLExprArgPos(junk,i);
      if(pos)
      {
         PCL2PosFree(pos);
      }
   }
   PDArrayFree(junk->args);
   PCLExprCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: PCLExprParse()
//
//   Parse a PCL-expression or Mini-expression
//
// Global Variables: -
//
// Side Effects    : Input, memory allocation
//
/----------------------------------------------------------------------*/

PCLExpr_p PCLExprParse(Scanner_p in, bool mini)
{
   PCLExpr_p handle=PCLExprAlloc();
   long      i, arg_no=0;
   ClauseInfo_p info = NULL;

   if(TestInpTok(in,PosInt))
   {
      handle->op = PCLOpQuote;
      if(mini)
      {
         PCLExprArgInt(handle,0)=ParseInt(in);
      }
      else
      {
         PCLExprArg(handle,0)=PCLIdParse(in);
      }
      if(TestInpTok(in,OpenBracket))
      {
         PCLExprArgPos(handle,0)=PCL2PosParse(in);
      }
      else
      {
         PCLExprArgPos(handle,0)=NULL;
      }
      handle->arg_no=1;
   }
   else if(TestInpId(in, "initial"))
   {
      handle->op = PCLOpInitial;
      NextToken(in);
      if(TestInpTok(in, OpenBracket))
      {
         info = ClauseInfoAllocEmpty();
         NextToken(in);
         info->source = DStrCopyCore(AktToken(in)->literal);
         AcceptInpTok(in, String);
         AcceptInpTok(in, Comma);
         info->name = DStrCopy(AktToken(in)->literal);
         AcceptInpTok(in, Name|PosInt|SQString);
         AcceptInpTok(in, CloseBracket);
         handle->arg_no = 1;
         PCLExprArg(handle,0) = info;
      }
      else
      {
         handle->arg_no = 0;
      }
   }
   else
   {
      CheckInpId(in, PCL_EVALGC"|"PCL_ER"|"PCL_PM"|"PCL_SPM"|"PCL_EF"|"
                 PCL_SAT"|"
                 PCL_CONDENSE"|"PCL_RW"|"PCL_SR"|"PCL_CSR"|"PCL_ACRES"|"
                 PCL_CN"|"PCL_SPLIT"|"PCL_SC"|"PCL_SE"|"PCL_FS"|"
                 PCL_NNF"|"PCL_ID"|"PCL_AD"|"PCL_SQ"|"PCL_VR"|"
                 PCL_SK"|"PCL_DSTR"|"PCL_ANNOQ"|"PCL_EVANS"|"PCL_NC  );

      if(TestInpId(in, PCL_EVALGC))
      {
         handle->op=PCLOpEvalGC;
         arg_no=1;
      }
      else if(TestInpId(in, PCL_ER))
      {
         handle->op=PCLOpEResolution;
         arg_no=1;
      }
      else if(TestInpId(in, PCL_PM))
      {
         handle->op=PCLOpParamod;
         arg_no=2;
      }
      else if(TestInpId(in, PCL_SPM))
      {
         handle->op=PCLOpSimParamod;
         arg_no=2;
      }
      else if(TestInpId(in, PCL_EF))
      {
         handle->op=PCLOpEFactoring;
         arg_no=1;
      }
      else if(TestInpId(in, PCL_SAT))
      {
         handle->op=PCLOpSatCheck;
         arg_no=PCL_VAR_ARG;
      }
      else if(TestInpId(in, PCL_CONDENSE))
      {
         handle->op=PCLOpCondense;
         arg_no=1;
      }
      else if(TestInpId(in, PCL_RW))
      {
         handle->op=PCLOpRewrite;
         arg_no=2;
      }
      else if(TestInpId(in, PCL_SR))
      {
         handle->op=PCLOpSimplifyReflect;
         arg_no=2;
      }
      else if(TestInpId(in, PCL_CSR))
      {
         handle->op=PCLOpContextSimplifyReflect;
         arg_no=2;
      }
      else if(TestInpId(in, PCL_ACRES))
      {
         handle->op=PCLOpACResolution;
         arg_no=PCL_VAR_ARG;
      }
      else if(TestInpId(in, PCL_CN))
      {
         handle->op=PCLOpClauseNormalize;
         arg_no=1;
      }
      else if(TestInpId(in, PCL_SPLIT))
      {
         handle->op=PCLOpSplitClause;
         arg_no=1;
      }
      else if(TestInpId(in, PCL_SC))
      {
         handle->op=PCLOpFOFSplitConjunct;
         arg_no=1;
      }
      else if(TestInpId(in, PCL_SE))
      {
         handle->op=PCLOpSplitEquiv;
         arg_no=1;
      }
      else if(TestInpId(in, PCL_FS))
      {
         handle->op=PCLOpFOFSimplify;
         arg_no=1;
      }
      else if(TestInpId(in, PCL_NNF))
      {
         handle->op=PCLOpFOFDeMorgan;
         arg_no=1;
      }
      else if(TestInpId(in, PCL_ID))
      {
         handle->op=PCLOpIntroDef;
         arg_no=0;
      }
      else if(TestInpId(in, PCL_AD))
      {
         handle->op=PCLOpApplyDef;
         arg_no=2;
      }
      else if(TestInpId(in, PCL_SQ))
      {
         handle->op=PCLOpFOFDistributeQuantors;
         arg_no=1;
      }
      else if(TestInpId(in, PCL_VR))
      {
         handle->op=PCLOpFOFVarRename;
         arg_no=1;
      }
      else if(TestInpId(in, PCL_SK))
      {
         handle->op=PCLOpFOFSkolemize;
         arg_no=1;
      }
      else if(TestInpId(in, PCL_DSTR))
      {
         handle->op=PCLOpFOFDistributeDisjunction;
         arg_no=1;
      }
      else if(TestInpId(in, PCL_ANNOQ))
      {
         handle->op=PCLOpAnnotateQuestion;
         arg_no=1;
      }
      else if(TestInpId(in, PCL_EVANS))
      {
         handle->op=PCLOpEvalAnswers;
         arg_no=1;
      }
      else if(TestInpId(in, PCL_NC))
      {
         handle->op=PCLOpFOFAssumeNegation;
         arg_no=1;
      }
      NextToken(in);
      if(arg_no)
      {
         AcceptInpTok(in,OpenBracket);
         PCLExprArg(handle,0)=PCLExprParse(in, mini);
         if(TestInpTok(in,OpenBracket))
         {
            PCLExprArgPos(handle,0)=PCL2PosParse(in);
         }
         else
         {
            PCLExprArgPos(handle,0)=NULL;
         }
         for(i=1; TestInpTok(in, Comma); i++)
         {
            AcceptInpTok(in, Comma);
            PCLExprArg(handle,i)=PCLExprParse(in, mini);
            if(TestInpTok(in,OpenBracket))
            {
               PCLExprArgPos(handle,i)=PCL2PosParse(in);
            }
            else
            {
               PCLExprArgPos(handle,i)=NULL;
            }
         }
         if((arg_no!=PCL_VAR_ARG) && (arg_no!=i))
         {
            AktTokenError(in, "Wrong number of arguments in PCL "
                          "expression", false);
         }
         AcceptInpTok(in,CloseBracket);
         handle->arg_no=i;
      }
      else
      {
         handle->arg_no=0;
      }
   }
   return handle;
}



/*-----------------------------------------------------------------------
//
// Function: PCLExprPrint()
//
//   Print a PCL expression.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void PCLExprPrint(FILE* out, PCLExpr_p expr, bool mini)
{
   long i;

   assert(expr);
   assert(expr->args);

   if(expr->op== PCLOpInitial)
   {
      if(expr->arg_no)
      {
         assert(expr->arg_no == 1);
         ClauseSourceInfoPrintPCL(out, PCLExprArg(expr,0));
      }
      else
      {
         fprintf(out, "initial");
      }
      return;
   }
   if(expr->op==PCLOpQuote)
   {
      assert(expr->arg_no==1);
      if(mini)
      {
         fprintf(out, "%ld", PCLExprArgInt(expr,0));
      }
      else
      {
         PCLIdPrint(out, PCLExprArg(expr,0));
      }
      if(PCLExprArgPos(expr,0))
      {
         PCL2PosPrint(out, PCLExprArgPos(expr,0));
      }
      return;
   }
   switch(expr->op)
   {
   case PCLOpIntroDef:
         fprintf(out, PCL_ID);
         assert(expr->arg_no==0);
         break;
   case PCLOpParamod:
         fprintf(out, PCL_PM);
         assert(expr->arg_no==2);
         break;
   case PCLOpSimParamod:
         fprintf(out, PCL_SPM);
         assert(expr->arg_no==2);
         break;
   case PCLOpEResolution:
         fprintf(out, PCL_ER);
         assert(expr->arg_no==1);
         break;
   case PCLOpEvalGC:
         fprintf(out, PCL_EVALGC);
         assert(expr->arg_no==1);
         break;
   case PCLOpEFactoring:
         fprintf(out, PCL_EF);
         assert(expr->arg_no==1);
         break;
   case PCLOpSatCheck:
         fprintf(out, PCL_SAT);
         assert(expr->arg_no>0);
         break;
   case PCLOpCondense:
         fprintf(out, PCL_CONDENSE);
         assert(expr->arg_no==1);
         break;
   case PCLOpSimplifyReflect:
         fprintf(out, PCL_SR);
         assert(expr->arg_no==2);
         break;
   case PCLOpContextSimplifyReflect:
         fprintf(out, PCL_CSR);
         assert(expr->arg_no==2);
         break;
   case PCLOpACResolution:
         fprintf(out, PCL_ACRES);
         assert(expr->arg_no>0);
         break;
   case PCLOpRewrite:
         fprintf(out, PCL_RW);
         assert(expr->arg_no==2);
         break;
   case PCLOpClauseNormalize:
         fprintf(out, PCL_CN);
         assert(expr->arg_no==1);
         break;
   case PCLOpApplyDef:
         fprintf(out, PCL_AD);
         assert(expr->arg_no==2);
         break;
   case PCLOpSplitClause:
         fprintf(out, PCL_SPLIT);
         assert(expr->arg_no==1);
         break;
   case PCLOpFOFSplitConjunct:
         fprintf(out, PCL_SC);
         assert(expr->arg_no==1);
         break;
   case PCLOpSplitEquiv:
         fprintf(out, PCL_SE);
         assert(expr->arg_no==1);
         break;
   case PCLOpFOFSimplify:
         fprintf(out, PCL_FS);
         assert(expr->arg_no==1);
         break;
   case PCLOpFOFDeMorgan:
         fprintf(out, PCL_NNF);
         assert(expr->arg_no==1);
         break;
   case PCLOpFOFDistributeQuantors:
         fprintf(out, PCL_SQ);
         assert(expr->arg_no==1);
         break;
   case PCLOpAnnotateQuestion:
         fprintf(out, PCL_ANNOQ);
         assert(expr->arg_no==1);
         break;
   case PCLOpEvalAnswers:
         fprintf(out, PCL_EVANS);
         assert(expr->arg_no==1);
         break;
   case PCLOpFOFDistributeDisjunction:
         fprintf(out, PCL_DSTR);
         assert(expr->arg_no==1);
         break;
   case PCLOpFOFVarRename:
         fprintf(out, PCL_VR);
         assert(expr->arg_no==1);
         break;
   case PCLOpFOFSkolemize:
         fprintf(out, PCL_SK);
         assert(expr->arg_no==1);
         break;
   case PCLOpFOFAssumeNegation:
         fprintf(out, PCL_NC);
         assert(expr->arg_no==1);
         break;
   default:
         assert(false && "Unknown PCL operator");
         break;
   }
   if(expr->arg_no)
   {
      fputc('(',out);
      PCLExprPrint(out, PCLExprArg(expr,0), mini);
      if(PCLExprArgPos(expr,0))
      {
         PCL2PosPrint(out, PCLExprArgPos(expr,0));
      }
      for(i=1; i<expr->arg_no; i++)
      {
         fputc(',',out);
         PCLExprPrint(out, PCLExprArg(expr,i), mini);
         if(PCLExprArgPos(expr,i))
         {
            PCL2PosPrint(out, PCLExprArgPos(expr,i));
         }
      }
      fputc(')',out);
   }
}


/*-----------------------------------------------------------------------
//
// Function: PCLExprPrintTSTP()
//
//   Print a PCL expression in TSTP format.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void PCLExprPrintTSTP(FILE* out, PCLExpr_p expr, bool mini)
{
   long i;
   bool needs_ans = false;
   char *status = ",[status(unknown)]",
      *status_thm = ",[status(thm)]",
      *status_cth = ",[status(cth)]",
      *status_esa = ",[status(esa)]";

   assert(expr);
   assert(expr->args);


   switch(expr->op)
   {
   case PCLOpInitial:
         if(expr->arg_no)
         {
            assert(expr->arg_no == 1);
            ClauseSourceInfoPrintTSTP(out, PCLExprArg(expr,0));
         }
         else
         {
            fprintf(out, "unknown()");
         }
         return;
   case PCLOpQuote:
         assert(expr->arg_no==1);
         if(mini)
         {
            /* fprintf(out, "c_0_%ld", PCLExprArgInt(expr,0)); */
            fprintf(out, "%ld", PCLExprArgInt(expr,0));
         }
         else
         {
            PCLIdPrintTSTP(out, PCLExprArg(expr,0));
         }
         return;
   case PCLOpIntroDef:
         fprintf(out, PCL_ID"(definition)");
         return;
   default:
         break;
   }

   fprintf(out, "inference(");
   switch(expr->op)
   {
   case PCLOpParamod:
         fprintf(out, PCL_PM);
         status = status_thm;
         assert(expr->arg_no==2);
         break;
   case PCLOpSimParamod:
         fprintf(out, PCL_SPM);
         status = status_thm;
         assert(expr->arg_no==2);
         break;
   case PCLOpEResolution:
         fprintf(out, PCL_ER);
         status = status_thm;
         assert(expr->arg_no==1);
         break;
   case PCLOpEvalGC:
         fprintf(out, PCL_EVALGC);
         status = status_thm;
         assert(expr->arg_no==1);
         break;
   case PCLOpEFactoring:
         fprintf(out, PCL_EF);
         status = status_thm;
         assert(expr->arg_no==1);
         break;
   case PCLOpACResolution:
         fprintf(out, PCL_ACRES);
         status = status_thm;
         assert(expr->arg_no>0);
         break;
   case PCLOpSatCheck:
         fprintf(out, PCL_SAT);
         status = status_thm;
         assert(expr->arg_no>0);
         break;
   case PCLOpCondense:
         fprintf(out, PCL_CONDENSE);
         status = status_thm;
         assert(expr->arg_no==1);
         break;
   case PCLOpSimplifyReflect:
         fprintf(out, PCL_SR);
         status = status_thm;
         assert(expr->arg_no==2);
         break;
   case PCLOpContextSimplifyReflect:
         fprintf(out, PCL_CSR);
         status = status_thm;
         assert(expr->arg_no==2);
         break;
   case PCLOpRewrite:
         fprintf(out, PCL_RW);
         status = status_thm;
         assert(expr->arg_no==2);
         break;
   case PCLOpClauseNormalize:
         fprintf(out, PCL_CN);
         status = status_thm;
         assert(expr->arg_no==1);
         break;
   case PCLOpApplyDef:
         fprintf(out, PCL_AD);
         status = status_thm;
         assert(expr->arg_no==2);
         break;
   case PCLOpSplitClause:
         fprintf(out, TSTP_SPLIT_BASE
                 ",["TSTP_SPLIT_BASE"("
                 TSTP_SPLIT_REFINED",[])]");
         status = "";
         assert(expr->arg_no==1);
         break;
   case PCLOpFOFSplitConjunct:
         fprintf(out, PCL_SC);
         status = status_thm;
         assert(expr->arg_no==1);
         break;
   case PCLOpSplitEquiv:
         fprintf(out, PCL_SE);
         status = status_thm;
         assert(expr->arg_no==1);
         break;
   case PCLOpFOFSimplify:
         fprintf(out, PCL_FS);
         status = status_thm;
         assert(expr->arg_no==1);
         break;
   case PCLOpFOFDeMorgan:
         fprintf(out, PCL_NNF);
         status = status_thm;
         assert(expr->arg_no==1);
         break;
   case PCLOpFOFDistributeQuantors:
         fprintf(out, PCL_SQ);
         status = status_thm;
         assert(expr->arg_no==1);
         break;
   case PCLOpAnnotateQuestion:
         fprintf(out, PCL_ANNOQ);
         status = status_thm;
         needs_ans      = true;
         assert(expr->arg_no==1);
         break;
   case PCLOpEvalAnswers:
         fprintf(out, PCL_EVANS);
         status = status_thm;
         needs_ans      = true;
         assert(expr->arg_no==1);
         break;
   case PCLOpFOFDistributeDisjunction:
         fprintf(out, PCL_DSTR);
         status = status_thm;
         assert(expr->arg_no==1);
         break;
   case PCLOpFOFVarRename:
         fprintf(out, PCL_VR);
         status = status_thm;
         assert(expr->arg_no==1);
         break;
   case PCLOpFOFSkolemize:
         fprintf(out, PCL_SK);
         status = status_esa;
         assert(expr->arg_no==1);
         break;
   case PCLOpFOFAssumeNegation:
         fprintf(out, PCL_NC);
         status = status_cth;
         assert(expr->arg_no==1);
         break;

   default:
         assert(false && "Unknown PCL operator");
         break;
   }
   fprintf(out, "%s,[", status);
   PCLExprPrintTSTP(out, PCLExprArg(expr,0), mini);
   for(i=1; i<expr->arg_no; i++)
   {
      fputc(',',out);
      PCLExprPrintTSTP(out, PCLExprArg(expr,i), mini);
   }
   if(needs_ans)
   {
      fputs(",theory(answers)", out);
   }
   fputs("])",out);
}


/*-----------------------------------------------------------------------
//
// Function: PCLStepExtract()
//
//   Given a PCL step "extra" string, return true if this should be
//   the root of a proof tree for extraction. Implemented here,
//   because it is used by both steps and ministeps.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool PCLStepExtract(char* extra)
{
   if(!extra)
   {
      return false;
   }
   /* printf("PCLStepExtract(%s)\n", extra); */
   if((*extra == '"') || (*extra == '\''))
   {
      extra++;
   }
   return (strncmp(extra,"proof",5)==0)||
      (strncmp(extra,"final",5)==0)||
      (strncmp(extra,"extract",7)==0);
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
