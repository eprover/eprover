/*-----------------------------------------------------------------------

File  : pcl_expressions.c

Author: Stephan Schulz

Contents

  PCL2 Expressions 

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Wed Mar 29 00:29:18 GMT 2000
    New

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

PCLExpr_p PCLExprAlloc()
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
      handle->arg_no = 0;
      NextToken(in);
   }
   else
   {
      CheckInpId(in, PCL_ER"|"PCL_PM"|"PCL_EF"|"PCL_RW"|"
		 PCL_SR"|"PCL_ACRES"|"PCL_CN"|"PCL_SPLIT);
      if(TestInpId(in, PCL_ER))
      {
	 handle->op=PCLOpEResolution;
	 arg_no=1;
      }
      else if(TestInpId(in, PCL_PM))
      {
	 handle->op=PCLOpParamod;
	 arg_no=2;
      }
      else if(TestInpId(in, PCL_EF))
      {
	 handle->op=PCLOpEFactoring;
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
      NextToken(in);
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
      fprintf(out, "initial");
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
   case PCLOpParamod:
	 fprintf(out, PCL_PM);
	 assert(expr->arg_no==2);
	 break;
   case PCLOpEResolution:
	 fprintf(out, PCL_ER);
	 assert(expr->arg_no==1);
	 break;
   case PCLOpEFactoring:
	 fprintf(out, PCL_EF);
	 assert(expr->arg_no==1);
	 break;
   case PCLOpSimplifyReflect:
	 fprintf(out, PCL_SR);
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
   case PCLOpSplitClause:
	 fprintf(out, PCL_SPLIT);
	 assert(expr->arg_no==1);
	 break;
   default:
	 assert(false && "Unknown PCL operator");
	 break;
   }
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



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


