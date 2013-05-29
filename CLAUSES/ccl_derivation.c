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

char *opids[] =
{
   "NOP",
   "QUOTE",
   /* Simplifying */
   PCL_RW,
   PCL_RW,
   PCL_AD,
   PCL_CSR,
   PCL_ER,
   PCL_SR,
   PCL_ACRES,
   PCL_CONDENSE,
   PCL_CN,
   PCL_EVANS,
   /* Simplification/Modfication for FOF */
   PCL_NC,
   PCL_FS,
   PCL_NNF,
   PCL_SQ,
   PCL_VR,
   PCL_SK,
   PCL_DSTR,
   PCL_ANNOQ,
   /* Generating */
   PCL_PM,
   PCL_SPM,
   PCL_OF,
   PCL_EF,
   PCL_ER,
   /* Others */
   PCL_SE,
   PCL_ID_DEF,
   PCL_SC
};

char *optheory [] =
{
   NULL,
   NULL,
   /* Simplifying */
   "equality",
   "equality",
   "equality,[symmetry]",
   "equality,[symmetry]",
   "equality",
   "equality",
   "equality",
   "equality,[symmetry]",
   "equality,[symmetry]",
   "answers",
   /* Simplification/Modfication for FOF */
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   "answers",
   /* Generating */
   "equality",
   "equality",
   "equality,[symmetry]",
   "equality",
   "equality",
   /* Others */
   NULL,
   NULL,
   NULL
};


char *opstatus [] =
{
   NULL,
   NULL,
   /* Simplifying */
   "thm",
   "thm",
   "thm",
   "thm",
   "thm",
   "thm",
   "thm",
   "thm",
   "thm",
   "thm",
   /* Simplification/Modfication for FOF */
   "cth",
   "thm",
   "thm",
   "thm",
   "thm",
   "esa",
   "thm",
   "thm",
   /* Generating */
   "thm",
   "thm",
   "thm",
   "thm",
   "thm",
   /* Others */
   "thm",
   NULL,
   "thm"
};



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
   key2 = d2->clause? (void*)d2->clause: (void*)d2->formula;

   return PCmp(key1, key2);
}

/*-----------------------------------------------------------------------
//
// Function: derived_get_derivation()
//
//   Given a derived cell, return the derivation of the clause or
//   formula (or NULL in none).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

PStack_p derived_get_derivation(Derived_p derived)
{
   assert(derived);

   if(derived->clause)
   {
      return derived->clause->derivation;
   }
   else
   {
      assert(derived->formula);
      return derived->formula->derivation;
   }
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
// Global Variables: BuildProofObject
//
// Side Effects    : May allocate new derivation stack.
//
/----------------------------------------------------------------------*/

