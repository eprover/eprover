/*-----------------------------------------------------------------------

  File  : cco_batch_spec.c

  Author: Stephan Schulz (schulz@eprover.org)

  Contents

  CASC-J5 and up batch specification file processing.

  Copyright 2010-2019 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Tue Jun 29 04:41:18 CEST 2010

  -----------------------------------------------------------------------*/

#include "cco_batch_spec.h"
#include "cco_gproc_ctrl.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

char* BatchFilters[] =
{
   "threshold010000",
   "gf600_h_gu_R05_F100_L20000"  ,   /* Protokoll_X----_auto_sine17 */
   "gf120_h_gu_R02_F100_L20000"  ,   /* protokoll_X----_auto_sine13 */
   "gf200_gu_RUU_F100_L20000"    ,   /* protokoll_X----_auto_sine08 */
   "gf200_h_gu_R03_F100_L20000"  ,   /* protokoll_X----_auto_sine16 */
   "gf120_h_gu_RUU_F100_L00100"  ,   /* protokoll_X----_auto_sine15 */
   "gf500_h_gu_R04_F100_L20000"  ,   /* protokoll_X----_auto_sine11 */
   "gf150_gu_RUU_F100_L20000"    ,   /* protokoll_X----_auto_sine04 */
   "gf120_h_gu_RUU_F100_L00500"  ,   /* protokoll_X----_auto_sine12 */
   "gf120_gu_RUU_F100_L01000"    ,   /* protokoll_X----_auto_sine21 */
   "gf120_gu_R02_F100_L20000"    ,   /* protokoll_X----_auto_sine03 */
   "gf500_gu_R04_F100_L20000"    ,   /* protokoll_X----_auto_sine01 */
   "gf600_gu_R05_F100_L20000"    ,   /* protokoll_X----_auto_sine07 */
   NULL
};

char* BatchStrategies[] =
{
   "--satauto-schedule --assume-incompleteness",
   "--satauto-schedule --assume-incompleteness",
   "--satauto-schedule --assume-incompleteness",
   "--satauto-schedule --assume-incompleteness",
   "--satauto-schedule --assume-incompleteness",
   "--satauto-schedule --assume-incompleteness",
   "--satauto-schedule --assume-incompleteness",
   "--satauto-schedule --assume-incompleteness",
   "--satauto-schedule --assume-incompleteness",
   "--satauto-schedule --assume-incompleteness",
   "--satauto-schedule --assume-incompleteness",
   "--satauto-schedule --assume-incompleteness",
   "--satauto-schedule --assume-incompleteness",
   NULL
};

char* BatchFiltersDiv[] =
{
   "threshold010000",
   "gf600_h_gu_R05_F100_L20000"  ,   /* protokoll_X----_auto_sine17 */
   "gf120_h_gu_R02_F100_L20000"  ,   /* protokoll_X----_auto_sine13 */
   "gf200_gu_RUU_F100_L20000"    ,   /* protokoll_X----_auto_sine08 */
   "gf200_h_gu_R03_F100_L20000"  ,   /* protokoll_X----_auto_sine16 */
   "gf120_h_gu_RUU_F100_L00100"  ,   /* protokoll_X----_auto_sine15 */
   "gf500_h_gu_R04_F100_L20000"  ,   /* protokoll_X----_auto_sine11 */
   "gf150_gu_RUU_F100_L20000"    ,   /* protokoll_X----_auto_sine04 */
   "gf120_h_gu_RUU_F100_L00500"  ,   /* protokoll_X----_auto_sine12 */
   "gf120_gu_RUU_F100_L01000"    ,   /* protokoll_X----_auto_sine21 */
   "gf120_gu_R02_F100_L20000"    ,   /* protokoll_X----_auto_sine03 */
   "gf500_gu_R04_F100_L20000"    ,   /* protokoll_X----_auto_sine01 */
   "gf600_gu_R05_F100_L20000"    ,   /* protokoll_X----_auto_sine07 */
   "gf600_h_gu_R05_F100_L20000"  ,   /* protokoll_X----_auto_sine17 */
   "gf600_h_gu_R05_F100_L20000"  ,   /* protokoll_X----_auto_sine17 */
   "gf600_h_gu_R05_F100_L20000"  ,   /* protokoll_X----_auto_sine17 */
   "gf600_h_gu_R05_F100_L20000"  ,   /* protokoll_X----_auto_sine17 */
   NULL
};


