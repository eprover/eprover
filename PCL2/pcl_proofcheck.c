/*-----------------------------------------------------------------------

  File  : pcl_proofcheck.c

  Author: Stephan Schulz

  Contents

  Data types and algorithms to realize proof checking for PCL2
  protocols.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: Mon Apr  3 23:02:28 GMT 2000

  -----------------------------------------------------------------------*/

#include "pcl_proofcheck.h"



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
// Function: pcl_run_prover()
//
//   Execute command and scan the output for success. If found, return
//   true, else return false;
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

static bool pcl_run_prover(char* command, char*success)
{
   bool   res=false;
   char   line[180],*l;
   FILE*  ppipe;

   if(OutputLevel>1)
   {
      fprintf(GlobalOut, COMCHAR" Running %s\n", command);
   }
   ppipe=popen(command, "r");
   if(!ppipe)
   {
      TmpErrno = errno;
      SysError("Cannot open pipe", SYS_ERROR);
   }
   while((l=fgets(line, 180, ppipe)))
   {
      if(strstr(line,success))
      {
         res = true;
      }
      if(OutputLevel >= 3)
      {
         fprintf(GlobalOut, COMCHAR"> %s", line);
      }
   }
   pclose(ppipe);

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: pcl_verify_eprover()
//
//   Run E on the problem, return true if a proof is found.
//
// Global Variables: -
//
// Side Effects    : Memory operations, by subroutines.
//
/----------------------------------------------------------------------*/

static bool pcl_verify_eprover(ClauseSet_p problem,char *executable,
                               long time_limit)
{
   bool   res;
   DStr_p command = DStrAlloc();
   char*  name=TempFileName();
   FILE*  problemfile;

   assert(OutputFormat == TPTPFormat);

   if(!executable)
   {
      executable=E_EXEC_DEFAULT;
   }
   problemfile = OutOpen(name);
   ClauseSetPrint(problemfile, problem, true);
   OutClose(problemfile);

   DStrAppendStr(command, executable);
   DStrAppendStr(command, " --tptp-in --prefer-initial-clauses --ac-handling=None"
                 " --cpu-limit=");
   DStrAppendInt(command, time_limit);
   DStrAppendChar(command, ' ');
   DStrAppendStr(command, name);

   res = pcl_run_prover(DStrView(command),COMCHAR" Proof found!");

   if(!res)
   {
      fprintf(GlobalOut, COMCHAR" ------------Problem begin--------------\n");
      FilePrint(GlobalOut, name);
      fprintf(GlobalOut, COMCHAR" ------------Problem end----------------\n");
   }

   TempFileRemove(name);
   FREE(name);
   DStrFree(command);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: eqn_print_otter()
//
//   Print a literal in Otter format.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void eqn_print_otter(FILE* out, Eqn_p eqn)
{
   assert(OutputFormat == LOPFormat);

   if(EqnIsEquLit(eqn))
   {
      if(EqnIsPositive(eqn))
      {
         EqnPrint(out, eqn, false, true);
      }
      else
      {
         fputc('-', out);
         EqnPrint(out, eqn, true, true);
      }
   }
   else
   {
      if(eqn->lterm==eqn->bank->true_term)
      {
         assert(eqn->rterm==eqn->bank->true_term);
         /* Special case, one more hack */;
         if(EqnIsPositive(eqn))
         {
            fputs("$T", out);
         }
         else
         {
            fputs("$F", out);
         }
      }
      else
      {
         /* Nonequational literal */
         if(EqnIsNegative(eqn))
         {
            fputc('-', out);
         }
         else
         {
            fputc(' ', out);
         }
         TBPrintTerm(out, eqn->bank, eqn->lterm, true);
      }
   }
}

/*-----------------------------------------------------------------------
//
// Function: clause_print_otter()
//
//   Print a clause in Otter format.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void clause_print_otter(FILE* out, Clause_p clause)
{
   Eqn_p handle;

   if(ClauseIsEmpty(clause))
   {
      /* For otter, the empty clause is propositional false: */

      fputs("$F.", out);
   }
   else
   {
      handle=clause->literals;
      eqn_print_otter(out, handle);
      handle=handle->next;
      while(handle)
      {
         fputs("|\n", out);
         eqn_print_otter(out, handle);
         handle=handle->next;
      }
      fputs(".\n", out);
   }
}

