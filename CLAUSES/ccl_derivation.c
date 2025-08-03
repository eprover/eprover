/*-----------------------------------------------------------------------

  File  : ccl_derivation.c

  Author: Stephan Schulz (schulz@eprover.org)

  Contents

  Functions related to the construction, manipulation, and printing of
  explicit proof objects in E.

  Copyright 2013-2018 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main E directory for details.
  Run "eprover -h" for contact information.

  Changes

  Created Sat Apr  6 10:58:29 CEST 2013

  -----------------------------------------------------------------------*/

#include "ccl_derivation.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

ProofObjectType PrintProofObject = 0;
bool            ProofObjectRecordsGCSelection = false;

char *opids[] =
{
   "NOP",
   "QUOTE",
   "AddArg",
   /* Simplifying */
   PCL_EVALGC,
   PCL_RW,
   PCL_LOCAL_RW,
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
   PCL_EXPDISTICT,
   /* Generating */
   PCL_PM,
   PCL_SPM,
   PCL_OF,
   PCL_EF,
   PCL_ER,
   PCL_DDC,
   PCL_SAT,
   PCL_PE_RESOLVE,
   /* Others */
   PCL_SE,
   PCL_ID_DEF,
   PCL_SC,
   PCL_EQ_TO_EQ,
   PCL_LL,
   PCL_FU,
   PCL_LIFT_ITE,
   PCL_EBV,
   /* HO inferences */
   PCL_DYN_CNF,
   PCL_FLEX_RESOLVE,
   PCL_ARG_CONG,
   PCL_NEG_EXT,
   PCL_POS_EXT,
   PCL_EXT_SUP,
   PCL_EXT_EQRES,
   PCL_EXT_EQFACT,
   PCL_INV_REC,
   PCL_CHOICE_AX,
   PCL_LEIBNIZ_ELIM,
   PCL_PRIM_ENUM,
   PCL_CHOICE_INST,
   PCL_TRIGGER,
   PCL_PRUNE_ARG
};

char *optheory [] =
{
   NULL,
   NULL,
   "NA",
   /* Simplifying */
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
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
   "distinct",
   /* Generating */
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   /* Others */
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   /* HO inferences */
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL
};


char *opstatus [] =
{
   NULL,
   NULL,
   "NA",
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
   "thm",
   /* Generating */
   "thm",
   "thm",
   "thm",
   "thm",
   "thm",
   "thm",
   "thm",
   "thm",
   /* Others */
   "thm",
   NULL,
   "thm",
   "thm",
   "thm",
   "thm",
   "thm",
   "thm",
   /* HO */
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
   "thm",
   "thm",
   "thm",
   "thm",
   "thm"
};


static char *node_gray = ",color=gray, fillcolor=gray",
   *node_axgray  = ",color=gray, fillcolor=gray66",
   *node_green   = ",color=green,fillcolor=palegreen",
   *node_axgreen = ",color=green,fillcolor=forestgreen",
   *node_red     = ",color=red,fillcolor=lightpink1",
   *node_axred   = ",color=red,fillcolor=firebrick1",
   *node_purple  = ",color=blue,fillcolor=darkorchid1",
   *node_blue    = ",color=blue,fillcolor=lightskyblue1",
   *node_axblue  = ",color=blue,fillcolor=dodgerblue";




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



