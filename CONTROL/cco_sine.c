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

   sig =   SigAlloc();
   SigInsertInternalCodes(sig);
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
// Function: 
//
//   
//
// Global Variables: 
//
// Side Effects    : 
//
/----------------------------------------------------------------------*/

static AxFilter_p sine_get_filter(char* fname, AxFilterSet_p *filters)
{
   AxFilter_p    filter;
   Scanner_p     in;

   in = CreateScanner(StreamTypeInternalString, fname, true, NULL);
   CheckInpTok(in, Name);
   if(TestTok(LookToken(in,1), NoToken))
   {
      *filters = AxFilterSetCreateInternal(AxFilterDefaultSet);
      filter = AxFilterSetFindFilter(*filters, fname);
      if(!filter)
      {
         Error("Unknown SinE-filter '%s' selected",USAGE_ERROR, fname);
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
      fname = "gf500_gu_R04_F100_L20000";
   }
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
   GCDeregisterFormulaSet(state->gc_original_terms, state->f_axioms);
   GCDeregisterClauseSet(state->gc_original_terms, state->axioms);   

   /* ...so we need to povide fresh, empty axioms sets */
   state->axioms   = ClauseSetAlloc();
   state->f_axioms = FormulaSetAlloc();
   GCRegisterFormulaSet(state->gc_original_terms, state->f_axioms);
   GCRegisterClauseSet(state->gc_original_terms, state->axioms);
  
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