/*-----------------------------------------------------------------------
//
// Function: clause_set_print_otter()
//
//   Print a set of clauses in Otter format (with prolog-variables).
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void clause_set_print_otter(FILE* out, ClauseSet_p set)
{
   Clause_p handle;

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
          handle->succ)
   {
      clause_print_otter(out, handle);
      fputc('\n', out);
   }
}


/*-----------------------------------------------------------------------
//
// Function: pcl_verify_otter()
//
//   Run Otter on the problem, return true if a proof is found.
//
// Global Variables: -
//
// Side Effects    : Memory operations, by subroutines.
//
/----------------------------------------------------------------------*/

static bool pcl_verify_otter(ClauseSet_p problem,char *executable,
                             long time_limit)
{
   bool   res;
   DStr_p command = DStrAlloc();
   char*  name=TempFileName();
   FILE*  problemfile;

   if(!executable)
   {
      executable=OTTER_EXEC_DEFAULT;
   }
   problemfile = OutOpen(name);
   fprintf(problemfile,
           "set(prolog_style_variables).\n"
           "clear(print_kept).\n"
           "clear(print_new_demod).\n"
           "clear(print_back_demod).\n"
           "clear(print_back_sub).\n"
           "set(auto).\n"
           "set(input_sos_first).\n"
           /* "set(para_from_vars).\n" */
           "assign(max_seconds, %ld).\n\n"
           "assign(max_mem, 100000).\n\n"
           "list(usable).\n\n"
           "equal(X,X).\n",time_limit);
   clause_set_print_otter(problemfile, problem);
   fprintf(problemfile,
           "end_of_list.\n");
   OutClose(problemfile);

   DStrAppendStr(command, executable);
   DStrAppendStr(command, " < ");
   DStrAppendStr(command, name);
   DStrAppendStr(command, " 2> /dev/null");

   res = pcl_run_prover(DStrView(command), "-------- PROOF --------");

   if(!res)
   {
      fprintf(GlobalOut, COMCHAR" ------------Problem begin--------------\n");
      FilePrint(GlobalOut, name);
      fprintf(GlobalOut, COMCHAR" ------------Problem end----------------\n");
   }

   TempFileRemove(name);
   FREE(name);
   DStrFree(command);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: sig_print_dfg()
//
//   Collect function symbols from set and print them in DFG syntax.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void sig_print_dfg(FILE* out, ClauseSet_p set, Sig_p sig)
{
   long     *symbol_distrib;
   FunCode  i;

   symbol_distrib = SizeMalloc((sig->size)*sizeof(long));
   for(i=0; i< sig->size; i++)
   {
      symbol_distrib[i] = 0;
   }
   ClauseSetAddSymbolDistribution(set, symbol_distrib);

   fprintf(out,"list_of_symbols.\nfunctions[(spass_hack,0)");
   for(i=sig->internal_symbols+1; i<sig->size; i++)
   {
      if(symbol_distrib[i]&&!SigIsPredicate(sig,i))
      {
         fprintf(out, ",(%s,%d)",
                 SigFindName(sig, i),
                 SigFindArity(sig, i));
      }
   }
   fprintf(out,"].\npredicates[(spass_pred_dummy,0)");
   for(i=sig->internal_symbols+1; i<sig->size; i++)
   {
      if(symbol_distrib[i]&&SigIsPredicate(sig,i))
      {
         fprintf(out, ",(%s,%d)",
                 SigFindName(sig, i),
                 SigFindArity(sig, i));
      }
   }
   fprintf(out,"].\nend_of_list.\n");
}


/*-----------------------------------------------------------------------
//
// Function: eqn_print_dfg()
//
//   Print an equation in DFG syntax.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void eqn_print_dfg(FILE* out, Eqn_p eqn)
{
   assert(OutputFormat == LOPFormat);
   assert(!EqnUseInfix);

   if(EqnIsNegative(eqn))
   {
      fputs("not(", out);
   }
   if(eqn->lterm==eqn->bank->true_term)
   {
      assert(eqn->rterm==eqn->bank->true_term);
      /* Special case, one more hack */;
      fputs("equal(spass_hack,spass_hack)", out);
   }
   else
   {
      EqnPrint(out,eqn,EqnIsNegative(eqn),true);
   }
   if(EqnIsNegative(eqn))
   {
      fputc(')', out);
   }
}