/*-----------------------------------------------------------------------
//
// Function: get_clauseform_id()
//
//   Return the identifier of the selected argument of the operator,
//   assuming that clauseform points to the corresponding clause or
//   formula.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long get_clauseform_id(DerivationCode op, int select, void* clauseform)
{
   long id = -1;
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
// Function: tstp_get_clauseform_id()
//
//    Return a TSTP identifier for a derivation stack clause- or
//    formula reference.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/


char* tstp_get_clauseform_id(DerivationCode op, int select,
                             void* clauseform)
{
   static char result[30];
   Clause_p   clause;
   WFormula_p form;

   result[0]='0';
   result[1]='\0';

   switch(select)
   {
   case 1:
         if(DCOpHasCnfArg1(op))
         {
            clause = clauseform;
            sprintf(result, "c_0_%ld", clause->ident);
         }
         else if(DCOpHasFofArg1(op))
         {
            form = clauseform;
            return WFormulaGetId(form);
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
            sprintf(result, "c_0_%ld", clause->ident);
         }
         else if(DCOpHasFofArg2(op))
         {
            form = clauseform;
            return WFormulaGetId(form);
         }
         else
         {
            assert(false && "Argument selected does not exist");
         }
         break;
   default:
         assert(false && "Illegal argument selector");
   }
   return result;
}


/*-----------------------------------------------------------------------
//
// Function: derivation_find_max_id()
//
//    Find the largest input id (in ClauseInfo fields) of any formula
//    in derivation.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static long derivation_find_max_id(Derivation_p derivation)
{
   PStackPointer sp;
   Derived_p     node;
   long          max_ident = -1;
   long          tmp;

   assert(derivation->ordered);

   for(sp=PStackGetSP(derivation->ordered_deriv)-1; sp>=0; sp--)
   {
      node = PStackElementP(derivation->ordered_deriv, sp);
      if(!node->clause)
      {
         tmp = ClauseInfoGetIdCounter(node->formula->info);
         if(tmp > max_ident)
         {
            max_ident = tmp;
         }
      }
   }
   return max_ident;
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: DerivedInProof()
//
//   Return true if the derived cell is known to be in proof. This is
//   the case if it is the empty clause, or if it marked as being in a
//   proof (presumably because one of its transitive descendants is
//   the empty clause).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool DerivedInProof(Derived_p derived)
{
   if(derived->clause)
   {
      if(ClauseIsEmpty(derived->clause))
      {
         return true;
      }
      if(ClauseQueryProp(derived->clause,CPIsProofClause))
      {
         return true;
      }
      return false;
   }
   return FormulaQueryProp(derived->formula,CPIsProofClause);
}

/*-----------------------------------------------------------------------
//
// Function: DerivedSetInProof()
//
//   Mark a derived cell as a proof cell.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void DerivedSetInProof(Derived_p derived, bool in_proof)
{
   if(in_proof)
   {
      if(derived->clause)
      {
         ClauseSetProp(derived->clause,CPIsProofClause);
      }
      else
      {
         FormulaSetProp(derived->formula, CPIsProofClause);
      }
   }
   else
   {
      if(derived->clause)
      {
         ClauseDelProp(derived->clause,CPIsProofClause);
      }
      else
      {
         FormulaDelProp(derived->formula, CPIsProofClause);
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: DerivedCollectFCodes()
//
//   Collect all f_codes from the logical clause/formula into
//   *tree. Return number of new entries found.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long DerivedCollectFCodes(Derived_p derived, NumTree_p *tree)
{
   if(derived->clause)
   {
      return ClauseCollectFCodes(derived->clause, tree);
   }
   else
   {
      return TermCollectFCodes(derived->formula->tformula, tree);
   }
}



/*-----------------------------------------------------------------------
//
// Function: ClausePushDerivation()
//
//   Push the derivation items (op-code and suitable number of
//   arguments) onto the derivation stack.
//
// Global Variables:
//
// Side Effects    : May allocate new derivation stack.
//
/----------------------------------------------------------------------*/

void ClausePushDerivation(Clause_p clause, DerivationCode op,
                          void* arg1, void* arg2)
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


/*-----------------------------------------------------------------------
//
// Function: ClausePushACResDerivation()
//
//   Push the derivation items (op-code and suitable number of
//   arguments) onto the derivation stack.
//
// Global Variables:
//
// Side Effects    : May allocate new derivation stack.
//
/----------------------------------------------------------------------*/

void ClausePushACResDerivation(Clause_p clause, Sig_p sig)
{
   assert(clause);

   CLAUSE_ENSURE_DERIVATION(clause);

   PStackPushInt(clause->derivation, DCACRes);
   PStackPushInt(clause->derivation, PStackGetSP(sig->ac_axioms));
   /* printf("Pushed: %d\n", PStackGetSP(sig->ac_axioms)); */
}

/*-----------------------------------------------------------------------
//
// Function: WFormulaPushDerivation()
//
//   Push the derivation items (op-code and suitable number of
//   arguments) onto the derivation stack.
//
// Global Variables:
//
// Side Effects    : May allocate new derivation stack.
//
/----------------------------------------------------------------------*/

