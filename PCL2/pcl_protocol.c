/*-----------------------------------------------------------------------

File  : pcl_protocol.c

Author: Stephan Schulz

Contents

  Protocols (=trees) of PCL steps, all inclusive ;-) 

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Sun Apr  2 01:49:33 GMT 2000
    New

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

   handle->terms = TBAlloc(TPIgnoreProps, SigAlloc());
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
			       (ComparisonFunctionType)PCLStepIdCompare);
   if(res)
   {
      prot->number--;
      prot->in_order = false;
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

   cell = PTreeObjFind(&(prot->steps), &tmp,
		       (ComparisonFunctionType)PCLStepIdCompare);
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
   DStr_p     source_name, errpos;
   long       line, column;
   StreamType type;

   while(TestInpTok(in, PosInt))
   {
      line = AktToken(in)->line;
      column = AktToken(in)->column;
      source_name = DStrGetRef(AktToken(in)->source);
      type = AktToken(in)->stream_type;

      step = PCLStepParse(in, prot->terms);
      cell = PCLProtInsertStep(prot, step);
      if(cell)
      {
	 errpos = DStrAlloc();
	 
	 DStrAppendStr(errpos, PosRep(type, source_name, line, column));
	 DStrAppendStr(errpos, " duplicate PCL identifier");
	 Error(DStrView(errpos), SYNTAX_ERROR);
	 DStrFree(errpos);
      }
      DStrReleaseRef(source_name);
      res++;
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
// Function: PCLProtMarkProofClauses()
//
//   Mark all proof steps in protocol with CPIsProofClause. Return
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
      if(ClauseIsEmpty(step->clause))
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
// Side Effects    : -
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
// Side Effects    : -
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
			     bool just_clauses, OutputFormatType format)
{
   PCLStep_p step;
   PStackPointer i;
   
   PCLProtSerialize(prot);
   
   for(i=0; i<PStackGetSP(prot->in_order); i++)
   {
      step = PStackElementP(prot->in_order, i);
      if(PCLStepQueryProp(step,prop))
      {
	 if(just_clauses)
	 {
	    ClausePrint(out, step->clause, true);
	    #ifdef NEVER_DEFINED
	    fprintf(GlobalOut, "/* %f */", step->lemma_quality);
	    #endif
	 }
	 else
	 {
	    PCLStepPrintFormat(out, step, false, format);
	 }
	 fputc('\n',out);
      }
      
   }
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/




