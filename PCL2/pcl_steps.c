/*-----------------------------------------------------------------------

File  : pcl_steps.c

Author: Stephan Schulz

Contents

  Individual PCL steps and related stuff.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Thu Mar 30 19:04:05 MET DST 2000
    New

-----------------------------------------------------------------------*/

#include "pcl_steps.h"


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


/* Support PCL without logical content - useful for analysing very
 * very very large proofs, and faster for Isabelle proofs that don't
 * need the intermediate results. This can only be set to true in
 * tools  that work only on the proof structure, not contents. */
bool SupportShellPCL = false;

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: print_shell_pcl_warning()
//
//   Print a warning that a shell PCL step was encountered where a
//   normal one was expected.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void print_shell_pcl_warning(FILE* out, PCLStep_p step)
{
   if(PCLStepIsShell(step))
   {
      Warning("Shell PCL step encountered where "
              "full PCL step was required");
      fprintf(out, COMCHAR" Step ");
      PCLIdPrint(out, step->id);
      fprintf(out, " omitted (Shell)\n");
   }
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: PCLStepFree()
//
//   Free a PCL step.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void PCLStepFree(PCLStep_p junk)
{
   assert(junk && junk->id);

   PCLIdFree(junk->id);
   if(!PCLStepIsShell(junk))
   {
      if(PCLStepIsClausal(junk))
      {
         ClauseFree(junk->logic.clause);
      }
      else
      {
         /* tformula collected by garbage collector */
      }
   }
   PCLExprFree(junk->just);
   if(junk->extra)
   {
      FREE(junk->extra);
   }
   PCLStepCellFree(junk);
}

/*-----------------------------------------------------------------------
//
// Function: PCLParseExternalType()
//
//   Parse a list of type annotations for PCL steps and return a
//   property word that can be used with SetProp() to set all
//   necessary properties (the type field and the lemma bit).
//
// Global Variables: -
//
// Side Effects    : Reads input
//
/----------------------------------------------------------------------*/

PCLStepProperties PCLParseExternalType(Scanner_p in)
{
   PCLStepProperties type = PCLTypeAxiom, extra = PCLNoProp;

   while(!TestInpTok(in, Colon))
   {
      if(TestInpId(in, "conj"))
      {
         type = PCLTypeConjecture;
         NextToken(in);
      }
      else if(TestInpId(in, "que"))
      {
         type = PCLTypeQuestion;
         NextToken(in);
      }
      else if(TestInpId(in, "neg"))
      {
         type = PCLTypeNegConjecture;
         NextToken(in);
      }
      else if(TestInpId(in, "lemma"))
      {
         extra = PCLIsLemma;
         NextToken(in);
      }
      else
      {
         CheckInpId(in, "conj|neg|lemma");
      }
      if(!TestInpTok(in, Colon))
      {
         AcceptInpTok(in, Comma);
      }
   }
   return type | extra;
}

/*-----------------------------------------------------------------------
//
// Function: PCLStepParse()
//
//   Parse a PCL step.
//
// Global Variables: -
//
// Side Effects    : Input, memory operations
//
/----------------------------------------------------------------------*/

PCLStep_p PCLStepParse(Scanner_p in, TB_p bank)
{
   PCLStep_p handle = PCLStepCellAlloc();

   assert(in);
   assert(bank);

   handle->bank = bank;
   PCLStepResetTreeData(handle, false);
   handle->id = PCLIdParse(in);
   AcceptInpTok(in, Colon);
   handle->properties = PCLParseExternalType(in);
   AcceptInpTok(in, Colon);
   if(SupportShellPCL && TestInpTok(in, Colon))
   {
      handle->logic.clause = NULL;
      PCLStepSetProp(handle, PCLIsShellStep);
   }
   else if(TestInpTok(in, OpenSquare))
   {
      handle->logic.clause = ClausePCLParse(in, bank);
      PCLStepDelProp(handle, PCLIsFOFStep);
   }
   else
   {
      handle->logic.formula = TFormulaTPTPParse(in, bank);
      PCLStepSetProp(handle, PCLIsFOFStep);
   }
   AcceptInpTok(in, Colon);
   handle->just = PCLFullExprParse(in);
   if(TestInpTok(in, Colon))
   {
      NextToken(in);
      CheckInpTok(in, SQString|Name|PosInt);
      handle->extra = DStrCopy(AktToken(in)->literal);
      NextToken(in);
   }
   else
   {
      handle->extra = NULL;
   }
   PCLStepDelProp(handle, PCLIsProofStep);
   if(handle->just->op == PCLOpInitial)
   {
      PCLStepSetProp(handle, PCLIsInitial);
   }
   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: PCLPrintExternalType()
//
//   Print the type(s) of a PCL step encoded in props.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void PCLPrintExternalType(FILE* out, PCLStepProperties props)
{
   char *prepend="";

   if(props&PCLIsLemma)
   {
      fputs("lemma", out);
      prepend = ",";
   }
   props = props & PCLTypeMask;
   switch(props)
   {
   case PCLTypeNegConjecture:
         fputs(prepend, out);
         fputs("neg", out);
         break;
   case PCLTypeConjecture:
         fputs(prepend, out);
         fputs("conj", out);
         break;
   case PCLTypeQuestion:
         fputs(prepend, out);
         fputs("que", out);
         break;
   default:
         break;
   }
}


/*-----------------------------------------------------------------------
//
// Function: PCLStepPrintExtra()
//
//   Print a PCL step.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void PCLStepPrintExtra(FILE* out, PCLStep_p step, bool data)
{
   assert(step);

   PCLIdPrintFormatted(out, step->id, true);
   fputs(" : ", out);
   PCLPrintExternalType(out, step->properties);
   fputs(" : ", out);
   if(!PCLStepIsShell(step))
   {
      if(PCLStepIsFOF(step))
      {
         TFormulaTPTPPrint(out, step->bank, step->logic.formula, true, true);
      }
      else
      {
         ClausePCLPrint(out, step->logic.clause, true);
      }
   }
   fputs(" : ", out);
   PCLFullExprPrint(out, step->just);
   if(step->extra)
   {
      fputs(" : ", out);
      fprintf(out, "%s", step->extra);
   }
   else if(PCLStepQueryProp(step, PCLIsLemma))
   {
      fputs(" : 'lemma'", out);
   }
#ifdef NEVER_DEFINED
   fprintf(out, "/* %ld -> %f */", step->proof_tree_size, step->lemma_quality);
#endif
   if(data)
   {
#ifdef NEVER_DEFINED
      fprintf(out, " /* %#8X %6ld %6ld %3ld %3ld %3ld %3ld %4.3f */",
         step->properties,
         step->proof_dag_size,
         step->proof_tree_size,
         step->active_pm_refs,
         step->other_generating_refs,
         step->active_simpl_refs,
         step->passive_simpl_refs,
         step->lemma_quality);
#endif
       fprintf(out, " /* %3ld %3ld %3ld %3ld %3ld  */",
         step->contrib_simpl_refs,
         step->contrib_gen_refs,
         step->useless_simpl_refs,
         step->useless_gen_refs,
         step->proof_distance);
   }
}


/*-----------------------------------------------------------------------
//
// Function: PCLPropToTSTPType()
//
//   Given PCL properties, return the best string describing the
//   type.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

char * PCLPropToTSTPType(PCLStepProperties props)
{
   switch(props & PCLTypeMask)
   {
   case PCLTypeConjecture:
         return "conjecture";
   case PCLTypeQuestion:
         return "question";
   case PCLTypeNegConjecture:
         return "negated_conjecture";
   default:
         if(props&PCLIsLemma)
         {
            return "lemma";
         }
         else
         {
            if(props&PCLIsInitial)
            {
               return "axiom";
            }
            else
            {
               return "plain";
            }
         }
   }
}


/*-----------------------------------------------------------------------
//
// Function: PCLStepPrintTSTP()
//
//   Print a PCL step in TSTP format.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void PCLStepPrintTSTP(FILE* out, PCLStep_p step)
{
   assert(step);

   if(PCLStepIsClausal(step))
   {
      fprintf(out, "cnf(");
      PCLIdPrintTSTP(out, step->id);
      fputc(',', out);
      fputs(PCLPropToTSTPType(step->properties), out);
      fputc(',', out);
      if(PCLStepIsShell(step))
      {
      }
      else
      {
         ClauseTSTPCorePrint(out, step->logic.clause, true);
      }
   }
   else
   {
      fprintf(out, "fof(");
      PCLIdPrintTSTP(out, step->id);
      fputc(',', out);
      fputs(PCLPropToTSTPType(step->properties), out);
      fputc(',', out);
      if(PCLStepIsShell(step))
      {
      }
      else
      {
         TFormulaTPTPPrint(out, step->bank, step->logic.formula, true, true);
      }

   }
   fputc(',', out);
   PCLExprPrintTSTP(out, step->just, false);
   if(step->extra)
   {
      fprintf(out, ",[%s]", step->extra);
   }
   /* else if(PCLStepQueryProp(step, PCLIsLemma))
   {
      fputs(",['lemma']", out);
      }*/
   fputs(").", out);
}