void WFormulaPushDerivation(WFormula_p form, DerivationCode op,
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
// Function: ClauseIsEvalGC()
//
//   Return true if the clause is the form of the given clause that
//   was evaluation and then selected for processing. This assumes
//   that the DCCnfEvalGC opcode is on top of the derivation stack of
//   such clauses.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool ClauseIsEvalGC(Clause_p clause)
{
   if(clause->derivation)
   {
      return PStackTopInt(clause->derivation)==DCCnfEvalGC;
   }
   return false;
}



/*-----------------------------------------------------------------------
//
// Function: ClauseIsDummyQuote()
//
//   Return true if the clause is just generated as a quote of its
//   single parent.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool ClauseIsDummyQuote(Clause_p clause)
{
   if(clause->derivation)
   {
      if((PStackGetSP(clause->derivation)==2) &&
         (PStackElementInt(clause->derivation, 0)==DCCnfQuote))
      {
         return true;
      }
   }
   return false;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseIsDummyFOFQuote()
//
//   Return true if the clause is just generated as a quote of its
//   single (FOF) parent.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool ClauseIsDummyFOFQuote(Clause_p clause)
{
   if(clause->derivation)
   {
      if((PStackGetSP(clause->derivation)==2) &&
         (PStackElementInt(clause->derivation, 0)==DCFofQuote))
      {
         return true;
      }
   }
   return false;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseDerivFindFirst()
//
//   Given a clause, check if it's part of a reference cascade (i.e. has
//   just on parent and is justified by a simple reference to the
//   parent (via OpCode DCCnfQuote)). If yes, track back the reference
//   cascade and return the first (original) occurrence of the clause.
//   Otherwise return the clause.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Clause_p ClauseDerivFindFirst(Clause_p clause)
{
   Clause_p parent = clause;

   if(ClauseIsDummyQuote(clause))
   {
      parent = PStackElementP(clause->derivation, 1);
      parent = ClauseDerivFindFirst(parent);
   }
   return parent;
}



/*-----------------------------------------------------------------------
//
// Function: WFormulaDerivFindFirst()
//
//   Given a formula, check if it's part of a reference cascade (i.e. has
//   just on parent and is justified by a simple reference to the
//   parent (via OpCode DCFofQuote)). If yes, track back the reference
//   cascade and return the first (original) occurrence of the formula.
//   Otherwise return the formula.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

WFormula_p WFormulaDerivFindFirst(WFormula_p form)
{
   WFormula_p parent = form;

   if(form->derivation)
   {
      if((PStackGetSP(form->derivation)==2) &&
         (PStackElementInt(form->derivation, 0)==DCFofQuote))
      {
         parent = PStackElementP(form->derivation, 1);
         parent = WFormulaDerivFindFirst(parent);
      }
   }
   return parent;
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
   PStackPointer i, sp, j;
   long res = 0;
   long numarg1 = 0;
   DerivationCode op;

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
            for(j = 0; j<numarg1; j++)
            {
               PStackPushP(res_clauses, PStackElementP(sig->ac_axioms, j));
            }
         }
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: DerivStackExtractOptParents()
//
//   Given a derivation stack (derivation-codes with arguments),
//   return all the (occurrences of) all the original instances of the
//   side premises referenced in the derivation (via the result
//   stacks). Return value is the number of premises found. Modify the
//   derivation to replace references to a parent with references to
//   the original instance of that parent.
//
// Global Variables: -
//
// Side Effects    : (via PStackPushP())
//
/----------------------------------------------------------------------*/

long DerivStackExtractOptParents(PStack_p derivation,
                                 Sig_p    sig,
                                 PStack_p res_clauses,
                                 PStack_p res_formulas)
{
   PStackPointer i, sp, j;
   long res = 0;
   long numarg1 = 0;
   DerivationCode op;
   Clause_p   cparent;
   WFormula_p fparent;

   assert(res_clauses);
   assert(res_formulas);

   if(derivation)
   {
      sp = PStackGetSP(derivation);
      i  = 0;

      while(i<sp)
      {
         op = PStackElementInt(derivation, i);
         //printf("i=%ld, sp=%ld, op=%u (%u), args=%X\n",i, sp,op%255,DOVarRename,op>>8);
         i++;
         if(DCOpHasCnfArg1(op))
         {
            cparent = ClauseDerivFindFirst(PStackElementP(derivation, i));
            PStackAssignP(derivation, i, cparent);
            PStackPushP(res_clauses, cparent);
            i++;
            res++;
         }
         else if(DCOpHasFofArg1(op))
         {
            fparent = WFormulaDerivFindFirst(PStackElementP(derivation, i));
            PStackAssignP(derivation, i, fparent);
            PStackPushP(res_formulas, fparent);
            i++;
            res++;
         }
         else if(DCOpHasNumArg1(op))
         {
            numarg1 = PStackElementInt(derivation, i);
            //printf("Numarg1: %ld\n", numarg1);
            i++;
         }
         if(DCOpHasCnfArg2(op))
         {
            cparent = ClauseDerivFindFirst(PStackElementP(derivation, i));
            PStackAssignP(derivation, i, cparent);
            PStackPushP(res_clauses, cparent);
            i++;
            res++;
         }
         else if(DCOpHasFofArg2(op))
         {
            fparent = WFormulaDerivFindFirst(PStackElementP(derivation, i));
            PStackAssignP(derivation, i, fparent);
            PStackPushP(res_formulas, fparent);
            i++;
            res++;
         }
         else if(DCOpHasNumArg2(op))
         {
            i++;
         }
         if(op==DCACRes)
         {
            //printf("ACRes: Numarg1: %ld\n", numarg1);
            for(j = 0; j<numarg1; j++)
            {
               //printf("j=:%ld\n", j);
               PStackPushP(res_clauses, PStackElementP(sig->ac_axioms, j));
               //printf("OK\n");
            }
         }
      }
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: DerivStackCountSearchInferences()
//
//   Given a derivation stack (derivation-codes with arguments),
//   count the number of generating and simplifying inferences in the
//   stack.
//
// Global Variables: -
//
// Side Effects    : (via PStackPushP())
//
/----------------------------------------------------------------------*/

void DerivStackCountSearchInferences(PStack_p derivation,
                                     unsigned long *generating_count,
                                     unsigned long *simplifying_count)
{
   PStackPointer i, sp;
   DerivationCode op;

   if(derivation)
   {
      sp = PStackGetSP(derivation);
      i  = 0;

      while(i<sp)
      {
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
         switch(op)
         {
         case DCParamod:
         case DCSimParamod:
         case DCOrderedFactor:
         case DCEqFactor:
         case DCEqRes:
               (*generating_count)++;
               break;
         case DCRewrite:
         case DCUnfold:
         case DCApplyDef:
         case DCContextSR:
         case DCDesEqRes:
         case DCSR:
         case DCACRes:
         case DCCondense:
         case DCNormalize:
         case DCEvalAnswers:
               (*simplifying_count)++;
               break;
         default:
               break;
         }
      }
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
   handle->is_root   = false;
   handle->is_fresh  = true;
   handle->ref_count = 0;
   handle->clause    = NULL;
   handle->formula   = NULL;

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: DerivationStackPCLPrint()
//
//   Print a very short description of the derivation for debug
//   purposes.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void DerivationDebugPrint(FILE* out, PStack_p derivation)
{
   if(derivation)
   {
      PStackPointer sp = PStackGetSP(derivation);
      PStackPointer i  = 0;
      DerivationCode op;
      while(i<sp)
      {
         op = PStackElementInt(derivation, i);
         i++;
         if(DCOpHasArg1(op))
         {
            if(DCOpHasCnfArg1(op))
            {
               DBG_PRINT(out, "[", ClausePrintDBG(out, PStackElementP(derivation, i)), "]");
            }
            else if(DCOpHasFofArg1(op))
            {
               WFormula_p f = PStackElementP(derivation, i);
               DBG_PRINT(out, "[", WFormulaTSTPPrint(stderr, f, true, true), "]");
            }
            i++;
         }
         if(DCOpHasArg2(op))
         {
            if(DCOpHasCnfArg2(op))
            {
               DBG_PRINT(out, "[", ClausePrintDBG(out, PStackElementP(derivation, i)), "]");
            }
            else if(DCOpHasFofArg2(op))
            {
               WFormula_p f = PStackElementP(derivation, i);
               DBG_PRINT(out, "[", WFormulaTSTPPrint(stderr, f, true, true), "]");
            }
            i++;
         }
         fprintf(out, "<%s%s>", opids[DPOpGetOpCode(op)], i==sp?"":",");
      }
   }
   else
   {
      fprintf(out, " - ");
   }
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
   PStack_p arg_stack;
   PStackPointer i, j, sp, ac_limit;
   DerivationCode op;
   Clause_p        ax;

   if(derivation)
   {
      subexpr_stack = PStackAlloc();
      arg_stack     = PStackAlloc();

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
         case DCCnfAddArg:
               PStackPushP(arg_stack, PStackElementP(derivation, i+1));
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
         if(op != DCCnfAddArg)
         {
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
            while(!PStackEmpty(arg_stack))
            {
               ax = PStackPopP(arg_stack);
               fprintf(out, ", %ld", ax->ident);
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
      }
      /* Cleanup */
      PStackFree(arg_stack);
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
   PStack_p arg_stack;
   PStackPointer i, j, sp, ac_limit;
   DerivationCode op, opc;
   Clause_p        ax;

   if(derivation)
   {
      subexpr_stack = PStackAlloc();
      arg_stack     = PStackAlloc();

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
         case DCCnfAddArg:
               PStackPushP(arg_stack, PStackElementP(derivation, i+1));
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
         if(op != DCCnfAddArg)
         {
            if(DCOpHasParentArg1(op))
            {
               if(i!=0)
               {
                  fprintf(out, ", ");
               }
               fprintf(out, "%s",
                       tstp_get_clauseform_id(op, 1, PStackElementP(derivation, i+1)));
               if(DCOpHasParentArg2(op))
               {
                  fprintf(out, ", %s",
                          tstp_get_clauseform_id(op, 2, PStackElementP(derivation, i+2)));
               }
            }
            while(!PStackEmpty(arg_stack))
            {
               ax = PStackPopP(arg_stack);
               fprintf(out, ", c_0_%ld", ax->ident);
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
      }
      /* Cleanup */
      PStackFree(arg_stack);
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
// Function: DerivedDotNodeColour()
//
//   Return a string description of the colour to use for a given node
//   in a derivation.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

char* DerivedDotNodeColour(Derived_p derived)
{
   if(!DerivedInProof(derived))
   {
      return DerivedGetDerivstack(derived)?node_gray:node_axgray;
   }
   if(derived->clause)
   {
      if(ClauseIsEmpty(derived->clause))
      {
         return node_purple;
      }
      else
      {
         switch(ClauseQueryTPTPType(derived->clause))
         {
         case CPTypeConjecture:
         case CPTypeNegConjecture:
               return DerivedGetDerivstack(derived)?node_blue:node_axblue;
         default:
               return DerivedGetDerivstack(derived)?node_green:node_axgreen;
         }
      }
   }
   else
   {
      switch(FormulaQueryType(derived->formula))
      {
      case CPTypeConjecture:
            return DerivedGetDerivstack(derived)?node_red:node_axred;
      case CPTypeNegConjecture:
            return DerivedGetDerivstack(derived)?node_blue:node_axblue;
      default:
            return DerivedGetDerivstack(derived)?node_green:node_axgreen;
      }
   }
   assert(false);
}



/*-----------------------------------------------------------------------
//
// Function: DerivedDotClauseLinkColour()
//
//   Return a string description of the colour to use for a given link
//   in a derivation.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

char* DerivedDotClauseLinkColour(Derived_p child, Clause_p parent)
{
   if(!ClauseQueryProp(parent, CPIsProofClause))
   {
      return node_gray;
   }
   if(!DerivedInProof(child))
   {
      return node_gray;
   }
   return DerivedDotNodeColour(child);
}


/*-----------------------------------------------------------------------
//
// Function: DerivedDotFormulaLinkColour()
//
//   Return a string description of the colour to use for a given link
//   in a derivation.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

char* DerivedDotFormulaLinkColour(Derived_p child, WFormula_p parent)
{
   if(!FormulaQueryProp(parent, CPIsProofClause))
   {
      return node_gray;
   }
   if(!DerivedInProof(child))
   {
      return node_gray;
   }
   return DerivedDotNodeColour(child);
}



/*-----------------------------------------------------------------------
//
// Function: DerivedDotPrint()
//
//   Print a "Derived" cell - i.e. the clause or formula, and its
//   derivation, in GraphViz DOT format
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void DerivedDotPrint(FILE* out, Sig_p sig, Derived_p derived,
                     ProofOutput print_derivation)
{
   PStack_p parent_clauses = PStackAlloc();
   PStack_p parent_formulas = PStackAlloc();
   PStack_p deriv;
   long id;
   Clause_p   cparent;
   WFormula_p fparent;
   // long       parent_count;
   ClauseInfo_p info;
   char* shape="box";

   if(derived->clause)
   {
      id = derived->clause->ident;
      deriv = derived->clause->derivation;
      info  = derived->clause->info;
      if(ClauseIsEvalGC(derived->clause) &&
         ClauseQueryProp(derived->clause,CPIsProcessed))
      {
         shape="ellipse";
      }
   }
   else
   {
      assert(derived->formula);
      id = derived->formula->ident;
      deriv = derived->formula->derivation;
      info  = derived->formula->info;
   }
   if(deriv)
   {
      DerivStackExtractOptParents(deriv,
                                  sig,
                                  parent_clauses,
                                  parent_formulas);
   }

   //parent_count = PStackGetSP(parent_clauses)+
   //PStackGetSP(parent_formulas);

   fprintf(out, "  %ld [shape=%s%s,style=filled,label=\"",
           id, shape, DerivedDotNodeColour(derived));

   if(derived->clause)
   {
      if(print_derivation > POGraph1)
      {
         ClauseTSTPPrint(out, derived->clause, true, false);
      }
      else
      {
         fprintf(out, "c%ld", derived->clause->ident);
      }
   }
   else
   {
      assert(derived->formula);
      if(print_derivation > POGraph1)
      {
         WFormulaTSTPPrint(out, derived->formula, true, false);
      }
      else
      {
         fprintf(out, "%s", WFormulaGetId(derived->formula));
      }
   }
   if(print_derivation >= POGraph2)
   {
      if(deriv)
      {
         fprintf(out, ",\\n");
         DerivationStackTSTPPrint(out, sig, deriv);
      }
      else if(info)
      {
         fprintf(out, ",\\n");
         ClauseSourceInfoPrintTSTP(out, info);
      }
      if(print_derivation >= POGraph1)
      {
         fprintf(out, ").");
      }
   }
   fprintf(out, "\"]\n");

   while(!PStackEmpty(parent_clauses))
   {
      cparent = PStackPopP(parent_clauses);
      fprintf(out, "    %ld -> %ld [style=\"bold\"%s]\n", cparent->ident, id,
              DerivedDotClauseLinkColour(derived, cparent));
   }
   while(!PStackEmpty(parent_formulas))
   {
      fparent = PStackPopP(parent_formulas);
      fprintf(out, "    %ld -> %ld [style=\"bold\"%s]\n", fparent->ident, id,
              DerivedDotFormulaLinkColour(derived, fparent));
   }
   PStackFree(parent_clauses);
   PStackFree(parent_formulas);
}



/*-----------------------------------------------------------------------
//
// Function: DerivedIsEvalGC()
//
//   Return true if the step corresponds to the evaluated and selected
//   form of a given clause.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool DerivedIsEvalGC(Derived_p derived)
{
   if(derived->clause)
   {
      return ClauseIsEvalGC(derived->clause);
   }
   return false;
}

/*-----------------------------------------------------------------------
//
// Function: DerivStackIndicatesInitialClause()
//
//   Return true if the derivation stack is empty, or if all parents
//   are formulas (not clauses). This is ugly - it cannot reuse
//   DerivStackExtractParents() since the the signature is not known.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool DerivStackIndicatesInitialClause(PStack_p deriv)
{
   PStackPointer i, sp;
   DerivationCode op;

   if(!deriv)
   {
      return true;
   }

   if(deriv)
   {
      sp = PStackGetSP(deriv);
      i  = 0;

      while(i<sp)
      {
         op = PStackElementInt(deriv, i);
         i++;
         if(DCOpHasCnfArg1(op))
         {
            return false;
         }
         else if(DCOpHasArg1(op))
         {
            i++;
         }
         if(DCOpHasCnfArg2(op))
         {
            return false;
         }
         else if(DCOpHasArg2(op))
         {
            i++;
         }
         if(op==DCACRes)
         {
            /* AC parents are always clauses */
            return false;
         }
      }
   }
   return true;
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

   handle->sig            = sig;
   handle->ordered        = false;
   handle->has_conjecture = false;
   handle->deriv          = NULL;
   handle->roots          = PStackAlloc();
   handle->ordered_deriv  = PStackAlloc();

   handle->clause_step_count        = 0;
   handle->formula_step_count       = 0;
   handle->initial_clause_count     = 0;
   handle->initial_formula_count    = 0;
   handle->clause_conjecture_count  = 0;
   handle->formula_conjecture_count = 0;
   handle->generating_inf_count     = 0;
   handle->simplifying_inf_count    = 0;

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
      DerivedSetInProof(node,DerivedInProof(node));
   }

   while(!PStackEmpty(stack))
   {
      node = PStackPopP(stack);

      deriv = derived_get_derivation(node);

      assert(PStackEmpty(parent_clauses));
      assert(PStackEmpty(parent_formulas));
      DerivStackExtractOptParents(deriv,
                                  derivation->sig,
                                  parent_clauses,
                                  parent_formulas);
      while(!PStackEmpty(parent_clauses))
      {
         clause = PStackPopP(parent_clauses);
         newnode = DerivationGetDerived(derivation, clause, NULL);
         assert(newnode);
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
         assert(newnode);
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
// Function: DerivationMarkProofSteps()
//
//   Go through the derivation, marking all proof steps. Assumes that
//   derivation->roots provides (direct or indirect) access to all
//   proof steps. Sets derivation->has_conjecture if a conjecture-type
//   clause or formula is in the proof tree.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

long DerivationMarkProofSteps(Derivation_p derivation)
{
   long          proof_steps = 0;
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
      DerivedSetInProof(node, DerivedInProof(node));
   }

   while(!PStackEmpty(stack))
   {
      node = PStackPopP(stack);

      deriv = derived_get_derivation(node);
      if(DerivedInProof(node))
      {
         proof_steps++;
         assert(PStackEmpty(parent_clauses));
         assert(PStackEmpty(parent_formulas));
         DerivStackExtractOptParents(deriv,
                                     derivation->sig,
                                     parent_clauses,
                                     parent_formulas);
         while(!PStackEmpty(parent_clauses))
         {
            clause = PStackPopP(parent_clauses);
            if(ClauseIsConjecture(clause))
            {
               derivation->has_conjecture = true;
            }
            newnode = DerivationGetDerived(derivation, clause, NULL);
            if(!DerivedInProof(newnode))
            {
               DerivedSetInProof(newnode,true);
               PStackPushP(stack, newnode);
            }
         }
         while(!PStackEmpty(parent_formulas))
         {
            form = PStackPopP(parent_formulas);
            if(FormulaIsConjecture(form))
            {
               derivation->has_conjecture = true;
            }
            newnode = DerivationGetDerived(derivation, NULL, form);
            if(!DerivedInProof(newnode))
            {
               DerivedSetInProof(newnode,true);
               PStackPushP(stack, newnode);
            }
         }
      }
   }
   PStackFree(parent_clauses);
   PStackFree(parent_formulas);
   PStackFree(stack);


   return proof_steps;
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

   idents = derivation_find_max_id(derivation)+1;
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
         //printf("Renumbering step %ld",  node->formula->ident);
         node->formula->ident = idents++;
         //printf(" to %ld\n",  node->formula->ident);
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
      clause = ClauseDerivFindFirst(clause);
      node = DerivationGetDerived(res, clause, NULL);
      node->is_root = true;
      PStackPushP(res->roots, node);
   }
   DerivationExtract(res, root_clauses);
   DerivationMarkProofSteps(res);
   DerivationTopoSort(res);
   DerivationRenumber(res);

   return res;
}




/*-----------------------------------------------------------------------
//
// Function: DerivationAnalyse()
//
//   Compute a number of statistics for a derivation.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void DerivationAnalyse(Derivation_p derivation)
{
   PStackPointer sp;
   Derived_p     handle;

   if(!derivation->ordered)
   {
      DerivationTopoSort(derivation);
   }
   for(sp=0; sp<PStackGetSP(derivation->ordered_deriv); sp++)
   {
      handle = PStackElementP(derivation->ordered_deriv, sp);
      if(handle->clause)
      {
         derivation->clause_step_count++;
         if(ClauseIsConjecture(handle->clause))
         {
            derivation->clause_conjecture_count++;
         }
         if(DerivStackIndicatesInitialClause(handle->clause->derivation))
         {
            derivation->initial_clause_count++;
         }
         DerivStackCountSearchInferences(handle->clause->derivation,
                                         &(derivation->generating_inf_count),
                                         &(derivation->simplifying_inf_count));
      }
      else
      {
         derivation->formula_step_count++;
         if(FormulaIsConjecture(handle->formula))
         {
            derivation->formula_conjecture_count++;
         }
         if(!handle->formula->derivation)
         {
            derivation->initial_formula_count++;
         }
      }
   }
}

/*-----------------------------------------------------------------------
//
// Function: DerivationCollectFCodes()
//
//   Collect all f_codes from derivation into tree.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long DerivationCollectFCodes(Derivation_p derivation, NumTree_p *tree)
{
   PStackPointer sp;
   Derived_p     handle;
   long          res = 0;

   if(!derivation->ordered)
   {
      DerivationTopoSort(derivation);
   }
   for(sp=0; sp<PStackGetSP(derivation->ordered_deriv); sp++)
   {
      handle = PStackElementP(derivation->ordered_deriv, sp);
      res += DerivedCollectFCodes(handle, tree);
   }
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
            fprintf(out, COMCHAR" Output format not implemented.");
            break;
      }
      fprintf(out, "\n");
   }
}



/*-----------------------------------------------------------------------
//
// Function: DerivationDotPrint()
//
//   Print a derivation as a DOT graph.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void DerivationDotPrint(FILE* out, Derivation_p derivation,
                        ProofOutput print_derivation)
{
   PStackPointer sp;
   Derived_p     node;
   bool          axiom_open = true;

   assert(derivation->ordered);

   fprintf(out,
           "digraph proof{\n"
           "  rankdir=TB\n"
           "  graph [splines=true overlap=false];\n"
           "  subgraph ax{\n"
           "  rank=\"same\";\n"
      );

   for(sp=PStackGetSP(derivation->ordered_deriv)-1; sp>=0; sp--)
   {
      node = PStackElementP(derivation->ordered_deriv, sp);
      if(axiom_open && DerivedGetDerivstack(node))
      { /* Axioms come first, and this is not one anymore */
         fprintf(out, "   }\n");
         axiom_open = false;
      }
      DerivedDotPrint(out, derivation->sig, node, print_derivation);
   }
   fprintf(out, "}\n");
}



/*-----------------------------------------------------------------------
//
// Function: DerivationPrintConditional()
//
//   Print a derivation and its statistics, based on the selected
//   inputs.
//
// Global Variables: -
//
// Side Effects    : Output, memory operations
//
/----------------------------------------------------------------------*/

void DerivationPrintConditional(FILE* out, char* status, Derivation_p derivation,
                                Sig_p sig, ProofOutput print_derivation,
                                bool print_analysis)
{
   if(print_derivation == POList)
   {
      fprintf(out, COMCHAR" SZS output start %s\n", status);
      if(sig->typed_symbols)
      {
         NumTree_p symbols = NULL;
         PTree_p types = NULL;

         DerivationCollectFCodes(derivation, &symbols);
         SigFCodesCollectTypes(sig, symbols, &types);
         TypeBankPrintSelectedSortDefs(out, sig->type_bank, types);
         SigPrintTypeDeclsTSTPSelective(out, sig, &symbols);
         NumTreeFree(symbols);
         PTreeFree(types);
      }
      DerivationPrint(GlobalOut, derivation);
      fprintf(out, COMCHAR" SZS output end %s\n", status);
   }
   else if(print_derivation >= POGraph1)
   {
      DerivationDotPrint(GlobalOut, derivation, print_derivation);
   }
   DerivationAnalyse(derivation);
   if(print_analysis)
   {
      fprintf(GlobalOut, COMCHAR" Proof object total steps             : %lu\n",
              derivation->clause_step_count+derivation->formula_step_count);
      fprintf(GlobalOut, COMCHAR" Proof object clause steps            : %lu\n",
              derivation->clause_step_count);
      fprintf(GlobalOut, COMCHAR" Proof object formula steps           : %lu\n",
              derivation->formula_step_count);
      fprintf(GlobalOut, COMCHAR" Proof object conjectures             : %lu\n",
              derivation->clause_conjecture_count+derivation->formula_conjecture_count);
      fprintf(GlobalOut, COMCHAR" Proof object clause conjectures      : %lu\n",
              derivation->clause_conjecture_count);
      fprintf(GlobalOut, COMCHAR" Proof object formula conjectures     : %lu\n",
              derivation->formula_conjecture_count);
      fprintf(GlobalOut, COMCHAR" Proof object initial clauses used    : %lu\n",
              derivation->initial_clause_count);
      fprintf(GlobalOut, COMCHAR" Proof object initial formulas used   : %lu\n",
              derivation->initial_formula_count);
      fprintf(GlobalOut, COMCHAR" Proof object generating inferences   : %lu\n",
              derivation->generating_inf_count);
      fprintf(GlobalOut, COMCHAR" Proof object simplifying inferences  : %lu\n",
              derivation->simplifying_inf_count);
   }
}


/*-----------------------------------------------------------------------
//
// Function: DerivationComputeAndPrint()
//
//   Compute, print, and discard a derivation.
//
// Global Variables: -
//
// Side Effects    : Output, memory operations
//
/----------------------------------------------------------------------*/

void DerivationComputeAndPrint(FILE* out, char* status, PStack_p root_clauses,
                               Sig_p sig, ProofOutput print_derivation,
                               bool print_analysis)
{
   assert(sizeof(opids) / sizeof(char*) == sizeof(optheory) / sizeof(char*));
   assert(sizeof(opids) / sizeof(char*) == sizeof(opstatus) / sizeof(char*));

   Derivation_p derivation = DerivationCompute(root_clauses, sig);

   DerivationPrintConditional(out, status, derivation, sig,
                              print_derivation, print_analysis);

   DerivationFree(derivation);
}




/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
