/*-----------------------------------------------------------------------

  File  : ccl_tcnf.c

  Author: Stephan Schulz

  Contents

  Functions for CNF conversion of a term-encoded FOF formula.

  Copyright 2003-2024 by the author(s).
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Thu Feb 26 00:21:17 CET 2004

  -----------------------------------------------------------------------*/

#include "ccl_tcnf.h"
#include "ccl_formulafunc.h"
#include "cte_lambda.h"


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

/* long SimplificationCounter = 0; */

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/

TFormula_p do_simplify_decoded(TB_p terms, TFormula_p form, bool unroll_implications);

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: tprop_arg_return_other()
//
//   If one of the args is a propositional formula of the desired
//   type, return the other one, else return NULL.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

TFormula_p tprop_arg_return_other(Sig_p sig, TFormula_p arg1, TFormula_p arg2,
                                  bool positive)
{
   if(TFormulaIsPropConst(sig, arg1, positive))
   {
      return arg2;
   }
   else if (TFormulaIsPropConst(sig, arg2, positive))
   {
      return arg1;
   }
   return NULL;
}

/*-----------------------------------------------------------------------
//
// Function: tprop_arg_return()
//
//   If one of the args is a propositional formula of the desired
//   type, return it, else return NULL.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

TFormula_p tprop_arg_return(Sig_p sig, TFormula_p arg1, TFormula_p arg2,
                            bool positive)
{
   if(TFormulaIsPropConst(sig, arg1, positive))
   {
      return arg1;
   }
   else if (TFormulaIsPropConst(sig, arg2, positive))
   {
      return arg2;
   }
   return NULL;
}


/*-----------------------------------------------------------------------
//
// Function: negate_form()
//
//   Negate the formula, and flatten the negation (only one step).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static Term_p negate_form(TB_p bank, Term_p t)
{
   Sig_p sig = bank->sig;
   if(!TypeIsBool(t->type))
   {
      return t;
   }
   else if (TermIsDBVar(t))
   {
      return TFormulaFCodeAlloc(bank, sig->not_code, t, NULL);
   }

   if(t == bank->true_term)
   {
      return bank->false_term;
   }
   if(t == bank->false_term)
   {
      return bank->true_term;
   }
   if(t->f_code == sig->not_code)
   {
      return t->args[0];
   }
   else if(t->f_code == sig->eqn_code)
   {
      return TFormulaFCodeAlloc(bank, sig->neqn_code, t->args[0], t->args[1]);
   }
   else if(t->f_code == sig->neqn_code)
   {
      return TFormulaFCodeAlloc(bank, sig->eqn_code, t->args[0], t->args[1]);
   }
   else if(t->f_code == sig->equiv_code)
   {
      return TFormulaFCodeAlloc(bank, sig->xor_code, t->args[0], t->args[1]);
   }
   else if(t->f_code == sig->xor_code)
   {
      return TFormulaFCodeAlloc(bank, sig->equiv_code, t->args[0], t->args[1]);
   }
   else
   {
      return TFormulaFCodeAlloc(bank, sig->not_code, t, NULL);
   }
}

/*-----------------------------------------------------------------------
//
// Function: fold_and_or()
//
//   Make a formula which applies args to a binary symbol fc.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static Term_p fold_and_or(TB_p bank, PStack_p args, FunCode fc)
{
   if(PStackGetSP(args) == 1)
   {
      return PStackTopP(args);
   }
   else
   {
      Term_p lhs = PStackPopP(args);
      Term_p rhs = PStackPopP(args);
      lhs = TFormulaFCodeAlloc(bank, fc, lhs, rhs);
      while(!PStackEmpty(args))
      {
         lhs = TFormulaFCodeAlloc(bank, fc, lhs, PStackPopP(args));
      }
      return lhs;
   }
}

/*-----------------------------------------------------------------------
//
// Function: unroll_binary()
//
//   Puts all the arguments of binary fcode fc to args.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void unroll_binary(Term_p form, FunCode fc, PStack_p args)
{
   PStack_p tasks = PStackAlloc();
   PStackPushP(tasks, form);

   while(!PStackEmpty(tasks))
   {
      Term_p task = PStackPopP(tasks);
      if(!TermIsDBVar(task) && task->arity == 2 && task->f_code == fc)
      {
         PStackPushP(tasks, task->args[1]);
         PStackPushP(tasks, task->args[0]);
      }
      else
      {
         PStackPushP(args, task);
      }
   }

   PStackFree(tasks);
}

/*-----------------------------------------------------------------------
//
// Function: troot_nnf()
//
//   Apply all NNF-transformation rules that can be applied at the
//   root level form and return it.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

TFormula_p troot_nnf(TB_p terms, TFormula_p form, int polarity)
{
   TFormula_p handle = form, arg1, arg2, arg21, arg22;
   FunCode f_code;

   assert((polarity<=1) && (polarity >=-1));

   while(handle)
   {
      handle = NULL;

      if(form->f_code == terms->sig->not_code)
      {
         if(TFormulaIsLiteral(terms->sig, form->args[0]))
         {
            f_code = SigGetOtherEqnCode(terms->sig, form->args[0]->f_code);
            handle = TFormulaFCodeAlloc(terms, f_code,
                                        form->args[0]->args[0],
                                        form->args[0]->args[1]);
         }
         else if(TermIsTrueTerm(form->args[0]))
         {
            handle = TFormulaFCodeAlloc(terms, terms->sig->neqn_code,
                                        form->args[0],
                                        form->args[0]);

         }
         else if(TermIsFalseTerm(form->args[0]))
         {
            handle = TFormulaFCodeAlloc(terms, terms->sig->eqn_code,
                                        form->args[0],
                                        form->args[0]);

         }
         else if(form->args[0]->f_code == terms->sig->not_code)
         {
            handle = form->args[0]->args[0];
         }
         else if(form->args[0]->f_code == terms->sig->or_code)
         {
            arg1 = TFormulaFCodeAlloc(terms, terms->sig->not_code,

                  form->args[0]->args[0],
                                      NULL);
            arg2 = TFormulaFCodeAlloc(terms, terms->sig->not_code,
                                      form->args[0]->args[1],
                                      NULL);
            handle = TFormulaFCodeAlloc(terms, terms->sig->and_code,
                                        arg1, arg2);
         }
         else if(form->args[0]->f_code == terms->sig->and_code)
         {
            arg1 = TFormulaFCodeAlloc(terms, terms->sig->not_code,
                                      form->args[0]->args[0],
                                      NULL);
            arg2 = TFormulaFCodeAlloc(terms, terms->sig->not_code,
                                      form->args[0]->args[1],
                                      NULL);
            handle = TFormulaFCodeAlloc(terms, terms->sig->or_code,
                                        arg1, arg2);
         }
         else if(form->args[0]->f_code == terms->sig->qall_code)
          {
            arg1 = TFormulaFCodeAlloc(terms, terms->sig->not_code,
                                      form->args[0]->args[1],
                                      NULL);
            handle = TFormulaQuantorAlloc(terms, terms->sig->qex_code,
                                          form->args[0]->args[0], arg1);
         }
         else if(form->args[0]->f_code == terms->sig->qex_code)
         {
            arg1 = TFormulaFCodeAlloc(terms, terms->sig->not_code,
                                      form->args[0]->args[1],
                                      NULL);
            handle = TFormulaQuantorAlloc(terms, terms->sig->qall_code,
                                          form->args[0]->args[0], arg1);
         }
      }
      else if (form->f_code == terms->sig->impl_code)
      {
         arg1 = TFormulaFCodeAlloc(terms, terms->sig->not_code,
                                form->args[0], NULL);
         handle = TFormulaFCodeAlloc(terms, terms->sig->or_code,
                                  arg1, form->args[1]);
      }
      else if (form->f_code == terms->sig->equiv_code)
      {
         assert((polarity == 1) || (polarity == -1));
         if(polarity == 1)
         {
            arg1 = TFormulaFCodeAlloc(terms, terms->sig->impl_code,
                                      form->args[0], form->args[1]);
            arg2 = TFormulaFCodeAlloc(terms, terms->sig->impl_code,
                                      form->args[1], form->args[0]);
            handle = TFormulaFCodeAlloc(terms, terms->sig->and_code,
                                        arg1, arg2);
         }
         else
         {
            arg21 = TFormulaFCodeAlloc(terms, terms->sig->not_code,
                                      form->args[0], NULL);
            arg22 = TFormulaFCodeAlloc(terms, terms->sig->not_code,
                                      form->args[1], NULL);
            arg2 = TFormulaFCodeAlloc(terms, terms->sig->and_code,
                                      arg21, arg22);
            arg1 = TFormulaFCodeAlloc(terms, terms->sig->and_code,
                                      form->args[0],
                                      form->args[1]);
            handle = TFormulaFCodeAlloc(terms, terms->sig->or_code,
                                        arg1, arg2);
         }
      }
      if(handle)
      {
         form = handle;
      }
   }
   assert(form);
   return form;
}


/*-----------------------------------------------------------------------
//
// Function: tformula_rec_skolemize()
//
//   Recursively Skolemize form. Note that it is not quite trivial
//   that it this works, as it works on a shared structure, and the
//   same subformula may occur in different contexts. It _does_ work
//   (I hope) because we require that every quantor binds a distinct
//   variable, and hence terms that were originally equal are either
//   invariant with respect to context (i.e. they are ground) or
//   contain different variables, and hence are not shared.
//
// Global Variables: -
//
// Side Effects    : Changes form (and term bank), adds Skolem terms
//                   to term bank, modifies signature.
//
/----------------------------------------------------------------------*/

