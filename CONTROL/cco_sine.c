/*-----------------------------------------------------------------------

File  : cco_sine.c

Author: Stephan Schulz (schulz@eprover.org)

Contents

  SinE-like specification filtering.

  Copyright 2012 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Thu May 10 15:39:26 CEST 2012
    New

-----------------------------------------------------------------------*/

#include "cco_sine.h"




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
// Function: sine_get_filter()
//
//   Given a filter string (a definition or a name), return the
//   described filter. Initialize  filters with a set of filters
//   including the described one.
//
// Global Variables: -
//
// Side Effects    : Termination in error case.
//
/----------------------------------------------------------------------*/

static AxFilter_p sine_get_filter(char* fname, AxFilterSet_p *filters)
{
   AxFilter_p    filter;
   Scanner_p     in;

   in = CreateScanner(StreamTypeOptionString, fname, true, NULL);
   CheckInpTok(in, Name);
   if(TestTok(LookToken(in,1), NoToken))
   {
      *filters = AxFilterSetCreateInternal(AxFilterDefaultSet);
      filter = AxFilterSetFindFilter(*filters, fname);
      if(!filter)
      {
         DStr_p fstring = DStrAlloc();

         AxFilterSetAddNames(fstring, *filters);
         Error("Unknown SinE-filter '%s' selected (valid choices: %s)",
               USAGE_ERROR,
               fname,
               DStrView(fstring));
         DStrFree(fstring);
      }
   }
   else
   {
      *filters = AxFilterSetAlloc();
      filter  = AxFilterDefParse(in);
      AxFilterSetAddFilter(*filters, filter);
   }
   DestroyScanner(in);
   return filter;
}


/*-----------------------------------------------------------------------
//
// The following variable settings describe the association of (raw)
// problem class and SInE-Strategy. This is automatically generated
// from suitable test runs. Note that the named strategies must be
// described in che_axfilter.c
//
/----------------------------------------------------------------------*/

/* -------------------------------------------------------*/
/* The following code is generated automagically with     */
/* generate_auto.py. Yes, it is fairly ugly ;-)           */
/* -------------------------------------------------------*/

/* Class dir used:  */


