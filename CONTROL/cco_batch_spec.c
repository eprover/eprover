/*-----------------------------------------------------------------------

File  : cco_batch_spec.c

Author: Stephan Schulz (schulz@eprover.org)

Contents
 
  CASC-J5 batch specification file.

  Copyright 2010 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
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


/*-----------------------------------------------------------------------
//
// Function: batch_create_runner()
//
//   Create a EPCtrl block associated with a running instance of E.
//
// Global Variables: 
//
// Side Effects    : 
//
/----------------------------------------------------------------------*/

EPCtrl_p batch_create_runner(StructFOFSpec_p ctrl,
                             char *executable,
                             long cpu_time,
                             AxFilter_p ax_filter)
{
   EPCtrl_p pctrl;
   char     *file;
   FILE     *fp;
   char     name[80];

   PStack_p cspec = PStackAlloc();
   PStack_p fspec = PStackAlloc();

   StructFOFSpecGetProblem(ctrl, 
                           ax_filter,
                           cspec,
                           fspec);
   fprintf(GlobalOut, "# Spec has %d clauses and %d formulas (%lld)\n",
           PStackGetSP(cspec), PStackGetSP(fspec), GetSecTimeMod());
   
   file = TempFileName();
   fp   = fopen(file, "w");
   PStackClausePrintTSTP(fp, cspec);
   PStackFormulaPrintTSTP(fp, fspec);
   fclose(fp);
   
   fprintf(GlobalOut, "# Written new problem (%lld)\n", GetSecTimeMod());

   sprintf(name, "SinE(%d, %f)", ax_filter->gen_measure, ax_filter->benevolence);
   pctrl = ECtrlCreate(executable, name, cpu_time, file);

   //PStackFormulaDelProp(fspec, WPIsRelevant);
   //PStackClauseDelProp(cspec, CPIsRelevant);

   PStackFree(cspec);
   PStackFree(fspec);
   
   return pctrl;
}


/*-----------------------------------------------------------------------
//
// Function: parse_op_line()
//
//   Parse an output line into batchspec
//
// Global Variables: -
//
// Side Effects    : Reads input
//
/----------------------------------------------------------------------*/

void parse_op_line(Scanner_p in, BatchSpec_p spec, BOOutputType state)
{
   while(TestInpId(in, "Assurance|Proof|Model|Answer|ListOfFOF"))
   {
      if(TestInpId(in, "Assurance"))
      {
         spec->res_assurance = state;
      }
      else if(TestInpId(in, "Proof"))
      {
         spec->res_proof = state;
      }
      else if(TestInpId(in, "Model"))
      {
         spec->res_model = state;
      }
      else if(TestInpId(in, "Answer"))
      {
         spec->res_answer = state;
      }
      else if(TestInpId(in, "ListOfFOF"))
      {
         spec->res_list_fof = state;
      }
      AcceptInpTok(in, Ident);
   }
}


/*-----------------------------------------------------------------------
//
// Function: print_op_line()
//
//   Print an output line in spec to out
//
// Global Variables: -
//
// Side Effects    : Reads input
//
/----------------------------------------------------------------------*/

void print_op_line(FILE* out, BatchSpec_p spec, BOOutputType state)
{
   if( spec->res_assurance == state)
   {
      fprintf(out,  " Assurance"); 
   }
   if( spec->res_proof == state)
   {
      fprintf(out, " Proof"); 
   }
   if( spec->res_model == state)
   {
      fprintf(out, " Model"); 
   }
   if( spec->res_answer == state)
   {
      fprintf(out, " Answer"); 
   }
   if( spec->res_list_fof == state)
   {
      fprintf(out, " ListOfFOF"); 
   }
}

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