TFormula_p tformula_rek_skolemize(TB_p terms, TFormula_p form,
                                  PStack_p free_vars)
{
   Term_p sk_term, var;
   TFormula_p handle, handle2=NULL;
   bool modified = false;

   if(TermIsGround(form))
   {
      /* All is well */
   }
   else if(TFormulaIsLiteral(terms->sig, form) ||
           TypeIsArrow(form->type))
   {
      form = TFormulaCopy(terms, form);
   }
   else if(!SigIsLogicalSymbol(terms->sig, form->f_code) &&
           form->type == terms->sig->type_bank->bool_type)
   {
      handle = TFormulaCopy(terms, form);
   }
   else if(form->f_code == terms->sig->qex_code)
   {
      var = form->args[0];
      assert(TermIsFreeVar(var));
      assert(!var->binding);
      sk_term = TBAllocNewSkolem(terms, free_vars, var->type);
      var->binding = sk_term;
      form = tformula_rek_skolemize(terms, form->args[1],
                                    free_vars);
      var->binding = NULL;
   }
   else if(form->f_code == terms->sig->qall_code)
   {
      var = form->args[0];
      assert(TermIsFreeVar(var));
      assert(!var->binding);
      PStackPushP(free_vars, var);
      handle = tformula_rek_skolemize(terms, form->args[1],
                                      free_vars);
      form = TFormulaFCodeAlloc(terms, terms->sig->qall_code,
                                var, handle);
      (void)PStackPopP(free_vars);
   }
   else
   {
      assert(TFormulaHasSubForm1(terms->sig, form));
      handle  = tformula_rek_skolemize(terms, form->args[0],
                                       free_vars);
      modified = handle!=form->args[0];
      if(TFormulaHasSubForm2(terms->sig, form))
      {
         handle2 = tformula_rek_skolemize(terms, form->args[1],
                                          free_vars);
         modified |= handle2!=form->args[1];
      }
      if(modified)
      {
         form = TFormulaFCodeAlloc(terms, form->f_code,
                                   handle, handle2);
      }
   }
   return form;
}


/*-----------------------------------------------------------------------
//
// Function: tformula_rename_test()
//
//   Return true if the formula at argument position i should be
//   renamed, false otherwise. Polarity is the polarity of root, not
//   root|i. def_limit determines how often a subformula can be
//   replicated before it is renamed.
//
// Global Variables:
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool tformula_rename_test(TB_p bank, TFormula_p root, int pos,
                          int polarity, long def_limit)
{
   int subform_sign;

   assert((polarity<=1) && (polarity >=-1));

   if((root->f_code == bank->sig->qex_code) ||
      (root->f_code == bank->sig->qall_code))
   {
      return false;
   }
   if(root->f_code == bank->sig->equiv_code)
   {
      if(TFormulaEstimateClauses(bank, root->args[pos],true) > def_limit)
      {
         return true;
      }
      if(TFormulaEstimateClauses(bank, root->args[pos],false) > def_limit)
      {
         return true;
      }
   }
   else
   {
      switch(polarity)
      {
      case 1:
            if(root->f_code == bank->sig->or_code &&
               TFormulaEstimateClauses(bank, root->args[pos],true)
               > def_limit)
            {
               return true;
            }
            subform_sign = (pos==2?true:false);
            if(root->f_code == bank->sig->impl_code &&
               TFormulaEstimateClauses(bank, root->args[pos],subform_sign)
               > def_limit)
            {
               return true;
            }
            break;
      case -1:
            if(root->f_code == bank->sig->and_code &&
               TFormulaEstimateClauses(bank, root->args[pos],false)
               > def_limit)
            {
               return true;
            }
            break;
      case 0:
            if((root->f_code == bank->sig->and_code ||
                root->f_code == bank->sig->or_code ||
                root->f_code == bank->sig->impl_code) &&
               (TFormulaEstimateClauses(bank, root->args[pos],true) >
                def_limit ||
                TFormulaEstimateClauses(bank, root->args[pos],false) >
                def_limit))
            {
               return true;
            }
            break;
      default:
            assert(false && "Impossible polarity.");
            break;
      }
   }
   return false;
}



/*-----------------------------------------------------------------------
//
// Function: extract_formula_core()
//
//   Remove all (universal) quantifiers from Skolemized form in NNF
//   and push the corresponding variables onto varstack.
//
// Global Variables: -
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

TFormula_p extract_formula_core(TB_p terms, TFormula_p form, PStack_p varstack)
{
   TFormula_p narg0, narg1;
   PStackPointer sp;

   while(TFormulaIsQuantified(terms->sig, form))
   {
      /* Skip over variables */
      assert(form->f_code == terms->sig->qall_code);
      PStackPushP(varstack, form->args[0]);
      form = form->args[1];
   }
   if((form->f_code == terms->sig->and_code)||
      (form->f_code == terms->sig->or_code))
   {
      sp = PStackGetSP(varstack);
      narg0 = extract_formula_core(terms, form->args[0], varstack);
      narg1 = extract_formula_core(terms, form->args[1], varstack);
      if(PStackGetSP(varstack)!=sp)
      {
         form = TFormulaFCodeAlloc(terms, form->f_code, narg0, narg1);
      }
      else
      {
         /* Should be no change... */
         assert(narg0 == form->args[0]);
         assert(narg1 == form->args[1]);
      }
   }
   /* else form already is elementary and quantor-free */

   return form;
}



/*-----------------------------------------------------------------------
//
// Function: extract_formula_core2()
//
//   Remove all quantifiers from form in NNF and push the
//   corresponding quantifier/variable pairs onto varstack.
//
// Global Variables: -
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

TFormula_p extract_formula_core2(TB_p terms, TFormula_p form, PStack_p varstack)
{
   TFormula_p narg0, narg1;
   PStackPointer sp;

   while(TFormulaIsQuantified(terms->sig, form))
   {
      /* Skip over variables */
      PStackPushInt(varstack, form->f_code);
      PStackPushP(varstack, form->args[0]);
      form = form->args[1];
   }
   if((form->f_code == terms->sig->and_code)||
      (form->f_code == terms->sig->or_code))
   {
      sp = PStackGetSP(varstack);
      narg0 = extract_formula_core2(terms, form->args[0], varstack);
      narg1 = extract_formula_core2(terms, form->args[1], varstack);
      if(PStackGetSP(varstack)!=sp)
      {
         form = TFormulaFCodeAlloc(terms, form->f_code, narg0, narg1);
      }
      else
      {
         /* Should be no change... */
         assert(narg0 == form->args[0]);
         assert(narg1 == form->args[1]);
      }
   }
   /* else form already is elementary and quantor-free */

   return form;
}

/*-----------------------------------------------------------------------
//
// Function: tform_mark_varocc()
//
//   Mark all subforms/subterms in form in which var occurs.
//
// Global Variables: -
//
// Side Effects    : Sets TPOpFlag, TPCheckFlag
//
/----------------------------------------------------------------------*/

/* __attribute__((noinline)) */
static bool tform_mark_varocc(TFormula_p form, Term_p var, TermProperties proc)
{
   bool res = false;

   if(TermCellGiveProps(form, TPOpFlag)!=proc)
   {
      TermProperties found = TPIgnoreProps;
      if(form == var)
      {
         found = TPCheckFlag;
      }
      else
      {
         int i;

         for(i=0; i<form->arity; i++)
         {
            if(tform_mark_varocc(form->args[i], var, proc))
            {
               found = TPCheckFlag;
            }
         }
      }
      TermCellAssignProp(form, TPOpFlag|TPCheckFlag, proc|found);
   }
   res = TermCellQueryProp(form, TPCheckFlag);

   //printf(COMCHAR" FCode: %ld VCode: %ld, Mark: %d  Real: %d\n",
   //       form->f_code, var->f_code,
   //       res, TBTermIsSubterm(form, var));
   assert(res == TBTermIsSubterm(form,var));
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: miniscope_qex()
//
//   Assume var is existentially quantified in var and move the
//   quantifier inward as far as possible. Assumes that form is in NNF
//   and that TPCheckFlag is set in all subformulas of form in which
//   var occurs.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static TFormula_p miniscope_qex(TB_p terms, TFormula_p form, Term_p var,
                                TermProperties proc)
{
   TFormula_p arg1, arg2;

   if(TermCellQueryProp(form, TPCheckFlag))
   {
      if(form->f_code == terms->sig->and_code)
      {
         if(!TermCellQueryProp(form->args[0], TPCheckFlag))
         {
            arg1 = form->args[0];
            arg2 = miniscope_qex(terms, form->args[1], var, proc);
            form = TFormulaFCodeAlloc(terms, form->f_code, arg1, arg2);

         }
         else if(!TermCellQueryProp(form->args[1], TPCheckFlag))
         {
            arg1 = miniscope_qex(terms, form->args[0], var, proc);
            arg2 = form->args[1];
            form = TFormulaFCodeAlloc(terms, form->f_code, arg1, arg2);
         }
         else
         {
            form = TFormulaQuantorAlloc(terms, terms->sig->qex_code, var, form);
         }
      }
      else if(form->f_code == terms->sig->or_code)
      {
         arg1 = miniscope_qex(terms, form->args[0], var, proc);
         arg2 = miniscope_qex(terms, form->args[1], var, proc);
         form = TFormulaFCodeAlloc(terms, form->f_code, arg1, arg2);
      }
      else
      {
         form = TFormulaQuantorAlloc(terms, terms->sig->qex_code, var, form);
      }
      TermCellAssignProp(form, TPOpFlag, proc);
   }
   /* Else we don't need a quantifier */

   return form;
}



/*-----------------------------------------------------------------------
//
// Function: miniscope_qall()
//
//   Assume var is universally quantified in var and move the
//   quantifier inward as far as possible. Assumes that form is in NNF
//   and that TPCheckFlag is set in all subformulas of form in which
//   var occurs.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static TFormula_p miniscope_qall(TB_p terms, TFormula_p form, Term_p var,
                                TermProperties proc)
{
   TFormula_p arg1, arg2;

   if(TermCellQueryProp(form, TPCheckFlag))
   {
      if(form->f_code == terms->sig->or_code)
      {
         if(!TermCellQueryProp(form->args[0], TPCheckFlag))
         {
            arg1 = form->args[0];
            arg2 = miniscope_qall(terms, form->args[1], var, proc);
            form = TFormulaFCodeAlloc(terms, form->f_code, arg1, arg2);
         }
         else if(!TermCellQueryProp(form->args[1], TPCheckFlag))
         {
            arg1 = miniscope_qall(terms, form->args[0], var, proc);
            arg2 = form->args[1];
            form = TFormulaFCodeAlloc(terms, form->f_code, arg1, arg2);
         }
         else
         {
            form = TFormulaQuantorAlloc(terms, terms->sig->qall_code, var, form);
         }
      }
      else if(form->f_code == terms->sig->and_code)
      {
         arg1 = miniscope_qall(terms, form->args[0], var, proc);
         arg2 = miniscope_qall(terms, form->args[1], var, proc);
         form = TFormulaFCodeAlloc(terms, form->f_code, arg1, arg2);
      }
      else
      {
         form = TFormulaQuantorAlloc(terms, terms->sig->qall_code, var, form);
      }
      TermCellAssignProp(form, TPOpFlag, proc);
   }
   /* Else we don't need a quantifier */

   return form;
}