void ClausePushDerivation(Clause_p clause, DerivationCodes op, 
                          void* arg1, void* arg2)
{
   if(BuildProofObject)
   {
      assert(clause);
      assert(op);
      
      CLAUSE_ENSURE_DERIVATION(clause);
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
}


/*-----------------------------------------------------------------------
//
// Function: ClausePushDerivation()
//
//   Push the derivation items (op-code and suitable number of
//   arguments) onto the derivation stack.
//
// Global Variables: BuildProofObject
//
// Side Effects    : May allocate new derivation stack.
//
/----------------------------------------------------------------------*/

void ClausePushACResDerivation(Clause_p clause, Sig_p sig)
{
   if(BuildProofObject)
   {
      assert(clause);
      
      CLAUSE_ENSURE_DERIVATION(clause);

      PStackPushInt(clause->derivation, DCACRes);
      PStackPushInt(clause->derivation, PStackGetSP(sig->ac_axioms));
      /* printf("Pushed: %d\n", PStackGetSP(sig->ac_axioms)); */
   }
}

/*-----------------------------------------------------------------------
//
// Function: WFormulaPushDerivation()
//
//   Push the derivation items (op-code and suitable number of
//   arguments) onto the derivation stack.
//
// Global Variables: BuildProofObject
//
// Side Effects    : May allocate new derivation stack.
//
/----------------------------------------------------------------------*/

void WFormulaPushDerivation(WFormula_p form, DerivationCodes op, 
                            void* arg1, void* arg2)
{
   if(BuildProofObject)
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
                              Sig_p    sig,
                              PStack_p res_clauses, 
                              PStack_p res_formulas)
{
   PStackPointer i, sp;
   long res = 0;
   long numarg1 = 0;
   DerivationCodes op;

   assert(res_clauses);
   assert(res_formulas);

   if(derivation)
   {
      sp = PStackGetSP(derivation);
      i  = 0;

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
         else if(DCOpHasNumArg1(op))
         {
            numarg1 = PStackElementInt(derivation, i);
            i++;
         }
         if(DCOpHasCnfArg2(op))
         {
            PStackPushP(res_clauses, PStackElementP(derivation, i));
            i++;
            res++;
         }
         else if(DCOpHasFofArg2(op))
         {
            PStackPushP(res_formulas, PStackElementP(derivation, i));
            i++;
            res++;
         }
         else if(DCOpHasNumArg2(op))
         {
            i++;
         }
         if(op==DCACRes)
         {
            int sp;
            for(sp = 0; sp<numarg1; sp++)
            {
               PStackPushP(res_clauses, PStackElementP(sig->ac_axioms,sp));
            }
         }
      }
   }
   return res;
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
   handle->is_root   = false;
   handle->is_fresh  = true;
   handle->ref_count = 0;
   handle->clause    = NULL;
   handle->formula   = NULL;

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: get_clauseform_id()
//
//   
//
// Global Variables: 
//
// Side Effects    : 
//
/----------------------------------------------------------------------*/

long get_clauseform_id(DerivationCodes op, int select, void* clauseform)
{
   long id;
   Clause_p   clause;
   WFormula_p form;

   switch(select)
   {
   case 1:
         if(DCOpHasCnfArg1(op))
         {
            clause = clauseform;
            id = clause->ident;
         }
         else if(DCOpHasFofArg1(op))
         {
            form = clauseform;
            id = form->ident;
         }
         else
         {
            assert(false && "Argument selected does not exist");
         }
         break;
   case 2:
         if(DCOpHasCnfArg2(op))
         {
            clause = clauseform;
            id = clause->ident;
         }
         else if(DCOpHasFofArg2(op))
         {
            form = clauseform;
            id = form->ident;
         }
         else
         {
            assert(false && "Argument selected does not exist");
         }
         break;
   default:
         assert(false && "Illegal argument selector");
   }   
   return id;
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

void DerivationStackPCLPrint(FILE* out, Sig_p sig, PStack_p derivation)
{
   PStack_p subexpr_stack;
   PStackPointer i, j, sp, ac_limit;
   DerivationCodes op;
   Clause_p        ax;

   if(derivation)
   {
      subexpr_stack = PStackAlloc();
      
      /* Find the beginnings of the subexpressions */
      sp = PStackGetSP(derivation);      
      i  = 0;
      while(i<sp)
      {
         PStackPushInt(subexpr_stack, i);

         op = PStackElementInt(derivation, i);
         i++;
         if(DCOpHasArg1(op))
         {
            i++;
         }
         if(DCOpHasArg2(op))
         {
            i++;
         }
      }
      /* Print the beginning of the subexpressions */
      
      for(sp = PStackGetSP(subexpr_stack)-1; sp>=0; sp--)
      {
         i = PStackElementInt(subexpr_stack, sp);
         op = PStackElementInt(derivation, i);
         switch(op)
         {
         case DCCnfQuote:
         case DCFofQuote:
               break;
         case DCIntroDef:
               fprintf(out, "%s", "introduced");
               break;
         default:
               fprintf(out, "%s(", opids[DPOpGetOpCode(op)]);
               break;
         }
      }      
      /* And finish the expressions */
      
      for(sp = 0; sp < PStackGetSP(subexpr_stack); sp++)
      {
         i = PStackElementInt(subexpr_stack, sp);
         op = PStackElementInt(derivation, i);
         if(DCOpHasParentArg1(op))
         {
            if(i!=0)
            {
               fprintf(out, ", ");
            }         
            fprintf(out, "%ld", 
                    get_clauseform_id(op, 1, PStackElementP(derivation, i+1)));
            if(DCOpHasParentArg2(op))
            {
               fprintf(out, ", %ld", 
                       get_clauseform_id(op, 2, PStackElementP(derivation, i+2)));
            }
         }
         switch(op)
         {
         case DCCnfQuote:
         case DCFofQuote:
               break;
         case DCIntroDef:
               break;
         case DCACRes:
               ac_limit = PStackElementInt(derivation, i+1);
               for(j=0; j<ac_limit; j++)
               {
                  ax = PStackElementP(sig->ac_axioms, j);
                  fprintf(out, ", %ld", ax->ident);
               }
               fprintf(out, ")");
               break;
         default:
               fprintf(out, ")");
               break;
         }
      }      
      /* Cleanup */
      PStackFree(subexpr_stack);      
   }
}





/*-----------------------------------------------------------------------
//
// Function: DerivationStackTSTPPrint()
//
//   Print the derivation stack as a TSTP expression.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void DerivationStackTSTPPrint(FILE* out, Sig_p sig, PStack_p derivation)
{
   PStack_p subexpr_stack;
   PStackPointer i, j, sp, ac_limit;
   DerivationCodes op, opc;
   Clause_p        ax;

   if(derivation)
   {
      subexpr_stack = PStackAlloc();
      
      /* Find the beginnings of the subexpressions */
      sp = PStackGetSP(derivation);      
      i  = 0;
      while(i<sp)
      {
         PStackPushInt(subexpr_stack, i);

         op = PStackElementInt(derivation, i);
         i++;
         if(DCOpHasArg1(op))
         {
            i++;
         }
         if(DCOpHasArg2(op))
         {
            i++;
         }
      }
      /* Print the beginning of the subexpressions */
      
      for(sp = PStackGetSP(subexpr_stack)-1; sp>=0; sp--)
      {
         i = PStackElementInt(subexpr_stack, sp);
         op  = PStackElementInt(derivation, i);
         opc = DPOpGetOpCode(op);
         switch(op)
         {
         case DCCnfQuote:
         case DCFofQuote:
               break;
         case DCIntroDef:
               fprintf(out, "%s", opids[DPOpGetOpCode(op)]);
               break;
         default:
               fprintf(out, "inference(%s,[status(%s)],[", 
                       opids[opc],
                       opstatus[opc]);
               break;
         }
      }      
      /* And finish the expressions */
      
      for(sp = 0; sp < PStackGetSP(subexpr_stack); sp++)
      {
         i = PStackElementInt(subexpr_stack, sp);
         op  = PStackElementInt(derivation, i);
         opc = DPOpGetOpCode(op);
         if(DCOpHasParentArg1(op))
         {
            if(i!=0)
            {
               fprintf(out, ", ");
            }         
            fprintf(out, "c_0_%ld", 
                    get_clauseform_id(op, 1, PStackElementP(derivation, i+1)));
            if(DCOpHasParentArg2(op))
            {
               fprintf(out, ", c_0_%ld", 
                       get_clauseform_id(op, 2, PStackElementP(derivation, i+2)));
            }
         }
         switch(op)
         {
         case DCCnfQuote:
         case DCFofQuote:
               break;
         case DCIntroDef:
               break;
         case DCACRes:
               ac_limit = PStackElementInt(derivation, i+1);
               for(j=0; j<ac_limit; j++)
               {
                  ax = PStackElementP(sig->ac_axioms, j);
                  fprintf(out, ", c_0_%ld", ax->ident);
               }
               if(optheory[opc])
               {
                  fprintf(out, ", theory(%s)",optheory[opc]);
               }
               fprintf(out, "])");
               break;
         default:
              if(optheory[opc])
               {
                  fprintf(out, ", theory(%s)",optheory[opc]);
               }
               fprintf(out, "])");
               break;
         }
      }      
      /* Cleanup */
      PStackFree(subexpr_stack);      
   }
}