/* CLASS_LMSSMLL     : protokoll_X----_auto_300       0    */
/* CLASS_MMSSMSL     : protokoll_X----_auto_300       3    */
/* CLASS_LMLSMSL     : protokoll_X----_auto_sine16    20   */
/* CLASS_MMSSMSM     : protokoll_X----_auto_300       18   */
/* CLASS_MSSMLSM     : protokoll_X----_auto_300       0    */
/* CLASS_LMLSLLL     : protokoll_X----_auto_sine31    1    */
/* CLASS_MMSMMSM     : protokoll_X----_auto_300       13   */
/* CLASS_LMMSMLL     : protokoll_X----_auto_sine03    24   */
/* CLASS_LLLLMLL     : protokoll_X----_auto_sine11    76   */
/* CLASS_MSLMMSL     : protokoll_X----_auto_300       47   */
/* CLASS_MSLSMSL     : protokoll_X----_auto_sine11    209  */
/* CLASS_SSSSLSM     : protokoll_X----_auto_300       205  */
/* CLASS_SSSSLSL     : protokoll_X----_auto_300       23   */
/* CLASS_SSSSMSL     : protokoll_X----_auto_300       216  */
/* CLASS_SSSSMSM     : protokoll_X----_auto_300       2302 */
/* CLASS_SSSSMSS     : protokoll_X----_auto_300       6294 */
/* CLASS_SMSSMSM     : protokoll_X----_auto_sine16    4    */
/* CLASS_LSMSMSL     : protokoll_X----_auto_sine18    23   */
/* CLASS_SSSSLSS     : protokoll_X----_auto_NIX       154  */
/* CLASS_LLLSMSL     : protokoll_X----_auto_sine17    3    */
/* CLASS_MSSSMML     : protokoll_X----_auto_sine13    6    */
/* CLASS_MSSSMSS     : protokoll_X----_auto_sine17    12   */
/* CLASS_LMLMMLL     : protokoll_X----_auto_sine12    7    */
/* CLASS_LMLMLLL     : protokoll_X----_auto_sine13    23   */
/* CLASS_LSLSMSL     : protokoll_X----_auto_sine11    19   */
/* CLASS_MMSSMLL     : protokoll_X----_auto_sine31    2    */
/* CLASS_MSSSMSL     : protokoll_X----_auto_sine17    14   */
/* CLASS_MSSSMSM     : protokoll_X----_auto_300       348  */
/* CLASS_LSSSMSM     : protokoll_X----_auto_300       4    */
/* CLASS_MSMSMSL     : protokoll_X----_auto_300       22   */
/* CLASS_LMLLMSL     : protokoll_X----_auto_300       42   */
/* CLASS_MSSSLSM     : protokoll_X----_auto_300       26   */
/* CLASS_SSSSMLL     : protokoll_X----_auto_300       1    */
/* CLASS_LLLMLLL     : protokoll_X----_auto_sine31    10   */
/* CLASS_LMMMMLL     : protokoll_X----_auto_sine16    6    */
/* CLASS_MSSSMLL     : protokoll_X----_auto_sine12    24   */
/* CLASS_MMMSMLL     : protokoll_X----_auto_sine12    2    */
/* Raw association */
char* raw_class[] =
{
   "LMSSMLL",  /*      0 protokoll_X----_auto_300 */
   "MMSSMSL",  /*      3 protokoll_X----_auto_300 */
   "LMLSMSL",  /*     20 protokoll_X----_auto_sine16 */
   "MMSSMSM",  /*     18 protokoll_X----_auto_300 */
   "MSSMLSM",  /*      0 protokoll_X----_auto_300 */
   "LMLSLLL",  /*      1 protokoll_X----_auto_sine31 */
   "MMSMMSM",  /*     13 protokoll_X----_auto_300 */
   "LMMSMLL",  /*     24 protokoll_X----_auto_sine03 */
   "LLLLMLL",  /*     76 protokoll_X----_auto_sine11 */
   "MSLMMSL",  /*     47 protokoll_X----_auto_300 */
   "MSLSMSL",  /*    209 protokoll_X----_auto_sine11 */
   "SSSSLSM",  /*    205 protokoll_X----_auto_300 */
   "SSSSLSL",  /*     23 protokoll_X----_auto_300 */
   "SSSSMSL",  /*    216 protokoll_X----_auto_300 */
   "SSSSMSM",  /*   2302 protokoll_X----_auto_300 */
   "SSSSMSS",  /*   6294 protokoll_X----_auto_300 */
   "SMSSMSM",  /*      4 protokoll_X----_auto_sine16 */
   "LSMSMSL",  /*     23 protokoll_X----_auto_sine18 */
   "SSSSLSS",  /*    154 protokoll_X----_auto_NIX */
   "LLLSMSL",  /*      3 protokoll_X----_auto_sine17 */
   "MSSSMML",  /*      6 protokoll_X----_auto_sine13 */
   "MSSSMSS",  /*     12 protokoll_X----_auto_sine17 */
   "LMLMMLL",  /*      7 protokoll_X----_auto_sine12 */
   "LMLMLLL",  /*     23 protokoll_X----_auto_sine13 */
   "LSLSMSL",  /*     19 protokoll_X----_auto_sine11 */
   "MMSSMLL",  /*      2 protokoll_X----_auto_sine31 */
   "MSSSMSL",  /*     14 protokoll_X----_auto_sine17 */
   "MSSSMSM",  /*    348 protokoll_X----_auto_300 */
   "LSSSMSM",  /*      4 protokoll_X----_auto_300 */
   "MSMSMSL",  /*     22 protokoll_X----_auto_300 */
   "LMLLMSL",  /*     42 protokoll_X----_auto_300 */
   "MSSSLSM",  /*     26 protokoll_X----_auto_300 */
   "SSSSMLL",  /*      1 protokoll_X----_auto_300 */
   "LLLMLLL",  /*     10 protokoll_X----_auto_sine31 */
   "LMMMMLL",  /*      6 protokoll_X----_auto_sine16 */
   "MSSSMLL",  /*     24 protokoll_X----_auto_sine12 */
   "MMMSMLL",  /*      2 protokoll_X----_auto_sine12 */
   NULL
};
char* raw_sine[] =
{
   NULL,
   NULL,
   "gf200_h_gu_R03_F100_L20000",
   NULL,
   NULL,
   "gf120_h_gu_RUU_F100_L01000",
   NULL,
   "gf120_gu_R02_F100_L20000",
   "gf500_h_gu_R04_F100_L20000",
   NULL,
   "gf500_h_gu_R04_F100_L20000",
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   "gf200_h_gu_R03_F100_L20000",
   "gf200_h_gu_RUU_F100_L20000",
   NULL,
   "gf600_h_gu_R05_F100_L20000",
   "gf120_h_gu_R02_F100_L20000",
   "gf600_h_gu_R05_F100_L20000",
   "gf120_h_gu_RUU_F100_L00500",
   "gf120_h_gu_R02_F100_L20000",
   "gf500_h_gu_R04_F100_L20000",
   "gf120_h_gu_RUU_F100_L01000",
   "gf600_h_gu_R05_F100_L20000",
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   "gf120_h_gu_RUU_F100_L01000",
   "gf200_h_gu_R03_F100_L20000",
   "gf120_h_gu_RUU_F100_L00500",
   "gf120_h_gu_RUU_F100_L00500",
   NULL
};
/* Predicted solutions: 10203 */


