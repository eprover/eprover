/*-----------------------------------------------------------------------

File  : ccl_formulafunc.c

Author: Stephan Schulz

Contents

  Higher level formula functions that need to know about sets (and
  CNFing).

  Copyright 2004 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sun Apr  4 14:12:05 CEST 2004
    New

-----------------------------------------------------------------------*/

#include "ccl_formulafunc.h"
#include "ccl_clausefunc.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/
extern bool app_encode;

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/

typedef TFormula_p (*FOOLFormulaProcessor)(TFormula_p, TB_p);

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: fool_process_formula()
//
//   Applies processor to form. If formula is changed it alters
//   the proof object by saying FOOL processing has been applied.
//
// Global Variables: -
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

bool fool_process_formula(WFormula_p form, TB_p terms,
                          FOOLFormulaProcessor processor)
{
   TFormula_p original = form->tformula;
   bool       changed = false;

   form->tformula = processor(original, terms);

   if(form->tformula != original)
   {
      WFormulaPushDerivation(form, DCFoolUnroll, NULL, NULL);
      changed = true;
   }

   return changed;
}

/*-----------------------------------------------------------------------
//
// Function: ignore_include()
//
//   Ignore includes and echoes the ignored declaration. Used for
//   app encoding only.
//
// Global Variables: -
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void ignore_include(Scanner_p in)
{
   assert(app_encode);

   AcceptInpId(in, "include");
   AcceptInpTok(in, OpenBracket);
   CheckInpTok(in, SQString);
   char* name = DStrCopyCore(AktToken(in)->literal);
   NextToken(in);
   AcceptInpTok(in, CloseBracket);
   AcceptInpTok(in, Fullstop);

   fprintf(stdout, "include('%s').\n", name);
   FREE(name);
}


/*-----------------------------------------------------------------------
//
// Function: answer_lit_alloc()
//
//   Allocate a FOF literal of the form ~$answer(skn(x1, ... xn)),
//   where the xi are the variables on varstack and skn is a new
//   skolem symbol.
//
// Global Variables: -
//
// Side Effects    : Memory operations, changes the term bank.
//
/----------------------------------------------------------------------*/

