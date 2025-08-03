/*-----------------------------------------------------------------------

File  : ccl_inferencedoc.c

Author: Stephan Schulz

Contents

  Functions and constants for reporting on the proof process.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Tue Jan  5 16:15:48 MET 1999
    New

-----------------------------------------------------------------------*/

#include "ccl_inferencedoc.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


OutputFormatType DocOutputFormat   = no_format;
bool             PCLFullTerms   = true;
bool             PCLStepCompact = false;
int              PCLShellLevel = 0;

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                    Internal Functions -- Clauses                    */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: PCLTypeStr()
//
//   Given an E-internal type of clause, return a string describing
//   the type (default type is plain/ax and is represented by the
//   empty string).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

char* PCLTypeStr(FormulaProperties type)
{
   char *res;

   switch(type)
   {
   case CPTypeConjecture:
         res = "conj";
         break;
   case CPTypeQuestion:
         res = "que";
         break;
   case CPTypeNegConjecture:
         res = "neg";
         break;
    default:
         res = "";
         break;
   }

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: pcl_print_start()
//
//   Print the "<id> :<type> : <clause> : " part of a pcl step.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void pcl_print_start(FILE* out, Clause_p clause, bool print_clause)
{
   fprintf(out, PCLStepCompact?"%ld:":"%6ld : ", clause->ident);
   fprintf(out, "%s:", PCLTypeStr(ClauseQueryTPTPType(clause)));
   if(print_clause)
   {
      ClausePCLPrint(out, clause, PCLFullTerms);
   }
   fputs(" : ", out);
}

/*-----------------------------------------------------------------------
//
// Function: pcl_print_end()
//
//   Print the optional comment and new line
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void pcl_print_end(FILE* out, char* comment, Clause_p clause)
{
   if(ClauseQueryProp(clause, CPWatchOnly)&&comment)
   {
      fprintf(out, PCLStepCompact?":'wl,%s'" : ": 'wl,%s'",
         comment);
   }
   else if(comment)
   {
      fprintf(out, PCLStepCompact?":'%s'":" : '%s'",comment);
   }
   else if(ClauseQueryProp(clause, CPWatchOnly))
   {
      fprintf(out, PCLStepCompact?":'wl'":" : 'wl'");
   }
   fputc('\n', out);
}


/*-----------------------------------------------------------------------
//
// Function: tstp_print_end()
//
//   Print the optional comment and new line
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void tstp_print_end(FILE* out, char* comment, Clause_p clause)
{
   if(ClauseQueryProp(clause, CPWatchOnly)&&comment)
   {
      fprintf(out, ",['wl,%s']", comment);
   }
   else if(comment)
   {
      fprintf(out, ",['%s']", comment);
   }
   else if(ClauseQueryProp(clause, CPWatchOnly))
   {
      fprintf(out, ",['wl']");
   }
   fputs(").\n", out);
}


/*-----------------------------------------------------------------------
//
// Function: print_initial()
//
//   Print an initial clause (axiom).
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void print_initial(FILE* out, Clause_p clause, char* comment)
{
   switch(DocOutputFormat)
   {
   case pcl_format:
         printf("XX\n");
         pcl_print_start(out, clause, PCLShellLevel<2);
         printf("XX\n");
         ClauseSourceInfoPrintPCL(out, clause->info);
         pcl_print_end(out, comment, clause);
         break;
   case tstp_format:
         ClauseTSTPPrint(out, clause, PCLFullTerms, false);
         fprintf(out, ", ");
         ClauseSourceInfoPrintTSTP(out, clause->info);
         tstp_print_end(out, comment, clause);
         break;
   default:
         fprintf(out, COMCHAR" Output format not implemented.\n");
    break;
   }
}



/*-----------------------------------------------------------------------
//
// Function: print_paramod()
//
//   Print a clause creation by (simultaneous) paramodulation (or
//   superposition).
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void print_paramod(FILE* out, Clause_p clause, Clause_p
           parent1, Clause_p parent2, char* inf, char* comment)
{
   switch(DocOutputFormat)
   {
   case pcl_format:
    pcl_print_start(out, clause, PCLShellLevel<1);
    fprintf(out, "%s(%ld,%ld)", inf, parent1->ident,
       parent2->ident);
    pcl_print_end(out, comment, clause);
    break;
   case tstp_format:
    ClauseTSTPPrint(out, clause, PCLFullTerms, false);
    fprintf(out,
       ",inference(%s,[status(thm)],[c_0_%ld,c_0_%ld])",
                 inf,
       parent1->ident,
       parent2->ident);
    tstp_print_end(out, comment, clause);
    break;
   default:
    fprintf(out, COMCHAR" Output format not implemented.\n");
    break;
   }
}


/*-----------------------------------------------------------------------
//
// Function: print_eres()
//
//   Print a clause creation by equality resolution.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void print_eres(FILE* out, Clause_p clause, Clause_p
           parent1, char* comment)
{
   switch(DocOutputFormat)
   {
   case pcl_format:
    pcl_print_start(out, clause, PCLShellLevel<1);
    fprintf(out, PCL_ER"(%ld)", parent1->ident);
    pcl_print_end(out, comment, clause);
    break;
   case tstp_format:
    ClauseTSTPPrint(out, clause, PCLFullTerms, false);
    fprintf(out,
       ",inference("PCL_ER",[status(thm)],[c_0_%ld])",
       parent1->ident);
    tstp_print_end(out, comment, clause);
    break;
   default:
    fprintf(out, COMCHAR" Output format not implemented.\n");
    break;
   }
}


/*-----------------------------------------------------------------------
//
// Function: print_des_eres()
//
//   Print a clause modification by destructive equality resolution.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void print_des_eres(FILE* out, Clause_p clause, long old_id,
                           char* comment)
{
   switch(DocOutputFormat)
   {
   case pcl_format:
    pcl_print_start(out, clause, PCLShellLevel<1);
    fprintf(out, PCL_ER"(%ld)", old_id);
    pcl_print_end(out, comment, clause);
    break;
   case tstp_format:
    ClauseTSTPPrint(out, clause, PCLFullTerms, false);
    fprintf(out,
       ",inference("PCL_ER",[status(thm)],[c_0_%ld])",
       old_id);
    tstp_print_end(out, comment, clause);
    break;
   default:
    fprintf(out, COMCHAR" Output format not implemented.\n");
    break;
   }
}


/*-----------------------------------------------------------------------
//
// Function: print_efactor()
//
//   Print a clause creation by equality factoring.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void print_efactor(FILE* out, Clause_p clause, Clause_p
           parent1, char* comment)
{
   switch(DocOutputFormat)
   {
   case pcl_format:
    pcl_print_start(out, clause, PCLShellLevel<1);
    fprintf(out, PCL_EF"(%ld)", parent1->ident);
    pcl_print_end(out, comment, clause);
    break;
   case tstp_format:
    ClauseTSTPPrint(out, clause, PCLFullTerms, false);
    fprintf(out,
       ",inference("PCL_EF",[status(thm)],[c_0_%ld])",
       parent1->ident);
    tstp_print_end(out, comment, clause);
    break;
   default:
    fprintf(out, COMCHAR" Output format not implemented.\n");
    break;
   }
}


/*-----------------------------------------------------------------------
//
// Function: print_factor()
//
//   Print a clause creation by (ordinary) factoring.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void print_factor(FILE* out, Clause_p clause, Clause_p
           parent1, char* comment)
{
   switch(DocOutputFormat)
   {
   case pcl_format:
    pcl_print_start(out, clause, PCLShellLevel<1);
    fprintf(out, PCL_OF"(%ld)", parent1->ident);
    pcl_print_end(out, comment, clause);
    break;
   case tstp_format:
    ClauseTSTPPrint(out, clause, PCLFullTerms, false);
    fprintf(out,
       ",inference("PCL_OF
                 ",[status(thm)],[c_0_%ld])",
       parent1->ident);
    tstp_print_end(out, comment, clause);
    break;
   default:
    fprintf(out, COMCHAR" Output format not implemented.\n");
    break;
   }
}



/*-----------------------------------------------------------------------
//
// Function: print_split()
//
//   Print a clause creation by splitting.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void print_split(FILE* out, Clause_p clause, Clause_p
           parent1, char* comment)
{
   switch(DocOutputFormat)
   {
   case pcl_format:
    pcl_print_start(out, clause, PCLShellLevel<1);
    fprintf(out, PCL_SPLIT"(%ld)", parent1->ident);
    pcl_print_end(out, comment, clause);
    break;
   case tstp_format:
    ClauseTSTPPrint(out, clause, PCLFullTerms, false);
    fprintf(out,
       ",inference("
                 TSTP_SPLIT_BASE
                 ",["TSTP_SPLIT_BASE"("TSTP_SPLIT_REFINED
                 ",[])],[c_0_%ld])",
       parent1->ident);
    tstp_print_end(out, comment, clause);
    break;
    default:
    fprintf(out, COMCHAR" Output format not implemented.\n");
    break;
   }
}

/*-----------------------------------------------------------------------
//
// Function: print_simplify_reflect()
//
//   Print a clause modification by simplify-reflect.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void print_simplify_reflect(FILE* out, Clause_p clause, long
               old_id, Clause_p partner, char* comment)
{
   switch(DocOutputFormat)
   {
   case pcl_format:
    pcl_print_start(out, clause, PCLShellLevel<1);
    fprintf(out, PCL_SR"(%ld,%ld)", old_id,
       partner->ident);
    pcl_print_end(out, comment, clause);
    break;
  case tstp_format:
    ClauseTSTPPrint(out, clause, PCLFullTerms, false);
    fprintf(out,
       ",inference("PCL_SR",[status(thm)],[c_0_%ld,c_0_%ld])",
       old_id,
       partner->ident);
    tstp_print_end(out, comment, clause);
    break;
   default:
    fprintf(out, COMCHAR" Output format not implemented.\n");
    break;
   }
}

/*-----------------------------------------------------------------------
//
// Function: print_context_simplify_reflect()
//
//   Print a clause modification by contextual simplify-reflect.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void print_context_simplify_reflect(FILE* out, Clause_p clause, long
               old_id, Clause_p partner, char* comment)
{
   switch(DocOutputFormat)
   {
   case pcl_format:
    pcl_print_start(out, clause, PCLShellLevel<1);
    fprintf(out, PCL_CSR"(%ld,%ld)", old_id,
       partner->ident);
    pcl_print_end(out, comment, clause);
    break;
 case tstp_format:
    ClauseTSTPPrint(out, clause, PCLFullTerms, false);
    fprintf(out,
       ",inference("PCL_CSR
                 ",[status(thm)],[c_0_%ld,c_0_%ld])",
       old_id,
       partner->ident);
    tstp_print_end(out, comment, clause);
    break;
   default:
    fprintf(out, COMCHAR" Output format not implemented.\n");
    break;
   }
}


/*-----------------------------------------------------------------------
//
// Function: print_ac_res()
//
//   Print a clause modification by AC-resolution.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void print_ac_res(FILE* out, Clause_p clause, long
          old_id, Sig_p sig, char* comment)
{
   PStackPointer i, sp;
   Clause_p      ax;

   switch(DocOutputFormat)
   {
   case pcl_format:
    pcl_print_start(out, clause, PCLShellLevel<1);
    fprintf(out, PCL_ACRES"(%ld", old_id);
    assert(!PStackEmpty(sig->ac_axioms));
    sp = PStackGetSP(sig->ac_axioms);
    for(i=0; i< sp; i++)
    {
            ax = PStackElementP(sig->ac_axioms,i);
       fprintf(out, ",%ld", ax->ident);
    }
    fputc(')', out);
    pcl_print_end(out, comment, clause);
    break;
   case tstp_format:
    ClauseTSTPPrint(out, clause, PCLFullTerms, false);
    fprintf(out,
       ",inference("PCL_ACRES
                 ",[status(thm)],[c_0_%ld", old_id);
    assert(!PStackEmpty(sig->ac_axioms));
    sp = PStackGetSP(sig->ac_axioms);
    for(i=0; i< sp; i++)
    {
            ax = PStackElementP(sig->ac_axioms,i);
       fprintf(out, ",c_0_%ld", ax->ident);
    }
    fputs("])", out);
    tstp_print_end(out, comment, clause);
    break;
   default:
    fprintf(out, COMCHAR" Output format not implemented.\n");
    break;
   }
}


/*-----------------------------------------------------------------------
//
// Function: print_minimize()
//
//   Print a clause modification by clause-internal simplification
//   (elemination of redundant literals)
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void print_minimize(FILE* out, Clause_p clause, long
          old_id, char* comment)
{
   switch(DocOutputFormat)
   {
   case pcl_format:
    pcl_print_start(out, clause, PCLShellLevel<1);
    fprintf(out, PCL_CN"(%ld)", old_id);
    pcl_print_end(out, comment, clause);
    break;
   case tstp_format:
         ClauseTSTPPrint(out, clause, PCLFullTerms, false);
         fprintf(out,
                 ",inference("PCL_CN
                 ",[status(thm)],[c_0_%ld])",
                 old_id);
         tstp_print_end(out, comment, clause);
         break;
   default:
    fprintf(out, COMCHAR" Output format not implemented.\n");
    break;
   }
}


/*-----------------------------------------------------------------------
//
// Function: print_condense()
//
//   Print a clause modification by condensation.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void print_condense(FILE* out, Clause_p clause, long
                           old_id, char* comment)
{
   switch(DocOutputFormat)
   {
   case pcl_format:
    pcl_print_start(out, clause, PCLShellLevel<1);
    fprintf(out, PCL_CONDENSE"(%ld)", old_id);
    pcl_print_end(out, comment, clause);
    break;
   case tstp_format:
         ClauseTSTPPrint(out, clause, PCLFullTerms, false);
         fprintf(out,
                 ",inference("PCL_CONDENSE
                 ",[status(thm)],[c_0_%ld])",
                 old_id);
         tstp_print_end(out, comment, clause);
         break;
   default:
    fprintf(out, COMCHAR" Output format not implemented.\n");
    break;
   }
}

/*-----------------------------------------------------------------------
//
// Function: print_eval_answer()
//
//   Print a clause modification by answer-literal-elimination.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void print_eval_answer(FILE* out, Clause_p clause, long
                              old_id, char* comment)
{
   switch(DocOutputFormat)
   {
   case pcl_format:
    pcl_print_start(out, clause, PCLShellLevel<1);
    fprintf(out, PCL_EVANS"(%ld)", old_id);
    pcl_print_end(out, comment, clause);
    break;
   case tstp_format:
         ClauseTSTPPrint(out, clause, PCLFullTerms, false);
         fprintf(out,
                 ",inference("PCL_EVANS",[status(thm)],[c_0_%ld, theory(answers)])",
                 old_id);
         tstp_print_end(out, comment, clause);
         break;
   default:
    fprintf(out, COMCHAR" Output format not implemented.\n");
    break;
   }
}

/*-----------------------------------------------------------------------
//
// Function: print_rewrite()
//
//   Print a series of rewrite steps.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void print_rewrite(FILE* out, ClausePos_p rewritten, long
           old_id, Term_p old_term, char* comment)
{
   PStack_p rwsteps = PStackAlloc();
   PStackPointer i;
   bool     tmp;
   Term_p   nf = ClausePosGetSide(rewritten);
   Clause_p demod;

   /* printf("Rewrite: ");
   TBPrintTerm(stdout, rewritten->literal->bank, old_term, true);
   printf(" => ");
   TBPrintTerm(stdout, rewritten->literal->bank, nf, true);
   printf(" \n"); */

   switch(DocOutputFormat)
   {
   case pcl_format:
    tmp = TermComputeRWSequence(rwsteps, old_term, nf, 0);
    UNUSED(tmp); assert(tmp);
    pcl_print_start(out, rewritten->clause, PCLShellLevel<1);
    for(i=0; i<PStackGetSP(rwsteps); i++)
    {
       fputs(PCL_RW"(", out);
    }
    fprintf(out, "%ld", old_id);
    for(i=0; i<PStackGetSP(rwsteps); i++)
    {
            demod = PStackElementP(rwsteps,i);
       fprintf(out, ",%ld)", demod->ident);
    }
    pcl_print_end(out, comment, rewritten->clause);
    break;
   case tstp_format:
    tmp = TermComputeRWSequence(rwsteps, old_term, nf, 0);
    UNUSED(tmp); assert(tmp);
    ClauseTSTPPrint(out, rewritten->clause, PCLFullTerms, false);
    fputc(',', out);
    for(i=0; i<PStackGetSP(rwsteps); i++)
    {
       fprintf(out,"inference("PCL_RW", [status(thm)],[");
    }
    fprintf(out, "c_0_%ld", old_id);
    for(i=0; i<PStackGetSP(rwsteps); i++)
    {
            demod = PStackElementP(rwsteps,i);
       fprintf(out, ",c_0_%ld])",
          demod->ident);
    }
    tstp_print_end(out, comment, rewritten->clause);
    break;
   default:
    fprintf(out, COMCHAR" Output format not implemented.\n");
    break;
   }
   PStackFree(rwsteps);
}


