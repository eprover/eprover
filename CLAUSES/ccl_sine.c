/*-----------------------------------------------------------------------

File  : ccl_sine.c

Author: Stephan Schulz (schulz@eprover.org)

Contents
 
  Implementation of generalized SinE axiom selection.

  Copyright 2010 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Fri Jul  2 01:15:26 CEST 2010
    New

-----------------------------------------------------------------------*/

#include "ccl_sine.h"



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
// Function: DRelAlloc()
//
//   Allocate an initialized DRelCell for f_code.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

DRel_p DRelAlloc(FunCode f_code)
{
   DRel_p handle = DRelCellAlloc();

   handle->f_code     = f_code;
   handle->activated  = false;
   handle->d_clauses  = PStackAlloc();
   handle->d_formulas = PStackAlloc();

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: DRelFree()
//
//   Free a DRel-Cell. Clauses and Formulas are external!
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void DRelFree(DRel_p rel)
{
   PStackFree(rel->d_clauses);
   PStackFree(rel->d_formulas);
   DRelCellFree(rel);
}



/*-----------------------------------------------------------------------
//
// Function: DRelationAlloc()
//
//   Allocate a complete DRelation.
//
// Global Variables: 
//
// Side Effects    : 
//
/----------------------------------------------------------------------*/

DRelation_p DRelationAlloc(void)
{
   DRelation_p handle = DRelationCellAlloc();

   handle->relation = PDArrayAlloc(10, 0);

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: DRelationFree()
//
//   Free a DRelation.
//
// Global Variables: 
//
// Side Effects    : 
//
/----------------------------------------------------------------------*/

void DRelationFree(DRelation_p rel)
{
   long i;

   for(i=1; i<rel->relation->size; i++)
   {
      if(PDArrayElementP(rel->relation, i))
      {
         DRelFree(PDArrayElementP(rel->relation, i));
      }
   }
   PDArrayFree(rel->relation);
   DRelationCellFree(rel);
}

/*-----------------------------------------------------------------------
//
// Function: DRelationGetFEntry()
//
//   Return the entry for the DRel for f_code. Create one if it does
//   not exist.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

DRel_p DRelationGetFEntry(DRelation_p rel, FunCode f_code)
{
   DRel_p res = PDArrayElementP(rel->relation, f_code);
   if(!res)
   {
      res = DRelAlloc(f_code);
      PDArrayAssignP(rel->relation, f_code, res);
   }
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: DRelationAddClause()
//
//   Add a clause to the D-Relation.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void DRelationAddClause(DRelation_p drel,
                        GenDistrib_p generality, 
                        GeneralityMeasure gentype,
                        double benevolence,
                        Clause_p clause)
{
   PStack_p symbols = PStackAlloc();
   FunCode  symbol;
   DRel_p   rel;

   ClauseComputeDRel(generality, 
                     gentype,
                     benevolence,
                     clause, 
                     symbols);
   while(!PStackEmpty(symbols))
   {
      symbol = PStackPopInt(symbols);
      rel = DRelationGetFEntry(drel, symbol);
      PStackPushP(rel->d_clauses, clause);
   }
   
   PStackFree(symbols);
}


/*-----------------------------------------------------------------------
//
// Function: DRelationAddFormula()
//
//   Add a forrmula to the D-Relation
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void DRelationAddFormula(DRelation_p drel,
                         GenDistrib_p generality, 
                         GeneralityMeasure gentype,
                         double benevolence,
                         WFormula_p form)
{
   PStack_p symbols = PStackAlloc();
   FunCode  symbol;
   DRel_p   rel;

   FormulaComputeDRel(generality, 
                      gentype,
                      benevolence,
                      form, 
                     symbols);
   while(!PStackEmpty(symbols))
   {
      symbol = PStackPopInt(symbols);
      rel = DRelationGetFEntry(drel, symbol);
      PStackPushP(rel->d_formulas, form);
   }
   PStackFree(symbols);
}


/*-----------------------------------------------------------------------
//
// Function: DRelationAddClauseSet()
//
//   Add all clauses in set to the D-Relation.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void DRelationAddClauseSet(DRelation_p drel,
                           GenDistrib_p generality, 
                           GeneralityMeasure gentype,
                           double benevolence,
                           ClauseSet_p set)
{
   Clause_p handle;
   
   for(handle = set->anchor->succ; 
       handle != set->anchor;
       handle = handle->succ)
   {
      DRelationAddClause(drel,
                         generality, 
                         gentype,
                         benevolence,
                         handle);
   } 
}


/*-----------------------------------------------------------------------
//
// Function: DRelationAddFormulaSet()
//
//   Add all formulas in set to the D-Relation.
//
// Global Variables: 
//
// Side Effects    : 
//
/----------------------------------------------------------------------*/

void DRelationAddFormulaSet(DRelation_p drel,
                            GenDistrib_p generality, 
                            GeneralityMeasure gentype,
                            double benevolence,
                            FormulaSet_p set)
{
   WFormula_p handle;
   
   for(handle = set->anchor->succ; 
       handle != set->anchor;
       handle = handle->succ)
   {
      DRelationAddFormula(drel,
                         generality, 
                         gentype,
                         benevolence,
                         handle);
   }    
}


/*-----------------------------------------------------------------------
//
// Function: DRelationAddClauseSets()
//
//   Add all clauses in sets on stack into the D-Relation.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void DRelationAddClauseSets(DRelation_p drel,
                            GenDistrib_p generality, 
                            GeneralityMeasure gentype,
                            double benevolence,
                            PStack_p sets)
{
   PStackPointer i;

   for(i=0; i<PStackGetSP(sets); i++)
   {
      DRelationAddClauseSet(drel,
                            generality, 
                            gentype,
                            benevolence,
                            PStackElementP(sets, i));
   }
}

/*-----------------------------------------------------------------------
//
// Function: DRelationAddFormulaSets()
//
//    Add all formulas in sets on stack into the D-Relation.
//
// Global Variables: 
//
// Side Effects    : 
//
/----------------------------------------------------------------------*/

void DRelationAddFormulaSets(DRelation_p drel,
                             GenDistrib_p generality, 
                             GeneralityMeasure gentype,
                             double benevolence,
                             PStack_p sets)
{
   PStackPointer i;
   
   for(i=0; i<PStackGetSP(sets); i++)
   {
      DRelationAddFormulaSet(drel,
                            generality, 
                            gentype,
                            benevolence,
                            PStackElementP(sets, i));
   }
}  


/*-----------------------------------------------------------------------
//
// Function: PQueueStoreClause()
//
//   Store the tuple (type, clause) in axioms.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void PQueueStoreClause(PQueue_p axioms, Clause_p clause)
{
   PQueueStoreInt(axioms, ATClause);
   PQueueStoreP(axioms, clause); 
}


/*-----------------------------------------------------------------------
//
// Function: PQueueStoreFormula()
//
//   Store the tuple (type, form) in axioms.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void PQueueStoreFormula(PQueue_p axioms, WFormula_p form)
{
   PQueueStoreInt(axioms, ATFormula);
   PQueueStoreP(axioms, form); 
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetFindHypotheses()
//
//   Find all hypotheses in set and store them in res. Returns number
//   of hypotheses found.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ClauseSetFindHypotheses(ClauseSet_p set, PQueue_p res)
{
   long ret = 0;
   Clause_p handle;
   

   for(handle = set->anchor->succ; 
       handle != set->anchor;
       handle = handle->succ)
   {
      if(ClauseIsConjecture(handle))
      {
         PQueueStoreClause(res, handle);
         ret++;
      }
   } 
   return ret;
}

/*-----------------------------------------------------------------------
//
// Function: FormulaSetFindHypotheses()
//
//   Find all hypotheses in set and store them in res. Returns number
//   of hypotheses found.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long FormulaSetFindHypotheses(FormulaSet_p set, PQueue_p res)
{
   long ret = 0;
   WFormula_p handle;
   

   for(handle = set->anchor->succ; 
       handle != set->anchor;
       handle = handle->succ)
   {
      if(FormulaIsConjecture(handle))
      {
         PQueueStoreFormula(res, handle);
         ret++;
      }
   } 
   return ret;
}

/*-----------------------------------------------------------------------
//
// Function: SelectDefiningAxioms()
//
//   Perform SinE-like axiom selection. All initially selected
//   "axioms" (typically the hypothesis) have to be in axioms, in the
//   form of (type, pointer) values. Returns the number of axioms
//   selected. 
//
// Global Variables: -
//
// Side Effects    : Changes activation bits in drel and the axioms.
//
/----------------------------------------------------------------------*/

long SelectDefiningAxioms(DRelation_p drel, 
                          Sig_p sig,
                          PQueue_p axioms,
                          PStack_p res_clauses, 
                          PStack_p res_formulas)
{
   AxiomType  type;
   WFormula_p form;
   Clause_p   clause;
   long       *dist_array = SizeMalloc((sig->f_count+1)*sizeof(long));
   long       res = 0;
   DRel_p     frel;
   FunCode    i;
   PStackPointer sp;

   while(!PQueueEmpty(axioms))
   {
      res++;
      memset(dist_array, 0, (sig->f_count+1)*sizeof(long));

      type = PQueueGetNextInt(axioms);
      switch(type)
      {
      case ATClause:
            clause = PQueueGetNextP(axioms);
            if(ClauseQueryProp(clause, CPIsRelevant))
            {
               continue;
            }
            ClauseSetProp(clause, CPIsRelevant);
            PStackPushP(res_clauses, clause);
            ClauseAddSymbolDistribution(clause, dist_array);
            break;
      case ATFormula:
            form = PQueueGetNextP(axioms);
            if(FormulaQueryProp(form, WPIsRelevant))
            {
               continue;
            }
            FormulaSetProp(form, WPIsRelevant);
            PStackPushP(res_formulas, form);
            TermAddSymbolDistribution(form->tformula, dist_array);
            break;
      default:
            assert(false && "Unknown axiom type!");
            break;
      }
      for(i=sig->internal_symbols+1; i<=sig->f_count; i++)
      {
         if(dist_array[i] && 
            (frel = PDArrayElementP(drel->relation, i)) &&
            !frel->activated)
         {
            frel->activated = true;
            for(sp=0; sp<PStackGetSP(frel->d_clauses); sp++)
            {
               clause = PStackElementP(frel->d_formulas, sp);
               PQueueStoreClause(axioms, clause);
            }
            for(sp=0; sp<PStackGetSP(frel->d_formulas); sp++)
            {
               form = PStackElementP(frel->d_formulas, sp);
               PQueueStoreFormula(axioms, form);
            }
         }
      }
   }
   SizeFree(dist_array, (sig->f_count+1)*sizeof(long));
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: SelectAxioms()
//
//   Given a function symbol distribution, input sets (clauses and
//   formulas) which contain the hypotheses (in a restricted part
//   indicated by hyp_start), select axioms according to the
//   D-Relation described by gen_measure and benevolence. Selected
//   axioms are pushed tono res_clauses and res_formulas, the total
//   number of selected axioms is returned.
//
// Global Variables: -
//
// Side Effects    : Many, none expected permanent.
//
/----------------------------------------------------------------------*/

long SelectAxioms(GenDistrib_p      f_distrib,
                  PStack_p          clause_sets,
                  PStack_p          formula_sets,
                  PStackPointer     hyp_start,
                  GeneralityMeasure gen_measure,
                  double            benevolence,
                  PStack_p          res_clauses, 
                  PStack_p          res_formulas)
{
   long          res = 0;
   long          hypos = 0;
   DRelation_p   drel = DRelationAlloc();
   PQueue_p      selq = PQueueAlloc();
   PStackPointer i;

   assert(PStackGetSP(clause_sets)==PStackGetSP(formula_sets));

   DRelationAddClauseSets(drel, f_distrib, gen_measure, 
                          benevolence, clause_sets);
   DRelationAddFormulaSets(drel, f_distrib, gen_measure, 
                           benevolence, formula_sets);
 
   for(i=hyp_start; i<PStackGetSP(clause_sets); i++)
   {
      hypos += ClauseSetFindHypotheses(PStackElementP(clause_sets, i),
                                       selq);
      hypos += FormulaSetFindHypotheses(PStackElementP(formula_sets, i),
                                        selq);
   }
   VERBOSE(fprintf(stderr, "# Found %ld hypotheses\n", hypos););
   if(!hypos)
   {
      /* Use all clauses/formulas -> later */
   }
   res = SelectDefiningAxioms(drel,
                              f_distrib->sig,
                              selq,
                              res_clauses,
                              res_formulas);

   PQueueFree(selq);
   DRelationFree(drel);
 
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: DRelPrintDebug()
//
//   Print a hint about clauses and formulas in D-Drelation with a
//   given f_code.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void DRelPrintDebug(FILE* out, DRel_p rel, Sig_p sig)
{
   fprintf(out, "# %6ld %-15s: %6d clauses, %6d formulas\n",
           rel->f_code,
           SigFindName(sig, rel->f_code),
           PStackGetSP(rel->d_clauses),
           PStackGetSP(rel->d_formulas));
}



/*-----------------------------------------------------------------------
//
// Function: DRelationPrintDebug()
//
//   Print a hint of the D-Relation to see what's going on.
//
// Global Variables: -
//
// Side Effects    : Input
//
/----------------------------------------------------------------------*/

void DRelationPrintDebug(FILE* out, DRelation_p rel, Sig_p sig)
{
   long i;
   
   for(i=1; i<rel->relation->size; i++)
   {
      if(PDArrayElementP(rel->relation, i))
      {
         DRelPrintDebug(out, PDArrayElementP(rel->relation, i), sig);
      }
   }
}



/*-----------------------------------------------------------------------
//
// Function: PStackClauseDelProp()
//
//   Delete prop in all clauses on stack.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void PStackClauseDelProp(PStack_p stack, ClauseProperties prop)
{
   PStackPointer i;
   Clause_p clause;
   
   for(i=0; i<PStackGetSP(stack); i++)
   {
      clause = PStackElementP(stack, i);
      ClauseDelProp(clause, prop);
   }
}



/*-----------------------------------------------------------------------
//
// Function: PStackFormulaeDelProp()
//
//   Delete prop in all formulas on stack.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void PStackFormulaDelProp(PStack_p stack, WFormulaProperties prop)
{
   PStackPointer i;
   WFormula_p form;
   
   for(i=0; i<PStackGetSP(stack); i++)
   {
      form = PStackElementP(stack, i);
      FormulaDelProp(form, prop);
   }
}


/*-----------------------------------------------------------------------
//
// Function: PStackClausePrintTSTP()
//
//   Print the clauses on the stack in TSTP format.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void PStackClausePrintTSTP(FILE* out, PStack_p stack)
{
   PStackPointer i;
   Clause_p clause;
   
   for(i=0; i<PStackGetSP(stack); i++)
   {
      clause = PStackElementP(stack, i);
      ClauseTSTPPrint(out, clause, true, true);
      fputc('\n', out);
   }
}


/*-----------------------------------------------------------------------
//
// Function: PStackFormulaPrintTSTP()
//
//   Print all the formulas on the stack.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void PStackFormulaPrintTSTP(FILE* out, PStack_p stack)
{
   PStackPointer i;
   WFormula_p form;
   
   for(i=0; i<PStackGetSP(stack); i++)
   {
      form = PStackElementP(stack, i);
      WFormulaTSTPPrint(out, form, true, true);
      fputc('\n', out);
   }
}




/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