/*-----------------------------------------------------------------------
//
// Function: find_auto_sine()
//
//   Given a proof state, return the name of the "best" SInE-Strategy,
//   or NULL if SInE is not recommended.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static char* find_auto_sine(ProofState_p state)
{
   SpecLimitsCell      limits;
   RawSpecFeatureCell features;
   int i;

   limits.ax_some_limit        = 1199;
   limits.ax_many_limit        = 10396;
   limits.term_medium_limit    = 664277;
   limits.term_large_limit     = 5573560;
   limits.symbols_medium_limit = 2471;
   limits.symbols_large_limit  = 4140;

   limits.predc_medium_limit     = 0;
   limits.predc_large_limit      = 2;
   limits.pred_medium_limit      = 1225;
   limits.pred_large_limit       = 4000;
   limits.func_medium_limit      = 8;
   limits.func_large_limit       = 110;
   limits.fun_medium_limit       = 360;
   limits.fun_large_limit        = 400;


   RawSpecFeaturesCompute(&features, state);
   RawSpecFeaturesClassify(&features, &limits, "aaaaaaa");

   /* Hard-coded exception - no conjecture & no hypotheses == no
      useful SInE! */
   if(!(features.conjecture_count+features.hypothesis_count))
   {
      return NULL;
   }

   for(i=0; raw_class[i]; i++)
   {
      if(strcmp(raw_class[i], features.class)==0)
      {
         return raw_sine[i];
      }
   }
   return NULL;
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/




/*-----------------------------------------------------------------------
//
// Function: StructFOFSpecCreate()
//
//   Create a FOF spec, given the term bank (and thus the sig).
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

StructFOFSpec_p StructFOFSpecCreate(TB_p terms)
{
   StructFOFSpec_p handle = StructFOFSpecCellAlloc();

   handle->sig             = terms->sig;
   handle->terms           = terms;
   handle->clause_sets     = PStackAlloc();
   handle->formula_sets    = PStackAlloc();
   handle->parsed_includes = NULL;
   handle->f_distrib       = GenDistribAlloc(handle->sig);
   handle->shared_ax_sp    = 0;

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
   Sig_p sig;
   TB_p  terms;
   SortTable_p sort_table;

   sort_table = DefaultSortTableAlloc();
   sig =   SigAlloc(sort_table);
   SigInsertInternalCodes(sig);
   /* We assume free numers for LTB to avoid problems with the
      TFF-enabled parser */
   sig->distinct_props = sig->distinct_props&~
      (FPIsInteger|FPIsRational|FPIsFloat);

   terms = TBAlloc(sig);
   return StructFOFSpecCreate(terms);
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

   if(ctrl->sig)
   {
      if(ctrl->sig->sort_table)
      {
         SortTableFree(ctrl->sig->sort_table);
         ctrl->sig->sort_table = NULL;
      }
      SigFree(ctrl->sig);
      ctrl->terms->sig = NULL;
   }
   if(ctrl->terms)
   {
      TBFree(ctrl->terms);
      ctrl->terms = NULL;
   }
   StrTreeFree(ctrl->parsed_includes);
   GenDistribFree(ctrl->f_distrib);

   StructFOFSpecCellFree(ctrl);
}