TFormula_p answer_lit_alloc(TB_p terms, PStack_p varstack)
{
   TFormula_p res;
   Term_p handle;

   handle       = TBAllocNewSkolem(terms, varstack, NULL);
   res          = TermTopAlloc(terms->sig->answer_code, 1);
   res->args[0] = handle;
   res          = TBTermTopInsert(terms, res);
   res          = EqnTermsTBTermEncode(terms, res, terms->true_term, false, PENormal);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: verify_name()
//
//   If name_selector is NULL, return true. Otherwise, check if
//   info->name is in name_selector. Return true if yes, false
//   otherwise.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static bool verify_name(StrTree_p *name_selector, ClauseInfo_p info)
{
   StrTree_p handle;

   if(!(*name_selector))
   {
      return true;
   }
   handle = StrTreeFind(name_selector, info->name);
   if(!handle)
   {
      return false;
   }
   handle->val1.i_val = 1; /* Mark as found */
   return true;
}


/*-----------------------------------------------------------------------
//
// Function: check_all_found()
//
//   Check if all names in name_selector are marked as found. Print a
//   useful error message and terminate otherwise.
//
// Global Variables: -
//
// Side Effects    : Possibly exit, memory operations.
//
/----------------------------------------------------------------------*/

static void check_all_found(Scanner_p in, StrTree_p name_selector)
{
   PStack_p trav_stack, err_stack = PStackAlloc();
   StrTree_p handle;

   trav_stack = StrTreeTraverseInit(name_selector);

   while((handle = StrTreeTraverseNext(trav_stack)))
   {
      if(!handle->val1.i_val)
      {
         PStackPushP(err_stack, handle->key);
      }
   }
   StrTreeTraverseExit(trav_stack);

   if(!PStackEmpty(err_stack))
   {
      DStr_p err_str = DStrAlloc();
      char *sep = "";
      PStackPointer i;

      if(in->include_pos)
      {
         DStrSet(err_str, in->include_pos);
         DStrAppendStr(err_str, " ");
      }
      DStrAppendStr(err_str, "\"include\" statement cannot "
                    "find the following requested clauses/formulae in ");
      DStrAppendDStr(err_str, Source(in));
      DStrAppendStr(err_str, ": ");

      for(i=0; i< PStackGetSP(err_stack); i++)
      {
         DStrAppendStr(err_str, sep);
         DStrAppendStr(err_str, PStackElementP(err_stack, i));
         sep = ", ";
      }
      Error(DStrView(err_str), INPUT_SEMANTIC_ERROR);
      DStrFree(err_str);
   }

   PStackFree(err_stack);
}

/*-----------------------------------------------------------------------
//
// Function: do_fool_unroll()
//
//   Unroll boolean arguments of terms. For example, subformula
//   "f(a, p&q) = a" is replaced with "(~(p&q)|f(a,$true)=a) &
//   (p&q)|f(a, $false)=a".
//
// Global Variables: -
//
// Side Effects    : Changes enclosed formula
//
/----------------------------------------------------------------------*/

TFormula_p do_fool_unroll(TFormula_p form, TB_p terms)
{
   TFormula_p unrolled1 = NULL;
   TFormula_p unrolled2 = NULL;
   if(TFormulaIsLiteral(terms->sig, form))
   {
      TermPos_p pos = PStackAlloc();
      PStackPushP(pos, form);
      PStackPushInt(pos, 0);
      if(!TermFindFOOLSubterm(form->args[0], pos))
      {
         PStackDiscardTop(pos);
         PStackPushInt(pos, 1);
         if(!TermFindFOOLSubterm(form->args[1], pos))
         {
            PStackReset(pos);
         }
      }

      if(!PStackEmpty(pos))
      {
         TFormula_p subform =
            ((Term_p)PStackBelowTopP(pos))->args[PStackTopInt(pos)];
         assert(TypeIsBool(subform->type));

         if(subform->f_code > terms->sig->internal_symbols)
         {
            // This is a Skolem symbol that is not yet encoded as literal
            subform = EqnTermsTBTermEncode(terms, subform, terms->true_term,
                                           true, PENormal);
         }

         Term_p subform_t = TBTermPosReplace(terms, terms->true_term, pos,
                                             DEREF_NEVER, 0, subform);
         Term_p subform_f = TBTermPosReplace(terms, terms->false_term, pos,
                                             DEREF_NEVER, 0, subform);

         TFormula_p neg_subf = TFormulaNegate(subform, terms);

         // ~(p&q)|f(a,$true)=a from the above example
         TFormula_p fst_impl = TFormulaFCodeAlloc(terms, terms->sig->or_code,
                                                   neg_subf, subform_t);
         // (p&q)|f(a, $false)=a
         TFormula_p snd_impl = TFormulaFCodeAlloc(terms, terms->sig->or_code,
                                                   subform, subform_f);

         // the whole formula
         form = TFormulaFCodeAlloc(terms, terms->sig->and_code,
                                    do_fool_unroll(fst_impl, terms),
                                    do_fool_unroll(snd_impl, terms));
      }
      PStackFree(pos);
   }
   else
   {
      if(TFormulaIsQuantified(terms->sig, form))
      {
         unrolled1 = do_fool_unroll(form->args[1], terms);
         if(form->args[1] != unrolled1)
         {
            form = TFormulaQuantorAlloc(terms, form->f_code,
                                        form->args[0], unrolled1);
         }
      }
      else
      {
         if(TFormulaHasSubForm1(terms->sig, form))
         {
            unrolled1 = do_fool_unroll(form->args[0], terms);
         }
         if(TFormulaHasSubForm2(terms->sig, form))
         {
            unrolled2 = do_fool_unroll(form->args[1], terms);
         }

         if((unrolled1 && unrolled1 != form->args[0]) ||
            (unrolled2 && unrolled2 != form->args[1]))
         {
            form = TFormulaFCodeAlloc(terms, form->f_code, unrolled1, unrolled2);
         }
      }
   }
   return form;
}


/*-----------------------------------------------------------------------
//
// Function: do_bool_eqn_replace ()
//
//   Replace boolean equations with equivalences. Goes inside literals
//   as well. For example, "f(a, p = q) = b" will be translated to
//   "f(a, p <=> q) = b".
//
// Global Variables: -
//
// Side Effects    : Changes enclosed formula
//
/----------------------------------------------------------------------*/

TFormula_p do_bool_eqn_replace(TFormula_p form, TB_p terms)
{
   const Sig_p sig = terms->sig;
   bool  changed   = false;
   if(form->f_code == sig->eqn_code || form->f_code == sig->neqn_code)
   {
      assert(form->arity == 2);
      if(!TermIsVar(form->args[0]) && !TermIsVar(form->args[1]) &&
         SigIsLogicalSymbol(terms->sig, form->args[0]->f_code) &&
         SigIsLogicalSymbol(terms->sig, form->args[1]->f_code) &&
         form->args[1] != terms->true_term)
      {
         // DAS literal is encoded as <predicate> = TRUE.
         // Our boolean equalities are <formula> = <formula>
         form = TFormulaFCodeAlloc(terms,
                                   form->f_code == terms->sig->eqn_code ?
                                     terms->sig->equiv_code : terms->sig->xor_code,
                                   do_bool_eqn_replace(form->args[0], terms),
                                   do_bool_eqn_replace(form->args[1], terms));
         changed = true;
      }
   }
   if(!TermIsVar(form) && !changed)
   {
      TFormula_p tmp = TermTopAlloc(form->f_code, form->arity);
      tmp->type = form->type;
      for(int i=0; i<form->arity; i++)
      {
         tmp->args[i] = do_bool_eqn_replace(form->args[i], terms);
      }
      form = TBTermTopInsert(terms, tmp);
   }
   return form;
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: TformulaCollectClause()
//
//   Given a term-encoded formula that is a disjunction of literals,
//   transform it into a clause.
//
// Global Variables: -
//
// Side Effects    : Same as in TFormulaConjunctiveToCNF() below.
//
/----------------------------------------------------------------------*/

/* Clause_p TformulaCollectClause(TFormula_p form, TB_p terms, */
/*                                VarBank_p fresh_vars) */
/* { */
/*    Clause_p res; */
/*    Eqn_p lit_list = NULL, tmp_list = NULL, lit; */
/*    PStack_p stack, lit_stack; */
/*    Subst_p  normsubst = SubstAlloc(); */

/*    /\*printf("tformula_collect_clause(): "); */
/*      TFormulaTPTPPrint(GlobalOut, terms, form, true); */
/*      printf("\n"); *\/ */

/*    litstack = PStackAlloc(); */
/*    stack = PStackAlloc(); */

/*    PStackPushP(stack, form); */
/*    while(!PStackEmpty(stack)) */
/*    { */
/*       form = PStackPopP(stack); */
/*       if(form->f_code == terms->sig->or_code) */
/*       { */
/*          PStackPushP(stack, form->args[0]); */
/*          PStackPushP(stack, form->args[1]); */
/*       } */
/*       else */
/*       { */
/*          assert(TFormulaIsLiteral(terms->sig, form)); */
/*          lit = EqnTBTermDecode(terms, form); */
/*          PStackPushP(lit_stack, lit); */

/*       } */
/*    } */
/*    PStackFree(stack); */
/*    while(!PStackEmpty(lit_stack)) */
/*    { */
/*       lit = PStackPopP(lit_stack); */
/*       EqnListInsertFirst(&lit_list, lit); */
/*    } */
/*    PStackFree(lit_stack); */

/*    VarBankResetVCounts(fresh_vars); */
/*    NormSubstEqnList(lit_list, normsubst, fresh_vars); */
/*    tmp_list = EqnListCopy(lit_list, terms); */
/*    res = ClauseAlloc(tmp_list); */
/*    EqnListFree(lit_list); /\* Created just for this *\/ */
/*    SubstDelete(normsubst); */
/*    return res; */
/* } */



/*-----------------------------------------------------------------------
//
// Function: WFormulaConjectureNegate()
//
//   If formula is a conjecture, negate it and delete that property
//   (but set WPInitialConjecture). Returns true if formula was a
//   conjecture.
//
// Global Variables: -
//
// Side Effects    : Changes formula
//
/----------------------------------------------------------------------*/

bool WFormulaConjectureNegate(WFormula_p wform)
{
   FormulaProperties ftype = FormulaQueryType(wform);

   if(ftype==CPTypeConjecture)
   {
      wform->tformula = TFormulaFCodeAlloc(wform->terms,
                                           wform->terms->sig->not_code,
                                           wform->tformula,
                                           NULL);
      FormulaSetType(wform, CPTypeNegConjecture);
      DocFormulaModificationDefault(wform, inf_neg_conjecture);
      WFormulaPushDerivation(wform, DCNegateConjecture, NULL, NULL);
      return true;
   }
   return false;
}


/*-----------------------------------------------------------------------
//
// Function: TFormulaAnnotateQuestion()
//
//   Take a formula of the form ((\exists X)*.F) and convert it to
//   ((\exists Xi)*.(F&~$answer(skn(X1,...Xn))), i.e. add an answer
//   literal encoding all leading existentially quantified variables.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

TFormula_p TFormulaAnnotateQuestion(TB_p terms,
                                    TFormula_p form,
                                    NumTree_p *question_assoc)
{
   TFormula_p res, handle, tmp;
   PStack_p varstack = PStackAlloc();

   handle = form;
   while(handle->f_code == terms->sig->qex_code)
   {
      PStackPushP(varstack, handle->args[0]);
      handle = handle->args[1];
   }
   if(PStackEmpty(varstack))
   {
      /* Not a "real" question, nothing to do */
      res = form;
   }
   else
   {
      tmp = answer_lit_alloc(terms, varstack);
      res = TFormulaFCodeAlloc(terms, terms->sig->and_code, handle, tmp);
      while(!PStackEmpty(varstack))
      {
         handle = PStackPopP(varstack);
         res    = TFormulaFCodeAlloc(terms, terms->sig->qex_code, handle, res);
      }
   }
   PStackFree(varstack);
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: WFormulaAnnotateQuestion()
//
//   If formula is a question, convert it into the equivalent
//   conjecture with answer annotation. Returns true if formula was a
//   question. Add the association of the new skolem symbol in the
//   answer literal to the clause id.
//
// Global Variables: -
//
// Side Effects    : Changes formula
//
/----------------------------------------------------------------------*/

bool WFormulaAnnotateQuestion(WFormula_p wform, bool add_answer_lits,
                              bool conjectures_are_questions,
                              NumTree_p *question_assoc)
{
   if(FormulaQueryProp(wform, CPTypeQuestion)||
      (FormulaQueryProp(wform, CPTypeConjecture)&&conjectures_are_questions))
   {
      if(add_answer_lits)
      {
         wform->tformula = TFormulaAnnotateQuestion(wform->terms,
                                                    wform->tformula,
                                                    question_assoc);
      }
      FormulaSetType(wform, CPTypeConjecture);
      DocFormulaModificationDefault(wform, inf_annotate_question);
      WFormulaPushDerivation(wform, DCAnnoQuestion, NULL, NULL);
      return true;
   }
   return false;
}




/*-----------------------------------------------------------------------
//
// Function: FormulaSetPreprocConjectures()
//
//   Negate all conjectures to make the implication to prove into an
//   formula set that is inconsistent if the implication is true. Note
//   that multiple conjectures are implicitely disjunctively
//   connected! Returns number of conjectures.
//
// Global Variables: -
//
// Side Effects    : Changes formula, may print warning if number of
//                   conjectures is different from 1.
//
/----------------------------------------------------------------------*/

long FormulaSetPreprocConjectures(FormulaSet_p set,
                                  FormulaSet_p archive,
                                  bool add_answer_lits,
                                  bool conjectures_are_questions)
{
   long res = 0;
   WFormula_p handle;

   handle = set->anchor->succ;

   while(handle!=set->anchor)
   {
      WFormulaAnnotateQuestion(handle, add_answer_lits,
                               conjectures_are_questions,
                               NULL);

      if(WFormulaConjectureNegate(handle))
      {
         res++;
      }
      handle = handle->succ;
   }
   return res;
}




/*-----------------------------------------------------------------------
//
// Function: WFormulaSimplify()
//
//   Apply standard simplifications to the wrapped formula. Return
//   true if the formula has changed. Outputs inferences!
//
// Global Variables: -
//
// Side Effects    : Output, memory operations.
//
/----------------------------------------------------------------------*/

bool WFormulaSimplify(WFormula_p form, TB_p terms)
{
   TFormula_p simplified;
   bool res = false;

   //assert(!terms->freevarsets);
   simplified = TFormulaSimplify(terms, form->tformula, true);
   // TBVarSetStoreFree(terms);

   if(simplified!=form->tformula)
   {
      form->tformula = simplified;
      DocFormulaModificationDefault(form, inf_fof_simpl);
      WFormulaPushDerivation(form, DCFofSimplify, NULL, NULL);
      res = true;
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: WFormulaCNF()
//
//   Transform the formula of a wrapped formula into CNF.
//
// Global Variables: -
//
// Side Effects    : Changes formula, memory operations
//
/----------------------------------------------------------------------*/

long WFormulaCNF(WFormula_p form, ClauseSet_p set,
                 TB_p terms, VarBank_p fresh_vars)
{
   if(form->is_clause)
   {
      Clause_p clause = WFormClauseToClause(form);
      ClausePushDerivation(clause, DCFofQuote, form, NULL);
      ClauseSetInsert(set, clause);
      return 1;
   }
   WTFormulaConjunctiveNF(form, terms);
   return TFormulaToCNF(form, FormulaQueryType(form),
                        set, terms, fresh_vars);
}



/*-----------------------------------------------------------------------
//
// Function: WFormulaCNF2()
//
//   Transform the formula of a wrapped formula into CNF.
//
// Global Variables: -
//
// Side Effects    : Changes formula, memory operations
//
/----------------------------------------------------------------------*/

long WFormulaCNF2(WFormula_p form, ClauseSet_p set,
                  TB_p terms, VarBank_p fresh_vars,
                  long miniscope_limit)
{
   if(form->is_clause)
   {
      Clause_p clause = WFormClauseToClause(form);
      ClausePushDerivation(clause, DCFofQuote, form, NULL);
      ClauseSetInsert(set, clause);
      return 1;
   }
   WTFormulaConjunctiveNF3(form, terms, miniscope_limit);
   return TFormulaToCNF(form, FormulaQueryType(form),
                        set, terms, fresh_vars);
}



/*-----------------------------------------------------------------------
//
// Function: FormulaSetSimplify()
//
//   Apply standard FOF simplification rules to all formulae in the
//   set. Returns number of changed formulas.
//
// Global Variables: -
//
// Side Effects    : Changes formulas, output of inference steps.
//
/----------------------------------------------------------------------*/

long FormulaSetSimplify(FormulaSet_p set, TB_p terms)
{
   WFormula_p handle;
   long res = 0;
   long old_nodes = TBNonVarTermNodes(terms);
   long gc_threshold = old_nodes*TFORMULA_GC_LIMIT;
   bool changed;

   handle = set->anchor->succ;
   while(handle!=set->anchor)
   {
      // printf("Simplifying: \n");
      // WFormulaPrint(stdout, handle, true);
      // printf("\n");
      changed =  WFormulaSimplify(handle, terms);
      // printf("Simplified %d\n", changed);
      // WFormulaPrint(stdout, handle, true);
      // printf("\n");
      if(changed)
      {
         res++;
         if(TBNonVarTermNodes(terms)>gc_threshold)
         {
            assert(terms == handle->terms);
            GCCollect(terms->gc);
            old_nodes = TBNonVarTermNodes(terms);
            gc_threshold = old_nodes*TFORMULA_GC_LIMIT;
         }
      }
      handle = handle->succ;
   }
   // printf("All simplified\n");
   if(TBNonVarTermNodes(terms)!=old_nodes)
   {
      GCCollect(terms->gc);
   }
   // printf("Garbage collected\n");
   return res;

}

/*-----------------------------------------------------------------------
//
// Function: FormulaSetCNF()
//
//   Transform all formulae in set into CNF. Return number of clauses
//   generated.
//
// Global Variables: -
//
// Side Effects    : Plenty of memory stuff.
//
/----------------------------------------------------------------------*/

long FormulaSetCNF(FormulaSet_p set, FormulaSet_p archive,
                   ClauseSet_p clauseset, TB_p terms,
                   VarBank_p fresh_vars, GCAdmin_p gc)
{
   WFormula_p form, handle;
   long res = 0;
   long old_nodes = TBNonVarTermNodes(terms);
   long gc_threshold = old_nodes*TFORMULA_GC_LIMIT;

   FormulaSetSimplify(set, terms);
   // printf("FormulaSetSimplify done\n");
   TFormulaSetIntroduceDefs(set, archive, terms);
   // printf("Definitions introduced\n");

   while(!FormulaSetEmpty(set))
   {
      handle = FormulaSetExtractFirst(set);
      // WFormulaPrint(stdout, handle, true);
      // fprintf(stdout, "\n");
      form = WFormulaFlatCopy(handle);
      FormulaSetInsert(archive, handle);
      WFormulaPushDerivation(form, DCFofQuote, handle, NULL);
      handle = form;
      res += WFormulaCNF(handle,clauseset, terms, fresh_vars);
      FormulaSetInsert(archive, handle);
      if(handle->tformula &&
         (TBNonVarTermNodes(terms)>gc_threshold))
      {
         assert(terms == handle->terms);
         GCCollect(gc);
         old_nodes = TBNonVarTermNodes(terms);
         gc_threshold = old_nodes*TFORMULA_GC_LIMIT;
      }
   }
   if(TBNonVarTermNodes(terms)!=old_nodes)
   {
      GCCollect(gc);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: FormulaSetCNF2()
//
//   Transform all formulae in set into CNF. Return number of clauses
//   generated.
//
// Global Variables: -
//
// Side Effects    : Plenty of memory stuff.
//
/----------------------------------------------------------------------*/

long FormulaSetCNF2(FormulaSet_p set, FormulaSet_p archive,
                    ClauseSet_p clauseset, TB_p terms,
                    VarBank_p fresh_vars, GCAdmin_p gc, long miniscope_limit)
{
   WFormula_p form, handle;
   long res = 0;
   long old_nodes = TBNonVarTermNodes(terms);
   long gc_threshold = old_nodes*TFORMULA_GC_LIMIT;

   TFormulaSetUnrollFOOL(set, archive, terms);

   //printf("# Introducing definitions\n");
   TFormulaSetIntroduceDefs(set, archive, terms);
   //printf("# Definitions introduced\n");

   while(!FormulaSetEmpty(set))
   {
      handle = FormulaSetExtractFirst(set);
      //WFormulaPrint(stdout, handle, true);
      //fprintf(stdout, "\n");
      form = WFormulaFlatCopy(handle);
      FormulaSetInsert(archive, handle);
      WFormulaPushDerivation(form, DCFofQuote, handle, NULL);
      handle = form;
      res += WFormulaCNF2(handle,clauseset, terms, fresh_vars,
                          miniscope_limit);
      FormulaSetInsert(archive, handle);
      if(handle->tformula &&
         (TBNonVarTermNodes(terms)>gc_threshold))
      {
         assert(terms == handle->terms);
         GCCollect(gc);
         old_nodes = TBNonVarTermNodes(terms);
         gc_threshold = old_nodes*TFORMULA_GC_LIMIT;
      }
   }
   if(TBNonVarTermNodes(terms)!=old_nodes)
   {
      GCCollect(gc);
   }
   return res;
}




/*-----------------------------------------------------------------------
//
// Function: FormulaAndClauseSetParse()
//
//   Parse a mixture of clauses and formulas (if the syntax supports
//   it). Return number of elements parsed (even if discarded by
//   filter). Watch list clauses are parsed as clauses in wlset,
//   everything else (even clauses) is parsed as a formula and put
//   into fset.
//
// Global Variables: -
//
// Side Effects    : Input, changes termbank and sets.
//
/----------------------------------------------------------------------*/


long FormulaAndClauseSetParse(Scanner_p in, FormulaSet_p fset,
                              ClauseSet_p wlset, TB_p terms,
                              StrTree_p *name_selector,
                              StrTree_p *skip_includes)
{
   long res = 0;
   WFormula_p form, nextform;
   Clause_p   clause, nextclause;
   StrTree_p  stand_in = NULL;

   if(!name_selector)
   {
      name_selector = &stand_in;
   }

   switch(ScannerGetFormat(in))
   {
   case LOPFormat:
         //* LOP does not at the moment support full FOF, or inline watchlists */
         SetProblemType(PROBLEM_FO);
         while(ClauseStartsMaybe(in))
         {
            form = WFormClauseParse(in, terms);
            // fprintf(stdout, "Parsed: ");
            // WFormulaPrint(stdout, form, true);
            // fprintf(stdout, "\n");
            FormulaSetInsert(fset, form);
            res++;
         }
         break;
   default:
#ifndef ENABLE_LFHO
         if(TestInpId(in, "thf"))
         {
            Error("To support LFHOL reasoning, recompile the E prover"
                  " using \'./configure --enable-ho && make rebuild\' \n",
                  SYNTAX_ERROR);
         }
#endif
         while(TestInpId(in, "input_formula|input_clause|fof|cnf|tff|thf|tcf|include"))
         {
            if(TestInpId(in, "include"))
            {
               if(app_encode)
               {
                  ignore_include(in);
                  continue;
               }

               StrTree_p new_limit = NULL;
               Scanner_p new_in;
               FormulaSet_p nfset = FormulaSetAlloc();
               ClauseSet_p  nwlset = ClauseSetAlloc();
               new_in = ScannerParseInclude(in, &new_limit, skip_includes);

               if(new_in)
               {
                  res += FormulaAndClauseSetParse(new_in,
                                                  nfset,
                                                  nwlset,
                                                  terms,
                                                  &new_limit,
                                                  skip_includes);
                  DestroyScanner(new_in);
               }
               StrTreeFree(new_limit);
               FormulaSetInsertSet(fset, nfset);
               ClauseSetInsertSet(wlset, nwlset);
               assert(ClauseSetEmpty(nfset));
               assert(ClauseSetEmpty(nwlset));
               FormulaSetFree(nfset);
               ClauseSetFree(nwlset);
            }
            else
            {
               if(TestInpId(in, "input_formula|fof|tff|thf|tcf"))
               {
                  form = WFormulaParse(in, terms);
                  // fprintf(stdout, "Parsed: ");
                  // WFormulaPrint(stdout, form, true);
                  // fprintf(stdout, "\n");
               }
               else
               {
                  assert(TestInpId(in, "input_clause|cnf"));
                  //clause = ClauseParse(in, terms);
                  //ClauseSetInsert(cset, clause);
                  SetProblemType(PROBLEM_FO);
                  form = WFormClauseParse(in, terms);
               }
               if(FormulaQueryType(form)==CPTypeWatchClause)
               {
                  assert(form->is_clause);
                  clause = WFormClauseToClause(form);
                  ClauseSetInsert(wlset, clause);
                  WFormulaFree(form);
               }
               else
               {
                  FormulaSetInsert(fset, form);
               }
               res++;
            }
         }
         break;
   }
   if(*name_selector)
   {
      form = fset->anchor->succ;
      while(form!= fset->anchor)
      {
         nextform = form->succ;
         if(!verify_name(name_selector, form->info))
         {
            FormulaSetDeleteEntry(form);
         }
         form = nextform;
      }
      clause = wlset->anchor->succ;
      while(clause!= wlset->anchor)
      {
         nextclause = clause->succ;
         if(!verify_name(name_selector, clause->info))
         {
            ClauseSetDeleteEntry(clause);
         }
         clause = nextclause;
      }
      check_all_found(in, *name_selector);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: TFormulaToCNF()
//
//   Convert a term-encoded formula from conjunctive normal form into
//   a set of (variable-normalized) clauses. Return number of clauses
//   generated.
//
// Global Variables: -
//
// Side Effects    : Memory operations, may allocate new variables in
//                   fresh_vars, may create new terms in the term
//                   bank.
//
/----------------------------------------------------------------------*/

long TFormulaToCNF(WFormula_p form, FormulaProperties type, ClauseSet_p set,
                  TB_p terms, VarBank_p fresh_vars)
{
   TFormula_p handle;
   long old_clause_number = set->members;
   PStack_p stack = PStackAlloc();
   Clause_p clause;

   /* Skip quantors */
   for(handle = form->tformula;
       handle->f_code == terms->sig->qall_code;
       handle = handle->args[1])
   {
      assert(handle);
   }
   PStackPushP(stack, handle);
   while(!PStackEmpty(stack))
   {
      handle = PStackPopP(stack);
      if(handle->f_code == terms->sig->and_code)
      {
         PStackPushP(stack, handle->args[0]);
         PStackPushP(stack, handle->args[1]);
      }
      else
      {
         clause = TFormulaCollectClause(handle, terms, fresh_vars);
         ClauseSetTPTPType(clause, type);
         DocClauseFromForm(GlobalOut, OutputLevel, clause, form);
         ClausePushDerivation(clause, DCSplitConjunct, form, NULL);

         if(ClauseEliminateNakedBooleanVariables(clause))
         {
            ClausePushDerivation(clause, DCEliminateBVar, NULL, NULL);
         }

         ClauseSetInsert(set, clause);
      }
   }
   PStackFree(stack);
   return set->members - old_clause_number;
}


/*-----------------------------------------------------------------------
//
// Function: TFormulaSetDelTermpProp()
//
//   Go through a set of term-encoded formulas and delete prop in all
//   term and formula cells.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void TFormulaSetDelTermpProp(FormulaSet_p set, TermProperties prop)
{
   WFormula_p handle;

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
          handle->succ)
   {
      if(handle->tformula)
      {
         TermDelProp(handle->tformula, DEREF_NEVER, prop);
      }
   }
}

/*-----------------------------------------------------------------------
//
// Function: TFormulaSetFindDefs()
//
//   Go through a set of formulas and generate and record all
//   necessary definitions. Assumes that the formulas are simplified!
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void TFormulaSetFindDefs(FormulaSet_p set, TB_p terms, NumXTree_p *defs,
                         PStack_p renamed_forms)
{
   WFormula_p handle;

   // printf("TFormulaSetFindDefs()...\n");
   for(handle = set->anchor->succ; handle!=set->anchor; handle =
          handle->succ)
   {
      assert(handle->tformula);

      if(handle->tformula && FormulaDefLimit)
      {
         TFormulaFindDefs(terms, handle->tformula, 1,
                          FormulaDefLimit, defs,  renamed_forms);
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: TFormulaApplyDefs()
//
//   Given a formula and a number of definitions represented by defs
//   and tags in bank, apply all apropriate definitions to simplify
//   the formula. Return the number of definitions used. Note that
//   defs has to contain the defined atoms in val2 and the ident of
//   the corresponding definition in val1 of its cells.
//
// Global Variables: -
//
// Side Effects    : Simplifies set, may print simplification steps.
//
/----------------------------------------------------------------------*/

long TFormulaApplyDefs(WFormula_p form, TB_p terms, NumXTree_p *defs)
{
   TFormula_p reduced;
   long       res = 0;
   PStack_p   defs_used = PStackAlloc();
   PStackPointer i;
   reduced = TFormulaCopyDef(terms, form->tformula, form->ident,
                             defs, defs_used);
   if(!PStackEmpty(defs_used))
   {
      assert(form->tformula != reduced);
      form->tformula = reduced; /* Old one will be picked up by gc */
      DocFormulaIntroDefsDefault(form, defs_used);
      res = PStackGetSP(defs_used);
      for(i=0; i<res; i++)
      {
         WFormulaPushDerivation(form,
                                DCApplyDef,
                                PStackElementP(defs_used, i),
                                NULL);
      }
   }
   else
   {
      assert(form->tformula == reduced);
   }

   PStackFree(defs_used);
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: TFormulaUnrollFOOL()
//
//   Translate FOOL features into FOL. Performs following translations:
//      - Takes formulas as arguments out of the term, leaving
//        only $true, $false and boolean vars as the argument of the term
//      - TODO: Unfolds ite expressions used as terms
//      - TODO: Unfolds ite expressions used as formulas
//
// Global Variables: -
//
// Side Effects    : Changes enclosed formulas and proof objects
//
/----------------------------------------------------------------------*/

bool TFormulaUnrollFOOL(WFormula_p form, TB_p terms)
{
   return fool_process_formula(form, terms, do_fool_unroll);
}

/*-----------------------------------------------------------------------
//
// Function: TFormulaReplaceEqnWithEquiv()
//
//   If input formula contains subformulas of type \alpha = \beta,
//   replace those subformulas with \alpha <=> \beta and alter
//   proof object accordingly.
//
// Global Variables: -
//
// Side Effects    : Changes enclosed formulas and proof objects
//
/----------------------------------------------------------------------*/

bool TFormulaReplaceEqnWithEquiv(WFormula_p form, TB_p terms)
{
   return fool_process_formula(form, terms, do_bool_eqn_replace);
}


/*-----------------------------------------------------------------------
//
// Function: TFormulaSetUnrollFOOL()
//
//   Unrolls FOOL features for the set of formulas.
//
// Global Variables: -
//
// Side Effects    : Simplifies set, may print simplification steps.
//
/----------------------------------------------------------------------*/

long TFormulaSetUnrollFOOL(FormulaSet_p set, FormulaSet_p archive, TB_p terms)
{
   long res = 0;
   for(WFormula_p formula = set->anchor->succ; formula!=set->anchor; formula=formula->succ)
   {
      TFormulaReplaceEqnWithEquiv(formula, terms);
      if(TFormulaUnrollFOOL(formula, terms))
      {
         res++;
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: TFormulaSetIntroduceDefs()
//
//   Transform a formula set by renaming certain subformulae and
//   adding the necessary definitions. Returns the number of
//   definitions.
//
//    Note that NumXTree cells are used as follows:
//    key is the term ident of the formula to be replaced
//    vals[0].i_val starts as the polarity of that formula, but turns
//                  into the id of the "virtual" definition used for
//                  output
//    vals[1].p_val is a pointer to the defined predicate term.
//    vals[2].i_val is the id of the real definition used to protect
//                  the definition to be applied to itself.
//    vals[3].p_val is a pointer to the polarity 0 definition
//
// Global Variables: -
//
// Side Effects    : Changes set.
//
/----------------------------------------------------------------------*/

long TFormulaSetIntroduceDefs(FormulaSet_p set, FormulaSet_p archive, TB_p terms)
{
   long res = 0;
   NumXTree_p defs = NULL, cell;
   PStack_p  renamed_forms = PStackAlloc();
   PStackPointer i;
   TFormula_p form, def, newdef;
   long       polarity;
   WFormula_p w_def, c_def, formula, arch_form;

   //printf("TFormulaSetIntroduceDefs()...\n");
   TFormulaSetDelTermpProp(set, TPCheckFlag|TPPosPolarity|TPNegPolarity);
   //printf("Deleted properties\n");
   FormulaSetMarkPolarity(set);
   //printf("Marked polarites\n");

   //printf("About to find defs\n");
   TFormulaSetFindDefs(set, terms, &defs, renamed_forms);
   
   res = PStackGetSP(renamed_forms);
   //printf("About to Create defs\n");

   for(i=0; i<PStackGetSP(renamed_forms); i++)
   {
      form = PStackElementP(renamed_forms,i);
      cell = NumXTreeFind(&defs, form->entry_no);
      assert(cell);
      polarity = TFormulaDecodePolarity(terms, form);
      def      = cell->vals[1].p_val;
      newdef = TFormulaCreateDef(terms, def, form,
                                 0);
      w_def = WTFormulaAlloc(terms, newdef);
      DocFormulaCreationDefault(w_def, inf_fof_intro_def, NULL, NULL);
      cell->vals[0].i_val = w_def->ident; /* Replace polarity with
                                        * definition id */
      arch_form = WFormulaFlatCopy(w_def);
      WFormulaPushDerivation(arch_form, DCIntroDef, NULL, NULL);
      FormulaSetInsert(archive, arch_form);
      WFormulaPushDerivation(w_def, DCFofQuote, arch_form, NULL);

      cell->vals[3].p_val=arch_form;
      if(polarity == 0)
      {
         cell->vals[2].i_val = w_def->ident; /* ..and this is the
                                                blocking id of the
                                                actual definition.*/
         FormulaSetInsert(set, w_def);
      }
      else
      {
         newdef = TFormulaCreateDef(terms, def, form,
                                 polarity);
         c_def = WTFormulaAlloc(terms, newdef);
         DocFormulaCreationDefault(c_def, inf_fof_split_equiv, w_def, NULL);
         cell->vals[2].i_val = c_def->ident; /* ..and this is the
                                                blocking id of the actual
                                                definition.*/
         WFormulaPushDerivation(c_def, DCSplitEquiv, arch_form, NULL);
         FormulaSetInsert(set, c_def);
         WFormulaFree(w_def);
      }
   }
   PStackFree(renamed_forms);

   // printf("About to apply defs\n");
   for(formula = set->anchor->succ; formula!=set->anchor; formula=formula->succ)
   {
      TFormulaApplyDefs(formula, terms, &defs);
   }
   NumXTreeFree(defs);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: FormulaSetArchive()
//
//   Move each formula from set to archive, replace it by a copy that
//   quoted the archived formula as the parent.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

void FormulaSetArchive(FormulaSet_p set, FormulaSet_p archive)
{
   FormulaSet_p tmpset;
   WFormula_p handle, newform;

   tmpset = FormulaSetAlloc();

   while((handle = FormulaSetExtractFirst(set)))
   {
      newform = WFormulaFlatCopy(handle);
      WFormulaPushDerivation(newform, DCFofQuote, handle, NULL);
      FormulaSetInsert(tmpset, newform);
      FormulaSetInsert(archive, handle);
   }
   assert(FormulaSetEmpty(set));

   FormulaSetInsertSet(set, tmpset);
   FormulaSetFree(tmpset);
}


/*-----------------------------------------------------------------------
//
// Function: FormulaSetDocInital()
//
//   If level >= 2, print all formula as initials.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void FormulaSetDocInital(FILE* out, long level, FormulaSet_p set)
{
   WFormula_p handle;

   if(level>=2)
   {
      for(handle = set->anchor->succ; handle!=set->anchor; handle =
             handle->succ)
      {
         DocFormulaCreationDefault(handle, inf_initial, NULL,NULL);
      }
   }
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