/*-----------------------------------------------------------------------
//
// Function: tform_find_miniscopeable()
//
//   Find all maximal miniscopable subformulas. A formula is
//   miniscopable, if it has at most limit subformulae, starts with a
//   universal quantifier, and contains an existential quantifier (we
//   only miniscope with the goal of moving existential quantifier out
//   of the scope of universal quantifiers - and that not at all
//   costs ;-). Return value is the size, candidates are stored in the
//   ptree at candidates, and the existance of an existential
//   quantifier is returned via *exq. Sometime I'm longing for
//   tuples...
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

long tform_find_miniscopeable(Sig_p sig, TFormula_p form, long limit,
                              PTree_p *candidates, bool *exq)
{
   bool lexq = false;
   long size;
   PTree_p lcands = NULL;

   assert(!TermIsFreeVar(form));

   if(!form->v_count)
   {
      return LONG_MAX;
   }
   if(TFormulaIsLiteral(sig, form) || TypeIsArrow(form->type))
   {
      return 1;
   }
   if(TFormulaIsQuantified(sig, form))
   {
      size = 1+tform_find_miniscopeable(sig, form->args[1], limit, &lcands,  &lexq);
      if(form->f_code == sig->qex_code)
      {
         *exq = true;
         PTreeMerge(candidates, lcands);
      }
      else /* Universal quantifier */
      {
         if(size<=limit && lexq)
         {
            PTreeFree(lcands);
            PTreeStore(candidates, form);
         }
         else
         {
            PTreeMerge(candidates, lcands);
         }
         *exq = lexq;
      }
   }
   else
   {
      size = 1;
      if(TFormulaHasSubForm1(sig, form))
      {
         size += size+tform_find_miniscopeable(sig, form->args[0],
                                               limit, &lcands,  &lexq);
         PTreeMerge(candidates, lcands);
         lcands = NULL;
         *exq |= lexq;
      }
      if(TFormulaHasSubForm2(sig, form))
      {
         size += size+tform_find_miniscopeable(sig, form->args[1],
                                               limit, &lcands,  &lexq);
         PTreeMerge(candidates, lcands);
         *exq |= lexq;
      }
   }
   return size;
}


/*-----------------------------------------------------------------------
//
// Function: tform_copy_mod()
//
//   Copy a formula, following "binding" when it is set. Ground
//   formumas, variables, and literals are returned as-is.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

TFormula_p tform_copy_mod(TB_p terms, TFormula_p form)
{
   TFormula_p arg1=NULL, arg2=NULL;
   bool changed = false;

   if(TFormulaIsLiteral(terms->sig, form)
      ||TypeIsArrow(form->type)
      ||!form->v_count
      ||TermIsFreeVar(form))
   {
      return form;
   }
   if(form->binding)
   {
      return form->binding;
   }
   if(TFormulaHasSubForm1(terms->sig, form))
   {
      arg1 = tform_copy_mod(terms, form->args[0]);
      changed |= arg1!=form->args[0];
   }
   if(TFormulaHasSubForm2(terms->sig, form))
   {
      arg2 = tform_copy_mod(terms, form->args[1]);
      changed |= arg2!=form->args[1];
   }
   if(changed)
   {
      form = TFormulaFCodeAlloc(terms, form->f_code, arg1, arg2);
   }
   return form;
}



/*-----------------------------------------------------------------------
//
// Function: do_simplify_decoded()
//
//   Function that actually performs the simplification on decoded formulas.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

// lifted function that cannot be declared locally (anonymously)
static Term_p simplify_args(TB_p bank, Term_p t, bool unroll_implications)
{
   if(TermIsAnyVar(t) || t->arity == 0)
   {
      return t;
   }
   else
   {
      Term_p res = TermTopCopyWithoutArgs(t);
      bool changed = false;
      for(int i=0; i<t->arity; i++)
      {
         res->args[i] = do_simplify_decoded(bank, t->args[i], unroll_implications);
         changed = changed || res->args[i] != t->args[i];
      }

      if(changed)
      {
         res = TBTermTopInsert(bank, res);
      }
      else
      {
         TermTopFree(res);
         res = t;
      }

      return res;
   }
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

// lifted function that cannot be declared locally (anonymously)
static int term_compare(const void* v1, const void* v2)
{
   const IntOrP* e1 = (const IntOrP*) v1;
   const IntOrP* e2 = (const IntOrP*) v2;
   int res = PCmp(e1->p_val, e2->p_val);

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: do_simplify_decoded()
//
//
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

TFormula_p do_simplify_decoded(TB_p terms, TFormula_p form, bool unroll_implications)
{
   // making sure that no comparisons below
   // will be invalid because something is DB var
   if(TermIsDBVar(form))
   {
      return form;
   }

   assert(terms);
   Sig_p sig = terms->sig;

   Term_p res = form;
   if((form->f_code == sig->or_code ||
       form->f_code == sig->and_code))
   {
      Term_p neutral_element =
         form->f_code == sig->or_code ? terms->false_term : terms->true_term;
      Term_p asbsorbing_element =
         form->f_code == sig->or_code ? terms->true_term : terms->false_term;
      if(form->arity == 1)
      {
         form = simplify_args(terms, form, true);
         Type_p bool_ty = terms->sig->type_bank->bool_type;
         if (form->args[0] == neutral_element)
         {
            res = CloseWithDBVar(terms, bool_ty,
                                 TBRequestDBVar(terms, bool_ty, 0));
         }
         else if (form->args[0] == asbsorbing_element)
         {
            res = CloseWithDBVar(terms, bool_ty, asbsorbing_element);
         }
      }
      else if(form->arity == 2)
      {
         res = NULL;
         bool changed = false;
         PStack_p args = PStackAlloc();
         unroll_binary(form, form->f_code, args);

         PStack_p res_args = PStackAlloc();
         for(PStackPointer i=0; !res && i<PStackGetSP(args); i++)
         {
            Term_p arg = PStackElementP(args, i);
            Term_p simplified = do_simplify_decoded(terms, arg, unroll_implications);
            if(arg!=simplified)
            {
               changed=true;
               arg = simplified;
            }

            if(arg != neutral_element)
            {
               if(arg != asbsorbing_element)
               {
                  PStackPushP(res_args, arg);
               }
               else
               {
                  res = asbsorbing_element;
               }
            }
            else
            {
               changed = true;
            }
         }
         if(!res)
         {
            PStackSort(res_args, term_compare);
            if(!PStackEmpty(res_args))
            {
               // removing duplicates;
               PStackReset(args);
               PStackPushP(args, PStackElementP(res_args, 0));
               for(PStackPointer i=1; i < PStackGetSP(res_args); i++)
               {
                  if(PStackElementP(res_args, i) != PStackElementP(res_args, i-1))
                  {
                     PStackPushP(args, PStackElementP(res_args, i));
                  }
                  else
                  {
                     changed = true;
                  }
               }
               SWAP(args, res_args);
            }


            for(PStackPointer i=0; !res && i<PStackGetSP(res_args); i++)
            {
               Term_p neg_arg = negate_form(terms, PStackElementP(res_args, i));
               IntOrP neg_arg_key = {.p_val = neg_arg};
               Term_p found =
                  bsearch(&neg_arg_key, res_args->stack, res_args->current,
                          sizeof(IntOrP), term_compare);
               if(found)
               {
                  res = asbsorbing_element;
               }
            }

            if(!res)
            {
               if(!changed)
               {
                  res = form;
               }
               else
               {
                  if(PStackEmpty(res_args))
                  {
                     res = neutral_element;
                  }
                  else
                  {
                     res = fold_and_or(terms, res_args, form->f_code);
                  }
               }
            }
         }
         PStackFree(res_args);
         PStackFree(args);
      }
   }
   else if(form->f_code == sig->not_code)
   {
      if(form->arity == 1)
      {
         res = negate_form(terms, form->args[0]);
      }
   }
   else if(form->f_code == sig->impl_code)
   {
      form = simplify_args(terms, form,
                           // unrolling implications only at the top level!
                           unroll_implications &&
                           (form->arity != 2 ||
                            form->args[1]->f_code != sig->impl_code));
      if(form->arity == 2)
      {
         res = NULL;
         if(unroll_implications)
         {
            PStack_p precedent = PStackAlloc();
            PStack_p consequent = PStackAlloc();

            Term_p tmp = form;
            while(tmp->f_code == sig->impl_code)
            {
               unroll_binary(tmp->args[0], sig->and_code, precedent);
               tmp = tmp->args[1];
            }

            unroll_binary(tmp, sig->or_code, consequent);

            PStackSort(precedent, term_compare);

            for(long i=0; !res && i < PStackGetSP(consequent); i++)
            {
               IntOrP arg_key = {.p_val = PStackElementP(consequent, i)};
               Term_p found =
                  bsearch(&arg_key, precedent->stack,
                          precedent->current, sizeof(IntOrP), term_compare);
               if(found)
               {
                  res = terms->true_term;
               }
            }

            PStackFree(precedent);
            PStackFree(consequent);
         }

         if(!res)
         {
            Term_p p = form->args[0], c = form->args[1];

            if(p == c || p == terms->false_term || c == terms->true_term)
            {
               res = terms->true_term;
            }
            else if(c == negate_form(terms, p) || p == negate_form(terms, c)
                    || p == terms->true_term)
            {
               res = c;
            }
            else if(c == terms->false_term)
            {
               res = negate_form(terms, p);
            }
            else
            {
               res = form;
            }
         }
      }
   }
   else if (form->f_code == sig->equiv_code ||
            form->f_code == sig->xor_code ||
            form->f_code == sig->eqn_code ||
            form->f_code == sig->neqn_code)
   {
      form = simplify_args(terms, form, true);
      if(form->arity == 2)
      {
         bool negative = form->f_code == sig->xor_code
                         || form->f_code == sig->neqn_code;

         if(form->args[0] == form->args[1])
         {
            res = negative ? terms->false_term : terms->true_term;
         }
         else if(form->args[0] == terms->true_term)
         {
            res = negative ? negate_form(terms, form->args[1]) : form->args[1];
         }
         else if(form->args[1] == terms->true_term)
         {
            res = negative ? negate_form(terms, form->args[0]) : form->args[0];
         }
         else if(form->args[0] == terms->false_term)
         {
            res = negative ? form->args[1] : negate_form(terms, form->args[1]);
         }
         else if(form->args[1] == terms->false_term)
         {
            res = negative ? form->args[0] : negate_form(terms, form->args[0]);
         }
      }
   }
   else if (form->f_code == sig->qex_code ||
            form->f_code == sig->qall_code)
   {
      form = simplify_args(terms, form, true);
      if(form->arity == 1 && TermIsLambda(form->args[0]))
      {
         Term_p matrix = form->args[0]->args[1];
         assert(TypeIsBool(matrix->type));
         if(TermIsDBClosed(matrix))
         {
            res = matrix;
         }
      }
   }
   else
   {
      res = simplify_args(terms, form, true);
   }
   return res;
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: TFormulaEstimateClauses()
//
//   Given a formula, estimate how many clauses would be generated by
//   it. Assumes that formulas with TPCheckFlag are renamed into
//   atoms. If too many clauses result, just return
//   TFORM_MANY_CLAUSES.
//
// Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

#define RETURN_IF_LARGE(param) \
        if((param) == TFORM_MANY_CLAUSES){return TFORM_MANY_CLAUSES;}

long TFormulaEstimateClauses(TB_p bank, TFormula_p form, bool pos)
{
   long posres1, posres2, negres1, negres2, res=0;

   assert(bank);
   assert(form);
   assert((pos == true) || (pos == false));

   /* printf("Estimating: ");TBPrintTermFull(stdout, bank, form);*/

   if(TermCellQueryProp(form, TPCheckFlag) ||
      TFormulaIsLiteral(bank->sig, form) ||
      // partially applied formula
      TypeIsArrow(form->type))
   {
      return 1;
   }
   if(TermIsTrueTerm(form))
   {
      return 0;
   }
   if(TermIsFalseTerm(form))
   {
      return 1;
   }
   if(TermIsAppliedFreeVar(form))
   {
      return 1;
   }
   if(pos)
   {
      if(form->f_code == bank->sig->and_code)
      {
         posres1 = TFormulaEstimateClauses(bank, form->args[0], true);
         RETURN_IF_LARGE(posres1);
         posres2 = TFormulaEstimateClauses(bank, form->args[1], true);
         RETURN_IF_LARGE(posres2);
         res = posres1 + posres2;
      }
      else if(form->f_code == bank->sig->or_code)
      {
         posres1 = TFormulaEstimateClauses(bank, form->args[0], true);
         RETURN_IF_LARGE(posres1);
         posres2 = TFormulaEstimateClauses(bank, form->args[1], true);
          RETURN_IF_LARGE(posres2);
         res = posres1 * posres2;
      }
      else if(form->f_code == bank->sig->impl_code)
      {
         negres1 = TFormulaEstimateClauses(bank, form->args[0], false);
         RETURN_IF_LARGE(negres1);
         posres2 = TFormulaEstimateClauses(bank, form->args[1], true);
         RETURN_IF_LARGE(posres2);
         res = negres1 * posres2;
      }
      else if(form->f_code == bank->sig->equiv_code)
      {
         posres1 = TFormulaEstimateClauses(bank, form->args[0], true);
         RETURN_IF_LARGE(posres1);
         posres2 = TFormulaEstimateClauses(bank, form->args[1], true);
         RETURN_IF_LARGE(posres2);
         negres1 = TFormulaEstimateClauses(bank, form->args[0], false);
         RETURN_IF_LARGE(negres1);
         negres2 = TFormulaEstimateClauses(bank, form->args[1], false);
         RETURN_IF_LARGE(negres2);
         res = posres1*negres2+negres1*posres2;
      }
      else if(form->f_code == bank->sig->not_code)
      {
         negres1 = TFormulaEstimateClauses(bank, form->args[0], false);
         RETURN_IF_LARGE(negres1);
         res = negres1;
      }
      else if(TFormulaIsQuantified(bank->sig,form))
      {
         posres1 = TFormulaEstimateClauses(bank, form->args[1], true);
         RETURN_IF_LARGE(posres1);
         res = posres1;
      }
      else
      {
         return 1;
#ifndef NDEBUG
         fprintf(stdout, COMCHAR" Error in ");
         TermPrintDbg(stdout, form, bank->sig, DEREF_NEVER);
         fprintf(stdout, "\n");
         TermPrettyPrintSimple(stdout, form, bank->sig, 0);
         fprintf(stdout, "\n");
         assert(false && "Formula not in correct simplified form");
#endif
      }
   }
   else
   {
      if(form->f_code == bank->sig->and_code)
      {
         negres1 = TFormulaEstimateClauses(bank, form->args[0], false);
         RETURN_IF_LARGE(negres1);
         negres2 = TFormulaEstimateClauses(bank, form->args[1], false);
         RETURN_IF_LARGE(negres2);
         res = negres1 * negres2;
      }
      else if(form->f_code == bank->sig->or_code)
      {
         negres1 = TFormulaEstimateClauses(bank, form->args[0], false);
         RETURN_IF_LARGE(negres1);
         negres2 = TFormulaEstimateClauses(bank, form->args[1], false);
         RETURN_IF_LARGE(negres2);
         res = negres1 + negres2;
      }
      else if(form->f_code == bank->sig->impl_code)
      {
         posres1 = TFormulaEstimateClauses(bank, form->args[0], true);
         RETURN_IF_LARGE(posres1);
         negres2 = TFormulaEstimateClauses(bank, form->args[1], false);
         RETURN_IF_LARGE(negres2);
         res = posres1 + negres2;
      }
      else if(form->f_code == bank->sig->equiv_code)
      {
         posres1 = TFormulaEstimateClauses(bank, form->args[0], true);
         RETURN_IF_LARGE(posres1);
         posres2 = TFormulaEstimateClauses(bank, form->args[1], true);
         RETURN_IF_LARGE(posres2);
         negres1 = TFormulaEstimateClauses(bank, form->args[0], false);
         RETURN_IF_LARGE(negres1);
         negres2 = TFormulaEstimateClauses(bank, form->args[1], false);
         RETURN_IF_LARGE(negres2);
         res = posres1*posres2+negres1*negres2;
      }
      else if(form->f_code == bank->sig->not_code)
      {
         posres1 = TFormulaEstimateClauses(bank, form->args[0], true);
         RETURN_IF_LARGE(posres1);
         res = posres1;
      }
      else if(TFormulaIsQuantified(bank->sig,form))
      {
         negres1 = TFormulaEstimateClauses(bank, form->args[1], false);
         RETURN_IF_LARGE(negres1);
         res = negres1;
      }
      else
      {
         return 1;
#ifndef NDEBUG
         fprintf(stdout, COMCHAR" Error in ");
         TermPrintDbg(stdout, form, bank->sig, DEREF_NEVER);
         fprintf(stdout, "\n");
         TermPrettyPrintSimple(stdout, form, bank->sig, 0);
         fprintf(stdout, "\n");
         assert(false && "Formula not in correct simplified form");
#endif
      }
   }
   if(res > TFORM_MANY_LIMIT)
   {
      res = TFORM_MANY_CLAUSES;
   }

   return res;
}