char* BatchStrategiesDiv[] =
{
   "--auto-schedule --assume-incompleteness",
   "--satauto-schedule --assume-incompleteness",
   "--satauto-schedule --assume-incompleteness",
   "--satauto-schedule --assume-incompleteness",
   "--satauto-schedule --assume-incompleteness",
   "--satauto-schedule --assume-incompleteness",
   "--satauto-schedule --assume-incompleteness",
   "--satauto-schedule --assume-incompleteness",
   "--satauto-schedule --assume-incompleteness",
   "--satauto-schedule --assume-incompleteness",
   "--satauto-schedule --assume-incompleteness",
   "--satauto-schedule --assume-incompleteness",
   "--satauto-schedule --assume-incompleteness",
   "-xAutoSched2 -tAutoSched2 --assume-incompleteness",
   "-xAutoSched3 -tAutoSched3 --assume-incompleteness",
   "-xAutoSched4 -tAutoSched4 --assume-incompleteness",
   "-xAutoSched5 -tAutoSched5 --assume-incompleteness",
   NULL
};

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
                             char* options,
                             char* extra_options,
                             long cpu_time,
                             AxFilter_p ax_filter)
{
   EPCtrl_p pctrl;
   char     *file;
   FILE     *fp;
   char     name[320];

   PStack_p cspec = PStackAlloc();
   PStack_p fspec = PStackAlloc();

   fprintf(GlobalOut, COMCHAR" Filtering for ");
   AxFilterPrint(GlobalOut, ax_filter);
   fprintf(GlobalOut, " (%lld)\n", GetSecTimeMod());
   StructFOFSpecGetProblem(ctrl,
                           ax_filter,
                           cspec,
                           fspec);
   fprintf(GlobalOut, COMCHAR" Spec has %ld clauses and %ld formulas (%lld)\n",
           PStackGetSP(cspec), PStackGetSP(fspec), GetSecTimeMod());

   file = TempFileName();
   fp   = SecureFOpen(file, "w");

   SigPrintTypeDeclsTSTP(fp, ctrl->terms->sig);
   PStackClausePrintTSTP(fp, cspec);
   PStackFormulaPrintTSTP(fp, fspec);
   SecureFClose(fp);
   //printf(COMCHAR" ====== Writing filtered file===========\n");
   //FilePrint(stdout, file);
   //printf(COMCHAR" =======Filtered file written===========\n");
   fprintf(GlobalOut, COMCHAR" Written new problem (%lld)\n", GetSecTimeMod());

   AxFilterPrintBuf(name, 320, ax_filter);
   pctrl = ECtrlCreateGeneric(executable, name, options, extra_options, cpu_time, file);

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




/*-----------------------------------------------------------------------
//
// Function: abstract_to_concrete()
//
//   Replace the * in an abstract name by the variant and append the
//   ending. Ignores everything after * in name. The result is
//   returned and must be freed by the caller.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

char* abstract_to_concrete(char* name, char* variant, char* postfix)
{
   char *res;
   DStr_p buffer = DStrAlloc();

   for(; *name && *name!='*'; name++)
   {
      DStrAppendChar(buffer, *name);
   }
   DStrAppendStr(buffer, variant);
   DStrAppendStr(buffer, postfix);

   res = DStrCopy(buffer);
   DStrFree(buffer);

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: concrete_batch_struct_FOF_spec_init()
//
//   Initialise a StructFOFSpecCell for the concrete problems encoded
//   in *variant.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void concrete_batch_struct_FOF_spec_init(BatchSpec_p spec,
                                         StructFOFSpec_p ctrl,
                                         char *default_dir,
                                         char *variant)
{
   PStack_p abstract_includes;
   long i;

   abstract_includes = spec->includes;
   spec->includes = PStackAlloc();
   for(i=0; i<PStackGetSP(abstract_includes); i++)
   {
      PStackPushP(spec->includes,
                  abstract_to_concrete(PStackElementP(abstract_includes,i),
                                       variant,
                                       ".ax"));
   }
   //BatchSpecPrint(GlobalOut, spec);
   BatchStructFOFSpecInit(spec, ctrl, default_dir);
   for(i=0; i<PStackGetSP(spec->includes); i++)
   {
      char* tmp = PStackPopP(spec->includes);
      FREE(tmp);
   }
   PStackFree(spec->includes);
   spec->includes = abstract_includes;
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

   handle->executable = SecureStrdup(executable);
   handle->format          = format;

   handle->category        = NULL;
   handle->train_dir       = NULL;
   handle->ordered         = false;
   handle->res_assurance   = BONone;
   handle->res_proof       = BONone;
   handle->res_model       = BONone;
   handle->res_answer      = BONone;
   handle->res_list_fof    = BONone;
   handle->per_prob_limit  = 0;
   handle->total_wtc_limit = 0;

   handle->includes        = PStackAlloc();
   handle->source_files    = PStackAlloc();
   handle->dest_files      = PStackAlloc();

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

   FREE(spec->executable);
   FREE(spec->category);
   if(spec->train_dir)
   {
      FREE(spec->train_dir);
   }

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
   if(spec->train_dir)
   {
      fprintf(out, "division.category.training_directory %s\n",
              spec->train_dir);
   }
   if(spec->ordered)
   {
      fprintf(out, "execution.order ordered\n");
   }
   fprintf(out, "output.required");
   print_op_line(out, spec, BORequired);
   fprintf(out, "\n");

   fprintf(out, "output.desired");
   print_op_line(out, spec, BODesired);
   fprintf(out, "\n");

   fprintf(out, "limit.time.problem.wc %ld\n", spec->per_prob_limit);
   fprintf(out, "limit.time.overall.wc %ld\n", spec->total_wtc_limit);
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

BatchSpec_p BatchSpecParse(Scanner_p in, char* executable,
                           char* category, char* train_dir,
                           IOFormat format)
{
   BatchSpec_p handle = BatchSpecAlloc(executable, format);
   char *dummy;

   handle->category  = SecureStrdup(category);
   if(train_dir)
   {
      handle->train_dir = SecureStrdup(train_dir);
   }
   /* Ugly hack to remain compatible with CASC-23 files */
   if(TestInpId(in, "execution"))
   {
      AcceptDottedId(in, "execution.order");
      handle->ordered = TestInpId(in, "ordered");
      AcceptInpId(in, "ordered|unordered");
   }
   AcceptDottedId(in, "output.required");
   parse_op_line(in, handle, BORequired);

   if(TestInpId(in, "output"))
   {
      AcceptDottedId(in, "output.desired");
      parse_op_line(in, handle, BODesired);
   }
   AcceptDottedId(in, "limit.time.problem.wc");
   handle->per_prob_limit = ParseInt(in);

   if(TestInpId(in, "limit"))
   {
      AcceptDottedId(in, "limit.time.overall.wc");
      handle->total_wtc_limit = ParseInt(in);
   }

   while(TestInpId(in, "include"))
   {
      dummy = ParseBasicInclude(in);
      PStackPushP(handle->includes, dummy);
      printf(COMCHAR" Accepted %s for parsing\n", dummy);
   }

   /* This is ugly! Fix the LTB format! */
   while(TestInpTok(in, Slash) || TestInpId(in, "Problem|Problems"))
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
// Function: BatchStructFOFSpecInit()
//
//   Initialize a BatchStructFOFSpecCell up to the symbol frequency.
//
// Global Variables: -
//
// Side Effects    : Yes ;-)
//
/----------------------------------------------------------------------*/

long BatchStructFOFSpecInit(BatchSpec_p spec,
                            StructFOFSpec_p ctrl,
                            char *default_dir)
{
   long res;

   res = StructFOFSpecParseAxioms(ctrl, spec->includes, spec->format, default_dir);
   StructFOFSpecInitDistrib(ctrl, false);
   //GenDistribPrint(stdout, ctrl->f_distrib, 10);
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
                             FormulaSet_p formulas,
                             bool trim)
{
   GenDistribSizeAdjust(ctrl->f_distrib, ctrl->terms->sig);
   TBGCRegisterClauseSet(ctrl->terms, clauses);
   PStackPushP(ctrl->clause_sets, clauses);
   TBGCRegisterFormulaSet(ctrl->terms, formulas);
   PStackPushP(ctrl->formula_sets, formulas);

   GenDistribAddClauseSet(ctrl->f_distrib, clauses, 1);
   GenDistribAddFormulaSet(ctrl->f_distrib, formulas, trim, 1);
}


/*-----------------------------------------------------------------------
//
// Function: StructFOFSpecBacktrackToSpec()
//
//   Backtrack the state to the spec state, i.e. backtrack the
//   frequency count and free the extra clause sets. Also backtracks
//   the signature to forget all new symbols.
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
      GCDeregisterClauseSet(ctrl->terms->gc, clauses);
      ClauseSetFree(clauses);
      formulas = PStackPopP(ctrl->formula_sets);
      GCDeregisterFormulaSet(ctrl->terms->gc, formulas);
      FormulaSetFree(formulas);
   }
   TBGCCollect(ctrl->terms);
   SigBacktrack(ctrl->terms->sig, ctrl->shared_ax_f_count);

   problemType = PROBLEM_NOT_INIT;
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
   long res = 0;

   switch(ax_filter->type)
   {
   case AFGSinE:
         res = SelectAxioms(ctrl->f_distrib,
                            ctrl->clause_sets,
                            ctrl->formula_sets,
                            ctrl->shared_ax_sp,
                            ax_filter,
                            res_clauses,
                            res_formulas);
         //printf(COMCHAR" AFGSinE selected %ld/%ld clauses/formulas\n",
         //PStackGetSP(res_clauses), PStackGetSP(res_formulas));
         break;
   case AFThreshold:
         res = SelectThreshold(ctrl->clause_sets,
                               ctrl->formula_sets,
                               ax_filter,
                               res_clauses,
                               res_formulas);
         break;
   case AFLambdaDefines:
         res = SelectDefinitions(ctrl->clause_sets,
                                ctrl->formula_sets,
                                res_clauses,
                                res_formulas);
         break;
   default:
         assert(false && "Unknown AxFilter type");
         break;
   }
   return res;
}




/*-----------------------------------------------------------------------
//
// Function: BatchProcessProblem()
//
//   Given an initialized StructFOFSpecCell for Spec, parse the problem
//   file and try to solve it. Return true if a proof has been found,
//   false otherwise.
//
// Global Variables: -
//
// Side Effects    : Plenty (IO, memory, time passes...)
//
/----------------------------------------------------------------------*/

bool BatchProcessProblem(BatchSpec_p spec,
                         long wct_limit,
                         StructFOFSpec_p ctrl,
                         char* jobname,
                         ClauseSet_p cset,
                         FormulaSet_p fset,
                         FILE* out,
                         int sock_fd,
                         bool interactive)
{
   bool res = false;
   EPCtrl_p handle;
   EPCtrlSet_p procs = EPCtrlSetAlloc();
   long long start, end, used, now, remaining;
   AxFilterSet_p filters = AxFilterSetCreateInternal(AxFilterDefaultSet);
   int i;
   char* answers = spec->res_answer==BONone ?"" : "--conjectures-are-questions";

   start = GetSecTime();

   StructFOFSpecAddProblem(ctrl,
                           cset,
                           fset,
                           false);

   start = GetSecTime();
   end   = start+wct_limit;
   i=0;

   while(!res && GetSecTime()<=end)
   {
      while(BatchFilters[i] &&
            (EPCtrlSetCardinality(procs)<MAX_CORES) &&
            ((now=GetSecTime())<=end))
      {
         used = now-start;
         handle = batch_create_runner(ctrl, spec->executable,
                                      BatchStrategies[i],
                                      answers,
                                      MIN((wct_limit+1)/2, wct_limit-used),
                                      AxFilterSetFindFilter(filters,
                                                            BatchFilters[i]));
         EPCtrlSetAddProc(procs, handle);
         i++;
      }
      handle = EPCtrlSetGetResult(procs, true);
      if(handle)
      {
         break;
      }
   }

   if(handle)
   {
      fprintf(GlobalOut, "%s for %s\n", PRResultTable[handle->result], jobname);
      res = true;
      now = GetSecTime();
      used = now - handle->start_time;
      remaining = handle->prob_time - used;
      fprintf(GlobalOut,
              COMCHAR" Solution found by %s (started %lld, remaining %lld)\n",
              handle->name, handle->start_time, remaining);
      if(out!=GlobalOut)
      {
         if(sock_fd != -1)
         {
            TCPStringSendX(sock_fd, DStrView(handle->output));
         }
         else
         {
            fprintf(out, "%s for %s\n", PRResultTable[handle->result], jobname);
            fprintf(out, "%s", DStrView(handle->output));
            fflush(out);
         }

      }
      if(interactive)
      {
         fprintf(GlobalOut, "%s", DStrView(handle->output));
      }
   }
   else
   {
      fprintf(GlobalOut, COMCHAR" SZS status GaveUp for %s\n", jobname);
      if(out!=GlobalOut)
      {

         char buffer[512];
         sprintf(buffer, COMCHAR" SZS status GaveUp for %s\n", jobname);
         if(sock_fd != -1)
         {
            TCPStringSendX(sock_fd, buffer);
         }
         else
         {
            fprintf(out, "%s", buffer);
            fflush(out);
         }
      }
   }

   StructFOFSpecBacktrackToSpec(ctrl);
   /* cset and fset are freed in Backtrack */

   AxFilterSetFree(filters);
   EPCtrlSetFree(procs, true);

   return res;
}



/*-----------------------------------------------------------------------
//
// Function: BatchProcessFile()
//
//   Given an initialized StructFOFSpecCell for Spec, parse the problem
//   file and try to solve it. Return true if a proof has been found,
//   false otherwise.
//
// Global Variables: -
//
// Side Effects    : Plenty (IO, memory, time passes...)
//
/----------------------------------------------------------------------*/

bool BatchProcessFile(BatchSpec_p spec,
                      long wct_limit,
                      StructFOFSpec_p ctrl,
                      char* default_dir,
                      char* source, char* dest)
{
   bool res = false;
   Scanner_p in;
   ClauseSet_p dummy;
   FormulaSet_p fset;
   FILE* fp;

   //fprintf(GlobalOut, "\n"COMCHAR" Processing %s -> %s\n", source, dest);
   //fprintf(GlobalOut, COMCHAR" SZS status Started for %s\n", source);
   //fflush(GlobalOut);

   in = CreateScanner(StreamTypeFile, source, true, default_dir, true);
   //printf(COMCHAR" Scanner for '%s' created\n", source);
   fflush(stdout);
   ScannerSetFormat(in, TSTPFormat);

   dummy = ClauseSetAlloc();
   fset = FormulaSetAlloc();
   FormulaAndClauseSetParse(in, fset, dummy, ctrl->terms,
                            NULL,
                            &(ctrl->parsed_includes));
   DestroyScanner(in);

   fp = SecureFOpen(dest, "w");

   // dummy and fset are handed over to BatchProcessProblem and are
   // freed there (via StructFOFSpecBacktrackToSpec()).
   res = BatchProcessProblem(spec,
                             wct_limit,
                             ctrl,
                             source,
                             dummy,
                             fset,
                             fp,
                             -1,
                             false);
   SecureFClose(fp);

   //fprintf(GlobalOut, COMCHAR" SZS status Ended for %s\n\n", source);
   //fflush(GlobalOut);

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

long BatchProcessProblems(BatchSpec_p spec, StructFOFSpec_p ctrl,
                          long total_wtc_limit, char* default_dir,
                          char* dest_dir)
{
   long res = 0;
   PStackPointer i;
   PStackPointer sp;
   long wct_limit, prop_time, now, used, rest;
   long start = GetSecTime();
   DStr_p dest_name = DStrAlloc();

   sp = PStackGetSP(spec->source_files);
   for(i=0; i<sp; i++)
   {
      if(total_wtc_limit)
      {
         now       = GetSecTime();
         used      = now - start;
         rest      = total_wtc_limit - used;
         prop_time = rest/(sp-i)+1; /* Bias up a bit - some problems will
                                     * use less time anyways */

         if(spec->per_prob_limit)
         {
            wct_limit = MIN(prop_time, spec->per_prob_limit);
         }
         else
         {
            wct_limit = prop_time;
         }
      }
      else
      {
         wct_limit = spec->per_prob_limit;
      }
      /* printf(COMCHAR"######## Remaining %d probs, %ld secs, limit %ld\n",
         sp-i, rest, wct_limit); */

      DStrReset(dest_name);
      if(dest_dir)
      {
         DStrSet(dest_name, dest_dir);
         DStrAppendChar(dest_name, '/');
      }
      DStrAppendStr(dest_name, PStackElementP(spec->dest_files, i));

      if(BatchProcessFile(spec,
                          wct_limit,
                          ctrl,
                          default_dir,
                          PStackElementP(spec->source_files, i),
                          DStrView(dest_name)))
      {
         res++;
      }
   }
   DStrFree(dest_name);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: BatchProcessInteractive()
//
//   Perform interactive processing of problems relating to the batch
//   processing spec in spec and the axiom sets stored in ctrl.
//
// Global Variables: -
//
// Side Effects    : I/O, blocks on reading fp, initiates processing.
//
/----------------------------------------------------------------------*/

void BatchProcessInteractive(BatchSpec_p spec,
                             StructFOFSpec_p ctrl,
                             FILE* fp)
{
   DStr_p input   = DStrAlloc();
   DStr_p jobname = DStrAlloc();
   bool done = false;
   Scanner_p in;
   ClauseSet_p dummy;
   FormulaSet_p fset;
   long         wct_limit=30;

   if(spec->per_prob_limit)
   {
      wct_limit = spec->per_prob_limit;
   }

   while(!done)
   {
      DStrReset(input);

      fprintf(fp, COMCHAR" Enter job, 'help' or 'quit', followed by 'go.' on a line of its own:\n");
      fflush(fp);
      if(!ReadTextBlock(input, stdin, "go.\n"))
      {
         fprintf(fp, COMCHAR" Error: Read failed (probably EOF)\n");
         break;
      }

      in = CreateScanner(StreamTypeUserString,
                         DStrView(input),
                         true,
                         NULL, true);
      ScannerSetFormat(in, TSTPFormat);
      if(TestInpId(in, "quit"))
      {
         done = true;
      }
      else if(TestInpId(in, "help"))
      {
         fprintf(fp, "\
"COMCHAR" Enter a job, 'help' or 'quit'. Finish any action with 'go.' on a line\n\
"COMCHAR" of its own. A job consists of an optional job name specifier of the\n\
"COMCHAR" form 'job <ident>.', followed by a specification of a first-order\n\
"COMCHAR" problem in TPTP-3 syntax (including any combination of 'cnf', 'fof' and\n\
"COMCHAR" 'include' statements. The system then tries to solve the specified\n\
"COMCHAR" problem (including the constant background theory) and prints the\n\
"COMCHAR" results of this attempt.\n");
      }
      else
      {
         DStrReset(jobname);
         if(TestInpId(in, "job"))
         {
            AcceptInpId(in, "job");
            DStrAppendDStr(jobname, AktToken(in)->literal);
            AcceptInpTok(in, Identifier);
            AcceptInpTok(in, Fullstop);
         }
         else
         {
            DStrAppendStr(jobname, "unnamed_job");
         }
         fprintf(fp, "\n"COMCHAR" Processing started for %s\n", DStrView(jobname));

         dummy = ClauseSetAlloc();
         fset = FormulaSetAlloc();
         FormulaAndClauseSetParse(in, fset, dummy, ctrl->terms,
                                  NULL,
                                  &(ctrl->parsed_includes));

         // cset and fset are handed over to BatchProcessProblem and are
         // freed there (via StructFOFSpecBacktrackToSpec()).
         (void)BatchProcessProblem(spec,
                                   wct_limit,
                                   ctrl,
                                   DStrView(jobname),
                                   dummy,
                                   fset,
                                   fp,
                                   -1,
                                   true);
         fprintf(fp, "\n"COMCHAR" Processing finished for %s\n\n", DStrView(jobname));
      }
      DestroyScanner(in);
   }
   DStrFree(jobname);
   DStrFree(input);
}

/*-----------------------------------------------------------------------
//
// Function: BatchProcessVariants()
//
//    Try to solve the abstract problems in spec by going through the
//    concrete variants indicated by variants.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void BatchProcessVariants(BatchSpec_p spec, char* variants[], char* provers[],
                          long start, char* default_dir, char* outdir)
{
   StructFOFSpec_p ctrl;
   long variant,
      solved_count,
      var_count,
      prob_count,
      concrete_prob_count,
      i,
      now,
      remaining,
      per_prob_time;
   PDArray_p solved = PDIntArrayAlloc(10,0);
   char      *abstract_name, *concrete_name;
   DStr_p dest_name = DStrAlloc();
   bool success;
   char* save_exec;

   solved_count = 0;
   prob_count   = PStackGetSP(spec->source_files);
   var_count    = StringArrayCardinality(variants);

   concrete_prob_count = prob_count*var_count;
   fprintf(GlobalOut,
           COMCHAR" Initial: %ld abstract problems, %ld variants, %ld concrete problems\n",
           prob_count, var_count, concrete_prob_count);
   for(variant = 0; variants[variant]; variant++)
   {
      now = GetSecTime();
      save_exec = spec->executable;
      spec->executable = provers[variant];
      remaining = spec->total_wtc_limit-(now-start);
      concrete_prob_count = (prob_count-solved_count)*(var_count-variant);

      fprintf(GlobalOut, COMCHAR" Round %ld, working on variant %s, remaining time %lds\n",
              variant, variants[variant], remaining);

      fprintf(GlobalOut, COMCHAR" %ld unsolved abstract problems, %ld remaining variants,"
              " %ld concrete problems\n",
              prob_count-solved_count, var_count-variant, concrete_prob_count);

      // Loading axioms here!
      // DISABLED FOR CASC-28 - no shared axioms. Inconsistent Spec!
      //ctrl = StructFOFSpecAlloc();
      //concrete_batch_struct_FOF_spec_init(spec,
      //                                   ctrl,
      //                                   default_dir,
      //                                  variants[variant]);

      for(i=0; i<PStackGetSP(spec->source_files); i++)
      {
         // CASC-28/J10-Hack
         ctrl = StructFOFSpecAlloc();
         concrete_batch_struct_FOF_spec_init(spec,
                                             ctrl,
                                             default_dir,
                                             variants[variant]);
         // CASC-28/J10-Hack ends
         abstract_name = PStackElementP(spec->source_files, i);
         if(PDArrayElementInt(solved, i))
         {
            fprintf(GlobalOut, COMCHAR" Abstract problem %s already solved\n",
                    abstract_name);
         }
         else
         {
            now = GetSecTime();
            remaining = spec->total_wtc_limit-(now-start);
            per_prob_time = (remaining/concrete_prob_count)+1;

            concrete_name = abstract_to_concrete(abstract_name, variants[variant], ".p");
            fprintf(GlobalOut, COMCHAR" Trying abstract problem %s via %s for %lds\n",
                    abstract_name, concrete_name, per_prob_time);

            DStrReset(dest_name);
            if(outdir)
            {
               DStrSet(dest_name, outdir);
               DStrAppendChar(dest_name, '/');
            }
            DStrAppendStr(dest_name, PStackElementP(spec->dest_files, i));


            fprintf(GlobalOut, "\n"COMCHAR" Processing %s -> %s\n",
                    concrete_name, DStrView(dest_name));
            fprintf(GlobalOut, COMCHAR" SZS status Started for %s\n", concrete_name);
            fflush(GlobalOut);


            EGPCtrl_p handle = EGPCtrlCreate("E-LTB wrapper", 1, 1000000);
            char buffer[EGPCTRL_BUFSIZE];
            if(!handle)
            {
               // It's the wrapped child, do work
               success = BatchProcessFile(spec, per_prob_time,
                                          ctrl, default_dir,
                                          concrete_name,
                                          DStrView(dest_name));
               exit(success);
            }
            // else
            while(!EGPCtrlGetResult(handle, buffer,EGPCTRL_BUFSIZE))
            {
               // Nothing
            }
            success = (handle->result==PRTheorem)||(handle->result==PRUnsatisfiable);
            fprintf(GlobalOut, "%s", DStrView(handle->output));
            EGPCtrlFree(handle);

            if(success)
            {
               solved_count++;
               PDArrayAssignInt(solved, i, 1);
               concrete_prob_count -= (var_count-variant);
               //printf(COMCHAR" SUCCESS: %ld abstract solved, %ld concrete remaining\n",
               //solved_count, concrete_prob_count);
            }
            else
            {
               concrete_prob_count--;
               //printf(COMCHAR" FAILURE: %ld abstract solved, %ld concrete remaining\n",
               //solved_count, concrete_prob_count);
            }
            fprintf(GlobalOut, COMCHAR" SZS status Ended for %s\n\n", concrete_name);
            fflush(GlobalOut);

            FREE(concrete_name);
         }
         // CASC-28-Hack
         StructFOFSpecFree(ctrl);
      }
      spec->executable = save_exec;
      // See above - disabled for CASC-28
      //StructFOFSpecFree(ctrl);
   }
   DStrFree(dest_name);
   PDArrayFree(solved);
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