BatchSpec_p BatchSpecAlloc(char* executable, IOFormat format)
{
   BatchSpec_p handle = BatchSpecCellAlloc();

   handle->category      = NULL;
   handle->per_prob_time = 0;
   handle->total_time    = 0;
   handle->includes      = PStackAlloc();
   handle->source_files  = PStackAlloc();
   handle->format        = format;
   handle->res_assurance = BONone;
   handle->res_proof     = BONone;
   handle->res_model     = BONone;
   handle->res_answer    = BONone;
   handle->res_list_fof  = BONone;

   handle->dest_files    = PStackAlloc();
   handle->executable    = SecureStrdup(executable);

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

   FREE(spec->executable);
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
   fprintf(out, "output.required");
   print_op_line(out, spec, BORequired);
   fprintf(out, "\n");
  
   fprintf(out, "output.desired");
   print_op_line(out, spec, BODesired);
   fprintf(out, "\n");

   fprintf(out, "limit.time.problem.wc %ld\n", spec->per_prob_time);
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

BatchSpec_p BatchSpecParse(Scanner_p in, char* executable, IOFormat format)
{
   BatchSpec_p handle = BatchSpecAlloc(executable, format);
   char *dummy;
   
   dummy = ParseDottedId(in);
   if(strcmp(dummy, "division.category")!= 0)
   {
      Error("Expected \"division.category\"\n", SYNTAX_ERROR);
   }
   FREE(dummy);
   handle->category = ParseDottedId(in);
   
   dummy = ParseDottedId(in);
   if(strcmp(dummy, "output.required")!= 0)
   {
      Error("Expected \"output.required\"\n", SYNTAX_ERROR);
   }
   parse_op_line(in, handle, BORequired);

   dummy = ParseDottedId(in);
   if(strcmp(dummy, "output.desired")!= 0)
   {
      Error("Expected \"output.desired\"\n", SYNTAX_ERROR);
   }
   parse_op_line(in, handle, BODesired);

   dummy = ParseDottedId(in);
   if(strcmp(dummy, "limit.time.problem.wc")!= 0)
   {
      Error("Expected \"limit.time.problem.wc\"\n", SYNTAX_ERROR);
   }
   FREE(dummy);
   handle->per_prob_time = ParseInt(in);

   while(TestInpId(in, "include"))
   {
      dummy = ParseBasicInclude(in);
      PStackPushP(handle->includes, dummy);
   }
   
   while(TestInpTok(in, Slash))
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
// Function: StructFOFSpecAlloc()
//
//   Allocate a Structures problem data structure.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

StructFOFSpec_p StructFOFSpecAlloc(void)
{
   StructFOFSpec_p handle = StructFOFSpecCellAlloc();

   handle->sig             = SigAlloc();
   SigInsertInternalCodes(handle->sig);
   handle->terms           = TBAlloc(handle->sig);
   handle->clause_sets     = PStackAlloc();
   handle->formula_sets    = PStackAlloc();
   handle->parsed_includes = NULL;
   handle->f_distrib       = GenDistribAlloc(handle->sig);

   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: StructFOFSpecFree()
//
//   Free a StructFOFSpec data structure.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void StructFOFSpecFree(StructFOFSpec_p ctrl)
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
   
   StructFOFSpecCellFree(ctrl);
}


/*-----------------------------------------------------------------------
//
// Function: StructFOFSpecParseAxioms()
//
//   Initialize a StructFOFSpeclCell by parsing all the include files
//   in in axfiles.
//
// Global Variables: -
//
// Side Effects    : I/O, memory operations
//
/----------------------------------------------------------------------*/

long StructFOFSpecParseAxioms(StructFOFSpec_p ctrl, PStack_p axfiles, 
                             IOFormat parse_format)
{
   PStackPointer i;
   char*        iname;
   FormulaSet_p fset;
   ClauseSet_p  cset;
   Scanner_p    in;
   long         res = 0;
   IntOrP       dummy;

   for(i=0; i<PStackGetSP(axfiles); i++)
   {
      iname = PStackElementP(axfiles, i);
      if(!StrTreeFind(&(ctrl->parsed_includes), iname))
      {
         in = CreateScanner(StreamTypeFile, iname, true, NULL);
         ScannerSetFormat(in, parse_format);

         fprintf(GlobalOut, "# Parsing %s\n", iname);
         cset = ClauseSetAlloc();
         fset = FormulaSetAlloc();
         res += FormulaAndClauseSetParse(in, cset, fset, ctrl->terms, 
                                         NULL, 
                                         &(ctrl->parsed_includes));
         PStackPushP(ctrl->clause_sets, cset);
         PStackPushP(ctrl->formula_sets, fset);
         StrTreeStore(&(ctrl->parsed_includes), iname, dummy, dummy);
         
         DestroyScanner(in);
      }
   }
   ctrl->shared_ax_sp = PStackGetSP(ctrl->clause_sets);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: StructFOFSpecInitDistrib()
//
//   Initialize the f_distrib element of an otherwise initialized
//   structured problem cell.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void StructFOFSpecInitDistrib(StructFOFSpec_p ctrl)
{
   GenDistribSizeAdjust(ctrl->f_distrib, ctrl->sig);
   GenDistribAddClauseSets(ctrl->f_distrib, ctrl->clause_sets);
   GenDistribAddFormulaSets(ctrl->f_distrib, ctrl->formula_sets);
}

/*-----------------------------------------------------------------------
//
// Function: StructFOFSpecInit()
//
//   Initialize a StructFOFSpecCell up to the symbol frequency.
//
// Global Variables: -
//
// Side Effects    : Yes ;-)
//
/----------------------------------------------------------------------*/

long StructFOFSpecInit(BatchSpec_p spec, StructFOFSpec_p ctrl)
{
   long res;

   res = StructFOFSpecParseAxioms(ctrl, spec->includes, spec->format);
   StructFOFSpecInitDistrib(ctrl);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: StructFOFSpecAddProblem()
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

void StructFOFSpecAddProblem(StructFOFSpec_p ctrl, 
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
// Function: StructFOFSpecBacktrackToSpec()
//
//   Backtrack the state to the spec state, i.e. backtrack the
//   frequency count and free the extra clause sets.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void StructFOFSpecBacktrackToSpec(StructFOFSpec_p ctrl)
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
// Function: StructFOFSpecGetProblem()
//
//   Given a prepared StructFOFSpec, get the clauses and formulas
//   describing the problem.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long StructFOFSpecGetProblem(StructFOFSpec_p ctrl,
                             AxFilter_p      ax_filter,
                             PStack_p        res_clauses, 
                             PStack_p        res_formulas)
{
   long res;

   
   res = SelectAxioms(ctrl->f_distrib,
                      ctrl->clause_sets,
                      ctrl->formula_sets,
                      ctrl->shared_ax_sp,
                      ax_filter,
                      res_clauses, 
                      res_formulas);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: BatchProcessProblem()
//
//   Given an initialized StructFOFSpecCell for Spec, parse the problem
//   file and try to solve it.
//
// Global Variables: -
//
// Side Effects    : Plenty (IO, memory, time passes...)
//
/----------------------------------------------------------------------*/

bool BatchProcessProblem(BatchSpec_p spec, 
                         StructFOFSpec_p ctrl, 
                         char* source, char* dest)
{
   bool res = false;
   Scanner_p in;
   ClauseSet_p cset;
   FormulaSet_p fset;
   EPCtrl_p handle;
   EPCtrlSet_p procs = EPCtrlSetAlloc();
   FILE* fp;
   long long secs;
   AxFilterSet_p filters = AxFilterSetCreateInternal(AxFilterDefaultSet);
   int i;


   fprintf(GlobalOut, "\n# Processing %s -> %s\n", source, dest);
   fprintf(GlobalOut, "# SZS status Started for %s\n", source);
   fflush(GlobalOut);
   
   in = CreateScanner(StreamTypeFile, source, true, NULL);
   ScannerSetFormat(in, TSTPFormat);

   cset = ClauseSetAlloc();
   fset = FormulaSetAlloc();
   FormulaAndClauseSetParse(in, cset, fset, ctrl->terms, 
                            NULL, 
                            &(ctrl->parsed_includes));
   DestroyScanner(in);
   
   fprintf(GlobalOut, "# Adding problem (%lld)\n", GetSecTimeMod());
   StructFOFSpecAddProblem(ctrl, 
                          cset, 
                          fset);
   fprintf(GlobalOut, "# Added problem (%lld)\n", GetSecTimeMod());


   secs = GetSecTime();
   handle = batch_create_runner(ctrl, spec->executable, spec->per_prob_time, 
                                AxFilterSetGetFilter(filters, 0));
   EPCtrlSetAddProc(procs, handle);
   
   i=1;
   while(((GetSecTime()-secs) < (spec->per_prob_time/2)) && 
         (i<AxFilterSetElements(filters)))
   {
      handle = batch_create_runner(ctrl, spec->executable, spec->per_prob_time, 
                                   AxFilterSetGetFilter(filters, i));
      EPCtrlSetAddProc(procs, handle);
      i++;
   }
   AxFilterSetFree(filters);


   handle = NULL;
   while(!EPCtrlSetEmpty(procs))
   {
      handle = EPCtrlSetGetResult(procs);
      if(handle)
      {
         break;
      }
   }
   if(handle)
   {
      fprintf(GlobalOut, "# Proof found by %s\n", handle->name);
      fp = fopen(dest, "w");
      fprintf(fp, "%s", DStrView(handle->output));
      fclose(fp);
      fprintf(GlobalOut, "%s", DStrView(handle->output));
   }
   else
   {
      fprintf(GlobalOut, "# SZS status GaveUp for %s\n", source);
   }
   
   StructFOFSpecBacktrackToSpec(ctrl);
   /* cset and fset are freed in Backtrack */

   EPCtrlSetFree(procs);

   fprintf(GlobalOut, "# SZS status Ended for %s\n\n", source);
   fflush(GlobalOut);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: BatchProcessProblems()
//
//   Process all the problems in the StructFOFSpec structure. Return
//   number of proofs found.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool BatchProcessProblems(BatchSpec_p spec, StructFOFSpec_p ctrl)
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