/*-----------------------------------------------------------------------
//
// Function: TFormulaDefRename()
//
//   Given a tformula, return a renaming atom for it and register
//   the (potential) need for a renaming formula of the proper
//   polarity in defs. In defs, the key is the entry_no, val1 is the
//   most general polarity, and val2 is the renaming atom.
//
// Global Variables: -
//
// Side Effects    : Changes bank and defs.
//
/----------------------------------------------------------------------*/

TFormula_p TFormulaDefRename(TB_p bank, TFormula_p form, int polarity,
                             NumXTree_p *defs, PStack_p renamed_forms)
{
   NumXTree_p def = NumXTreeFind(defs, form->entry_no);

   assert((polarity<=1) && (polarity >=-1));

   if(def)
   {
      if(polarity!=def->vals[0].i_val)
      {
         def->vals[0].i_val = 0;
      }
      return def->vals[1].p_val;
   }
   else
   {
      PTree_p free_vars = NULL;
      PStack_p var_stack = PStackAlloc();
      TFormula_p rename_atom;

      VarBankVarsSetProp(bank->vars, TPIsFreeVar);
      TFormulaCollectFreeVars(bank, form, &free_vars);
      PTreeToPStack(var_stack, free_vars);
      /* printf(COMCHAR" Found %d free variables\n", PStackGetSP(var_stack)); */

      rename_atom = TBAllocNewSkolem(bank, var_stack, bank->sig->type_bank->bool_type);
      rename_atom = EqnTermsTBTermEncode(bank, rename_atom,
                                         bank->true_term, true, PENormal);
      PStackFree(var_stack);
      PTreeFree(free_vars);

      def = NumXTreeCellAlloc();
      def->key = form->entry_no;
      def->vals[0].i_val = polarity;
      def->vals[1].p_val = rename_atom;
      NumXTreeInsert(defs, def);
      TermCellSetProp(form, TPCheckFlag);
      PStackPushP(renamed_forms, form);

      return def->vals[1].p_val;
   }
}