/*-----------------------------------------------------------------------
//
// Function: clause_print_dfg()
//
//   Print a clause in dfg format.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void clause_print_dfg(FILE* out, Clause_p clause)
{
   Eqn_p    handle;
   PTree_p  variables = NULL, cell;
   PStack_p stack;
   Term_p   var;
   long     var_no;

   fprintf(out, "clause(");
   var_no = ClauseCollectVariables(clause, &variables);
   if(var_no)
   {
      fprintf(out, "forall([");
      stack=PTreeTraverseInit(variables);
      cell = PTreeTraverseNext(stack);
      if(cell)
      {
         assert(clause->literals);
         var = cell->key;
         TBPrintTerm(out, clause->literals->bank, var, true);
         while((cell=PTreeTraverseNext(stack)))
         {
            fputc(',', out);
            var = cell->key;
            TBPrintTerm(out, clause->literals->bank, var, true);
         }
      }
      PTreeTraverseExit(stack);
      PTreeFree(variables);
      fprintf(out, "],");
   }
   fprintf(out, "or(");

   handle=clause->literals;
   if(handle)
   {
      eqn_print_dfg(out, handle);
      handle=handle->next;
      while(handle)
      {
         fputs(",", out);
         eqn_print_dfg(out, handle);
         handle=handle->next;
      }
   }
   else
   {
      fputs("not(equal(spass_hack,spass_hack))",out);
   }
   fprintf(out, ")%c, c%ld ).", (var_no?')':' '), clause->ident);
}


/*-----------------------------------------------------------------------
//
// Function: clause_set_print_dfg()
//
//   Print a set of clauses in DFG format.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void clause_set_print_dfg(FILE* out, ClauseSet_p set)
{
   Clause_p handle;

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
          handle->succ)
   {
      clause_print_dfg(out, handle);
      fputc('\n', out);
   }
}


/*-----------------------------------------------------------------------
//
// Function: pcl_verify_spass()
//
//   Run SPASS on the problem, return true if a proof is found.
//
// Global Variables: -
//
// Side Effects    : Memory operations, by subroutines.
//
/----------------------------------------------------------------------*/