/*-----------------------------------------------------------------------
//
// Function: PCLStepPrintTPTP
//
//   Print the logical part of a PCL step as a TPTP-2 clause or
//   formula.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void PCLStepPrintTPTP(FILE* out, PCLStep_p step)
{
   assert(step);

   if(PCLStepIsShell(step))
   {
      print_shell_pcl_warning(out, step);
   }
   else
   {
      if(PCLStepIsClausal(step))
      {
         ClausePrintTPTPFormat(out, step->logic.clause);
      }
      else
      {
         fprintf(out, "input_formula(");
         PCLIdPrintTSTP(out, step->id);
         fputc(',', out);
         fputs(PCLPropToTSTPType(step->properties), out);
         fputc(',', out);
         TFormulaTPTPPrint(out, step->bank, step->logic.formula, true, true);
         fputc(')',out);
      }
   }
}

/*-----------------------------------------------------------------------
//
// Function: PCLStepPrintLOP()
//
//   Print the logical part of a PCL step as a LOP clause or formula
//   (where TPTP core syntax has to stand in for missing LOP syntac).
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void PCLStepPrintLOP(FILE* out, PCLStep_p step)
{
   assert(step);

   if(PCLStepIsShell(step))
   {
      print_shell_pcl_warning(out, step);
   }
   else
   {
      if(PCLStepIsClausal(step))
      {
         ClausePrintLOPFormat(out, step->logic.clause, true);
      }
      else
      {
         TFormulaTPTPPrint(out, step->bank, step->logic.formula, true, true);
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: PCLStepPrintFormat()
//
//   Print a PCL step in the requested format.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void PCLStepPrintFormat(FILE* out, PCLStep_p step, bool data,
         OutputFormatType format)
{
   switch(format)
   {
   case pcl_format:
    PCLStepPrintExtra(out, step, data);
    break;
   case lop_format:
    PCLStepPrintLOP(out, step);
    break;
   case tptp_format:
    PCLStepPrintTPTP(out, step);
    break;
   case tstp_format:
    PCLStepPrintTSTP(out, step);
    break;
   default:
    assert(false);
    break;
   }
}


/*-----------------------------------------------------------------------
//
// Function: PCLStepPrintExampe()
//
//   Print a PCL step in the correct format for an E example file for
//   pattern-based learning. The format is as follows:
//   id: (pd, su, sf, gu, gs, ss):clause
//   where currently id is meaningless (a survivor from the old output
//   format), pd is the proof distance, su, sf, gu, gs are the
//   relative number of simplified or generated proof/nonproof
//   clauses, and ss is 0 (it used to be a subsumption count in the
//   old format, that information is no longer available in PCL).
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void PCLStepPrintExample(FILE* out, PCLStep_p step, long id,
                        long proof_steps, long total_steps)
{
   assert(!PCLStepQueryProp(step,PCLIsFOFStep));

   if(PCLStepIsShell(step))
   {
      print_shell_pcl_warning(out, step);
   }
   else
   {
      fprintf(out, "%4ld:(%ld, %f,%f,%f,%f):",
              id,
              step->proof_distance,
              step->contrib_simpl_refs/(float)(proof_steps+1),
              step->useless_simpl_refs/(float)(total_steps-proof_steps+1),
              step->contrib_gen_refs/(float)(proof_steps+1),
              step->useless_gen_refs/(float)(total_steps-proof_steps+1));
      ClausePrint(out, step->logic.clause, true);
   }
}


/*-----------------------------------------------------------------------
//
// Function: PCLStepIdCompare()
//
//   Compare two PCL steps by idents (forPTreeObj-Operations).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int PCLStepIdCompare(const void* s1, const void* s2)
{
   const PCLStep_p step1 = (const PCLStep_p)s1;
   const PCLStep_p step2 = (const PCLStep_p)s2;

   return PCLIdCompare(step1->id, step2->id);
}


/*-----------------------------------------------------------------------
//
// Function: PCLStepResetTreeData()
//
//   Reset all counters and size data elements in the step to 0.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void PCLStepResetTreeData(PCLStep_p step, bool just_weights)
{
   step->proof_dag_size        = PCLNoWeight;
   step->proof_tree_size       = PCLNoWeight;
   if(!just_weights)
   {
      step->active_pm_refs        = 0;
      step->other_generating_refs = 0;
      step->active_simpl_refs     = 0;
      step->passive_simpl_refs    = 0;
      step->pure_quote_refs       = 0;
      step->lemma_quality         = 0.0;
      step->contrib_simpl_refs    = 0;
      step->contrib_gen_refs      = 0;
      step->useless_simpl_refs    = 0;
      step->useless_gen_refs      = 0;
      step->proof_distance        = PCL_PROOF_DIST_UNKNOWN;
      PCLStepDelProp(step,PCLIsLemma|PCLIsMarked);
   }
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