/*-----------------------------------------------------------------------
//
// Function: TFormulaFindDefs()
//
//   Find all useful definitions in form and enter them in defs and
//   renamed_forms. def_limit determines when a formula is
//   replicated sufficiently often to warrant renaming.
//
//    Remember: TPCheckFlag means we already have a definition for
//    this formula (though possibly not of the right polarity).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void TFormulaFindDefs(TB_p bank, TFormula_p form, int polarity,
                      long def_limit, NumXTree_p *defs,
                      PStack_p renamed_forms)
{
   assert((polarity<=1) && (polarity >=-1));
   //printf("TFormulaFindDefs(%ld)...\n",TermDepth(form));

   if(TFormulaIsLiteral(bank->sig, form) || TypeIsArrow(form->type))
   {
      return;
   }
   if(TermCellQueryProp(form, TPCheckFlag))
   {
      /* We already have a definition, but it may be of the wrong
       * polarity to be applied here (and it will be, for
       * consistency). Easiest way to handle this is to just to re-add
       * it with the current polarity - if necessary, this will extend
       * the definition to deal with a more general polarity.*/

      TFormulaDefRename(bank, form, polarity,
                        defs, renamed_forms);
      /* And this potentially different polarity applies to
      // subformulae, hence "return" is WRONG!!!
      // return;*/
   }
   /* Check if we want to rename args[0] (we are doing depth
    * first). Also remember that we check if a _subformula_ should be
    * renamed! */
   if((form->f_code == bank->sig->and_code)||
      (form->f_code == bank->sig->or_code))
   {
      TFormulaFindDefs(bank, form->args[0], polarity, def_limit,
                       defs, renamed_forms);
      if(tformula_rename_test(bank, form, 0, polarity, def_limit))
      {
         TFormulaDefRename(bank, form->args[0], polarity,
                           defs, renamed_forms);
      }
   }
   else if((form->f_code == bank->sig->not_code)||
           (form->f_code == bank->sig->impl_code))
   {
      TFormulaFindDefs(bank, form->args[0], -polarity,
                       def_limit,defs, renamed_forms);
      if(tformula_rename_test(bank, form, 0, -polarity, def_limit))
      {
         TFormulaDefRename(bank, form->args[0], -polarity,
                           defs, renamed_forms);
      }
   }
   else if(form->f_code == bank->sig->equiv_code)
   {
      TFormulaFindDefs(bank, form->args[0], 0, def_limit,
                       defs, renamed_forms);
      if(tformula_rename_test(bank, form, 0, polarity, def_limit))
      {
         TFormulaDefRename(bank, form->args[0], 0,
                           defs, renamed_forms);
      }
   }
   /* Handle args[1] */
   if((form->f_code == bank->sig->and_code)||
      (form->f_code == bank->sig->or_code) ||
      (form->f_code == bank->sig->impl_code)||
      (form->f_code == bank->sig->qex_code)||
      (form->f_code == bank->sig->qall_code))
   {
      TFormulaFindDefs(bank, form->args[1], polarity, def_limit,
                       defs, renamed_forms);
      if(tformula_rename_test(bank, form, 1, polarity, def_limit))
      {
         TFormulaDefRename(bank, form->args[1], polarity, defs, renamed_forms);
      }
   }
   else if(form->f_code == bank->sig->equiv_code)
   {
      TFormulaFindDefs(bank, form->args[1], 0, def_limit,
                       defs, renamed_forms);
      if(tformula_rename_test(bank, form, 1, polarity, def_limit))
      {
         TFormulaDefRename(bank, form->args[1], 0, defs, renamed_forms);
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: TFormulaCopyDef()
//
//   Copy a formula, replacing all defined subformulas (except for the
//   blocked one, if any) with the proper definition). Record _all_
//   definitions (but not sub-definitions) on the stack (by pushing the
//   definition numbers onto the stack).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

TFormula_p TFormulaCopyDef(TB_p bank, TFormula_p form, long blocked,
                           NumXTree_p *defs, PStack_p defs_used)
{
   TFormula_p res = NULL, arg1, arg2 = NULL;
   NumXTree_p def_entry;
   long      realdef;

   if(TFormulaIsLiteral(bank->sig, form) ||
      TermIsAppliedFreeVar(form) ||
      TypeIsArrow(form->type) ||
      TermIsTrueTerm(form) ||
      TermIsFalseTerm(form) ||
      !SigIsLogicalSymbol(bank->sig, form->f_code))
   {
      res = form;
   }
   else if(TermCellQueryProp(form, TPCheckFlag))
   {
      def_entry = NumXTreeFind(defs, form->entry_no);
      assert(def_entry);
      realdef = def_entry->vals[2].i_val;
      if(realdef!=blocked)
      {
         res = def_entry->vals[1].p_val;
         PStackPushP(defs_used, def_entry->vals[3].p_val);
      }
   }
   if(!res)
   {
      if((form->f_code == bank->sig->and_code)||
         (form->f_code == bank->sig->or_code)||
         (form->f_code == bank->sig->impl_code)||
         (form->f_code == bank->sig->equiv_code)||
         (form->f_code == bank->sig->nand_code)||
         (form->f_code == bank->sig->nor_code)||
         (form->f_code == bank->sig->bimpl_code)||
         (form->f_code == bank->sig->xor_code)||
         (form->f_code == bank->sig->not_code))
      {
         arg1 = TFormulaCopyDef(bank, form->args[0], blocked, defs, defs_used);
      }
      else
      {
         assert((form->f_code == bank->sig->qex_code) ||
                (form->f_code == bank->sig->qall_code));
         arg1 = form->args[0];
      }
      if(form->f_code != bank->sig->not_code)
      {
         arg2 = TFormulaCopyDef(bank, form->args[1], blocked, defs, defs_used);
      }
      res = TFormulaFCodeAlloc(bank, form->f_code, arg1, arg2);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: TFormulaNegAlloc()
//
//   Return a formula equivalent to ~form. If form is of the form ~f,
//   return f, otherwise ~form.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

TFormula_p TFormulaNegAlloc(TB_p terms, TFormula_p form)
{
   if(form->f_code == terms->sig->not_code)
   {
      return form->args[0];
   }
   return TFormulaFCodeAlloc(terms, terms->sig->not_code,
                             form, NULL);
}


/*-----------------------------------------------------------------------
//
// Function: TFormulaExpandLiterals()
//
//   - Make all negation signs explicit  - $neqn(a,b)=> ~$eqn(a,b)
//   - Expand literals with Boolean operands - $eqn(x,y) => x <=> y.
//
//   This is used before FOOL-Unrolling, to clearly identify Boolean
//   positions and make sure that unrolling happens at the right
//   polarity.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

TFormula_p TFormulaExpandLiterals(TB_p terms, TFormula_p form)
{
   int i;
   TFormula_p handle, arg;
   bool changed = false;

   /* printf("TFormulaExpandLiterals ["); */
   /* TermPrettyPrintSimple(stdout, form, terms->sig, 0); */
   /* printf(" ]\n"); */

   if(TermIsDBVar(form))
   {
      return form;
   }

   if(form->f_code == terms->sig->neqn_code)
   {
      handle = TermTopCopy(form);
      handle->f_code = terms->sig->eqn_code;
      handle = TBTermTopInsert(terms, handle);
      form = TFormulaFCodeAlloc(terms, terms->sig->not_code, handle, NULL);
   }
   handle = TermTopCopyWithoutArgs(form);
   for(i=0; i<form->arity; i++)
   {
      arg = TFormulaExpandLiterals(terms, form->args[i]);
      handle->args[i] = arg;
      changed |= (arg!=form->args[i]);
   }
   if(changed)
   {
      form = TBTermTopInsert(terms, handle);
   }
   else
   {
      TermTopFree(handle);
   }
   if((form->arity==2)&&
      (form->f_code == terms->sig->eqn_code) &&
      (form->args[0]->type == terms->sig->type_bank->bool_type))
   {
      // Don't expand real literals
      if(!TermIsFreeVar(form->args[0]) && form->args[1] != terms->true_term)
      {
         form = TFormulaFCodeAlloc(terms, terms->sig->equiv_code,
                                   form->args[0], form->args[1]);
      }
      else if(!TermIsFreeVar(form->args[0]) &&
              (form->args[0]->f_code < terms->sig->internal_symbols) &&
              (form->args[0]->f_code != terms->sig->answer_code))
      {
         assert(form->args[1] == terms->true_term);
         form = form->args[0];
      }
   }
   /* printf("TFormulaExpandLiterals end ["); */
   /* TermPrettyPrintSimple(stdout, form, terms->sig, 0); */
   /* printf(" ]\n"); */
   return form;
}

#ifdef NEVER_DEFINED
/*-----------------------------------------------------------------------
//
// Function: TFormulaReEncodeLiterals()
//
//   Find non-trivial Boolean terms and re-encode them as
//   equations/disequations.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

TFormula_p TFormulaReEncodeLiterals(TB_p terms, TFormula_p form)
{
   return form;
}
#endif

/*-----------------------------------------------------------------------
//
// Function: TFormulaSimplify()
//
//   Maximally simplify a formula using (primarily)
//   the simplification rules (from [NW:SmallCNF-2001]).
//
//   P | P => P    P | T => T     P | F => P
//   P & P => F    P & T => P     P & F -> F
//   ~T = F        ~F = T
//   P <-> P => T  P <-> F => ~P  P <-> T => P
//   P <~> P => ~(P<->P)
//   P -> P => T   P -> T => T    P -> F => ~P
//   ...
//
//   We only check for redundant quantifiers in "small" formulas
//   (weight less than quopt_limit)
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

TFormula_p TFormulaSimplify(TB_p terms, TFormula_p form, long quopt_limit)
{
   TFormula_p handle, arg1=NULL, arg2=NULL, newform;
   FunCode f_code;
   bool modified=false;

   assert(terms);

   // printf("Simplify %p %ld: ", form, form->weight);/* TFormulaTPTPPrint(stdout, terms, form, true, false)*/;printf("\n");

   if(TFormulaIsLiteral(terms->sig, form)||TypeIsArrow(form->type))
   {
      return form;
   }
   if(TFormulaHasSubForm1(terms->sig,form))
   {
      arg1 = TFormulaSimplify(terms, form->args[0], quopt_limit);
      modified = arg1!=form->args[0];
   }
   else if(TFormulaIsQuantified(terms->sig, form))
   {
      arg1 = form->args[0];
   }

   if(TFormulaHasSubForm2(terms->sig, form)||
      TFormulaIsQuantified(terms->sig, form))
   {
      arg2 = TFormulaSimplify(terms, form->args[1], quopt_limit);
      modified |= arg2!=form->args[1];
   }
   if(modified)
   {
      assert(terms);
      form = TFormulaFCodeAlloc(terms, form->f_code, arg1, arg2);
   }
   modified = true;
   while(modified)
   {
      // printf("Loop %p %ld: ", form, form->weight);/*TFormulaTPTPPrint(stdout, terms, form, true, false);*/printf("\n");
      modified = false;
      newform = form; /* Inelegant, fix when awake! */
      if(form->f_code == terms->sig->not_code)
      {
         if(TFormulaIsLiteral(terms->sig, form->args[0]))
         {
            f_code = SigGetOtherEqnCode(terms->sig, form->args[0]->f_code);
            newform = TFormulaFCodeAlloc(terms, f_code,
                                         form->args[0]->args[0],
                                         form->args[0]->args[1]);
         }
      }
      else if(form->f_code == terms->sig->or_code)
      {
         if((handle = tprop_arg_return_other(terms->sig, form->args[0],
                                             form->args[1], false)))
         {
            newform = handle;
         }
         else if((handle = tprop_arg_return(terms->sig, form->args[0],
                                            form->args[1], true)))
         {
            newform = handle;
         }
         else if(TFormulaEqual(form->args[0], form->args[1]))
         {
            newform = form->args[0];
         }
      }
      else if(form->f_code == terms->sig->and_code)
      {
         if((handle = tprop_arg_return_other(terms->sig, form->args[0],
                                             form->args[1], true)))
         {
            newform = handle;
         }
         else if((handle = tprop_arg_return(terms->sig, form->args[0],
                                            form->args[1], false)))
         {
            newform = handle;
         }
         else if(TFormulaEqual(form->args[0], form->args[1]))
         {
            newform = form->args[0];
         }
      }
      else if(form->f_code == terms->sig->equiv_code)
      {
         if((handle = tprop_arg_return_other(terms->sig, form->args[0],
                                             form->args[1], true)))
         {
            /* p <=> T -> p */
            newform = handle;
         }
         else if((handle = tprop_arg_return_other(terms->sig, form->args[0],
                                                  form->args[1], false)))
         {
            /* p <=> F -> ~p */
            newform = TFormulaNegAlloc(terms, handle);
         }
         else if(TFormulaEqual(form->args[0], form->args[1]))
         {
            newform = TFormulaPropConstantAlloc(terms, true);
         }
      }
      else if(form->f_code == terms->sig->impl_code)
      {
         if(TFormulaIsPropTrue(terms->sig, form->args[0]))
         {
            /* T => p -> p */
            newform = form->args[1];
         }
         else if(TFormulaIsPropFalse(terms->sig, form->args[0]))
         {
            /* F => p -> T */
            newform = TFormulaPropConstantAlloc(terms, true);
         }
         else if(TFormulaIsPropFalse(terms->sig, form->args[1]))
         {
            /* p => F -> ~p */
            newform = TFormulaNegAlloc(terms, form->args[0]);
         }
         else if(TFormulaIsPropTrue(terms->sig, form->args[1]))
         {
            /* p => T -> T */
            newform = TFormulaPropConstantAlloc(terms, true);
         }
         else if(TFormulaEqual(form->args[0], form->args[1]))
         {
            /* p => p -> T */
            newform = TFormulaPropConstantAlloc(terms, true);
         }
      }
      else if(form->f_code == terms->sig->xor_code)
      {
         handle = TFormulaFCodeAlloc(terms, terms->sig->equiv_code,
                                     form->args[0], form->args[1]);
         newform = TFormulaFCodeAlloc(terms, terms->sig->not_code,
                                      handle, NULL);
         newform =  TFormulaSimplify(terms, newform, quopt_limit);
      }
      else if(form->f_code == terms->sig->bimpl_code)
      {
         newform = TFormulaFCodeAlloc(terms, terms->sig->impl_code,
                                      form->args[1], form->args[0]);
         newform = TFormulaSimplify(terms, newform, quopt_limit);
      }
      else if(form->f_code == terms->sig->nor_code)
      {
         handle = TFormulaFCodeAlloc(terms, terms->sig->or_code,
                                     form->args[0], form->args[1]);
         newform = TFormulaFCodeAlloc(terms, terms->sig->not_code,
                                      handle, NULL);
         newform =  TFormulaSimplify(terms, newform, quopt_limit);
      }
      else if(form->f_code == terms->sig->nand_code)
      {
         handle = TFormulaFCodeAlloc(terms, terms->sig->and_code,
                                     form->args[0], form->args[1]);
         newform = TFormulaFCodeAlloc(terms, terms->sig->not_code,
                                      handle, NULL);
         newform =  TFormulaSimplify(terms, newform, quopt_limit);
      }
      else if((form->f_code == terms->sig->qex_code)||
              (form->f_code == terms->sig->qall_code))
      {
         if(!form->v_count ||
            ((form->weight<=quopt_limit) &&
             !TFormulaVarIsFree(terms, form->args[1], form->args[0])))
         {
            newform = form->args[1];
         }
      }
      if(newform!=form)
      {
         modified = true;
         form = newform;
      }
   }
   return newform;
}

/*-----------------------------------------------------------------------
//
// Function: TFormulaSimplifyDecoded()
//
//    Like TFromulaSimplify, but works on decoded formulas and performs
//    some more simplifications [http://ceur-ws.org/Vol-2752/paper11.pdf]
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

TFormula_p TFormulaSimplifyDecoded(TB_p terms, TFormula_p form)
{
   Term_p res = do_simplify_decoded(terms, form, true);
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: TFormulaNNF()
//
//   Destructively transform a (simplified) formula into NNF.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

TFormula_p TFormulaNNF(TB_p terms, TFormula_p form, int polarity)
{
   bool normalform = false;
   TFormula_p handle, handle2;

   assert(form && terms);
   assert((polarity<=1) && (polarity >=-1));

   while(!normalform)
   {
      normalform = true;
      handle = troot_nnf(terms, form, polarity);
      assert(handle);
      form = handle;
      if(form->f_code == terms->sig->not_code)
      {
         handle = TFormulaNNF(terms, form->args[0], -polarity);
         if(handle!=form->args[0])
         {
            normalform = false;
            form = TFormulaFCodeAlloc(terms, terms->sig->not_code,
                                      handle, NULL);
         }
      }
      else if((form->f_code == terms->sig->qex_code)||
              (form->f_code == terms->sig->qall_code))
      {
         handle = TFormulaNNF(terms, form->args[1], polarity);
         if(handle!=form->args[1])
         {
            normalform = false;
            form = TFormulaFCodeAlloc(terms, form->f_code,
                                      form->args[0], handle);
         }
      }
      else if((form->f_code == terms->sig->and_code)||
              (form->f_code == terms->sig->or_code))
      {
         handle  = TFormulaNNF(terms, form->args[0], polarity);
         assert(handle);
         handle2 = TFormulaNNF(terms, form->args[1], polarity);
         assert(handle2);
         if((handle!=form->args[0]) || (handle2!=form->args[1]))
         {
            normalform = false;
            form = TFormulaFCodeAlloc(terms, form->f_code,
                                      handle, handle2);
         }
      }
      else if(TermIsTrueTerm(form))
      {
         form = TFormulaFCodeAlloc(terms, terms->sig->eqn_code, form, form);
      }
      else if(TermIsFalseTerm(form))
      {
         form = TFormulaFCodeAlloc(terms, terms->sig->neqn_code, form, form);
      }
      else if(TermIsAppliedFreeVar(form))
      {
         form = TFormulaFCodeAlloc(terms, terms->sig->eqn_code, form, terms->true_term);
      }
      else
      {
         form = EncodePredicateAsEqn(terms, form);
         // if(!TFormulaIsLiteral(terms->sig, form))
         // {
         //    DBG_TPRINT(stderr, "error: ", form, ".\n");
         //    DBG_PRINT(stderr, "type: ", TypePrintTSTP(stderr, terms->sig->type_bank, form->type), ".\n");
         // }
         /* assert(TFormulaIsLiteral(terms->sig, form) */
         /*        && "Top level term not in normal form"); */
      }
   }
   assert(form);
   return form;
}