static bool pcl_verify_spass(ClauseSet_p problem,char *executable,
                             long time_limit, Sig_p sig)
{
   bool   res;
   DStr_p command = DStrAlloc();
   char*  name=TempFileName();
   FILE*  problemfile;

   if(!executable)
   {
      executable=SPASS_EXEC_DEFAULT;
   }
   problemfile = OutOpen(name);
   fprintf(problemfile,
           "begin_problem(Unknown).\n");

   sig_print_dfg(problemfile, problem, sig);

   fprintf(problemfile, "list_of_clauses(axioms,cnf).\n");
   clause_set_print_dfg(problemfile, problem);
   fprintf(problemfile, "end_of_list.\n"
           "list_of_settings(SPASS).\n"
           "set_flag(TimeLimit, %ld).\n"
           "end_of_list.\n"
           "end_problem.\n", time_limit);
   OutClose(problemfile);

   DStrAppendStr(command, executable);
   DStrAppendStr(command, " ");
   DStrAppendStr(command, name);

   res = pcl_run_prover(DStrView(command), "Proof found.");
   if(!res)
   {
      fprintf(GlobalOut, COMCHAR" ------------Problem begin--------------\n");
      FilePrint(GlobalOut, name);
      fprintf(GlobalOut, COMCHAR" ------------Problem end----------------\n");
   }

   TempFileRemove(name);
   FREE(name);
   DStrFree(command);
   return res;
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: PCLCollectPreconds()
//
//   Collect copies of all clauses quoted in the justification of step
//   in set. Return number of clauses.
//
// Global Variables: -
//
// Side Effects    : Memory operations, changes set.
//
/----------------------------------------------------------------------*/

long PCLCollectPreconds(PCLProt_p prot, PCLStep_p step, ClauseSet_p
                        set)
{
   PTree_p   tree = NULL;
   PCLStep_p handle;
   Clause_p  clause;
   long      res = 0;

   assert(prot && step && set);

   PCLExprCollectPreconds(prot, step->just, &tree);
   while(tree)
   {
      handle = PTreeExtractRootKey(&tree);
      if(PCLStepIsClausal(handle))
      {
         clause = ClauseCopy(handle->logic.clause, prot->terms);
         ClauseSetInsert(set, clause);
         res++;
      }
      else
      {
         Warning("Cannot currently handle full first-order format!");
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: PCLNegSkolemizeClause()
//
//   Add the clauses resulting from negating and skolemizing
//   step->clause to set. The implementation is not very efficient,
//   but that should not matter for our application.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

long PCLNegSkolemizeClause(PCLProt_p prot, PCLStep_p step, ClauseSet_p
                           set)
{
   long     res = 0;
   Clause_p clause, new_clause;
   Eqn_p    handle, copy;

   if(PCLStepIsClausal(step))
   {
      clause = ClauseSkolemize(step->logic.clause, prot->terms);

      for(handle=clause->literals; handle; handle=handle->next)
      {
         copy = EqnCopy(handle,prot->terms);
         EqnFlipProp(copy, EPIsPositive);
         new_clause=ClauseAlloc(copy);
         ClauseSetTPTPType(new_clause, CPTypeHypothesis);
         ClauseSetInsert(set, new_clause);
         res++;
      }
      ClauseFree(clause);
   }
   else
   {
      Warning("Cannot currently handle full first-order format!");
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: PCLGenerateCheck()
//
//   Generate a clause set that is unsatisfiable if the clause in step
//   is a logical conclusion of the precondition. For initial steps,
//   return NULL.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

ClauseSet_p PCLGenerateCheck(PCLProt_p prot, PCLStep_p step)
{
   ClauseSet_p set = ClauseSetAlloc();

   if(PCLCollectPreconds(prot, step, set))
   {
      PCLNegSkolemizeClause(prot, step, set);
   }
   else
   {
      ClauseSetFree(set);
      set = NULL;
   }
   return set;
}


/*-----------------------------------------------------------------------
//
// Function: PCLStepCheck()
//
//   Check the validity of a single PCL step. Return true if it checks
//   ok, false otherwise. At the moment, just print the generated
//   problem.
//
// Global Variables: -
//
// Side Effects    : By called functions, may print output.
//
/----------------------------------------------------------------------*/

PCLCheckType PCLStepCheck(PCLProt_p prot, PCLStep_p step, ProverType
                          prover, char* executable, long time_limit)
{
   ClauseSet_p problem;
   PCLCheckType res=CheckFail;

   if(step->just->op==PCLOpSplitClause)
   {
      return CheckNotImplemented;
   }
   problem = PCLGenerateCheck(prot, step);
   if(!problem)
   {
      return CheckByAssumption;
   }
   else
   {
      switch(prover)
      {
      case EProver:
            if(pcl_verify_eprover(problem,executable,time_limit))
            {
               res = CheckOk;
            }
            break;
      case Otter:
            if(pcl_verify_otter(problem,executable,time_limit))
            {
               res = CheckOk;
            }
            break;
      case Spass:
            if(pcl_verify_spass(problem,executable,time_limit,prot->terms->sig))
            {
               res = CheckOk;
            }
            break;
      default:
            assert(false && "Not yet implemented");
            break;
      }
      ClauseSetFree(problem);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: PCLProtCheck()
//
//   Check all steps in a PCL listing. Return number of successful
//   steps.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long PCLProtCheck(PCLProt_p prot, ProverType prover, char* executable,
                  long time_limit, long *unchecked)
{
   PStack_p      trav_stack,stack = PStackAlloc();
   PStackPointer i;
   PTree_p       cell;
   PCLStep_p     step;
   long          res=0;
   PCLCheckType  check;

   *unchecked = 0;
   trav_stack = PTreeTraverseInit(prot->steps);
   while((cell=PTreeTraverseNext(trav_stack)))
   {
      step=cell->key;
      PStackPushP(stack, step);
   }
   PTreeTraverseExit(trav_stack);

   for(i=0; i<PStackGetSP(stack); i++)
   {
      step = PStackElementP(stack,i);
      if(OutputLevel)
      {
         fprintf(GlobalOut, COMCHAR" Checking ");
         PCLStepPrint(GlobalOut, step);
         fputc('\n', GlobalOut);
      }
      check = PCLStepCheck(prot, step, prover, executable,
                           time_limit);
      switch(check)
      {
      case CheckByAssumption:
            OUTPRINT(1,COMCHAR" Checked (by assumption)\n\n");
            res++;
            break;
      case CheckOk:
            OUTPRINT(1,COMCHAR" Checked (by prover)\n\n");
            res++;
            break;
      case CheckFail:
            OUTPRINT(1,COMCHAR" FAILED\n\n");
            break;
      case CheckNotImplemented:
            OUTPRINT(1,COMCHAR" Check not implemented, assuming true!\n\n");
            (*unchecked)++;
            break;
      default:
            assert(false);
            break;
      }
   }
   PStackFree(stack);
   return res;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