/*-----------------------------------------------------------------------
//
// Function: DerivedPCLPrint()
//
//   Print a "Derived" cell - i.e. the clause or formula, and its
//   derivation, in PCL format 
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void DerivedPCLPrint(FILE* out, Sig_p sig, Derived_p derived)
{
   if(derived->clause)
   {
      fprintf(out, "%6ld : ", derived->clause->ident);      
      fprintf(out, "%s : ", 
              PCLTypeStr(ClauseQueryTPTPType(derived->clause)));
      ClausePCLPrint(out, derived->clause, PCLFullTerms);
      fputs(" : ", out);
      if(derived->clause->derivation)
      {
         DerivationStackPCLPrint(out, sig, derived->clause->derivation);
      }
      else if(derived->clause->info)
      {
         ClauseSourceInfoPrintPCL(out, derived->clause->info); 
      }
      if(derived->is_root)
      {
         if(ClauseIsEmpty(derived->clause))
         {
            fprintf(out, " : 'proof'");      
         }
         else
         {
            fprintf(out, " : 'final'");      
         }
      }
   }
   else
   {
      assert(derived->formula);
      fprintf(out, "%6ld : ", derived->formula->ident);      
      fprintf(out, "%s : ", 
              PCLTypeStr(FormulaQueryType(derived->formula)));
      TFormulaTPTPPrint(out, 
                        derived->formula->terms, 
                        derived->formula->tformula, 
                        true, true);
      fputs(" : ", out);
      if(derived->formula->derivation)
      {
         DerivationStackPCLPrint(out, sig, derived->formula->derivation);
      }
      else if(derived->formula->info)
      {
         ClauseSourceInfoPrintPCL(out, derived->formula->info); 
      }
   }   
}


/*-----------------------------------------------------------------------
//
// Function: DerivedTSTPPrint()
//
//   Print a "Derived" cell - i.e. the clause or formula, and its
//   derivation, in TSTP format 
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void DerivedTSTPPrint(FILE* out, Sig_p sig, Derived_p derived)
{
   if(derived->clause)
   {
      // fprintf(out, "%p: ", derived->clause);      
      ClauseTSTPPrint(out, derived->clause, true, false);
      if(derived->clause->derivation)
      {
         fprintf(out, ", ");
         DerivationStackTSTPPrint(out, sig, derived->clause->derivation);
      }
      else
      {
         if(derived->clause->info)
         {
            fprintf(out, ", ");
            ClauseSourceInfoPrintTSTP(out, derived->clause->info); 
         }
      }
      if(derived->is_root)
      {
         if(ClauseIsEmpty(derived->clause))
         {
            fprintf(out, ", ['proof']");      
         }
         else
         {
            fprintf(out, ", ['final']");      
         }
      }
      fprintf(out, ").");
   }
   else
   {
      assert(derived->formula);
      // fprintf(out, "%p: ", derived->formula);      
      WFormulaTSTPPrint(out, derived->formula, true, false);
      if(derived->formula->derivation)
      {
         fprintf(out, ", ");
         DerivationStackTSTPPrint(out, sig, derived->formula->derivation);
      }
      else
      {
         if(derived->formula->info)
         {
            fprintf(out, ", ");
            ClauseSourceInfoPrintTSTP(out, derived->formula->info); 
         }
      }
      fprintf(out, ").");
   }   
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

Derivation_p DerivationAlloc(Sig_p sig)
{
   Derivation_p handle = DerivationCellAlloc();

   handle->sig           = sig;
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
   Clause_p      clause;
   WFormula_p    form;   
   Derived_p     node, newnode;
   PStack_p      deriv;
   PStack_p      stack, parent_clauses, parent_formulas;

   parent_clauses  = PStackAlloc();
   parent_formulas = PStackAlloc();
   PStackPointer i;

   stack = PStackCopy(derivation->roots);

   for(i=0; i<PStackGetSP(stack); i++)
   {
      node = PStackElementP(stack,i);
      node->is_fresh = false;
   }

   while(!PStackEmpty(stack))
   {
      node = PStackPopP(stack);

      deriv = derived_get_derivation(node);

      assert(PStackEmpty(parent_clauses));
      assert(PStackEmpty(parent_formulas));
      DerivStackExtractParents(deriv,
                               derivation->sig,
                               parent_clauses, 
                               parent_formulas);
      while(!PStackEmpty(parent_clauses))
      {
         clause = PStackPopP(parent_clauses);
         newnode = DerivationGetDerived(derivation, clause, NULL);
         if(newnode->is_fresh)
         {
            newnode->is_fresh = false;
            PStackPushP(stack, newnode);
         }
         newnode->ref_count++;
      }
      while(!PStackEmpty(parent_formulas))
      {
         form = PStackPopP(parent_formulas);
         newnode = DerivationGetDerived(derivation, NULL, form);
         if(newnode->is_fresh)
         {
            newnode->is_fresh = false;
            PStackPushP(stack, newnode);
         }
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
//   Perform a topological sort of the derivation. This is slightly
//   hacked because axioms (nodes without further parents) always come
//   first, so that axioms are listed first (for convenience and user
//   expectation). 
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
   PStack_p      ax_stack;
   PStackPointer sp;
   Clause_p      clause;
   WFormula_p    form;   
   Derived_p     node, newnode;
   PStack_p      deriv;
   PStack_p      parent_clauses, parent_formulas;

   PStackReset(derivation->ordered_deriv);

   work_queue = PQueueAlloc();
   ax_stack   = PStackAlloc();
   parent_clauses  = PStackAlloc();
   parent_formulas = PStackAlloc();

   for(sp=0; sp<PStackGetSP(derivation->roots); sp++)
   {
      node = PStackElementP(derivation->roots, sp);
      if(node->ref_count == 0)
      {
         PQueueStoreP(work_queue, node);
      }
   }    
   
   while(!PQueueEmpty(work_queue))
   {
      node = PQueueGetNextP(work_queue);
      assert(node->ref_count == 0);
      PStackPushP(derivation->ordered_deriv, node);

      deriv = derived_get_derivation(node);

      (void)DerivStackExtractParents(deriv,
                                           derivation->sig,
                                           parent_clauses, 
                                           parent_formulas);
      while(!PStackEmpty(parent_clauses))
      {
         clause = PStackPopP(parent_clauses);
         newnode = DerivationGetDerived(derivation, clause, NULL);
         newnode->ref_count--;
         if(!newnode->ref_count)
         {
            if(derived_get_derivation(newnode))
            {
               PQueueStoreP(work_queue, newnode);
            }
            else
            {
               PStackPushP(ax_stack, newnode);
            }
         }
      }
      while(!PStackEmpty(parent_formulas))
      {
         form = PStackPopP(parent_formulas);
         newnode = DerivationGetDerived(derivation, NULL, form);
         newnode->ref_count--;
         if(!newnode->ref_count)
         {
            if(derived_get_derivation(newnode))
            {
               PQueueStoreP(work_queue, newnode);
            }
            else
            {
               PStackPushP(ax_stack, newnode);
            }
         }         
      }     
   }

   PStackPushStack(derivation->ordered_deriv, ax_stack);

   PQueueFree(work_queue);
   PStackFree(ax_stack);
   PStackFree(parent_clauses);
   PStackFree(parent_formulas);

   derivation->ordered = true;
   return PStackGetSP(derivation->ordered_deriv);
}

/*-----------------------------------------------------------------------
//
// Function: DerivationRenumber()
//
//   Renumber clauses and formulas in a derivation in order.
//
// Global Variables: -
//
// Side Effects    : See main ;-)
//
/----------------------------------------------------------------------*/