/*-----------------------------------------------------------------------
//
// Function: TFormulaMiniScope()
//
//   Perform mini-scoping, i.e. move quantors inward as far as
//   possible.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

TFormula_p TFormulaMiniScope(TB_p terms, TFormula_p form)
{
   bool modified;
   TFormula_p arg1,arg2;
   FunCode op, quant;

   if(TFormulaIsQuantified(terms->sig, form))
   {
      op = form->args[1]->f_code;
      quant = form->f_code;

      if((op == terms->sig->and_code) || (op == terms->sig->or_code))
      {
         if(!TFormulaVarIsFree(terms, form->args[1]->args[0],
                               form->args[0]))
         {
            arg2 = TFormulaQuantorAlloc(terms, quant, form->args[0],
                                        form->args[1]->args[1]);
            arg1 = form->args[1]->args[0];
            form = TFormulaFCodeAlloc(terms, op, arg1, arg2);
         }
         else if(!TFormulaVarIsFree(terms, form->args[1]->args[1],
                               form->args[0]))
         {
            arg1 = TFormulaQuantorAlloc(terms, quant, form->args[0],
                                        form->args[1]->args[0]);
            arg2 = form->args[1]->args[1];
            form = TFormulaFCodeAlloc(terms, op, arg1, arg2);
         }
         else
         {
            if((op == terms->sig->and_code) && (quant == terms->sig->qall_code))
            {
               arg1 = TFormulaQuantorAlloc(terms, terms->sig->qall_code,
                                           form->args[0],
                                           form->args[1]->args[0]);
               arg2 = TFormulaQuantorAlloc(terms, terms->sig->qall_code,
                                           form->args[0],
                                           form->args[1]->args[1]);
               form = TFormulaFCodeAlloc(terms, terms->sig->and_code,
                                         arg1, arg2);
            }
            else if((op == terms->sig->or_code) && (quant == terms->sig->qex_code))
            {
               arg1 = TFormulaQuantorAlloc(terms, terms->sig->qex_code,
                                           form->args[0],
                                           form->args[1]->args[0]);
               arg2 = TFormulaQuantorAlloc(terms, terms->sig->qex_code,
                                           form->args[0],
                                           form->args[1]->args[1]);
               form = TFormulaFCodeAlloc(terms, terms->sig->or_code,
                                         arg1, arg2);
            }
         }
      }
   }
   arg1 = form->args[0];
   arg2 = form->args[1];
   modified = false;

   if(TFormulaHasSubForm1(terms->sig, form))
   {
      arg1 = TFormulaMiniScope(terms,form->args[0]);
      modified = arg1!=form->args[0];
   }
   if(TFormulaHasSubForm2(terms->sig, form) ||
      TFormulaIsQuantified(terms->sig, form))
   {
      arg2 = TFormulaMiniScope(terms,form->args[1]);
      modified |= arg2!=form->args[1];
   }
   if(modified)
   {
      form = TFormulaFCodeAlloc(terms, form->f_code, arg1, arg2);
      form = TFormulaMiniScope(terms, form);
   }
   return form;
}

/*-----------------------------------------------------------------------
//
// Function: TFormulaMiniScope2()
//
//   Perform mini-scoping, i.e. move quantors inward as far as
//   possible. Assumes that variables for each quantifier are unique,
//   and that the formula is in NNF. Only a finite amount of work is
//   spent on the formula (as defined by miniscope_limit).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

TFormula_p TFormulaMiniScope2(TB_p terms, TFormula_p form,
                              long miniscope_limit)
{
   Term_p var;
   FunCode quantor, qex, qall;
   PStack_p prenex = PStackAlloc();
   TermProperties proc = TPOpFlag;

   form = extract_formula_core2(terms, form, prenex);
   qall = terms->sig->qall_code;
   qex  = terms->sig->qex_code;

   TermDelPropOpt(form, TPOpFlag|TPCheckFlag);
   while(!PStackEmpty(prenex))
   {
      var = PStackPopP(prenex);
      //printf(COMCHAR" MiniScope ");TermPrint(stdout, var, terms->sig, DEREF_NEVER);printf("\n");
      quantor = PStackPopInt(prenex);

      if(miniscope_limit)
      {
         miniscope_limit--;

         assert(TermVerifyProp(form, DEREF_NEVER, TPOpFlag, proc?TPIgnoreProps:TPOpFlag));
         tform_mark_varocc(form, var, proc);
         assert(TermVerifyProp(form, DEREF_NEVER, TPOpFlag, proc));

         if(quantor == qex)
         {
            form = miniscope_qex(terms, form, var, proc);
         }
         else if(quantor == qall)
         {
            form = miniscope_qall(terms, form, var, proc);
         }
         else
         {
            assert(false && "Only universal or existential quantifier allowed");
         }
         proc = proc?TPIgnoreProps:TPOpFlag;
      }
      else
      {
         form = TFormulaFCodeAlloc(terms, quantor, var, form);
      }
   }
   PStackFree(prenex);
   return form;
}


/*-----------------------------------------------------------------------
//
// Function: TFormulaMiniScope3()
//
//   Perform (conditional) mini-scoping, i.e. move quantors inward as
//   far as possible if there are "small" subformulas that might
//   profit from miniscoping. Assumes that variables for each
//   quantifier are unique, and that the formula is in NNF.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

TFormula_p TFormulaMiniScope3(TB_p terms, TFormula_p form,
                              long miniscope_limit)
{
   PTree_p    ms_forms = NULL, entry;
   bool       exq = false;
   TFormula_p cand;

   tform_find_miniscopeable(terms->sig, form, miniscope_limit,
                            &ms_forms, &exq);

   if(ms_forms)
   {
      assert(exq);
      PStack_p iter;

      // printf(COMCHAR" Found %ld positions\n", PTreeNodes(ms_forms));
      iter = PTreeTraverseInit(ms_forms);
      while((entry = PTreeTraverseNext(iter)))
      {
         cand = entry->key;
         assert(!cand->binding);
         cand->binding = TFormulaMiniScope(terms, cand);
      }
      PTreeTraverseExit(iter);

      form = tform_copy_mod(terms, form);

      iter = PTreeTraverseInit(ms_forms);
      while((entry = PTreeTraverseNext(iter)))
      {
         cand = entry->key;
         cand->binding = NULL;
      }
      PTreeTraverseExit(iter);
   }
   PTreeFree(ms_forms);

   return form;
}




/*-----------------------------------------------------------------------
//
// Function: TFormulaVarRename()
//
//   Convert the formula into one where all the bound variables have
//   been replaced by fresh one.
//
//   IMPORTANT PRECONDITION: terms->vars->f_count _must_ point to a
//   variable bigger than all in form.
//
// Global Variables: -
//
// Side Effects    : Consumes fresh variables from the term bank.
//
/----------------------------------------------------------------------*/

