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

   in = CreateScanner(StreamTypeOptionString, fname, true, NULL, true);
   CheckInpTok(in, Name);
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
   DestroyScanner(in);
   return filter;
}

#ifdef NEVER_DEFINED

static AxFilter_p sine_get_filter(char* fname, AxFilterSet_p *filters)
{
   AxFilter_p    filter;
   Scanner_p     in;

   in = CreateScanner(StreamTypeOptionString, fname, true, NULL, true);
   CheckInpTok(in, Name);
   if(!TestInpId(in, "LambdaDef") && TestTok(LookToken(in,1), NoToken))
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

#endif

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
   "-LMSSMLL",  /*      0 protokoll_X----_auto_300 */
   "-MMSSMSL",  /*      3 protokoll_X----_auto_300 */
   "-LMLSMSL",  /*     20 protokoll_X----_auto_sine16 */
   "-MMSSMSM",  /*     18 protokoll_X----_auto_300 */
   "-MSSMLSM",  /*      0 protokoll_X----_auto_300 */
   "-LMLSLLL",  /*      1 protokoll_X----_auto_sine31 */
   "-MMSMMSM",  /*     13 protokoll_X----_auto_300 */
   "-LMMSMLL",  /*     24 protokoll_X----_auto_sine03 */
   "-LLLLMLL",  /*     76 protokoll_X----_auto_sine11 */
   "-MSLMMSL",  /*     47 protokoll_X----_auto_300 */
   "-MSLSMSL",  /*    209 protokoll_X----_auto_sine11 */
   "-SSSSLSM",  /*    205 protokoll_X----_auto_300 */
   "-SSSSLSL",  /*     23 protokoll_X----_auto_300 */
   "-SSSSMSL",  /*    216 protokoll_X----_auto_300 */
   "-SSSSMSM",  /*   2302 protokoll_X----_auto_300 */
   "-SSSSMSS",  /*   6294 protokoll_X----_auto_300 */
   "-SMSSMSM",  /*      4 protokoll_X----_auto_sine16 */
   "-LSMSMSL",  /*     23 protokoll_X----_auto_sine18 */
   "-SSSSLSS",  /*    154 protokoll_X----_auto_NIX */
   "-LLLSMSL",  /*      3 protokoll_X----_auto_sine17 */
   "-MSSSMML",  /*      6 protokoll_X----_auto_sine13 */
   "-MSSSMSS",  /*     12 protokoll_X----_auto_sine17 */
   "-LMLMMLL",  /*      7 protokoll_X----_auto_sine12 */
   "-LMLMLLL",  /*     23 protokoll_X----_auto_sine13 */
   "-LSLSMSL",  /*     19 protokoll_X----_auto_sine11 */
   "-MMSSMLL",  /*      2 protokoll_X----_auto_sine31 */
   "-MSSSMSL",  /*     14 protokoll_X----_auto_sine17 */
   "-MSSSMSM",  /*    348 protokoll_X----_auto_300 */
   "-LSSSMSM",  /*      4 protokoll_X----_auto_300 */
   "-MSMSMSL",  /*     22 protokoll_X----_auto_300 */
   "-LMLLMSL",  /*     42 protokoll_X----_auto_300 */
   "-MSSSLSM",  /*     26 protokoll_X----_auto_300 */
   "-SSSSMLL",  /*      1 protokoll_X----_auto_300 */
   "-LLLMLLL",  /*     10 protokoll_X----_auto_sine31 */
   "-LMMMMLL",  /*      6 protokoll_X----_auto_sine16 */
   "-MSSSMLL",  /*     24 protokoll_X----_auto_sine12 */
   "-MMMSMLL",  /*      2 protokoll_X----_auto_sine12 */
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


   const char* SINE_MASK = "-aaaaaaa";
   const int SINE_CLASS_LEN = strlen(SINE_MASK);
   RawSpecFeaturesCompute(&features, state);
   RawSpecFeaturesClassify(&features, &limits, (char*)SINE_MASK);

   /* Hard-coded exception - no conjecture & no hypotheses == no
      useful SInE! */
   if(!(features.conjecture_count+features.hypothesis_count))
   {
      return NULL;
   }

   for(i=0; raw_class[i]; i++)
   {
      if(strncmp(raw_class[i], features.class, SINE_CLASS_LEN)==0)
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

   //handle->sig             = terms->sig;
   handle->terms           = terms;
   //handle->gc_terms        = NULL;
   handle->clause_sets     = PStackAlloc();
   handle->formula_sets    = PStackAlloc();
   handle->parsed_includes = NULL;
   handle->f_distrib       = GenDistribAlloc(handle->terms->sig);
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
   TypeBank_p sort_table;
   StructFOFSpec_p res;

   sort_table = TypeBankAlloc();
   sig =   SigAlloc(sort_table);
   SigInsertInternalCodes(sig);
   /* We assume free numers for LTB to avoid problems with the
      TFF-enabled parser */
   sig->distinct_props = sig->distinct_props&~
      (FPIsInteger|FPIsRational|FPIsFloat);

   terms         = TBAlloc(sig);
   res           = StructFOFSpecCreate(terms);
   // res->gc_terms = GCAdminAlloc(terms);
   return res;
}




/*-----------------------------------------------------------------------
//
// Function: StructFOFSpecDestroy()
//
//   Dissassemble and Free the FOFSpec, but leave term bank and
//   signature alone.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void StructFOFSpecDestroy(StructFOFSpec_p ctrl)
{
   FormulaSet_p fset;
   ClauseSet_p  cset;

   while(!PStackEmpty(ctrl->clause_sets))
   {
      cset = PStackPopP(ctrl->clause_sets);
      GCDeregisterClauseSet(ctrl->terms->gc, cset);
      ClauseSetFree(cset);
   }
   PStackFree(ctrl->clause_sets);

   while(!PStackEmpty(ctrl->formula_sets))
   {
      fset = PStackPopP(ctrl->formula_sets);
      GCDeregisterFormulaSet(ctrl->terms->gc, fset);
      FormulaSetFree(fset);
   }
   PStackFree(ctrl->formula_sets);
   StrTreeFree(ctrl->parsed_includes);
   GenDistribFree(ctrl->f_distrib);

   StructFOFSpecCellFree(ctrl);
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
   TB_p  terms            = ctrl->terms;
   Sig_p sig              = terms->sig;
   TypeBank_p sort_table  = sig->type_bank;

   StructFOFSpecDestroy(ctrl);

   TypeBankFree(sort_table);
   SigFree(sig);
   terms->sig = NULL;
   TBFree(terms);
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
                              IOFormat parse_format, char* default_dir)
{
   PStackPointer i;
   char*        iname;
   FormulaSet_p fset;
   ClauseSet_p  cset;
   Scanner_p    in;
   long         res = 0;
   static IntOrP dummy = {0};

   //printf(COMCHAR" XXX Called with %ld axiom files\n", PStackGetSP(axfiles));
   for(i=0; i<PStackGetSP(axfiles); i++)
   {
      iname = PStackElementP(axfiles, i);
      if(!StrTreeFind(&(ctrl->parsed_includes), iname))
      {
         //printf("Calling with %s\n", iname);
         in = CreateScanner(StreamTypeFile, iname, true, default_dir, false);
         //printf("Result: %p\n", in);
         if(in)
         {
            ScannerSetFormat(in, parse_format);

            fprintf(GlobalOut, COMCHAR" Parsing %s\n", iname);
            cset = ClauseSetAlloc();
            fset = FormulaSetAlloc();
            TBGCRegisterFormulaSet(ctrl->terms, fset);
            TBGCRegisterClauseSet(ctrl->terms, cset);
            res += FormulaAndClauseSetParse(in, fset, cset, ctrl->terms,
                                            NULL,
                                            &(ctrl->parsed_includes));
            assert(ClauseSetCardinality(cset)==0);
            PStackPushP(ctrl->clause_sets, cset);
            PStackPushP(ctrl->formula_sets, fset);
            // printf(COMCHAR" %s has %ld formulas\n", iname, FormulaSetCardinality(fset));
            StrTreeStore(&(ctrl->parsed_includes), iname, dummy, dummy);

            DestroyScanner(in);
            //GenDistribSizeAdjust(ctrl->f_distrib, ctrl->terms->sig);
            //GenDistribAddClauseSet(ctrl->f_distrib, clauses, 1);
            //GenDistribAddFormulaSet(ctrl->f_distrib, formulas, trim, 1);
        }
         else
         {
            fprintf(GlobalOut, COMCHAR" Could not find %s\n", iname);
         }
      }
   }
   ctrl->shared_ax_sp = PStackGetSP(ctrl->clause_sets);
   ctrl->shared_ax_f_count = SigGetFCount(ctrl->terms->sig);

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

void StructFOFSpecInitDistrib(StructFOFSpec_p ctrl, bool trim)
{
   GenDistribSizeAdjust(ctrl->f_distrib, ctrl->terms->sig);
   GenDistribAddClauseSets(ctrl->f_distrib, ctrl->clause_sets);
   GenDistribAddFormulaSets(ctrl->f_distrib, ctrl->formula_sets, trim);
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
      fprintf(GlobalOut, COMCHAR" No SInE strategy applied\n");
      return 0;
   }
   fprintf(GlobalOut, COMCHAR" SinE strategy is %s\n", fname);

   filter = sine_get_filter(fname, &filters);

   axno_orig = ClauseSetCardinality(state->axioms)+
      FormulaSetCardinality(state->f_axioms);

   /* printf(COMCHAR" Magic happens here %s!\n", fname);
      printf(COMCHAR" Filter: ");
      AxFilterPrint(stdout, filter);
      printf("\n"); */

   formulas = PStackAlloc();
   clauses  = PStackAlloc();
   GCDeregisterFormulaSet(state->terms->gc, state->f_axioms);
   GCDeregisterClauseSet(state->terms->gc, state->axioms);

   spec = StructFOFSpecCreate(state->terms);
   StructFOFSpecAddProblem(spec, state->axioms, state->f_axioms, filter->trim_implications);


   //StructFOFSpecInitDistrib(spec, filter->trim_implications);

   //GenDistribPrint(stdout, spec->f_distrib, 10);
   StructFOFSpecGetProblem(spec,
                           filter,
                           clauses,
                           formulas);

   state->axioms   = ClauseSetAlloc();
   state->f_axioms = FormulaSetAlloc();
   TBGCRegisterFormulaSet(state->terms, state->f_axioms);
   TBGCRegisterClauseSet(state->terms, state->axioms);
   PStackClausesMove(clauses, state->axioms);
   PStackFormulasMove(formulas, state->f_axioms);
   PStackFree(formulas);
   PStackFree(clauses);
   /* ...so we need to povide fresh, empty axioms sets */

   /* Now rescue signature and term bank. */
   // spec->sig = NULL;
   // spec->terms = NULL;
   StructFOFSpecDestroy(spec);

   AxFilterSetFree(filters);

   axno = ClauseSetCardinality(state->axioms)+
      FormulaSetCardinality(state->f_axioms);

   //printf(COMCHAR" ...ProofStateSinE()=%ld/%ld\n", axno, axno_orig);

   return axno_orig-axno;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
