/*-----------------------------------------------------------------------

File  : ccl_sine.c

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Implementation of generalized SinE axiom selection.

  Copyright 2010 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
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

   for(i=0; i<rel->relation->size; i++)
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
                        long generosity,
                        Clause_p clause)
{
   PStack_p symbols = PStackAlloc();
   FunCode  symbol;
   DRel_p   rel;

   ClauseComputeDRel(generality,
                     gentype,
                     benevolence,
                     generosity,
                     clause,
                     symbols);
   if(PStackEmpty(symbols))
   {
      rel = DRelationGetFEntry(drel, 0);
      PStackPushP(rel->d_clauses, clause);
   }
   else
   {
      while(!PStackEmpty(symbols))
      {
         symbol = PStackPopInt(symbols);
         rel = DRelationGetFEntry(drel, symbol);
         PStackPushP(rel->d_clauses, clause);
      }
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
                         long generosity,
                         bool trim_impl,
                         bool force_definition,
                         WFormula_p form)
{
   PStack_p symbols = PStackAlloc();
   FunCode  symbol;
   DRel_p   rel;

   FormulaComputeDRel(generality,
                      gentype,
                      benevolence,
                      generosity,
                      form,
                      symbols,
                      trim_impl);
   if(force_definition)
   {
      FunCode l_fc = WFormulaGetLambdaDefinedSym(form);
      if(l_fc != -1 && !PStackFindInt(symbols, l_fc))
      {
         PStackPushInt(symbols, l_fc);
      }
   }
   if(PStackEmpty(symbols))
   {
      rel = DRelationGetFEntry(drel, 0);
      PStackPushP(rel->d_formulas, form);
   }
   else
   {
      while(!PStackEmpty(symbols))
      {
         symbol = PStackPopInt(symbols);
         rel = DRelationGetFEntry(drel, symbol);
         PStackPushP(rel->d_formulas, form);
      }
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
                           long generosity,
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
                         generosity,
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
                            long generosity,
                            bool trim_impl,
                            bool force_def,
                            FormulaSet_p set)
{
   WFormula_p handle;

   //printf(COMCHAR" Adding formula set with %ld formulas (fd:%d)\n", FormulaSetCardinality(set),force_def);

   for(handle = set->anchor->succ;
       handle != set->anchor;
       handle = handle->succ)
   {
      DRelationAddFormula(drel,
                          generality,
                          gentype,
                          benevolence,
                          generosity,
                          trim_impl,
                          force_def,
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
                           long generosity,
                            PStack_p sets)
{
   PStackPointer i;

   for(i=0; i<PStackGetSP(sets); i++)
   {
      DRelationAddClauseSet(drel,
                            generality,
                            gentype,
                            benevolence,
                            generosity,
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
                             long generosity,
                             bool trim_impl,
                             bool force_def,
                             PStack_p sets)
{
   PStackPointer i;

   for(i=0; i<PStackGetSP(sets); i++)
   {
      DRelationAddFormulaSet(drel,
                             generality,
                             gentype,
                             benevolence,
                             generosity,
                             trim_impl,
                             force_def,
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
// Function: ClauseSetFindAxSelectionSeeds()
//
//   Find all conjectures and optionally hypotheses in set and store
//   them in res. Returns number of seeds found.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ClauseSetFindAxSelectionSeeds(ClauseSet_p set, PQueue_p res, bool inc_hypos)
{
   long ret = 0;
   Clause_p handle;


   for(handle = set->anchor->succ;
       handle != set->anchor;
       handle = handle->succ)
   {
      if(ClauseIsConjecture(handle)||
         (inc_hypos && ClauseIsHypothesis(handle)))
      {
         PQueueStoreClause(res, handle);
         ret++;
      }
   }
   return ret;
}

/*-----------------------------------------------------------------------
//
// Function: FormulaSetFindAxSelectionSeeds()
//
//   Find all axiom selection seeds (conjecures and optionally
//   hypotheses) in set and store them in res. Returns number
//   of seeds found.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long FormulaSetFindAxSelectionSeeds(FormulaSet_p set, PQueue_p res, bool inc_hypos)
{
   long ret = 0;
   WFormula_p handle;


   for(handle = set->anchor->succ;
       handle != set->anchor;
       handle = handle->succ)
   {
      if(FormulaIsConjecture(handle)||
         (inc_hypos && FormulaIsHypothesis(handle)))
      {
         // printf("Found and added: "); WFormulaPrint(GlobalOut, handle, true); printf("\n");
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
//   "axioms" (typically the conjectures/hypotheses) have to be in
//   axioms, in the form of (type, pointer) values. Returns the number
//   of axioms selected.
//
// Global Variables: -
//
// Side Effects    : Changes activation bits in drel and the axioms.
//
/----------------------------------------------------------------------*/