/*-----------------------------------------------------------------------
//
// Function: print_eq_unfold()
//
//   Print a series of eq-unfoldings with demod.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void print_eq_unfold(FILE* out, Clause_p rewritten,
             long old_id, ClausePos_p demod,
             PStack_p demod_pos)
{
   PStackPointer i;

   switch(DocOutputFormat)
   {
   case pcl_format:
    pcl_print_start(out, rewritten, PCLShellLevel<1);
    for(i=0; i<PStackGetSP(demod_pos); i++)
    {
       fputs(PCL_RW"(", out);
    }
    fprintf(out, "%ld", old_id);
    for(i=0; i<PStackGetSP(demod_pos); i++)
    {
       fprintf(out, ",%ld)", demod->clause->ident);
    }
    pcl_print_end(out, "unfolding", rewritten);
    break;
   case tstp_format:
    ClauseTSTPPrint(out, rewritten, PCLFullTerms, false);
    fputc(',', out);
    for(i=0; i<PStackGetSP(demod_pos); i++)
    {
       fprintf(out,"inference("PCL_RW", [status(thm)],[");
    }
    fprintf(out, "c_0_%ld", old_id);
    for(i=0; i<PStackGetSP(demod_pos); i++)
    {
       fprintf(out, ",c_0_%ld])",
          demod->clause->ident);
    }
    tstp_print_end(out, "Unfolding", rewritten);
    break;
    default:
    fprintf(out, COMCHAR" Output format not implemented.\n");
    break;
   }
}




/*---------------------------------------------------------------------*/
/*             Internal Functions -- Formulae                          */
/*---------------------------------------------------------------------*/



