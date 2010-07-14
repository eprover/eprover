/*-----------------------------------------------------------------------

File  : cco_batch_spec.c

Author: Stephan Schulz (schulz@eprover.org)

Contents
 
  CASC-J5 batch specification file.

  Copyright 2010 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Tue Jun 29 04:41:18 CEST 2010
    New

-----------------------------------------------------------------------*/

#include "cco_batch_spec.h"



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
// Function: BatchSpecAlloc()
//
//   Allocate an empty, initialized batch spec file.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

BatchSpec_p BatchSpecAlloc(void)
{
   BatchSpec_p handle = BatchSpecCellAlloc();

   handle->category      = NULL;
   handle->per_prob_time = 0;
   handle->total_time    = 0;
   handle->includes      = PStackAlloc();
   handle->source_files  = PStackAlloc();
   handle->dest_files    = PStackAlloc();

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: BatchSpecFree()
//
//   Free a batch spec structure with all information.
//
// Global Variables: -
//
// Side Effects    : Memory management
//
/----------------------------------------------------------------------*/

void BatchSpecFree(BatchSpec_p spec)
{
   char* str;

   FREE(spec->category);
   
   while(!PStackEmpty(spec->includes))
   {
      str = PStackPopP(spec->includes);
      FREE(str);
   }
   PStackFree(spec->includes);

   while(!PStackEmpty(spec->source_files))
   {
      str = PStackPopP(spec->source_files);
      FREE(str);
   }
   PStackFree(spec->source_files);

   while(!PStackEmpty(spec->dest_files))
   {
      str = PStackPopP(spec->dest_files);
      FREE(str);
   }
   PStackFree(spec->dest_files);

   BatchSpecCellFree(spec);
}


/*-----------------------------------------------------------------------
//
// Function: BatchSpecPrint()
//
//   Print a BatchSpec cell in the original form (or as close as I can
//   make it).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void BatchSpecPrint(FILE* out, BatchSpec_p spec)
{
   PStackPointer i;

   fprintf(out, "%% SZS start BatchConfiguration\n");
   fprintf(out, "division.category %s\n", spec->category);
   fprintf(out, "limit.time.problem.wc %ld\n", spec->per_prob_time);
   fprintf(out, "limit.time.overall.wc %ld\n", spec->total_time);
   fprintf(out, "%% SZS end BatchConfiguration\n");
   fprintf(out, "%% SZS start BatchIncludes\n");

   for(i=0; i<PStackGetSP(spec->includes); i++)
   {
      fprintf(out, "include('%s').\n", 
              (char*)PStackElementP(spec->includes, i));
   }
   fprintf(out, "%% SZS end BatchIncludes\n");
   fprintf(out, "%% SZS start BatchProblems\n");

   for(i=0; i<PStackGetSP(spec->source_files); i++)
   {
      fprintf(out, "%s %s\n", 
              (char*)PStackElementP(spec->source_files, i),
              (char*)PStackElementP(spec->dest_files, i));
   }
   fprintf(out, "%% SZS end BatchProblems\n");
}


/*-----------------------------------------------------------------------
//
// Function: BatchSpecParse()
//
//   Parse a batch specification file. This is somewhat wonky - the
//   spec file syntax is not really well-defined, and what we know
//   about them is that comments and newlines are significant for the
//   structure. This just ignores those and hopes for the best.
//
// Global Variables: -
//
// Side Effects    : Input, memory
//
/----------------------------------------------------------------------*/

BatchSpec_p BatchSpecParse(Scanner_p in)
{
   BatchSpec_p handle = BatchSpecAlloc();
   char *dummy;
   
   dummy = ParseDottedId(in);
   if(strcmp(dummy, "division.category")!= 0)
   {
      Error("Expected \"division.category\"\n", SYNTAX_ERROR);
   }
   FREE(dummy);
   handle->category = ParseDottedId(in);
   
   dummy = ParseDottedId(in);
   if(strcmp(dummy, "limit.time.problem.wc")!= 0)
   {
      Error("Expected \"limit.time.problem.wc\"\n", SYNTAX_ERROR);
   }
   FREE(dummy);
   handle->per_prob_time = ParseInt(in);

   dummy = ParseDottedId(in);
   if(strcmp(dummy, "limit.time.overall.wc")!= 0)
   {
      Error("Expected \"limit.time.overall.wc\"\n", SYNTAX_ERROR);
   }
   FREE(dummy);
   handle->total_time = ParseInt(in);

   while(TestInpId(in, "include"))
   {
      dummy = ParseBasicInclude(in);
      PStackPushP(handle->includes, dummy);
   }
   
   while(!TestInpTok(in, NoToken))
   {
      dummy = ParseFilename(in);
      PStackPushP(handle->source_files, dummy);
      dummy = ParseFilename(in);
      PStackPushP(handle->dest_files, dummy);

   }      
   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: BatchControlAlloc()
//
//   Allocate a BatchControl data structure.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

BatchControl_p BatchControlAlloc(void)
{
   BatchControl_p handle = BatchControlCellAlloc();

   handle->sig             = SigAlloc();
   SigInsertFOFCodes(handle->sig);
   handle->terms           = TBAlloc(handle->sig);
   handle->clause_sets     = PStackAlloc();
   handle->formula_sets    = PStackAlloc();
   handle->parsed_includes = NULL;
   handle->f_distrib       = GenDistribAlloc(handle->sig);

   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: BatchControlFree()
//
//   Free a BatchControl data structure.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void BatchControlFree(BatchControl_p ctrl)
{
   FormulaSet_p fset;
   ClauseSet_p  cset;

   while(!PStackEmpty(ctrl->clause_sets))
   {
      cset = PStackPopP(ctrl->clause_sets);
      ClauseSetFree(cset);
   }
   PStackFree(ctrl->clause_sets);

   while(!PStackEmpty(ctrl->formula_sets))
   {
      fset = PStackPopP(ctrl->formula_sets);
      FormulaSetFree(fset);
   }   
   PStackFree(ctrl->formula_sets);

   SigFree(ctrl->sig);
   ctrl->terms->sig = NULL;
   TBFree(ctrl->terms);
   StrTreeFree(ctrl->parsed_includes);
   GenDistribFree(ctrl->f_distrib);

   BatchControlCellFree(ctrl);
}


/*-----------------------------------------------------------------------
//
// Function: BatchControlInitSpec()
//
//   Initialize a BatchControllCell by parsing all the include files
//   in spec.
//
// Global Variables: -
//
// Side Effects    : I/O, memory operations
//
/----------------------------------------------------------------------*/

long BatchControlInitSpec(BatchSpec_p spec, BatchControl_p ctrl)
{
   PStackPointer i;
   char*        iname;
   FormulaSet_p fset;
   ClauseSet_p  cset;
   Scanner_p    in;
   long         res = 0;
   IntOrP       dummy;

   for(i=0; i<PStackGetSP(spec->includes); i++)
   {
      iname = PStackElementP(spec->includes, i);
      if(!StrTreeFind(&(ctrl->parsed_includes), iname))
      {
         in = CreateScanner(StreamTypeFile, iname, true, NULL);
         ScannerSetFormat(in, TSTPFormat);

         fprintf(GlobalOut, "# Parsing %s\n", iname);
         cset = ClauseSetAlloc();
         fset = FormulaSetAlloc();
         res += FormulaAndClauseSetParse(in, cset, fset, ctrl->terms, 
                                         NULL, 
                                         &(ctrl->parsed_includes));
         PStackPushP(ctrl->clause_sets, cset);
         PStackPushP(ctrl->formula_sets, fset);
         StrTreeStore(&(ctrl->parsed_includes), iname, dummy, dummy);
         
         // ClauseSetPrint(stdout, cset, true);
         // FormulaSetPrint(stdout, fset, true);
         
         DestroyScanner(in);
      }
   }
   ctrl->shared_ax_sp = PStackGetSP(ctrl->clause_sets);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: BatchControlInitDistrib()
//
//   Initialize the f_distrib element of an otherwise initialized
//   batch control cell.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void BatchControlInitDistrib(BatchControl_p ctrl)
{
   GenDistribSizeAdjust(ctrl->f_distrib, ctrl->sig);
   GenDistribAddClauseSets(ctrl->f_distrib, ctrl->clause_sets);
   GenDistribAddFormulaSets(ctrl->f_distrib, ctrl->formula_sets);
}

/*-----------------------------------------------------------------------
//
// Function: BatchControlInit()
//
//   Initialize a BatchControlCell up to the symbol frequency.
//
// Global Variables: -
//
// Side Effects    : Yes ;-)
//
/----------------------------------------------------------------------*/

long BatchControlInit(BatchSpec_p spec, BatchControl_p ctrl)
{
   long res;

   res = BatchControlInitSpec(spec, ctrl);
   BatchControlInitDistrib(ctrl);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: BatchControlAddProblem()
//
//   Add a problem as one set of clauses and formulas, each. Note that
//   this transfers the two sets into ctrl, which is responsible for
//   freeing. 
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void BatchControlAddProblem(BatchControl_p ctrl, 
                            ClauseSet_p clauses, 
                            FormulaSet_p formulas)
{
   GenDistribSizeAdjust(ctrl->f_distrib, ctrl->sig);
   PStackPushP(ctrl->clause_sets, clauses);
   PStackPushP(ctrl->formula_sets, formulas);
   GenDistribAddClauseSet(ctrl->f_distrib, clauses, 1);
   GenDistribAddFormulaSet(ctrl->f_distrib, formulas, 1);   
}


/*-----------------------------------------------------------------------
//
// Function: BatchControlBacktrackToSpec()
//
//   Backtrack the state to the spec state, i.e. backtrack the
//   frequency count and free the extra clause sets.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void BatchControlBacktrackToSpec(BatchControl_p ctrl)
{
   ClauseSet_p clauses;
   FormulaSet_p formulas;

   GenDistribBacktrackClauseSets(ctrl->f_distrib, 
                                 ctrl->clause_sets,
                                 ctrl->shared_ax_sp);
   GenDistribBacktrackFormulaSets(ctrl->f_distrib, 
                                  ctrl->formula_sets,
                                  ctrl->shared_ax_sp);
    while(PStackGetSP(ctrl->clause_sets)>ctrl->shared_ax_sp)
    {
       clauses = PStackPopP(ctrl->clause_sets);
       ClauseSetFree(clauses);
       formulas = PStackPopP(ctrl->formula_sets);
       FormulaSetFree(formulas);
    }    
}


/*-----------------------------------------------------------------------
//
// Function: BatchControlGetProblem()
//
//   Given a prepared BatchControl, get the clauses and formulas
//   describing the problem.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long BatchControlGetProblem(BatchControl_p ctrl,
                            GeneralityMeasure gen_measure,
                            double            benevolence,
                            PStack_p          res_clauses, 
                            PStack_p          res_formulas)
{
   long res;

   res = SelectAxioms(ctrl->f_distrib,
                      ctrl->clause_sets,
                      ctrl->formula_sets,
                      ctrl->shared_ax_sp,
                      gen_measure,
                      benevolence,
                      res_clauses, 
                      res_formulas);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: BatchProcessProblem()
//
//   Given an initialized BatchControlCell for Spec, parse the problem
//   file and try to solve it.
//
// Global Variables: -
//
// Side Effects    : Plenty (IO, memory, time passes...)
//
/----------------------------------------------------------------------*/

bool BatchProcessProblem(BatchSpec_p spec, 
                         BatchControl_p ctrl, 
                         char* source, char* dest)
{
   bool res = false;
   Scanner_p in;
   ClauseSet_p cset;
   FormulaSet_p fset;
   PStack_p cspec = PStackAlloc();
   PStack_p fspec = PStackAlloc();

   fprintf(GlobalOut, "# Processing %s -> %s\n", source, dest);
   
   in = CreateScanner(StreamTypeFile, source, true, NULL);
   ScannerSetFormat(in, TSTPFormat);

   cset = ClauseSetAlloc();
   fset = FormulaSetAlloc();
   FormulaAndClauseSetParse(in, cset, fset, ctrl->terms, 
                            NULL, 
                            &(ctrl->parsed_includes));
   DestroyScanner(in);

   BatchControlAddProblem(ctrl, 
                          cset, 
                          fset);

   BatchControlGetProblem(ctrl, 
                          GMFormulas,
                          1,
                          cspec,
                          fspec);
   fprintf(GlobalOut, "# Spec 1  has %d clauses and %d formulas\n",
           PStackGetSP(cspec), PStackGetSP(fspec));
   PStackClausePrintTSTP(GlobalOut, cspec);
   PStackFormulaPrintTSTP(GlobalOut, fspec);

   PStackFormulaDelProp(fspec, WPIsRelevant);
   PStackClauseDelProp(cspec, CPIsRelevant);
   PStackReset(cspec);
   PStackReset(fspec);

   BatchControlGetProblem(ctrl, 
                          GMFormulas,
                          1.5,
                          cspec,
                          fspec);
   fprintf(GlobalOut, "# Spec 2  has %d clauses and %d formulas\n",
           PStackGetSP(cspec), PStackGetSP(fspec));
   PStackFormulaDelProp(fspec, WPIsRelevant);
   PStackClauseDelProp(cspec, CPIsRelevant);
   PStackReset(cspec);
   PStackReset(fspec);

   BatchControlGetProblem(ctrl, 
                          GMTerms,
                          1,
                          cspec,
                          fspec);
   fprintf(GlobalOut, "# Spec 3  has %d clauses and %d formulas\n",
           PStackGetSP(cspec), PStackGetSP(fspec));

   BatchControlBacktrackToSpec(ctrl);
   PStackFormulaDelProp(fspec, WPIsRelevant);
   PStackClauseDelProp(cspec, CPIsRelevant);

   PStackFree(cspec);
   PStackFree(fspec);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: BatchProcessProblems()
//
//   Process all the problems in the BatchControl structure. Return
//   number of proofs found.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool BatchProcessProblems(BatchSpec_p spec, BatchControl_p ctrl)
{
   long res = 0;
   PStackPointer i;

   for(i=0; i<PStackGetSP(spec->source_files); i++)
   {
      if(BatchProcessProblem(spec,
                             ctrl, 
                             PStackElementP(spec->source_files, i),
                             PStackElementP(spec->dest_files, i)))
      {
         res++;
      }
   }
   return res;
}
                          





/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


