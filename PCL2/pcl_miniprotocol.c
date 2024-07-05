/*-----------------------------------------------------------------------

  File  : pcl_miniprotocol.c

  Author: Stephan Schulz

  Contents

  Protocols (=arrays) of PCL steps, all inclusive ;-)

  Copyright 1998, 1999, 2002, 2024 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Mon Jul 15 23:13:21 CEST 2002

-----------------------------------------------------------------------*/

#include "pcl_miniprotocol.h"



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
// Function: PCLMiniProtAlloc()
//
//   Return an initialized PCLMini protocol data structure.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

PCLMiniProt_p PCLMiniProtAlloc(void)
{
   PCLMiniProt_p handle = PCLMiniProtCellAlloc();
   TypeBank_p sort_table = TypeBankAlloc();
   Sig_p sig = SigAlloc(sort_table);

   SigInsertInternalCodes(sig);
   handle->terms = TBAlloc(sig);
   handle->steps = PDArrayAlloc(1,500000);
   handle->max_ident = 0;

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: PCLMiniProtFree()
//
//   Free a PCLMini protocol
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void PCLMiniProtFree(PCLMiniProt_p junk)
{
   long i;
   PCLMiniStep_p step;

   assert(junk && junk->terms);

   for(i=0; i<=junk->max_ident; i++)
   {
      step = PDArrayElementP(junk->steps, i);
      if(step)
      {
    PCLMiniStepFree(step);
    PDArrayAssignP(junk->steps, i, NULL);
      }
   }
   TypeBankFree(junk->terms->sig->type_bank);
   SigFree(junk->terms->sig);
   junk->terms->sig = NULL;
   TBFree(junk->terms);
   PDArrayFree(junk->steps);
   PCLMiniProtCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: PCLMiniProtInsertStep()
//
//   Insert a step into prot. Return true if it was not already in the
//   protokol, otherwise false.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool PCLMiniProtInsertStep(PCLMiniProt_p prot, PCLMiniStep_p step)
{
   PCLMiniStep_p local_step;

   assert(prot&&step);

   local_step = PDArrayElementP(prot->steps, step->id);
   if(local_step)
   {
      assert(step==local_step);
      return false;
   }
   PDArrayAssignP(prot->steps, step->id, step);
   prot->max_ident = MAX(step->id, prot->max_ident);
   return true;
}


/*-----------------------------------------------------------------------
//
// Function: PCLMiniProtFindStep()
//
//   Given a PCLMini-Identifier, find the matching step in prot.
//
// Global Variables: -
//
// Side Effects    : May reorganize the tree.
//
/----------------------------------------------------------------------*/

PCLMiniStep_p PCLMiniProtFindStep(PCLMiniProt_p prot, unsigned long id)
{
   PCLMiniStep_p local_step;

   local_step = PDArrayElementP(prot->steps, id);

   return local_step;
}


/*-----------------------------------------------------------------------
//
// Function: PCLMiniProtExtractStep()
//
//   Extract the step from the protokol and return it.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

PCLMiniStep_p PCLMiniProtExtractStep(PCLMiniProt_p prot, PCLMiniStep_p
                 step)
{
   PCLMiniStep_p local_step;

   assert(prot&&step);

   local_step = PDArrayElementP(prot->steps, step->id);
   if(local_step)
   {
      assert(step==local_step);
      PDArrayAssignP(prot->steps, step->id, NULL);
      return step;
   }
   return NULL;
}


/*-----------------------------------------------------------------------
//
// Function: PCLMiniProtDeleteStep()
//
//   Delete a step from a protocol. Return true if the step existed in
//   the protocol, false otherwise. In the second case, the step is
//   _not_ freed.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool PCLMiniProtDeleteStep(PCLMiniProt_p prot, PCLMiniStep_p step)
{
   PCLMiniStep_p local_step;

   assert(prot&&step);

   local_step = PDArrayElementP(prot->steps, step->id);
   if(local_step)
   {
      assert(step==local_step);
      PDArrayAssignP(prot->steps, step->id, NULL);
      PCLMiniStepFree(local_step);
      return true;
   }
   return false;
}



/*-----------------------------------------------------------------------
//
// Function: PCLMiniProtParse()
//
//   Parse a PCLMini listing into prot. Return number of steps parsed.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

long PCLMiniProtParse(Scanner_p in, PCLMiniProt_p prot)
{
   long           res = 0;
   PCLMiniStep_p  step;
   DStr_p     source_name;
   long       line, column;
   StreamType type;

   while(TestInpTok(in, PosInt))
   {
      if(!in->ignore_comments)
      {
         fprintf(GlobalOut, "%s", DStrView(AktToken(in)->comment));
         DStrReset(AktToken(in)->comment);
      }
      line = AktToken(in)->line;
      column = AktToken(in)->column;
      source_name = DStrGetRef(AktToken(in)->source);
      type = AktToken(in)->stream_type;

      step = PCLMiniStepParse(in, prot->terms);
      if(!PCLMiniProtInsertStep(prot, step))
      {
         Error("%s duplicate PCL identifier",
               SYNTAX_ERROR,
               PosRep(type, source_name, line, column));
      }
      DStrReleaseRef(source_name);
      res++;
   }
   if(!in->ignore_comments)
   {
      fprintf(GlobalOut, "%s", DStrView(AktToken(in)->comment));
      DStrReset(AktToken(in)->comment);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: PCLMiniProtPrint()
//
//   Print a PCLMini protocol.
//
// Global Variables: -
//
// Side Effects    : Output, some memory operations
//
/----------------------------------------------------------------------*/

void PCLMiniProtPrint(FILE* out, PCLMiniProt_p prot,
            OutputFormatType format)
{
   long i;
   PCLMiniStep_p step;

   assert(prot && prot->terms);

   for(i=0; i<=prot->max_ident; i++)
   {
      step = PDArrayElementP(prot->steps, i);
      if(step)
      {
         PCLMiniStepPrintFormat(out, step, prot->terms, format);
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: PCLMiniExprCollectPreconds()
//
//   Collect all PCLMini steps referenced in expr into tree.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void PCLMiniExprCollectPreconds(PCLMiniProt_p prot, PCLExpr_p expr, PTree_p *tree)
{
   PCLMiniStep_p step;
   int       i;
   long    id;

   assert(prot && expr);

   switch(expr->op)
   {
   case PCLOpNoOp:
         assert(false);
         break;
   case PCLOpInitial:
         break;
   case PCLOpQuote:
         id = PCLExprArgInt(expr,0);
         step = PCLMiniProtFindStep(prot, id);
         if(!step)
         {
            Error("Dangling reference in PCL protocol!",
                  SYNTAX_ERROR);
         }
         PTreeStore(tree, step);
         break;
   default:
         for(i=0; i<expr->arg_no; i++)
         {
            PCLMiniExprCollectPreconds(prot, PCLExprArg(expr,i), tree);
         }
         break;
   }
}


/*-----------------------------------------------------------------------
//
// Function: PCLMiniProtMarkProofClauses()
//
//   Mark all proof steps in protokoll with PCLIsProofStep. Return
//   true if empty clause was encountered.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool PCLMiniProtMarkProofClauses(PCLMiniProt_p prot, bool fast)
{
   bool res = false;
   long i;
   PStack_p to_proc = PStackAlloc();
   PCLMiniStep_p step;
   PTree_p root = NULL;

   if(fast)
   {
      i = prot->max_ident;
      step = PCLMiniProtFindStep(prot, i);
      while(step&&(i>=0)&&PCLStepExtract(step->extra))
      {
         PStackPushP(to_proc, step);
         i--;
         if(i>=0)
         {
       step = PCLMiniProtFindStep(prot, i);
         }
      }
   }
   else
   {
      for(i=0; i<=prot->max_ident;i++)
      {
         step = PCLMiniProtFindStep(prot, i);

         if(step && PCLStepExtract(step->extra))
         {
            {
               PStackPushP(to_proc, step);
            }
         }
      }
   }
   while(!PStackEmpty(to_proc))
   {
      step = PStackPopP(to_proc);
      if((PCLStepIsShell(step)
          &&step->extra
          &&(strcmp(step->extra, "'proof'")==0))
         ||
         (!PCLStepIsShell(step) &&
          PCLStepIsClausal(step) &&
          (step->logic.clause->literal_no == 0)))
      {
         res = true;
      }
      if(!PCLStepQueryProp(step,PCLIsProofStep))
      {
         PCLStepSetProp(step, PCLIsProofStep);
         PCLMiniExprCollectPreconds(prot, step->just, &root);
         while(root)
         {
            step = PTreeExtractRootKey(&root);
            PStackPushP(to_proc, step);
         }
      }
   }
   PStackFree(to_proc);
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: PCLMiniProtSetClauseProp()
//
//   Set a property in a PCLMiniStep protocol.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void PCLMiniProtSetClauseProp(PCLMiniProt_p prot, PCLStepProperties props)
{
   long i;
   PCLMiniStep_p step;

   for(i=0; i<=prot->max_ident; i++)
   {
      step = PDArrayElementP(prot->steps, i);
      if(step)
      {
         PCLStepSetProp(step,props);
      }
   }
}

/*-----------------------------------------------------------------------
//
// Function: PCLMiniProtDelClauseProp()
//
//   Delete a property in a PCLMiniSteps protocol.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void PCLMiniProtDelClauseProp(PCLMiniProt_p prot, PCLStepProperties props)
{
   long i;
   PCLMiniStep_p step;

   for(i=0; i<=prot->max_ident; i++)
   {
      step = PDArrayElementP(prot->steps, i);
      if(step)
      {
         PCLStepDelProp(step,props);
      }
   }
}




/*-----------------------------------------------------------------------
//
// Function: PCLMiniProtPrintProofClauses()
//
//   Print a PCLMini protocol.
//
// Global Variables: -
//
// Side Effects    : Output, some memory operations
//
/----------------------------------------------------------------------*/

void PCLMiniProtPrintProofClauses(FILE* out, PCLMiniProt_p prot,
              OutputFormatType format)
{
   long i;
   PCLMiniStep_p step;

   assert(prot && prot->terms);

   for(i=0; i<=prot->max_ident; i++)
   {
      step = PDArrayElementP(prot->steps, i);
      if(step&&PCLStepQueryProp(step, PCLIsProofStep))
      {
         PCLMiniStepPrintFormat(out, step, prot->terms, format);
         fputc('\n',out);
      }
   }
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
