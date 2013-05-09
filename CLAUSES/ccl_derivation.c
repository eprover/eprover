/*-----------------------------------------------------------------------

File  : ccl_derivation.c

Author: Stephan Schulz (schulz@eprover.org)

Contents
 
  Functions related to the construction, manipulation, and printing of
  explicit proof objects in E.

  Copyright 2013 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main E directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Sat Apr  6 10:58:29 CEST 2013
    New

-----------------------------------------------------------------------*/

#include "ccl_derivation.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

ProofObjectType BuildProofObject = 0; 

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: derived_free_wrapper()
//
//   Free a Derived cell (for PObjTreeFree).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void derived_free_wrapper(void* junk)
{
   DerivedFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: derived_compare()
//
//   Compare two derived cells by their clause or formula.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

int derived_compare(const void* p1, const void* p2)
{
   Derived_p d1 = (Derived_p) p1;
   Derived_p d2 = (Derived_p) p2;
   void *key1, *key2;

   key1 = d1->clause? (void*)d1->clause: (void*)d1->formula;
   key1 = d2->clause? (void*)d2->clause: (void*)d2->formula;

   return PCmp(key1, key2);
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: ClausePushDerivation()
//
//   Push the derivation items (op-code and suitable number of
//   arguments) onto the derivation stack.
//
// Global Variables: -
//
// Side Effects    : May allocate new derivation stack.
//
/----------------------------------------------------------------------*/

void ClausePushDerivation(Clause_p clause, DerivationCodes op, 
                          void* arg1, void* arg2)
{
   assert(clause);
   assert(op);

   if(!clause->derivation)
   {
      clause->derivation = PStackVarAlloc(3);
   }
   assert(DCOpHasCnfArg1(op)||DCOpHasFofArg1(op)||!arg1);
   assert(DCOpHasCnfArg2(op)||DCOpHasFofArg2(op)||!arg2);
   assert(DCOpHasCnfArg1(op)||!DCOpHasCnfArg2(op));

   PStackPushInt(clause->derivation, op);
   if(arg1)
   {
      PStackPushP(clause->derivation, arg1);
      if(arg2)
      {
         PStackPushP(clause->derivation, arg2);
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: WFormulaPushDerivation()
//
//   Push the derivation items (op-code and suitable number of
//   arguments) onto the derivation stack.
//
// Global Variables: -
//
// Side Effects    : May allocate new derivation stack.
//
/----------------------------------------------------------------------*/

void WFormulaPushDerivation(WFormula_p form, DerivationCodes op, 
                            void* arg1, void* arg2)
{
   assert(form);
   assert(op);

   if(!form->derivation)
   {
      form->derivation = PStackVarAlloc(3);
   }
   assert(DCOpHasCnfArg1(op)||DCOpHasFofArg1(op)||!arg1);
   assert(DCOpHasCnfArg2(op)||DCOpHasFofArg2(op)||!arg2);
   assert(DCOpHasCnfArg1(op)||!DCOpHasCnfArg2(op));

   PStackPushInt(form->derivation, op);
   if(arg1)
   {
      PStackPushP(form->derivation, arg1);
      if(arg2)
      {
         PStackPushP(form->derivation, arg2);
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: DerivStackExtractParents()
//
//   Given a derivation stack (derivation-codes with arguments),
//   return all the (occurances of) all the side premises referenced
//   in the derivation (via the result stacks). Return value is the
//   number of premises found.
//
// Global Variables: -
//
// Side Effects    : (via PStackPushP())
//
/----------------------------------------------------------------------*/

long DerivStackExtractParents(PStack_p derivation, 
                              PStack_p res_clauses, 
                              PStack_p res_formulas)
{
   PStackPointer i, sp;
   long res = 0;
   DerivationCodes op;

   assert(res_clauses);
   assert(res_formulas);

   if(derivation)
   {
      sp = PStackGetSP(derivation);
      
      while(i<sp)
      {
         op = PStackElementInt(derivation, i);
         i++;
         if(DCOpHasCnfArg1(op))
         {
            PStackPushP(res_clauses, PStackElementP(derivation, i));
            i++;
            res++;
         }
         else if(DCOpHasFofArg1(op))
         {
            PStackPushP(res_formulas, PStackElementP(derivation, i));
            i++;
            res++;
         }
         if(DCOpHasCnfArg2(op))
         {
            PStackPushP(res_clauses, PStackElementP(derivation, i));
            i++;
            res++;
         }
         else if(DCOpHasFofArg1(op))
         {
            PStackPushP(res_formulas, PStackElementP(derivation, i));
            i++;
            res++;
         }
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: DerivationStackPCLPrint()
//
//   Print the derivation stack as a PCL expression.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void DerivationStackPCLPrint(FILE* out, PStack_p derivation)
{
   PStack_p subexpr_stack;
   PStackPointer i, sp;

   if(derivation)
   {
      subexpr_stack = PStackAlloc();
      
      sp = PStackGetSP(derivation);
      
      while(i<sp)
      {
         PStackPushInt(subexpr_stack, i);

         op = PStackElementInt(derivation, i);
         i++;
         if(DCOpHasCnfArg1(op)||DCOpHasFofArg1(op))
         {
            i++;
         }
         if(DCOpHasCnfArg2(op)||DCOpHasFofArg1(op))
         {
            i++;
         }
      }
      while(!PStackEmpty(subexpr_stack))
      {
         sp = PStackElementInt(
      }

      PStackFree(subexpr_stack);      
   }
}



/*-----------------------------------------------------------------------
//
// Function: DerivedAlloc()
//
//   Allocate an empty initialized DerivedCell.
//
// Global Variables: -
//
// Side Effects    : Memory  operations
//
/----------------------------------------------------------------------*/

Derived_p DerivedAlloc(void)
{
   Derived_p handle;

   handle            = DerivedCellAlloc();
   handle->ref_count = 0;
   handle->clause    = NULL;
   handle->formula   = NULL;

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: DerivationAlloc()
//
//   Allocate an empty derivation.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Derivation_p DerivationAlloc(void)
{
   Derivation_p handle = DerivationCellAlloc();

   handle->ordered       = false;
   handle->deriv         = NULL;
   handle->roots         = PStackAlloc();
   handle->ordered_deriv = PStackAlloc();
   
   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: DerivationFree()
//
//   Free a derivation.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void DerivationFree(Derivation_p junk)
{
   PObjTreeFree(junk->deriv, derived_free_wrapper);
   PStackFree(junk->ordered_deriv);
   PStackFree(junk->roots);
   DerivationCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: DerivationGetDerived()
//
//   Given a clause or formula, return the associated cell of the
//   derivation. If none exists, create a new one. Only one of
//   "clause", "formula" can be set.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Derived_p DerivationGetDerived(Derivation_p derivation, Clause_p clause,
                               WFormula_p formula)
{
   Derived_p handle, tmp;

   assert(clause||formula);
   assert(!clause||!formula);

   handle = DerivedAlloc();
   if(clause)
   {
      handle->clause = clause;
   }
   else
   {
      handle->formula = formula;
   }
   tmp = PTreeObjStore(&(derivation->deriv), handle, derived_compare);
   if(tmp)
   {
      DerivedFree(handle);
      handle = tmp;
   }
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: DerivationExtract()
//
//   Extract the proof tree of the clauses on root_clauses and
//   annotate each "Derived" node with the number of
//   in-references. Return number of roots.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

long DerivationExtract(Derivation_p derivation, PStack_p root_clauses)
{
   PStackPointer sp;
   Clause_p      clause;
   WFormula_p    form;   
   Derived_p     node, newnode;
   PStack_p      deriv;
   PStack_p      stack, parent_clauses, parent_formulas;

   stack = PStackAlloc();
   parent_clauses  = PStackAlloc();
   parent_formulas = PStackAlloc();

   for(sp=0; sp<PStackGetSP(root_clauses); sp++)
   {
      clause = PStackElementP(root_clauses, sp);
      
      node = DerivationGetDerived(derivation, clause, NULL);

      PStackPushP(stack, node);
      PStackPushP(derivation->roots, node);
   }    
   
   while(!PStackEmpty(stack))
   {
      node = PStackPopP(stack);
      if(node->clause)
      {
         deriv = node->clause->derivation;
      }
      else
      {
         assert(node->formula);
         deriv = node->formula->derivation;
      }
      assert(PStackEmpty(parent_clauses));
      assert(PStackEmpty(parent_formulas));
      DerivStackExtractParents(deriv, 
                               parent_clauses, 
                               parent_formulas);
      while(!PStackEmpty(parent_clauses))
      {
         clause = PStackPopP(parent_clauses);
         newnode = DerivationGetDerived(derivation, clause, NULL);
         newnode->ref_count++;
      }
      while(!PStackEmpty(parent_formulas))
      {
         form = PStackPopP(parent_formulas);
         newnode = DerivationGetDerived(derivation, NULL, form);
         newnode->ref_count++;
      }     
   }
   derivation->ordered = false;

   PStackFree(parent_clauses);
   PStackFree(parent_formulas);
   PStackFree(stack);

   return PStackGetSP(derivation->roots);
}


/*-----------------------------------------------------------------------
//
// Function: DerivationTopoSort()
//
//   Perform a topological sort of the derivation.
//
// Global Variables: -
//
// Side Effects    : Destroys the reference counts, pushes derivation
//                   onto ordered_deriv.
//
/----------------------------------------------------------------------*/

long DerivationTopoSort(Derivation_p derivation)
{
   PQueue_p      work_queue;
   PStackPointer sp;
   Clause_p      clause;
   WFormula_p    form;   
   Derived_p     node, newnode;
   PStack_p      deriv;
   PStack_p      parent_clauses, parent_formulas;

   PStackReset(derivation->ordered_deriv);

   work_queue = PQueueAlloc();
   parent_clauses  = PStackAlloc();
   parent_formulas = PStackAlloc();

   for(sp=0; sp<PStackGetSP(derivation->roots); sp++)
   {
      node = PStackElementP(derivation->roots, sp);
      
      PQueueStoreP(work_queue, node);
   }    
   
   while(!PQueueEmpty(work_queue))
   {
      node = PQueueGetNextP(work_queue);
      assert(node->ref_count == 0);
      PStackPushP(derivation->ordered_deriv, node);

      if(node->clause)
      {
         deriv = node->clause->derivation;
      }
      else
      {
         assert(node->formula);
         deriv = node->formula->derivation;
      }
      DerivStackExtractParents(deriv, 
                               parent_clauses, 
                               parent_formulas);
      while(!PStackEmpty(parent_clauses))
      {
         clause = PStackPopP(parent_clauses);
         newnode = DerivationGetDerived(derivation, clause, NULL);
         newnode->ref_count--;
         if(!newnode->ref_count)
         {
            PQueueStoreP(work_queue, newnode);
         }
      }
      while(!PStackEmpty(parent_formulas))
      {
         form = PStackPopP(parent_formulas);
         newnode = DerivationGetDerived(derivation, NULL, form);
         newnode->ref_count--;
         if(!newnode->ref_count)
         {
            PQueueStoreP(work_queue, newnode);
         }         
      }     
   }
   derivation->ordered = true;

   PQueueFree(work_queue);
   PStackFree(parent_clauses);
   PStackFree(parent_formulas);

   return PStackGetSP(derivation->ordered_deriv);
}


/*-----------------------------------------------------------------------
//
// Function: DerivationCompute()
//
//   Given a set (stack) of final clauses, generate an ordered
//   derivation from it.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Derivation_p DerivationCompute(PStack_p root_clauses)
{
   Derivation_p res = DerivationAlloc();
   
   DerivationExtract(res, root_clauses);
   DerivationTopoSort(res);
   
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: DerivationPrint()
//
//   Print a derivation.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void DerivationPrint(FILE* out, Derivation_p derivation)
{
   PStackPointer sp;
   Derived_p     node;

   assert(derivation->ordered);
   
   fprintf(out, "# ------- Derivation start ----------\n");
   for(sp=PStackGetSP(derivation->ordered_deriv)-1; sp>=0; sp--)
   {
      node = PStackElementP(derivation->ordered_deriv, sp);
      if(node->clause)
      {
         ClausePrint(out, node->clause, true);
         fprintf(out, "\n");
      }
      else
      {
         assert(node->formula);
         WFormulaPrint(out, node->formula, true);
         fprintf(out, "\n");
      }
   }
   fprintf(out, "# ------- Derivation end ----------\n");
}




/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