/*-----------------------------------------------------------------------
//
// Function: pcl_formula_print_start()
//
//   Print the "<id> : <clause> : " part of a pcl step.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void pcl_formula_print_start(FILE* out, WFormula_p form, bool print_form)
{
   fprintf(out, PCLStepCompact?"%ld:":"%6ld : ", form->ident);
   fprintf(out, "%s:", PCLTypeStr(FormulaQueryType(form)));

   if(print_form)
   {
      TFormulaTPTPPrint(out, form->terms, form->tformula, PCLFullTerms, true);
   }
   fputs(" : ", out);
}

/*-----------------------------------------------------------------------
//
// Function: pcl_formula_print_end()
//
//   Print the optional comment and new line
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void pcl_formula_print_end(FILE* out, char* comment)
{
   if(comment)
   {
      fprintf(out, PCLStepCompact?":'%s'":" : '%s'",comment);
   }
   fputc('\n', out);
}


/*-----------------------------------------------------------------------
//
// Function: tstp_formula_print_end()
//
//   Print the optional comment and new line
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void tstp_formula_print_end(FILE* out, char* comment)
{
   if(comment)
   {
      fprintf(out, ",['%s']", comment);
   }
   fputs(").\n", out);
}


/*-----------------------------------------------------------------------
//
// Function: print_formula_initial()
//
//   Print an initial formula.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void print_formula_initial(FILE* out, WFormula_p form, char* comment)
{
   switch(DocOutputFormat)
   {
   case pcl_format:
         pcl_formula_print_start(out, form, PCLShellLevel<2);
         ClauseSourceInfoPrintPCL(out, form->info);
         pcl_formula_print_end(out, comment);
         break;
   case tstp_format:
         WFormulaTSTPPrint(out, form, PCLFullTerms, false);
         fprintf(out, ", ");
         ClauseSourceInfoPrintTSTP(out, form->info);
         tstp_formula_print_end(out, comment);
         break;
   default:
         fprintf(out, COMCHAR" Output format not implemented.\n");
         break;
   }
}



/*-----------------------------------------------------------------------
//
// Function: print_fof_intro_def()
//
//   Print the introduction of a formula definition.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void print_fof_intro_def(FILE* out, WFormula_p form, char* comment)
{
   switch(DocOutputFormat)
   {
   case pcl_format:
    pcl_formula_print_start(out, form, PCLShellLevel<1);
         fprintf(out, PCL_ID);
    pcl_formula_print_end(out, comment);
    break;
   case tstp_format:
    WFormulaTSTPPrint(out, form, PCLFullTerms, false);
    fprintf(out, ", ");
         fprintf(out, PCL_ID_DEF);
    tstp_formula_print_end(out, comment);
    break;
   default:
    fprintf(out, COMCHAR" Output format not implemented.\n");
    break;
   }
}



/*-----------------------------------------------------------------------
//
// Function: print_fof_split_equiv()
//
//   Print the introduction of a formula by splitting <=> into => or
//   <=.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void print_fof_split_equiv(FILE* out, WFormula_p form,
                                  WFormula_p parent, char* comment)
{
   assert(parent);
   switch(DocOutputFormat)
   {
   case pcl_format:
    pcl_formula_print_start(out, form, PCLShellLevel<1);
         fprintf(out, PCL_SE"(%ld)",parent->ident);
    pcl_formula_print_end(out, comment);
    break;
   case tstp_format:
    WFormulaTSTPPrint(out, form, PCLFullTerms, false);
    fprintf(out, ", ");
         fprintf(out, "inference("PCL_SE", [status(thm)], [c_0_%ld])",
                 parent->ident);
    tstp_formula_print_end(out, comment);
    break;
   default:
    fprintf(out, COMCHAR" Output format not implemented.\n");
    break;
   }
}



/*-----------------------------------------------------------------------
//
// Function: print_fof_simpl()
//
//   Print a fof simplification step.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

static void print_fof_simpl(FILE* out, WFormula_p form,
                            long old_id, char *comment)
{
   switch(DocOutputFormat)
   {
   case pcl_format:
    pcl_formula_print_start(out, form, PCLShellLevel<1);
         fprintf(out, PCL_FS"(%ld)", old_id);
    pcl_formula_print_end(out, comment);
    break;
   case tstp_format:
    WFormulaTSTPPrint(out, form, PCLFullTerms, false);
         fprintf(out, ",inference("PCL_FS", [status(thm)],[c_0_%ld])", old_id);
         tstp_formula_print_end(out, comment);
    break;
   default:
    fprintf(out, COMCHAR" Output format not implemented.\n");
    break;
   }
}


/*-----------------------------------------------------------------------
//
// Function: print_neg_conj()
//
//   Print a conjecture negation step ("assume opposite")
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

static void print_neg_conj(FILE* out, WFormula_p form,
                            long old_id, char *comment)
{
   switch(DocOutputFormat)
   {
   case pcl_format:
    pcl_formula_print_start(out, form, PCLShellLevel<1);
         fprintf(out, PCL_NC"(%ld)", old_id);
    pcl_formula_print_end(out, comment);
    break;
   case tstp_format:
    WFormulaTSTPPrint(out, form, PCLFullTerms, false);
         fprintf(out, ",inference("PCL_NC", [status(cth)],[c_0_%ld])", old_id);
         tstp_formula_print_end(out, comment);
    break;
   default:
    fprintf(out, COMCHAR" Output format not implemented.\n");
    break;
   }
}


/*-----------------------------------------------------------------------
//
// Function: print_fof_nnf()
//
//   Print a fof negation normal form step.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

static void print_fof_nnf(FILE* out, WFormula_p form,
                          long old_id, char *comment)
{
   switch(DocOutputFormat)
   {
   case pcl_format:
    pcl_formula_print_start(out, form, PCLShellLevel<1);
         fprintf(out, PCL_NNF"(%ld)", old_id);
    pcl_formula_print_end(out, comment);
    break;
   case tstp_format:
    WFormulaTSTPPrint(out, form, PCLFullTerms, false);
         fprintf(out, ",inference("PCL_NNF", [status(thm)],[c_0_%ld])", old_id);
         tstp_formula_print_end(out, comment);
    break;
   default:
    fprintf(out, COMCHAR" Output format not implemented.\n");
    break;
   }
}


/*-----------------------------------------------------------------------
//
// Function: print_shift_quantors()
//
//   Print a shift quantor (in for miniskoping, out for final CNF'ing)
//   inference.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

static void print_shift_quantors(FILE* out, WFormula_p form,
                                 long old_id, char *comment)
{
   switch(DocOutputFormat)
   {
   case pcl_format:
    pcl_formula_print_start(out, form, PCLShellLevel<1);
         fprintf(out, PCL_SQ"(%ld)", old_id);
    pcl_formula_print_end(out, comment);
    break;
   case tstp_format:
    WFormulaTSTPPrint(out, form, PCLFullTerms, false);
         fprintf(out, ",inference("PCL_SQ", [status(thm)],[c_0_%ld])", old_id);
         tstp_formula_print_end(out, comment);
    break;
   default:
    fprintf(out, COMCHAR" Output format not implemented.\n");
    break;
   }
}



/*-----------------------------------------------------------------------
//
// Function: print_skolemize()
//
//   Print a Skolemization step.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

static void print_skolemize(FILE* out, WFormula_p form,
                      long old_id, char *comment)
{
   switch(DocOutputFormat)
   {
   case pcl_format:
    pcl_formula_print_start(out, form, PCLShellLevel<1);
         fprintf(out, PCL_SK"(%ld)", old_id);
    pcl_formula_print_end(out, comment);
    break;
   case tstp_format:
    WFormulaTSTPPrint(out, form, PCLFullTerms, false);
         fprintf(out, ",inference("PCL_SK", [status(esa)], [c_0_%ld])", old_id);
         tstp_formula_print_end(out, comment);
    break;
   default:
    fprintf(out, COMCHAR" Output format not implemented.\n");
    break;
   }
}


/*-----------------------------------------------------------------------
//
// Function: print_distribute()
//
//   Print a distributivity step (or steps).
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

static void print_distribute(FILE* out, WFormula_p form,
                             long old_id, char *comment)
{
   switch(DocOutputFormat)
   {
   case pcl_format:
    pcl_formula_print_start(out, form, PCLShellLevel<1);
         fprintf(out, PCL_DSTR"(%ld)", old_id);
    pcl_formula_print_end(out, comment);
    break;
   case tstp_format:
    WFormulaTSTPPrint(out, form, PCLFullTerms, false);
         fprintf(out, ",inference("PCL_DSTR", [status(thm)],[c_0_%ld])", old_id);
         tstp_formula_print_end(out, comment);
    break;
   default:
    fprintf(out, COMCHAR" Output format not implemented.\n");
    break;
   }
}

/*-----------------------------------------------------------------------
//
// Function: print_annotate_question()
//
//   Print a step adding answer literals.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void print_annotate_question(FILE* out, WFormula_p form,
                                    long old_id, char *comment)
{
   switch(DocOutputFormat)
   {
   case pcl_format:
    pcl_formula_print_start(out, form, PCLShellLevel<1);
         fprintf(out, PCL_ANNOQ"(%ld)", old_id);
    pcl_formula_print_end(out, comment);
    break;
   case tstp_format:
    WFormulaTSTPPrint(out, form, PCLFullTerms, false);
         fprintf(out, ",inference("PCL_ANNOQ", [status(thm)],[c_0_%ld,theory(answers)])", old_id);
         tstp_formula_print_end(out, comment);
    break;
   default:
    fprintf(out, COMCHAR" Output format not implemented.\n");
    break;
   }
}




/*-----------------------------------------------------------------------
//
// Function: print_var_rename()
//
//   Print a variable renaming step.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

static void print_var_rename(FILE* out, WFormula_p form,
                               long old_id, char *comment)
{
   switch(DocOutputFormat)
   {
   case pcl_format:
    pcl_formula_print_start(out, form, PCLShellLevel<1);
         fprintf(out, PCL_VR"(%ld)", old_id);
    pcl_formula_print_end(out, comment);
    break;
   case tstp_format:
    WFormulaTSTPPrint(out, form, PCLFullTerms, false);
         fprintf(out, ",inference("PCL_VR", [status(thm)],[c_0_%ld])", old_id);
         tstp_formula_print_end(out, comment);
    break;
   default:
    fprintf(out, COMCHAR" Output format not implemented.\n");
    break;
   }
}




/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: DocClauseCreation()
//
//   Document the creation of a new clause if level >=2
//
// Global Variables: ClauseIdentCounter
//
// Side Effects    : Output, generates new id for clause
//
/----------------------------------------------------------------------*/

