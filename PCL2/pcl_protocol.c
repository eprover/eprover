/*-----------------------------------------------------------------------

  File  : pcl_protocol.c

  Author: Stephan Schulz

  Contents

  Protocols (=trees) of PCL steps, all inclusive ;-)

  Copyright 1998, 1999, 2024 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Sun Apr  2 01:49:33 GMT 2000

-----------------------------------------------------------------------*/

#include "pcl_protocol.h"



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
// Function: PCLProtAlloc()
//
//   Return an initialized PCL protocol data structure.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

PCLProt_p PCLProtAlloc(void)
{
   PCLProt_p handle = PCLProtCellAlloc();
   TypeBank_p sort_table = TypeBankAlloc();
   Sig_p sig = SigAlloc(sort_table);

   SigInsertInternalCodes(sig);
   handle->terms = TBAlloc(sig);
   handle->number = 0;
   handle->steps = NULL;
   handle->in_order = PStackAlloc();
   handle->is_ordered = false;

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: PCLProtFree()
//
//   Free a PCL protocol
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void PCLProtFree(PCLProt_p junk)
{
   PStack_p stack;
   PTree_p  cell;
   TypeBank_p sort_table = junk->terms->sig->type_bank;

   assert(junk && junk->terms);

   stack = PTreeTraverseInit(junk->steps);
   while((cell=PTreeTraverseNext(stack)))
   {
      PCLStepFree(cell->key);
   }
   PStackFree(stack);
   PTreeFree(junk->steps);
   SigFree(junk->terms->sig);
   junk->terms->sig = NULL;
   TypeBankFree(sort_table);
   TBFree(junk->terms);
   PStackFree(junk->in_order);
   PCLProtCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: PCLProtExtractStep()
//
//   (Try to) take a step out of the protocol. Return true if it
//   exists, false otherwise.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

PCLStep_p PCLProtExtractStep(PCLProt_p prot, PCLStep_p step)
{
   PCLStep_p res;

   res = PTreeObjExtractObject(&(prot->steps), step,
                PCLStepIdCompare);
   if(res)
   {
      prot->number--;
      prot->is_ordered = false;
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: PCLProtDeleteStep()
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

bool PCLProtDeleteStep(PCLProt_p prot, PCLStep_p step)
{
   PCLStep_p tree_step;

   assert(prot&&step);

   tree_step = PCLProtExtractStep(prot,step);
   if(tree_step)
   {
      assert(step==tree_step);
      PCLStepFree(tree_step);
      return true;
   }
   return false;
}


/*-----------------------------------------------------------------------
//
// Function: PCLProtFindStep()
//
//   Given a PCL-Identifier, find the matching step in prot.
//
// Global Variables: -
//
// Side Effects    : May reorganize the tree.
//
/----------------------------------------------------------------------*/

PCLStep_p PCLProtFindStep(PCLProt_p prot, PCLId_p id)
{
   PCLStepCell tmp;
   PTree_p     cell;

   tmp.id = id;

   cell = PTreeObjFind(&(prot->steps), &tmp, PCLStepIdCompare);
   if(cell)
   {
      return cell->key;
   }
   return NULL;
}




/*-----------------------------------------------------------------------
//
// Function: PCLProtSerialize()
//
//   Ensure that prot->in_order is up to date
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void PCLProtSerialize(PCLProt_p prot)
{
   if(!prot->is_ordered)
   {
      PStack_p  stack;
      PTree_p   cell;
      PCLStep_p step;

      PStackReset(prot->in_order);
      stack = PTreeTraverseInit(prot->steps);
      while((cell=PTreeTraverseNext(stack)))
      {
    step = cell->key;
    PStackPushP(prot->in_order, step);
      }
      PStackFree(stack);
      prot->is_ordered = true;
   }
}



/*-----------------------------------------------------------------------
//
// Function: PCLProtParse()
//
//   Parse a PCL listing into prot. Return number of steps parsed.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

long PCLProtParse(Scanner_p in, PCLProt_p prot)
{
   long       res = 0;
   PCLStep_p  step;
   PTree_p    cell;
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

      step = PCLStepParse(in, prot->terms);
      cell = PCLProtInsertStep(prot, step);
      if(cell)
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
// Function: PCLProtPrintExtra()
//
//   Print a PCL protocol.
//
// Global Variables: -
//
// Side Effects    : Output, some memory operations
//
/----------------------------------------------------------------------*/

void PCLProtPrintExtra(FILE* out, PCLProt_p prot, bool data,
             OutputFormatType format)
{
   PCLStep_p step;
   PStackPointer i;

   PCLProtSerialize(prot);

   for(i=0; i<PStackGetSP(prot->in_order); i++)
   {
      step = PStackElementP(prot->in_order, i);
      PCLStepPrintFormat(out, step, data, format);
      fputc('\n',out);
   }
}


/*-----------------------------------------------------------------------
//
// Function: PCLStepHasFOFParent()
//
//   Return true if one of the parents of step is a FOF step, false
//   otherwise.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

bool PCLStepHasFOFParent(PCLProt_p prot, PCLStep_p step)
{
   PTree_p parents = NULL, cell;
   PCLStep_p parent;
   PStack_p iter_stack;
   bool res = false;

   PCLStepCollectPreconds(prot, step, &parents);

   iter_stack = PTreeTraverseInit(parents);
   while((cell = PTreeTraverseNext(iter_stack)))
   {
      parent = cell->key;
      if(PCLStepQueryProp(parent, PCLIsFOFStep))
      {
         res = true;
         break;
      }
   }
   PTreeTraverseExit(iter_stack);
   PTreeFree(parents);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: PCLProtStripFOF()
//
//   Remove all FOF steps from protocol. Make steps referencing a FOF
//   step into initials and rewrite the justification
//   accordingly. Expensive if there are FOF steps, reasonably cheap
//   otherwise...
//
// Global Variables: -
//
// Side Effects    : Changes prot, memory operations.
//
/----------------------------------------------------------------------*/

long PCLProtStripFOF(PCLProt_p prot)
{
   PCLStep_p step;
   PStackPointer i;
   long res;
   PStack_p fof_steps = PStackAlloc();

   PCLProtCollectPropSteps(prot, PCLIsFOFStep, fof_steps);
   res = PStackGetSP(fof_steps);

   if(res)
   {
      PCLProtSerialize(prot); /* Should be serialized, but let's play
                               * it safe */
      for(i=0; i<PStackGetSP(prot->in_order); i++)
      {
         step = PStackElementP(prot->in_order, i);
         if(!PCLStepQueryProp(step,PCLIsFOFStep)
            &&
            PCLStepHasFOFParent(prot, step))
         {
            PCLExprFree(step->just);
            step->just = PCLExprAlloc();
            step->just->arg_no = 0;
            step->just->op = PCLOpInitial;
         }
      }
   }
   while(!PStackEmpty(fof_steps))
   {
      bool check;

      step = PStackPopP(fof_steps);
      check = PCLProtDeleteStep(prot, step);
      UNUSED(check); assert(check);
   }
   PStackFree(fof_steps);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: PCLProtResetTreeData()
//
//   Reset the tree data counters in all steps in the protocol.
//
// Global Variables: -
//
// Side Effects    : As described
//
/----------------------------------------------------------------------*/

void PCLProtResetTreeData(PCLProt_p prot, bool just_weights)
{
   PCLStep_p step;
   PStackPointer i;

   PCLProtSerialize(prot);

   for(i=0; i<PStackGetSP(prot->in_order); i++)
   {
      step = PStackElementP(prot->in_order, i);
      PCLStepResetTreeData(step, just_weights);
   }

}


/*-----------------------------------------------------------------------
//
// Function: PCLExprCollectPreconds()
//
//   Collect all PCL steps referenced in expr into tree.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void PCLExprCollectPreconds(PCLProt_p prot, PCLExpr_p expr, PTree_p *tree)
{
   PCLStep_p step;
   int       i;

   assert(prot && expr);

   switch(expr->op)
   {
   case PCLOpNoOp:
    assert(false);
    break;
   case PCLOpInitial:
    break;
   case PCLOpQuote:
    step = PCLProtFindStep(prot,PCLExprArg(expr,0));
    PTreeStore(tree, step);
    break;
   default:
    for(i=0; i<expr->arg_no; i++)
    {
       PCLExprCollectPreconds(prot, PCLExprArg(expr,i), tree);
    }
    break;
   }
}


/*-----------------------------------------------------------------------
//
// Function: PCLExprGetQuotedArg()
//
//   If the designated arg is a quote expression, retrieve and return
//   the quoted step. Otherwise return NULL.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

PCLStep_p PCLExprGetQuotedArg(PCLProt_p prot, PCLExpr_p expr, int arg)
{
   PCLExpr_p argexpr;

   assert(arg < expr->arg_no);

   /* printf("pcl_expr_get_quoted_arg(%p, %p, %d)...\n", prot, expr,
      arg); */
   argexpr = PCLExprArg(expr,arg);
   if(argexpr->op == PCLOpQuote)
   {
      return PCLProtFindStep(prot,PCLExprArg(argexpr,0));
   }
   return NULL;
}


/*-----------------------------------------------------------------------
//
// Function: PCLProtMarkProofClauses()
//
//   Mark all proof steps in protocol with PCLIsProofStep. Return
//   true if protocol describes a proof (i.e. contains the empty
//   clause).
//   otherwise.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool PCLProtMarkProofClauses(PCLProt_p prot)
{
   bool res = false;
   PStack_p to_proc = PStackAlloc();
   PTree_p root = NULL;
   PCLStep_p step;
   PStackPointer i;

   PCLProtSerialize(prot);

   for(i=0; i<PStackGetSP(prot->in_order); i++)
   {
      step = PStackElementP(prot->in_order, i);
      if((PCLStepIsShell(step)
          &&step->extra
          &&(strcmp(step->extra, "'proof'")==0))
         ||
         (!PCLStepIsShell(step)&&
         !PCLStepIsFOF(step)&&
          ClauseIsEmpty(step->logic.clause)))
      {
    res = true;
      }
      if(PCLStepExtract(step->extra))
      {
    PStackPushP(to_proc, step);
      }
   }
   while(!PStackEmpty(to_proc))
   {
      step = PStackPopP(to_proc);
      if(!PCLStepQueryProp(step,PCLIsProofStep))
      {
    PCLStepSetProp(step,PCLIsProofStep);
    PCLExprCollectPreconds(prot, step->just, &root);
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
// Function: PCLProtSetProp()
//
//   Set props in all clauses in the protocol.
//
// Global Variables: -
//
// Side Effects    :  May sort the protocol.
//
/----------------------------------------------------------------------*/

void PCLProtSetProp(PCLProt_p prot, PCLStepProperties props)
{
   PCLStep_p step;
   PStackPointer i;

   PCLProtSerialize(prot);

   for(i=0; i<PStackGetSP(prot->in_order); i++)
   {
      step = PStackElementP(prot->in_order, i);
      PCLStepSetProp(step,props);
   }
}


/*-----------------------------------------------------------------------
//
// Function: PCLProtDelProp()
//
//   Set props in all clauses in the protocol.
//
// Global Variables: -
//
// Side Effects    :  May sort the protocol.
//
/----------------------------------------------------------------------*/

void PCLProtDelProp(PCLProt_p prot, PCLStepProperties props)
{
   PCLStep_p step;
   PStackPointer i;

   PCLProtSerialize(prot);

   for(i=0; i<PStackGetSP(prot->in_order); i++)
   {
      step = PStackElementP(prot->in_order, i);
      PCLStepDelProp(step,props);
   }
}


/*-----------------------------------------------------------------------
//
// Function: PCLProtCountProp()
//
//   Return the number of steps with all properties in props set.
//
// Global Variables: -
//
// Side Effects    : May sort the protocol
//
/----------------------------------------------------------------------*/

long PCLProtCountProp(PCLProt_p prot, PCLStepProperties props)
{
   PCLStep_p step;
   PStackPointer i;
   long res = 0;

   PCLProtSerialize(prot);

   for(i=0; i<PStackGetSP(prot->in_order); i++)
   {
      step = PStackElementP(prot->in_order, i);
      if(PCLStepQueryProp(step,props))
      {
         res++;
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: PCLProtCollectPropSteps()
//
//   Push all steps in prot with properties props set onto stack.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

long PCLProtCollectPropSteps(PCLProt_p prot, PCLStepProperties props,
                             PStack_p steps)
{
   PCLStep_p step;
   PStackPointer i;

   PCLProtSerialize(prot);

   for(i=0; i<PStackGetSP(prot->in_order); i++)
   {
      step = PStackElementP(prot->in_order, i);
      if(PCLStepQueryProp(step,props))
      {
         PStackPushP(steps, step);
      }
   }
   return PStackGetSP(steps);
}

/*-----------------------------------------------------------------------
//
// Function: PCLProtPrintPropClauses()
//
//   Print all steps with prop set.
//
// Global Variables: -
//
// Side Effects    : Output, some memory operations
//
/----------------------------------------------------------------------*/

void PCLProtPrintPropClauses(FILE* out, PCLProt_p prot,
              PCLStepProperties prop,
              OutputFormatType format)
{
   PCLStep_p step;
   PStackPointer i;

   PCLProtSerialize(prot);

   for(i=0; i<PStackGetSP(prot->in_order); i++)
   {
      step = PStackElementP(prot->in_order, i);
      if(PCLStepQueryProp(step,prop))
      {
         PCLStepPrintFormat(out, step, false, format);
         fputc('\n',out);
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: PCLProtPrintExamples()
//
//   Print all PCL steps that are marked as examples in example
//   format.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void PCLProtPrintExamples(FILE* out, PCLProt_p prot)
{
   long proof_steps;
   PCLStep_p step;
   PStackPointer i;

   proof_steps = PCLProtCountProp(prot, PCLIsProofStep);
   /* The above also serializes the protocol! */
   assert(prot->is_ordered);

   for(i=0; i<PStackGetSP(prot->in_order); i++)
   {
      step = PStackElementP(prot->in_order, i);
      if(PCLStepQueryProp(step,PCLIsExample))
      {
         PCLStepPrintExample(out, step, i, proof_steps, prot->number);
         fputc('\n', out);
      }
   }
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