void DerivationRenumber(Derivation_p derivation)
{
   PStackPointer sp;
   Derived_p     node;
   long          idents = 0;

   assert(derivation->ordered);
   for(sp=PStackGetSP(derivation->ordered_deriv)-1; sp>=0; sp--)
   {
      node = PStackElementP(derivation->ordered_deriv, sp);
      if(node->clause)
      {
         node->clause->ident = idents++;
      }
      else
      {
         assert(node->formula);
         node->formula->ident = idents++;
      }
   }
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

Derivation_p DerivationCompute(PStack_p root_clauses, Sig_p sig)
{
   Derivation_p  res = DerivationAlloc(sig);
   PStackPointer sp;
   Clause_p      clause;
   Derived_p     node;

   for(sp=0; sp<PStackGetSP(root_clauses); sp++)
   {
      clause = PStackElementP(root_clauses, sp);
      
      node = DerivationGetDerived(res, clause, NULL);
      node->is_root = true;
      PStackPushP(res->roots, node);
   }       
   DerivationExtract(res, root_clauses);
   DerivationTopoSort(res);
   DerivationRenumber(res);
   
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

void DerivationPrint(FILE* out, Derivation_p derivation, char* frame)
{
   PStackPointer sp;
   Derived_p     node;

   assert(derivation->ordered);
   
   fprintf(out, "# SZS output start %s.\n", frame);
   for(sp=PStackGetSP(derivation->ordered_deriv)-1; sp>=0; sp--)
   {
      node = PStackElementP(derivation->ordered_deriv, sp);
      switch(DocOutputFormat)
      {
      case pcl_format:
            DerivedPCLPrint(out, derivation->sig, node);
            break;
      case tstp_format:
            DerivedTSTPPrint(out, derivation->sig, node);
            break;
      default:
            fprintf(out, "# Output format not implemented.");
            break;
      }
      fprintf(out, "\n");
   }
   fprintf(out, "# SZS output end %s.\n", frame);
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