void DocClauseCreation(FILE* out, long level, Clause_p clause,
                       InfType op, Clause_p parent1,
                       Clause_p parent2, char* comment)
{
   if(level >= 2)
   {
      switch(op)
      {
      case inf_initial:
            assert(!parent1);
            assert(!parent2);
            clause->ident = ++ClauseIdentCounter;
            print_initial(out, clause, comment);
            break;
      case inf_paramod:
      case inf_sim_paramod:
            assert(parent1);
            assert(parent2);
            clause->ident = ++ClauseIdentCounter;
            print_paramod(out, clause, parent1, parent2,
                          op==inf_paramod?PCL_PM:PCL_SPM,comment);
       break;
      case inf_eres:
            assert(parent1);
            assert(!parent2);
            clause->ident = ++ClauseIdentCounter;
            print_eres(out, clause, parent1, comment);
            break;
      case inf_efactor:
            assert(parent1);
            assert(!parent2);
            clause->ident = ++ClauseIdentCounter;
            print_efactor(out, clause, parent1, comment);
            break;
      case inf_factor: /* Should not really occur, just for
                          completeness */
            assert(parent1);
            assert(!parent2);
            clause->ident = ++ClauseIdentCounter;
            print_factor(out, clause, parent1, comment);
            break;
      case inf_split:
            assert(parent1);
            assert(!parent2);
            clause->ident = ++ClauseIdentCounter;
            print_split(out, clause, parent1, comment);
            break;
      default:
            assert(false&&"Unsupported clause creation operation???");
            break;
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: DocClauseFromForm()
//
//   Document the creation of a clause from a conjunct of a formula.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void DocClauseFromForm(FILE* out, long level, Clause_p clause,
                       WFormula_p parent)
{
   assert(clause);
   assert(parent);

   ClauseDelProp(clause, CPInputFormula);
   if(level >= 2)
   {
      switch(DocOutputFormat)
      {
      case pcl_format:
            clause->ident = ++ClauseIdentCounter;
            pcl_print_start(out, clause, PCLShellLevel<1);
            fprintf(out, PCL_SC "(%ld)", parent->ident);
            pcl_print_end(out, NULL, clause);
            break;
      case tstp_format:
            clause->ident = ++ClauseIdentCounter;
            ClauseTSTPPrint(out, clause, PCLFullTerms, false);
            fprintf(out, ",inference("PCL_SC", [status(thm)],[c_0_%ld])",
                    parent->ident);
            tstp_print_end(out, NULL, clause);
            break;
      default:
            fprintf(out, COMCHAR" Output format not implemented.\n");
            break;
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: DocClauseModification()
//
//   Document the modification of a clause.
//
// Global Variables: ClauseIdentCounter
//
// Side Effects    : Output, generates new id for clause
//
/----------------------------------------------------------------------*/

void DocClauseModification(FILE* out, long level, Clause_p clause, InfType
                           op, Clause_p partner, Sig_p sig, char* comment)
{
   long old_id;

   ClauseDelProp(clause, CPInputFormula);
   if(level >= 2)
   {
      assert(clause);
      old_id = clause->ident;

      switch(op)
      {
      case inf_simplify_reflect:
       assert(clause);
       assert(partner);
       clause->ident = ++ClauseIdentCounter;
       print_simplify_reflect(out, clause, old_id, partner,
               comment);
       break;
      case inf_context_simplify_reflect:
       assert(clause);
       assert(partner);
       clause->ident = ++ClauseIdentCounter;
       print_context_simplify_reflect(out, clause, old_id, partner,
               comment);
       break;
      case inf_ac_resolution:
       assert(clause);
       assert(!partner);
       clause->ident = ++ClauseIdentCounter;
       print_ac_res(out, clause, old_id, sig, comment);
       break;
      case inf_condense:
       assert(clause);
       assert(!partner);
       clause->ident = ++ClauseIdentCounter;
       print_condense(out, clause, old_id, comment);
       break;
      case inf_minimize:
       assert(clause);
       assert(!partner);
       clause->ident = ++ClauseIdentCounter;
       print_minimize(out, clause, old_id, comment);
       break;
      case inf_eval_answers:
       assert(clause);
       assert(!partner);
       clause->ident = ++ClauseIdentCounter;
       print_eval_answer(out, clause, old_id, comment);
       break;
      case inf_eres:
       assert(clause);
       assert(partner);
       clause->ident = ++ClauseIdentCounter;
       print_des_eres(out, clause, old_id, comment);
       break;
         /* inf_rewrite is special and handled below !*/
      default:
       fprintf(out, COMCHAR" Clause modification %d not yet implemented.\n",op);
       break;
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: DocClauseQuote()
//
//   Print the clause with a new id as a descendent of itself only.
//   Useful for getting the comment out.
//
// Global Variables: ClauseIdentCounter
//
// Side Effects    : Output, generates new id for clause
//
/----------------------------------------------------------------------*/

void DocClauseQuote(FILE* out, long level, long target_level,
          Clause_p clause, char* comment, Clause_p
          opt_partner)
{
   long old_id;

   assert(clause);

   ClauseDelProp(clause, CPInputFormula);
   old_id = clause->ident;

   if(level >= target_level)
   {
      switch(DocOutputFormat)
      {
      case pcl_format:
       clause->ident = ++ClauseIdentCounter;
       pcl_print_start(out, clause, PCLShellLevel<1);
       fprintf(out, "%ld", old_id);
       if(opt_partner)
       {
          assert(comment);
          fprintf(out, " : '%s(%ld)'\n", comment, opt_partner->ident);
       }
       else
       {
          pcl_print_end(out, comment, clause);
       }
       break;
      case tstp_format:
       clause->ident = ++ClauseIdentCounter;
       ClauseTSTPPrint(out, clause, PCLFullTerms, false);
       fprintf(out, ", c_0_%ld", old_id);
       if(opt_partner)
       {
          assert(comment);
          fprintf(out, ",['%s(c_0_%ld)']).\n", comment, opt_partner->ident);
       }
       else if(comment)
       {
          fprintf(out, ",['%s']).\n", comment);
       }
       else
       {
          fprintf(out, ").\n");
       }
       break;

      default:
       fprintf(out, COMCHAR" Output format not implemented.\n");
       break;
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: DocClauseRewrite()
//
//   Document a series of rewrite steps performed on the literal
//   position described in pos, on the original term old_term.
//
// Global Variables: -
//
// Side Effects    : Output, changes clause identifier.
//
/----------------------------------------------------------------------*/

void DocClauseRewrite(FILE* out, long level, ClausePos_p rewritten,
            Term_p old_term, char* comment)
{
   long old_id;

   ClauseDelProp(rewritten->clause, CPInputFormula);
   if(level >= 2)
   {
      assert(rewritten);
      assert(rewritten->clause);
      assert(rewritten->literal);
      old_id = rewritten->clause->ident;
      rewritten->clause->ident = ++ClauseIdentCounter;
      print_rewrite(out, rewritten, old_id, old_term, comment);
   }
}


/*-----------------------------------------------------------------------
//
// Function: DocClauseEqUnfold()
//
//   Document rewrite steps caused by definition unfolding. Ugly and
//   incomplete.
//
// Global Variables: -
//
// Side Effects    :  Output, changes clause identifier.
//
/----------------------------------------------------------------------*/

void DocClauseEqUnfold(FILE* out, long level, Clause_p rewritten,
             ClausePos_p demod, PStack_p demod_pos)
{
   long old_id;

   ClauseDelProp(rewritten, CPInputFormula);
   if(level >= 2)
   {
      assert(rewritten);
      old_id = rewritten->ident;
      rewritten->ident = ++ClauseIdentCounter;
      print_eq_unfold(out, rewritten, old_id, demod, demod_pos);
   }

}

/*-----------------------------------------------------------------------
//
// Function: DocFormulaCreation()
//
//   Document the creation of a full FOF formula.
//
// Global Variables: ClauseIdentCounter
//
// Side Effects    : Output, generates new id for formula
//
/----------------------------------------------------------------------*/

void DocFormulaCreation(FILE* out, long level, WFormula_p formula,
                        InfType op, WFormula_p parent1,
                        WFormula_p parent2, char* comment)
{
   if(level >= 2)
   {
      switch(op)
      {
      case inf_initial:
            assert(!parent1);
            assert(!parent2);
            formula->ident = ++ClauseIdentCounter;
            print_formula_initial(out, formula, comment);
            break;
      case inf_fof_intro_def:
       assert(!parent1);
       assert(!parent2);
       formula->ident = ++ClauseIdentCounter;
       print_fof_intro_def(out, formula, comment);
       break;
      case inf_fof_split_equiv:
            assert(parent1);
            assert(!parent2);
            formula->ident = ++ClauseIdentCounter;
            print_fof_split_equiv(out, formula, parent1, comment);
            break;
      default:
            assert(false && "Unsupported formula creation method");
            break;
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: DocFormulaModification()
//
//   Document general clause modifications.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void DocFormulaModification(FILE* out, long level, WFormula_p form,
                            InfType op, char* comment)
{
   FormulaDelProp(form, CPInputFormula);
   if(level >= 2)
   {
      long old_id = form->ident;

      form->ident = ++ClauseIdentCounter;
      switch(op)
      {
      case inf_fof_simpl:
            print_fof_simpl(out, form, old_id, comment);
            break;
      case inf_neg_conjecture:
            print_neg_conj(out, form, old_id, comment);
            break;
      case inf_fof_nnf:
            print_fof_nnf(out, form, old_id, comment);
            break;
      case inf_shift_quantors:
            print_shift_quantors(out, form, old_id, comment);
            break;
      case inf_var_rename:
            print_var_rename(out, form, old_id, comment);
            break;
      case inf_skolemize_out:
            print_skolemize(out, form, old_id, comment);
            break;
      case inf_fof_distrib:
            print_distribute(out, form, old_id, comment);
            break;
      case inf_annotate_question:
            print_annotate_question(out, form, old_id, comment);
            break;
      default:
            break;
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: DocFormulaIntroDefs()
//
//   Print the application of a set of definitions to a formula.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void DocFormulaIntroDefs(FILE* out, long level, WFormula_p form,
                         PStack_p def_list, char* comment)
{
   PStackPointer i;
   WFormula_p def;

   if(level >= 2)
   {
      long old_id = form->ident;

      form->ident = ++ClauseIdentCounter;

      switch(DocOutputFormat)
      {
      case pcl_format:
            pcl_formula_print_start(out, form, PCLShellLevel<1);
            for(i=0; i<PStackGetSP(def_list); i++)
            {
               fputs(PCL_AD "(", out);
            }
            fprintf(out, "%ld", old_id);
            for(i=0; i<PStackGetSP(def_list); i++)
            {
               def = PStackElementP(def_list, i);
               fprintf(out, ",%ld)", def->ident);
            }
            pcl_formula_print_end(out, comment);
            break;
      case tstp_format:
            WFormulaTSTPPrint(out, form, PCLFullTerms, false);
            fprintf(out, ", ");
            for(i=0; i<PStackGetSP(def_list); i++)
            {
               fputs("inference("PCL_AD",[status(thm)],[", out);
            }
            fprintf(out, "c_0_%ld", old_id);
            for(i=0; i<PStackGetSP(def_list); i++)
            {
               def = PStackElementP(def_list, i);
               fprintf(out, ",c_0_%ld])", def->ident);
            }
            tstp_formula_print_end(out, comment);
            break;
      default:
            fprintf(out, COMCHAR" Output format not implemented.\n");
            break;
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: DocIntroSplitDef()
//
//   Print a split definition that defines the constant predicate
//   represented by def_pred as the (universal closure of)
//   clause_part.
//
// Global Variables: -
//
// Side Effects    : Sets new id in clause (if output is active)
//
/----------------------------------------------------------------------*/

void DocIntroSplitDef(FILE* out, long level, WFormula_p form)
{
   assert(form);

   if(level >= 2)
   {
      DocFormulaCreation(out, level,
                         form,
                         inf_fof_intro_def,
                         NULL,
                         NULL,
                         "split");
   }
}


/*-----------------------------------------------------------------------
//
// Function: DocIntroSplitDefRest()
//
//   Print the clause representation of the expanding implication of
//   a definition.
//
// Global Variables: -
//
// Side Effects    : Sets new id in clause (if output is active)
//
/----------------------------------------------------------------------*/

void DocIntroSplitDefRest(FILE* out, long level, Clause_p clause,
                          WFormula_p parent, char* comment)
{
   assert(clause);
   assert(clause->literals);
   if(level >= 2)
   {
      assert(parent);
      switch(DocOutputFormat)
      {
      case pcl_format:
            clause->ident = ++ClauseIdentCounter;
            pcl_print_start(out, clause, PCLShellLevel<1);
            fprintf(out, PCL_SE "(%ld)", parent->ident);
            pcl_print_end(out, NULL, clause);
            break;
      case tstp_format:
            clause->ident = ++ClauseIdentCounter;
            ClauseTSTPPrint(out, clause, PCLFullTerms, false);
            fprintf(out, ",inference("PCL_SE", [status(thm)],[c_0_%ld])",
                    parent->ident);
            tstp_print_end(out, NULL, clause);
            break;
      default:
            fprintf(out, COMCHAR" Output format not implemented.\n");
            break;
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: DocClauseApplyDefs()
//
//   Print the clause derivation describing the application of the
//   definitions in def_ids to parent.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void DocClauseApplyDefs(FILE* out, long level, Clause_p clause,
                        long parent_id, PStack_p def_ids, char* comment)
{
   PStackPointer i;

   assert(clause);

   if(level >= 2)
   {
      switch(DocOutputFormat)
      {
      case pcl_format:
            clause->ident = ++ClauseIdentCounter;
            pcl_print_start(out, clause, PCLShellLevel<1);
            for(i=0; i<PStackGetSP(def_ids); i++)
            {
               fputs(PCL_AD"(", out);
            }
            fprintf(out, "%ld", parent_id);
            for(i=0; i<PStackGetSP(def_ids); i++)
            {
               fprintf(out, ",%ld)", PStackElementInt(def_ids, i));
            }
            pcl_print_end(out, "split", clause);
            break;
      case tstp_format:
            clause->ident = ++ClauseIdentCounter;
            ClauseTSTPPrint(out, clause, PCLFullTerms, false);
            fputc(',', out);
            for(i=0; i<PStackGetSP(def_ids); i++)
            {
               fprintf(out,"inference("PCL_AD", [status(thm)],[");
            }
            fprintf(out, "c_0_%ld", parent_id);
            for(i=0; i<PStackGetSP(def_ids); i++)
            {
               fprintf(out, ",c_0_%ld])",
                       PStackElementInt(def_ids, i));
            }
            tstp_print_end(out, "split", clause);
            break;
      default:
            fprintf(out, COMCHAR" Output format not implemented.\n");
            break;
      }
   }
}





/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