long SelectDefiningAxioms(DRelation_p drel,
                          Sig_p sig,
                          int max_recursion_depth,
                          long max_set_size,
                          bool trim_implications,
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
   PStackPointer sp, ssp;
   PStack_p   symbol_stack = PStackAlloc();
   int        recursion_level = 0;

   memset(dist_array, 0, (sig->f_count+1)*sizeof(long));
   PQueueStoreInt(axioms, ATNoType);

   //printf("Drel has %ld elements\n", PDArrayMembers(drel->relation));
   //DRelationPrintDebug(stdout, drel, sig);

   while(!PQueueEmpty(axioms))
   {
      //printf("Selecting %ld from %ld at %d\n",
      //res,
      //PQueueCardinality(axioms),
      //recursion_level);

      if((res > max_set_size) ||
         (recursion_level > max_recursion_depth))
      {
         break;
      }

      type = PQueueGetNextInt(axioms);
      switch(type)
      {
      case ATNoType:
            recursion_level++;
            if(!PQueueEmpty(axioms))
            {
               PQueueStoreInt(axioms, ATNoType);
            }
            continue;
      case ATClause:
            clause = PQueueGetNextP(axioms);
            if(ClauseQueryProp(clause, CPIsRelevant))
            {
               continue;
            }
            ClauseSetProp(clause, CPIsRelevant);
            PStackPushP(res_clauses, clause);
            ClauseAddSymbolDistExist(clause, dist_array, symbol_stack);
            res++;
            break;
      case ATFormula:
            form = PQueueGetNextP(axioms);
            if(FormulaQueryProp(form, CPIsRelevant))
            {
               continue;
            }
            FormulaSetProp(form, CPIsRelevant);
            PStackPushP(res_formulas, form);
            // fprintf(stderr, "activating form: %s\n", WFormulaGetId(form));
            TermAddSymbolDistExist((FormulaIsConjecture(form) && trim_implications) ?
                                   TermTrimImplications(sig, form->tformula) :
                                   form->tformula,
                                   dist_array, symbol_stack);
            res++;
            break;
      default:
            assert(false && "Unknown axiom type!");
            break;
      }
      //printf(COMCHAR" SymbolstackSP: %ld\n", PStackGetSP(symbol_stack));
      //printf(COMCHAR" DRel total entries: %ld\n", DRelationTotalEntries(drel));
      for(ssp=0; ssp<PStackGetSP(symbol_stack); ssp++)
      {
         i = PStackElementInt(symbol_stack, ssp);
         if((i > sig->internal_symbols) &&
            (frel = PDArrayElementP(drel->relation, i)) &&
            !frel->activated)
         {
            //printf("!frel->activated [%ld]\n", i);
            frel->activated = true;
            //fprintf(stdout, " > |%s|: ", SigFindName(sig, i));
            for(sp=0; sp<PStackGetSP(frel->d_clauses); sp++)
            {
               clause = PStackElementP(frel->d_clauses, sp);
               PQueueStoreClause(axioms, clause);
            }
            for(sp=0; sp<PStackGetSP(frel->d_formulas); sp++)
            {
               form = PStackElementP(frel->d_formulas, sp);
               //fprintf(stdout, "%s; ", WFormulaGetId(form));
               PQueueStoreFormula(axioms, form);
            }
            //fprintf(stdout, "\n");
         }
         else
         {
            //printf("frel->activated [%ld]\n", i);
         }
         dist_array[i] = 0;
      }
      PStackReset(symbol_stack);
   }
   SizeFree(dist_array, (sig->f_count+1)*sizeof(long));
   PStackFree(symbol_stack);
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
//   axioms are pushed onto res_clauses and res_formulas, the total
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
                  PStackPointer     seed_start,
                  AxFilter_p        ax_filter,
                  PStack_p          res_clauses,
                  PStack_p          res_formulas)
{
   long          res   = 0;
   long          seeds = 0;
   DRelation_p   drel  = DRelationAlloc();
   PQueue_p      selq  = PQueueAlloc();
   PStackPointer i;
   long          ax_cardinality, max_result_size;

   assert(PStackGetSP(clause_sets)==PStackGetSP(formula_sets));

   //fprintf(GlobalOut, COMCHAR" Axiom selection starts (%ld, %lld)\n",
   //seed_start, GetSecTimeMod());

   //GenDistribPrint(stdout, f_distrib, 10);
   DRelationAddClauseSets(drel, f_distrib,
                          ax_filter->gen_measure,
                          ax_filter->benevolence,
                          ax_filter->generosity,
                          clause_sets);
   DRelationAddFormulaSets(drel, f_distrib,
                           ax_filter->gen_measure,
                           ax_filter->benevolence,
                           ax_filter->generosity,
                           ax_filter->trim_implications,
                           ax_filter->defined_symbols_in_drel,
                           formula_sets);
   /* fprintf(GlobalOut, COMCHAR" DRelation constructed (%lld)\n",
    * GetSecTimeMod()); */
   // DRelationPrintDebug(stderr, drel, f_distrib->sig);
   for(i=seed_start; i<PStackGetSP(clause_sets); i++)
   {
      seeds += ClauseSetFindAxSelectionSeeds(PStackElementP(clause_sets, i),
                                             selq,
                                             ax_filter->use_hypotheses);
      seeds += FormulaSetFindAxSelectionSeeds(PStackElementP(formula_sets, i),
                                              selq,
                                              ax_filter->use_hypotheses);
   }
   //fprintf(GlobalOut, COMCHAR" Hypotheses found %ld (%lld)\n", seeds,
   //GetSecTimeMod());
   VERBOSE(fprintf(stderr, COMCHAR" Found %ld seed clauses/formulas\n", seeds););
   if(!seeds)
   {
      /* No goals-> the empty set contains all relevant clauses */
   }
   else
   {
      ax_cardinality =
         FormulaSetStackCardinality(formula_sets)+
         ClauseSetStackCardinality(clause_sets);
      max_result_size = ax_filter->max_set_fraction*ax_cardinality;
      if(ax_filter->max_set_size < max_result_size)
      {
         max_result_size = ax_filter->max_set_size;
      }
      //printf(COMCHAR" max_result_size = %ld\n", max_result_size);
      if(true)
         /* "true" may be exported as an option eventually */
      {
         DRel_p no_symbol_axioms = PDArrayElementP(drel->relation, 0);
         if(no_symbol_axioms && ax_filter->add_no_symbol_axioms)
         {
            PStackPushStack(res_clauses,  no_symbol_axioms->d_clauses);
            PStackPushStack(res_formulas, no_symbol_axioms->d_formulas);
         }
         res = PStackGetSP(res_clauses)+PStackGetSP(res_formulas);
      }
      //GenDistribPrint(stdout, f_distrib, 10);
      res += SelectDefiningAxioms(drel,
                                 f_distrib->sig,
                                 ax_filter->max_recursion_depth,
                                 max_result_size,
                                 ax_filter->trim_implications,
                                 selq,
                                 res_clauses,
                                 res_formulas);
   }
   PStackFormulaDelProp(res_formulas, CPIsRelevant);
   // fprintf(stderr, "sine selected:\n");
   // for(PStackPointer i=0; i<PStackGetSP(res_formulas); i++)
   // {
   //    WFormula_p f = PStackElementP(res_formulas, i);
   //    fprintf(stderr, "%s, ", WFormulaGetId(f));
   // }
   // fprintf(stderr, ".\n");
   PStackClauseDelProp(res_clauses, CPIsRelevant);
   /* fprintf(GlobalOut, COMCHAR" Axioms selected (%lld)\n",
      GetSecTimeMod()); */
   PQueueFree(selq);
   DRelationFree(drel);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: SelectThreshold()
//
//   Dummy selector: If there are up to ax_filter->threshold
//   clauses and formulas, pass them all. Otherwise pass none.
//
// Global Variables: -
//
// Side Effects    : Only irrelevant (appart from the output to the
//                   result stacks).
//
/----------------------------------------------------------------------*/

long SelectThreshold(PStack_p          clause_sets,
                     PStack_p          formula_sets,
                     AxFilter_p        ax_filter,
                     PStack_p          res_clauses,
                     PStack_p          res_formulas)
{
   long ax_cardinality =
      FormulaSetStackCardinality(formula_sets)+
      ClauseSetStackCardinality(clause_sets);

   if(ax_cardinality <= ax_filter->threshold)
   {
      PStackPointer i;
      ClauseSet_p cset;
      FormulaSet_p fset;
      Clause_p clause;
      WFormula_p formula;

      for(i=0; i<PStackGetSP(clause_sets); i++)
      {
         cset = PStackElementP(clause_sets, i);
         for(clause = cset->anchor->succ;
             clause!=cset->anchor;
             clause=clause->succ)
         {
            PStackPushP(res_clauses, clause);
         }
      }
      for(i=0; i<PStackGetSP(formula_sets); i++)
      {
         fset = PStackElementP(formula_sets, i);
         for(formula = fset->anchor->succ;
             formula!=fset->anchor;
             formula=formula->succ)
         {
            PStackPushP(res_formulas, formula);
         }
      }

   }
   return PStackGetSP(res_clauses)+PStackGetSP(res_formulas);
}

/*-----------------------------------------------------------------------
//
// Function: SelectDefinitions()
//
//   Select lambda definitions only
//
// Global Variables: -
//
// Side Effects    : Only irrelevant (appart from the output to the
//                   result stacks).
//
/----------------------------------------------------------------------*/

long SelectDefinitions(PStack_p clause_sets, PStack_p formula_sets,
                       PStack_p res_clauses, PStack_p res_formulas)
{
   PStackPointer i;
   FormulaSet_p fset;
   WFormula_p formula;

   // ignoring clause sets because only formulas can be
   // tagged as definitions
   for(i=0; i<PStackGetSP(formula_sets); i++)
   {
      fset = PStackElementP(formula_sets, i);
      for(formula = fset->anchor->succ;
          formula!=fset->anchor;
          formula=formula->succ)
      {
         if(FormulaQueryProp(formula, CPIsLambdaDef))
         {
            PStackPushP(res_formulas, formula);
         }
         else if (FormulaIsConjecture(formula)||(FormulaIsHypothesis(formula)))
         {
            PStackPushP(res_formulas, formula);
         }
      }
   }

   return PStackGetSP(res_formulas);
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
   fprintf(out, COMCHAR" %6ld %-15s: %6ld clauses, %6ld formulas\n",
           rel->f_code,
           SigFindName(sig, rel->f_code),
           PStackGetSP(rel->d_clauses),
           PStackGetSP(rel->d_formulas));
   fprintf(out, COMCHAR"formulas: ");
   for(PStackPointer i = 0; i<PStackGetSP(rel->d_formulas); i++)
   {
      fprintf(out, "%s, ",
              WFormulaGetId(PStackElementP(rel->d_formulas, i)));
   }
   fputc('\n', stderr);

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
// Function: DRelationTotalEntries()
//
//   Return the total number of clause/formula references in the
//   D-Relation.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long DRelationTotalEntries(DRelation_p rel)
{
   long i, res = 0;
   DRel_p handle;

   for(i=1; i<rel->relation->size; i++)
   {
      if(PDArrayElementP(rel->relation, i))
      {
         handle = PDArrayElementP(rel->relation, i);
         res += PStackGetSP(handle->d_clauses);
         res += PStackGetSP(handle->d_formulas);
      }
   }
   return res;
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

void PStackClauseDelProp(PStack_p stack, FormulaProperties prop)
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

void PStackFormulaDelProp(PStack_p stack, FormulaProperties prop)
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

/*-----------------------------------------------------------------------
//
// Function: PStackClausesMove()
//
//   Move all clauses on stack from their old set to set.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void PStackClausesMove(PStack_p stack, ClauseSet_p set)
{
   PStackPointer i;
   Clause_p clause;

   for(i=0; i<PStackGetSP(stack); i++)
   {
      clause = PStackElementP(stack, i);
      ClauseSetMoveClause(set, clause);
   }
}


/*-----------------------------------------------------------------------
//
// Function: PStackFormulasMove()
//
//   Move all formulas on stack from their old set to set.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void PStackFormulasMove(PStack_p stack, FormulaSet_p set)
{
   PStackPointer i;
   WFormula_p form;

   for(i=0; i<PStackGetSP(stack); i++)
   {
      form = PStackElementP(stack, i);
      FormulaSetMoveFormula(set, form);
   }
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