TFormula_p TFormulaVarRename(TB_p terms, TFormula_p form)
{
   assert(!TermIsAppliedFreeVar(form));
   Term_p old_var = NULL, new_var = NULL;
   TFormula_p handle = NULL, arg1=NULL, arg2=NULL;

   if(TFormulaIsQuantified(terms->sig, form))
   {
      old_var = form->args[0]->binding;
      new_var = VarBankGetFreshVar(terms->vars, form->args[0]->type);
      assert(new_var != form->args[0]);
      form->args[0]->binding = new_var;
   }
   if(form->f_code == SIG_LET_CODE || form->f_code == SIG_ITE_CODE)
   {
      TFormula_p newform = TermTopCopyWithoutArgs(form);
      for(long i=0; i < newform->arity; i++)
      {
         newform->args[i] = TFormulaVarRename(terms, form->args[i]);
      }
      handle = TBTermTopInsert(terms, newform);
   }
   else if(TFormulaIsLiteral(terms->sig, form) || TypeIsArrow(form->type))
   {
      handle = TFormulaCopy(terms, form);
   }
   else if(!SigIsLogicalSymbol(terms->sig, form->f_code) &&
           form->type == terms->sig->type_bank->bool_type)
   {
      handle = TFormulaCopy(terms, form);
   }
   else
   {
      if(TFormulaIsQuantified(terms->sig, form))
      {
         arg1 = new_var;
         arg2 = TFormulaVarRename(terms, form->args[1]);
      }
      else if(TFormulaHasSubForm1(terms->sig, form))
      {
         arg1 = TFormulaVarRename(terms, form->args[0]);
      }
      if(TFormulaHasSubForm2(terms->sig, form))
      {
         arg2 = TFormulaVarRename(terms, form->args[1]);
      }
      handle = TFormulaFCodeAlloc(terms, form->f_code, arg1, arg2);
   }
   if(TFormulaIsQuantified(terms->sig, form))
   {
      form->args[0]->binding = old_var;
   }
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: FormulaSkolemizeOutermost()
//
//   Skolemize a formula in an outermost
//   manner. Interpretes the formula as its universal closure,
//   i.e. globally free variables in form are used as Skolem function
//   arguments. Also assumes that every quantor binds a new
//   variable.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

TFormula_p TFormulaSkolemizeOutermost(TB_p terms, TFormula_p form)
{
   TFormula_p res = NULL;
   PTree_p   free_vars = NULL;
   PStack_p  var_stack = PStackAlloc();
   Term_p    var;

   /* The next line should now be unnecessary, since formulas need to
      be closed. But let's be conservative for now... */
   TFormulaCollectFreeVars(terms, form, &free_vars);
   while(free_vars)
   {
      var = PTreeExtractRootKey(&free_vars);
      PStackPushP(var_stack, var);
   }
   res = tformula_rek_skolemize(terms, form, var_stack);
   PStackFree(var_stack);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: TFormulaShiftQuantors()
//
//   Shift all remaining all-quantors outward. This has several
//   premises:
//   - All quantified variables are disjoint from each other and from
//     the free variables.
//   - The formula is in negation normal form.
//   - All quantifiers are universal.
//
// Global Variables: -
//
// Side Effects    : Destroys original formula.
//
/----------------------------------------------------------------------*/

TFormula_p TFormulaShiftQuantors(TB_p terms, TFormula_p form)
{
   Term_p var;
   PStack_p varstack;

   varstack = PStackAlloc();
   form = extract_formula_core(terms, form, varstack);

   while(!PStackEmpty(varstack))
   {
      var = PStackPopP(varstack);
      form = TFormulaQuantorAlloc(terms,
                                  terms->sig->qall_code,
                                  var,
                                  form);
   }
   PStackFree(varstack);
   return form;
}



/*-----------------------------------------------------------------------
//
// Function: TFormulaShiftQuantors2()
//
//   Shift all all-quantors outward. This has several
//   premises:
//   - All quantified variables are disjoint from each other and from
//     the free variables.
//   - The formula is in negation normal form.
//
// Global Variables: -
//
// Side Effects    : Destroys original formula.
//
/----------------------------------------------------------------------*/

TFormula_p TFormulaShiftQuantors2(TB_p terms, TFormula_p form)
{
   Term_p var;
   FunCode quantor;
   PStack_p varstack;

   varstack = PStackAlloc();
   form = extract_formula_core2(terms, form, varstack);

   while(!PStackEmpty(varstack))
   {
      var = PStackPopP(varstack);
      quantor = PStackPopInt(varstack);
      form = TFormulaQuantorAlloc(terms,
                                  quantor,
                                  var,
                                  form);
   }
   PStackFree(varstack);
   return form;
}


/*-----------------------------------------------------------------------
//
// Function: TFormulaDistributeDisjunctions()
//
//   Apply distributivity law to transform a suitably preprocessed
//   formula into conjunctive normal form.
//
// Global Variables: -
//
// Side Effects    : Rebuilds formula
//
/----------------------------------------------------------------------*/

TFormula_p TFormulaDistributeDisjunctions(TB_p terms, TFormula_p form)
{
   TFormula_p handle, narg1=NULL, narg2=NULL;
   bool change = false;
   // formula is in NNF

   if(TermIsDBVar(form))
   {
      return form;
   }
   assert(TFormulaIsQuantified(terms->sig, form) ||
          form->f_code == terms->sig->or_code ||
          form->f_code == terms->sig->and_code ||
          TFormulaIsLiteral(terms->sig, form) ||
          TermIsTrueTerm(form) ||
          TermIsFalseTerm(form));

   //printf("TFormulaDistributeDisjunctions: ");
   //TFormulaTPTPPrint(GlobalOut, terms, form, true, false);
   //printf("\n");

   if(TFormulaHasSubForm1(terms->sig, form))
   {
      narg1 = TFormulaDistributeDisjunctions(terms, form->args[0]);
      change = narg1!=form->args[0];
   }
   else if(TFormulaIsQuantified(terms->sig, form))
   {
      narg1=form->args[0];
   }
   if(TFormulaHasSubForm2(terms->sig, form)||
      TFormulaIsQuantified(terms->sig, form))
   {
      narg2 = TFormulaDistributeDisjunctions(terms, form->args[1]);
      change |= narg2!=form->args[1];
   }
   if(change)
   {
      form = TFormulaFCodeAlloc(terms, form->f_code, narg1, narg2);
   }

   if(form->f_code == terms->sig->or_code)
   {
      if(!TermIsDBVar(form->args[0]) &&
         form->args[0]->f_code == terms->sig->and_code)
      {  /* or(and(f1,f2), f3) -> and(or(f1,f3), or(f2, f3) */
         narg1 = TFormulaFCodeAlloc(terms, terms->sig->or_code,
                                    form->args[0]->args[0], form->args[1]);
         narg2 = TFormulaFCodeAlloc(terms, terms->sig->or_code,
                                    form->args[0]->args[1], form->args[1]);
         handle = TFormulaFCodeAlloc(terms, terms->sig->and_code,
                                     narg1, narg2);
         form = TFormulaDistributeDisjunctions(terms, handle);
      }
      else if(!TermIsDBVar(form->args[1]) &&
              form->args[1]->f_code == terms->sig->and_code)
      {
         narg2 = TFormulaFCodeAlloc(terms, terms->sig->or_code,
                                    form->args[1]->args[1], form->args[0]);
         narg1 = TFormulaFCodeAlloc(terms, terms->sig->or_code,
                                    form->args[1]->args[0], form->args[0]);
         handle = TFormulaFCodeAlloc(terms, terms->sig->and_code,
                                     narg1, narg2);
         form = TFormulaDistributeDisjunctions(terms, handle);
      }
   }
   return form;
}



/*-----------------------------------------------------------------------
//
// Function: WTFormulaConjunctiveNF()
//
//   Transform a formula into Conjunctive Normal Form.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void WTFormulaConjunctiveNF(WFormula_p form, TB_p terms)
{
   TFormula_p handle;

   /*printf("Start: ");
   WFormulaPrint(GlobalOut, form, true);
   printf("\n");*/

   handle = TFormulaSimplify(terms, form->tformula, 1000);

   if(handle!=form->tformula)
   {
      form->tformula = handle;
      DocFormulaModificationDefault(form, inf_fof_simpl);
      WFormulaPushDerivation(form, DCFofSimplify, NULL, NULL);
   }

   handle = TFormulaNNF(terms, form->tformula, 1);
   if(handle!=form->tformula)
   {
      form->tformula = handle;
      DocFormulaModificationDefault(form, inf_fof_nnf);
      WFormulaPushDerivation(form, DCFNNF, NULL, NULL);
   }
   handle = TFormulaMiniScope(terms, form->tformula);
   if(handle!=form->tformula)
   {
      form->tformula = handle;
      DocFormulaModificationDefault(form, inf_shift_quantors);
      WFormulaPushDerivation(form, DCShiftQuantors, NULL, NULL);
   }
   TFormulaFindMaxVarCode(form->tformula);
   VarBankSetVCountsToUsed(terms->vars);
   handle = TFormulaVarRename(terms, form->tformula);

   if(handle!=form->tformula)
   {
      form->tformula = handle;
      DocFormulaModificationDefault(form, inf_var_rename);
      WFormulaPushDerivation(form, DCVarRename, NULL, NULL);
   }
   handle = TFormulaSkolemizeOutermost(terms, form->tformula);
   if(handle!=form->tformula)
   {
      form->tformula = handle;
      DocFormulaModificationDefault(form, inf_skolemize_out);
      WFormulaPushDerivation(form, DCSkolemize, NULL, NULL);
   }
   handle = TFormulaShiftQuantors(terms, form->tformula);
   if(handle!=form->tformula)
   {
      form->tformula = handle;
      DocFormulaModificationDefault(form, inf_shift_quantors);
      WFormulaPushDerivation(form, DCShiftQuantors, NULL, NULL);
   }

   handle = TFormulaDistributeDisjunctions(terms, form->tformula);

   if(handle!=form->tformula)
   {
      form->tformula = handle;
      DocFormulaModificationDefault(form, inf_fof_distrib);
      WFormulaPushDerivation(form, DCDistDisjunctions, NULL, NULL);
   }
}



/* /\*----------------------------------------------------------------------- */
/* // */
/* // Function: WTFormulaConjunctiveNF2() */
/* // */
/* //   Transform a formula into Conjunctive Normal Form. */
/* // */
/* // Global Variables: - */
/* // */
/* // Side Effects    : - */
/* // */
/* /----------------------------------------------------------------------*\/ */

/* void WTFormulaConjunctiveNF2(WFormula_p form, TB_p terms, */
/*                              long miniscope_limit, bool unroll_fool) */
/* { */
/*    TFormula_p handle; */

/*    // printf(COMCHAR" Start: "); WFormulaPrint(GlobalOut, form, true); printf("\n"); */

/*    handle = TFormulaSimplify(terms, form->tformula, 0); */

/*    if(handle!=form->tformula) */
/*    { */
/*       form->tformula = handle; */
/*       DocFormulaModificationDefault(form, inf_fof_simpl); */
/*       WFormulaPushDerivation(form, DCFofSimplify, NULL, NULL); */
/*    } */
/*    // printf(COMCHAR" Simplified\n"); */

/*    handle = TFormulaNNF(terms, form->tformula, 1); */
/*    if(handle!=form->tformula) */
/*    { */
/*       form->tformula = handle; */
/*       DocFormulaModificationDefault(form, inf_fof_nnf); */
/*       WFormulaPushDerivation(form, DCFNNF, NULL, NULL); */
/*    } */
/*    //printf(COMCHAR" NNFed\n"); */


/*    TFormulaFindMaxVarCode(form->tformula); */
/*    VarBankSetVCountsToUsed(terms->vars); */
/*    handle = TFormulaVarRename(terms, form->tformula); */

/*    if(handle!=form->tformula) */
/*    { */
/*       form->tformula = handle; */
/*       DocFormulaModificationDefault(form, inf_var_rename); */
/*       WFormulaPushDerivation(form, DCVarRename, NULL, NULL); */
/*    } */
/*    //printf(COMCHAR" Renamed\n"); */

/*    handle = TFormulaShiftQuantors2(terms, form->tformula); */
/*    if(handle!=form->tformula) */
/*    { */
/*       form->tformula = handle; */
/*       DocFormulaModificationDefault(form, inf_shift_quantors); */
/*       WFormulaPushDerivation(form, DCShiftQuantors, NULL, NULL); */
/*    } */

/*    //printf(COMCHAR" Prenexed\n"); */

/*    handle = TFormulaSimplify(terms, form->tformula, 100); */

/*    if(handle!=form->tformula) */
/*    { */
/*       form->tformula = handle; */
/*       DocFormulaModificationDefault(form, inf_fof_simpl); */
/*       WFormulaPushDerivation(form, DCFofSimplify, NULL, NULL); */
/*    } */
/*    //printf(COMCHAR" Resimplified\n"); */

/*    // Here efficient miniscoping */
/*    handle = TFormulaMiniScope2(terms, form->tformula, miniscope_limit); */

/*    if(handle!=form->tformula) */
/*    { */
/*       form->tformula = handle; */
/*       DocFormulaModificationDefault(form, inf_shift_quantors); */
/*       WFormulaPushDerivation(form, DCShiftQuantors, NULL, NULL); */
/*    } */

/*    handle = TFormulaSkolemizeOutermost(terms, form->tformula); */
/*    if(handle!=form->tformula) */
/*    { */
/*       form->tformula = handle; */
/*       DocFormulaModificationDefault(form, inf_skolemize_out); */
/*       WFormulaPushDerivation(form, DCSkolemize, NULL, NULL); */
/*    } */


/*    handle = TFormulaShiftQuantors(terms, form->tformula); */
/*    if(handle!=form->tformula) */
/*    { */
/*       form->tformula = handle; */
/*       DocFormulaModificationDefault(form, inf_shift_quantors); */
/*       WFormulaPushDerivation(form, DCShiftQuantors, NULL, NULL); */
/*    } */

/*    // Skolemization might have introduced Skolem predicates, which */
/*    // we have to unroll again  -- unrolling keeps things in NNF */
/*    if(unroll_fool) */
/*    { */
/*       TFormulaUnrollFOOL(form,terms); // handles proof object internally */
/*    } */
/*    handle = TFormulaNNF(terms, form->tformula, 1); */
/*    if(handle!=form->tformula) */
/*    { */
/*       form->tformula = handle; */
/*       DocFormulaModificationDefault(form, inf_fof_nnf); */
/*       WFormulaPushDerivation(form, DCFNNF, NULL, NULL); */
/*    } */

/*    handle = TFormulaDistributeDisjunctions(terms, form->tformula); */


/*    if(handle!=form->tformula) */
/*    { */
/*       form->tformula = handle; */
/*       DocFormulaModificationDefault(form, inf_fof_distrib); */
/*       WFormulaPushDerivation(form, DCDistDisjunctions, NULL, NULL); */
/*    } */
/* } */



/*-----------------------------------------------------------------------
//
// Function: WTFormulaConjunctiveNF3()
//
//   Transform a formula into Conjunctive Normal Form.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void WTFormulaConjunctiveNF3(WFormula_p form, TB_p terms,
                             long miniscope_limit, bool unroll_fool)
{
   TFormula_p handle;

   /* printf("Start: "); */
   /* WFormulaPrint(GlobalOut, form, true); */
   /* printf("\n"); */

   handle = TFormulaSimplify(terms, form->tformula, 1000);

   if(handle!=form->tformula)
   {
      form->tformula = handle;
      DocFormulaModificationDefault(form, inf_fof_simpl);
      WFormulaPushDerivation(form, DCFofSimplify, NULL, NULL);
   }

   handle = TFormulaNNF(terms, form->tformula, 1);
   if(handle!=form->tformula)
   {
      form->tformula = handle;
      DocFormulaModificationDefault(form, inf_fof_nnf);
      WFormulaPushDerivation(form, DCFNNF, NULL, NULL);
   }

   handle = TFormulaMiniScope3(terms, form->tformula, miniscope_limit);
   //handle = TFormulaMiniScope(terms, form->tformula);
   //printf("Miniscoping done\n");

   if(handle!=form->tformula)
   {
      form->tformula = handle;
      DocFormulaModificationDefault(form, inf_shift_quantors);
      WFormulaPushDerivation(form, DCShiftQuantors, NULL, NULL);
   }
   TFormulaFindMaxVarCode(form->tformula);
   VarBankSetVCountsToUsed(terms->vars);
   handle = TFormulaVarRename(terms, form->tformula);

   if(handle!=form->tformula)
   {
      form->tformula = handle;
      DocFormulaModificationDefault(form, inf_var_rename);
      WFormulaPushDerivation(form, DCVarRename, NULL, NULL);
   }
   handle = TFormulaSkolemizeOutermost(terms, form->tformula);
   if(handle!=form->tformula)
   {
      form->tformula = handle;
      DocFormulaModificationDefault(form, inf_skolemize_out);
      WFormulaPushDerivation(form, DCSkolemize, NULL, NULL);
   }
   handle = TFormulaShiftQuantors(terms, form->tformula);
   if(handle!=form->tformula)
   {
      form->tformula = handle;
      DocFormulaModificationDefault(form, inf_shift_quantors);
      WFormulaPushDerivation(form, DCShiftQuantors, NULL, NULL);
   }
   //printf("SNF\n");

   if(unroll_fool)
   {
      TFormulaUnrollFOOL(form,terms); // handles proof object internally
   }
   /* printf("Fool unrolled: \n"); */
   /* WFormulaPrint(GlobalOut, form, true); */
   /* printf("\n"); */
   handle = TFormulaNNF(terms, form->tformula, 1);
   if(handle!=form->tformula)
   {
      form->tformula = handle;
      DocFormulaModificationDefault(form, inf_fof_nnf);
      WFormulaPushDerivation(form, DCFNNF, NULL, NULL);
   }
   /* printf("NNF: \n"); */
   /* WFormulaPrint(GlobalOut, form, true); */
   /* printf("\n"); */
   handle = TFormulaDistributeDisjunctions(terms, form->tformula);
   //printf("Disjunctions distributed\n");

   if(handle!=form->tformula)
   {
      form->tformula = handle;
      DocFormulaModificationDefault(form, inf_fof_distrib);
      WFormulaPushDerivation(form, DCDistDisjunctions, NULL, NULL);
   }
}





/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