/*-----------------------------------------------------------------------
//
// Function: StructFOFSpecCollectFCode()
//
//   Push all formulas that contain f_code onto result. Return number
//   of formulas found. Ignores clauses (clauses are deprecated here).
//
// Global Variables: -
//
// Side Effects    : Only via PStackPushP()
//
/----------------------------------------------------------------------*/

long StructFOFSpecCollectFCode(StructFOFSpec_p ctrl,
                               FunCode f_code,
                               PStack_p res_formulas)
{
   long ret = 0;
   FormulaSet_p handle;
   PStackPointer i;

   for(i = 0;
       i <  PStackGetSP(ctrl->formula_sets);
       i++)
   {
      handle = PStackElementP(ctrl->formula_sets, i);
      ret += FormulaSetCollectFCode(handle, f_code, res_formulas);
   }
   return ret;
}


/*-----------------------------------------------------------------------
//
// Function: StructFOFSpecParseAxioms()
//
//   Initialize a StructFOFSpeclCell by parsing all the include files
//   in axfiles.
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
   static IntOrP dummy = {0};


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
         assert(ClauseSetCardinality(cset)==0);
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
// Function: ProofStateSinE()
//
//   Apply SinE with the specified filter to the proofstate (in
//   particular state->f_axioms and state->axioms). This is
//   destructive. Returns number of axioms deleted.
//
// Global Variables: -
//
// Side Effects    : Memory operations aplenty.
//
/----------------------------------------------------------------------*/

long ProofStateSinE(ProofState_p state, char* fname)
{
   long            axno_orig, axno;
   AxFilterSet_p   filters = NULL;
   AxFilter_p      filter;
   StructFOFSpec_p spec;
   PStack_p        clauses, formulas;

   if(!fname)
   {
      return 0;
   }

   if(strcmp(fname, "Auto")==0)
   {
      fname = find_auto_sine(state);
   }

   if(!fname)
   {
      printf("# No SInE strategy applied\n");
      return 0;
   }
   printf("# SinE strategy is %s\n", fname);

   filter = sine_get_filter(fname, &filters);

   axno_orig = ClauseSetCardinality(state->axioms)+
      FormulaSetCardinality(state->f_axioms);

   /* printf("# Magic happens here %s!\n", fname);
      printf("# Filter: ");
      AxFilterPrint(stdout, filter);
      printf("\n"); */

   formulas = PStackAlloc();
   clauses  = PStackAlloc();
   spec = StructFOFSpecCreate(state->terms);

   /* The following moves the responsibility for the sets into the spec! */
   StructFOFSpecAddProblem(spec, state->axioms, state->f_axioms);
   GCDeregisterFormulaSet(state->gc_terms, state->f_axioms);
   GCDeregisterClauseSet(state->gc_terms, state->axioms);

   /* ...so we need to povide fresh, empty axioms sets */
   state->axioms   = ClauseSetAlloc();
   state->f_axioms = FormulaSetAlloc();
   GCRegisterFormulaSet(state->gc_terms, state->f_axioms);
   GCRegisterClauseSet(state->gc_terms, state->axioms);

   StructFOFSpecInitDistrib(spec);
   StructFOFSpecGetProblem(spec,
                           filter,
                           clauses,
                           formulas);

   PStackClausesMove(clauses, state->axioms);
   PStackFormulasMove(formulas, state->f_axioms);
   PStackFree(formulas);
   PStackFree(clauses);

   /* Now rescue signature and term bank. */
   spec->sig = NULL;
   spec->terms = NULL;
   StructFOFSpecFree(spec);

   AxFilterSetFree(filters);

   axno = ClauseSetCardinality(state->axioms)+
      FormulaSetCardinality(state->f_axioms);

   return axno_orig-axno;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
