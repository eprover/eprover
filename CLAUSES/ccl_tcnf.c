/*-----------------------------------------------------------------------

File  : ccl_tcnf.c

Author: Stephan Schulz

Contents
 
  Functions for CNF conversion of a term-encoded FOF formula.

  Copyright 2003,2005 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Thu Feb 26 00:21:17 CET 2004
    New (from ccl_cnf.c)

-----------------------------------------------------------------------*/

#include "ccl_tcnf.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

/* long SimplificationCounter = 0; */

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


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

TFormula_p tprop_arg_return_other(TFormula_p arg1, TFormula_p arg2, bool positive)
{
   if(TFormulaIsPropConst(arg1, positive))
   {
      return arg2;
   }
   else if (TFormulaIsPropConst(arg2, positive))
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

TFormula_p tprop_arg_return(TFormula_p arg1, TFormula_p arg2, bool positive)
{
   if(TFormulaIsPropConst(arg1, positive))
   {
      return arg1;
   }
   else if (TFormulaIsPropConst(arg2, positive))
   {
      return arg2;
   }
   return NULL;
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
//   Recursively Skolemize form. Note that it is note quite trivial
//   that it this works, as it works on a shared structure, and the
//   same subformula may occur in different contexts. It _does_ work
//   (I hope) because we require that every quantor binds a distinct
//   variable, and hence terms that are equal are either invariant
//   with respect to context or contain different variables, and hence
//   are not shared. 
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
   
   if(TBTermIsGround(form))
   {
      /* All is well */
   }
   else if(TFormulaIsLiteral(terms->sig, form))
   {
      form = TFormulaCopy(terms, form);
   }
   else if(form->f_code == terms->sig->qex_code)
   {
      var = form->args[0];
      assert(TermIsVar(var));
      assert(!var->binding);
      sk_term = TBAllocNewSkolem(terms,free_vars, false);
      var->binding = sk_term;
      form = tformula_rek_skolemize(terms, form->args[1],
                                    free_vars);               
      var->binding = NULL;
   }
   else if(form->f_code == terms->sig->qall_code)
   {      
      var = form->args[0];
      assert(TermIsVar(var));
      assert(!var->binding);
      PStackPushP(free_vars, var);
      handle = tformula_rek_skolemize(terms, form->args[1],
                                      free_vars);
      form = TFormulaFCodeAlloc(terms, terms->sig->qall_code, 
                                var, handle);
      PStackPopP(free_vars);
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
//   Return true if formula|i should be renamed, false
//   otherwise. Polarity is the polarity of root, not root|i. 
//
// Global Variables: 
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool tformula_rename_test(TB_p bank, TFormula_p root, int pos, int polarity)
{
   int subform_sign;

   if((root->f_code == bank->sig->qex_code) || 
      (root->f_code == bank->sig->qall_code))
   {
      return false;
   }
   if(root->f_code == bank->sig->equiv_code)
   {
      if(TFormulaEstimateClauses(bank, root->args[pos],true) > TFORM_RENAME_LIMIT)
      {
         return true;
      }
      if(TFormulaEstimateClauses(bank, root->args[pos],false) > TFORM_RENAME_LIMIT)
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
               > TFORM_RENAME_LIMIT)
            {
               return true;
            }
            subform_sign = pos==2;
            if(root->f_code == bank->sig->impl_code && 
               TFormulaEstimateClauses(bank, root->args[pos],subform_sign) 
               > TFORM_RENAME_LIMIT)
            {
               return true;
            }
            break;
      case -1:
            if(root->f_code == bank->sig->and_code && 
               TFormulaEstimateClauses(bank, root->args[pos],false) 
               > TFORM_RENAME_LIMIT)
            {
               return true;
            }
            break;
      case 0:
            if((root->f_code == bank->sig->and_code ||
                root->f_code == bank->sig->or_code ||
                root->f_code == bank->sig->impl_code) &&
               (TFormulaEstimateClauses(bank, root->args[pos],true) >
                TFORM_RENAME_LIMIT ||
                TFormulaEstimateClauses(bank, root->args[pos],false) >
                TFORM_RENAME_LIMIT))
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
// Function: collect_applied_defs()
//
//   Given a tformula, find and record all the definitions for its
//   sub-formulas. 
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static long collect_applied_defs(Sig_p sig, TFormula_p form,
                                 NumTree_p *defs, PStack_p defs_used) 
{
   NumTree_p def_entry;
   long res = 0;
   
   if(TFormulaIsLiteral(sig, form))
   {
      /* done */
   }
   else 
   {
      if(TermCellQueryProp(form, TPCheckFlag))
      {
         def_entry = NumTreeFind(defs, form->entry_no);
         assert(def_entry);
         PStackPushInt(defs_used, def_entry->val1.i_val);
         res++;
      }
      if((form->f_code == sig->and_code)||
         (form->f_code == sig->or_code)||
         (form->f_code == sig->impl_code)||
         (form->f_code == sig->equiv_code)||
         (form->f_code == sig->not_code))
      {
         res += collect_applied_defs(sig, form->args[0], defs, defs_used);
      }
      if(form->f_code != sig->not_code)
      {
         res += collect_applied_defs(sig, form->args[1], defs, defs_used);
      }
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
//   atoms. If too many formulas result, just return
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


   if(TermCellQueryProp(form, TPCheckFlag) ||
      TFormulaIsLiteral(bank->sig, form))
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
         posres1 = TFormulaEstimateClauses(bank, form->args[0], true);
         RETURN_IF_LARGE(posres1);
         res = posres1;         
      }
      else
      {
         assert(false && "Formula not in correct simplified form");
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
         negres1 = TFormulaEstimateClauses(bank, form->args[0], false);
         RETURN_IF_LARGE(negres1);
         res = negres1;         
      }
      else
      {
         assert(false && "Formula not in correct simplified form");
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
                             NumTree_p *defs, PStack_p renamed_forms)
{
   NumTree_p def = NumTreeFind(defs, form->entry_no);
   
   if(def)
   {
      if(polarity!=def->val1.i_val)
      {
         def->val1.i_val = 0;
      }
      return def->val2.p_val;
   }
   else
   {
      PTree_p free_vars = NULL;
      PStack_p var_stack = PStackAlloc();
      TFormula_p rename_atom;      

      TFormulaCollectFreeVars(bank, form, &free_vars);
      PTreeToPStack(var_stack, free_vars);
      
      rename_atom = TBAllocNewSkolem(bank, var_stack, true);
      rename_atom = EqnTermsTBTermEncode(bank, rename_atom, 
                                         bank->true_term, true, PENormal); 
      PStackFree(var_stack);
      PTreeFree(free_vars);
      
      def = NumTreeCellAlloc();
      def->key = form->entry_no;      
      def->val1.i_val = polarity;
      def->val2.p_val = rename_atom;
      NumTreeInsert(defs, def);
      TermCellSetProp(form, TPCheckFlag);
      PStackPushP(renamed_forms, form);

      return def->val2.p_val;
   }
}


/*-----------------------------------------------------------------------
//
// Function: TFormulaFindDefs()
//
//   Find all useful definitions in form and enter them in defs and
//   renamed_forms.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void TFormulaFindDefs(TB_p bank, TFormula_p form, int polarity, 
                      NumTree_p *defs, PStack_p renamed_forms)
{
   if(TermCellQueryProp(form, TPCheckFlag) ||
      TFormulaIsLiteral(bank->sig, form))
   {
      return;
   }
   /* Handle args[0] (we are doing depth first) */
   if((form->f_code == bank->sig->and_code)||
      (form->f_code == bank->sig->or_code))
   {
      TFormulaFindDefs(bank, form->args[0], polarity, defs, renamed_forms);
      if(tformula_rename_test(bank, form, 0, polarity))
      {
         TFormulaDefRename(bank, form, polarity, 
                           defs, renamed_forms);         
      }
   }
   else if((form->f_code == bank->sig->not_code)||
           (form->f_code == bank->sig->impl_code))
   {
      TFormulaFindDefs(bank, form->args[0], -polarity, defs, renamed_forms);
      if(tformula_rename_test(bank, form, 0, polarity))
      {
         TFormulaDefRename(bank, form, -polarity, 
                           defs, renamed_forms);         
      }
   }
   else if((form->f_code == bank->sig->equiv_code))
   {
      TFormulaFindDefs(bank, form->args[0], 0, defs, renamed_forms);
      if(tformula_rename_test(bank, form, 0, polarity))
      {
         TFormulaDefRename(bank, form, 0, 
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
      TFormulaFindDefs(bank, form->args[1], polarity, defs, renamed_forms);               
      if(tformula_rename_test(bank, form, 1, polarity))
      {
         TFormulaDefRename(bank, form, polarity, defs, renamed_forms);         
      }
   }
   else if((form->f_code == bank->sig->equiv_code))
   {
      TFormulaFindDefs(bank, form->args[1], 0, defs, renamed_forms);
      if(tformula_rename_test(bank, form, 1, polarity))
      {
         TFormulaDefRename(bank, form, 0, defs, renamed_forms);         
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: TFormulaCopyDef()
//
//   Copy a formula, replacing all defined subformulas (except for the
//   blocked one, if any) with the proper definition). Record _all_
//   definitions (even sub-definitions) on the stack (by pushing the
//   definition numbers onto the stack).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

TFormula_p TFormulaCopyDef(TB_p bank, TFormula_p form, long blocked, 
                           NumTree_p *defs, PStack_p defs_used) 
{
   TFormula_p res = NULL, arg1, arg2 = NULL;
   NumTree_p def_entry;

   if(TFormulaIsLiteral(bank->sig, form))
   {
      res = form;
   }
   else if(TermCellQueryProp(form, TPCheckFlag))
   {
      def_entry = NumTreeFind(defs, form->entry_no);
      assert(def_entry);
      if(def_entry->val1.i_val!=blocked)
      {
         res = def_entry->val2.p_val;
         collect_applied_defs(bank->sig, form, defs, defs_used);
      }
   }
   if(!res)
   {
      if((form->f_code == bank->sig->and_code)||
         (form->f_code == bank->sig->or_code)||
         (form->f_code == bank->sig->impl_code)||
         (form->f_code == bank->sig->equiv_code)||
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
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

TFormula_p TFormulaSimplify(TB_p terms, TFormula_p form)
{
   TFormula_p handle, arg1=NULL, arg2=NULL, newform;
   FunCode f_code;
   bool modified=false;

   assert(terms);
   
   if(TFormulaIsLiteral(terms->sig, form))      
   {
      return form;
   }
   if(TFormulaHasSubForm1(terms->sig,form))
   {
      arg1 = TFormulaSimplify(terms, form->args[0]);
      modified = arg1!=form->args[0];
   }
   else if(TFormulaIsQuantified(terms->sig, form))
   {
      arg1 = form->args[0];
   }

   if(TFormulaHasSubForm2(terms->sig, form)||
      TFormulaIsQuantified(terms->sig, form))
   {
      arg2 = TFormulaSimplify(terms, form->args[1]);
      modified |= arg2!=form->args[1];      
   }
   if(modified)
   {
      assert(terms);
      form = TFormulaFCodeAlloc(terms, form->f_code, arg1, arg2);
   }
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
      if((handle = tprop_arg_return_other(form->args[0], form->args[1],
                                          false)))
      {
         newform = handle;
      }
      else if((handle = tprop_arg_return(form->args[0], form->args[1],
                                         true)))
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
      if((handle = tprop_arg_return_other(form->args[0], form->args[1],
                                          true)))
      {
         newform = handle;
      }
      else if((handle = tprop_arg_return(form->args[0], form->args[1],
                                         false)))
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
      if((handle = tprop_arg_return_other(form->args[0], form->args[1],
                                          true)))
      {
         newform = handle;
      }
      else if((handle = tprop_arg_return_other(form->args[0], form->args[1],
                                               false)))
      {
         newform = TFormulaFCodeAlloc(terms, terms->sig->not_code, 
                                      handle, NULL);
         newform = TFormulaSimplify(terms, newform);
      }
      else if(TFormulaEqual(form->args[0], form->args[1]))
      {
         newform = TFormulaPropConstantAlloc(terms, true);
      }
   }
   else if(form->f_code == terms->sig->impl_code)
   {
      if(TFormulaIsPropTrue(form->args[0]))
      {
         newform = form->args[1];
      }
      else if(TFormulaIsPropFalse(form->args[0]))
      {
         newform = TFormulaPropConstantAlloc(terms, true);
      }
      else if(TFormulaIsPropFalse(form->args[1]))
      {
         newform = TFormulaFCodeAlloc(terms, terms->sig->not_code, 
                                       form->args[0], NULL);
         newform = TFormulaSimplify(terms, newform);
      }
      else if(TFormulaIsPropTrue(form->args[1]))
      {
         newform = TFormulaPropConstantAlloc(terms, true);
      }
      else if(TFormulaEqual(form->args[0], form->args[1]))
      {
         newform = TFormulaPropConstantAlloc(terms, true);
      }
   }
   else if(form->f_code == terms->sig->xor_code)
   {
      handle = TFormulaFCodeAlloc(terms, terms->sig->equiv_code, 
                                  form->args[0], form->args[1]);
      newform = TFormulaFCodeAlloc(terms, terms->sig->not_code, 
                                   handle, NULL);
      newform =  TFormulaSimplify(terms, newform);
   }
   else if(form->f_code == terms->sig->bimpl_code)
   {
      newform = TFormulaFCodeAlloc(terms, terms->sig->impl_code, 
                                  form->args[1], form->args[0]);
      newform = TFormulaSimplify(terms, newform);
   }
   else if((form->f_code == terms->sig->qex_code)||
           (form->f_code == terms->sig->qex_code))
   {
      if(!TFormulaVarIsFree(terms, form->args[1], form->args[0]))
      {
         newform = form->args[1];
      }      
   }
   return newform;
}


/*-----------------------------------------------------------------------
//
// Function: TFormulaNNF()
//
//   Destructively transform a (simpified) formula into NNF.
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

   assert(form);
      
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
      else
      {
         assert(TFormulaIsLiteral(terms->sig, form)
                && "Top level term not in normal form");
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
      op = form->args[0]->f_code;
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
         if(!TFormulaVarIsFree(terms, form->args[1]->args[1], 
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
   arg1 = NULL;
   arg2 = NULL;
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
   Term_p old_var = NULL, new_var = NULL;
   TFormula_p handle = NULL, arg1=NULL, arg2=NULL;

   if(TFormulaIsQuantified(terms->sig, form))
   {
      old_var = form->args[0]->binding;
      new_var = VarBankGetFreshVar(terms->vars);
      assert(new_var != form->args[0]);
      form->args[0]->binding = new_var;
   }
   if(TFormulaIsLiteral(terms->sig, form))
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
//   aarguments. Also assumes that every quantor binds a new
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
//   Shift all remaining all-quantors outward.
//
// Global Variables: -
//
// Side Effects    : Destroys original formula.
//
/----------------------------------------------------------------------*/

TFormula_p TFormulaShiftQuantors(TB_p terms, TFormula_p form)
{
   TFormula_p handle, narg1=NULL, narg2=NULL, newform;
   bool modified = false;
   Term_p var;

   if(TFormulaIsQuantified(terms->sig, form))
   {
      narg1 = form->args[0];
   }
   else if(TFormulaHasSubForm1(terms->sig, form))
   {
      narg1 = TFormulaShiftQuantors(terms, form->args[0]);
      modified = narg1!=form->args[0];
   }
   if(TFormulaHasSubForm2(terms->sig, form)||
      TFormulaIsQuantified(terms->sig, form))
   {
      narg2 = TFormulaShiftQuantors(terms, form->args[1]);
      modified |= narg2!=form->args[1];
   }   
   if(modified)
   {
      form = TFormulaFCodeAlloc(terms, form->f_code, narg1, narg2);
   }
   if((form->f_code == terms->sig->and_code)||
      (form->f_code == terms->sig->or_code))
   {
      if(form->args[0]->f_code == terms->sig->qall_code)
      {
         narg1  = form->args[0]->args[1];
         narg2  = form->args[1];
         var    = form->args[0]->args[0];
         assert(!TFormulaVarIsFree(terms, narg2, var));
         handle = TFormulaFCodeAlloc(terms, form->f_code, narg1, narg2);
         newform = TFormulaQuantorAlloc(terms, terms->sig->qall_code, var, handle);
         form = TFormulaShiftQuantors(terms, newform);
      }
      else if(form->args[1]->f_code == terms->sig->qall_code)
      {
         narg1  = form->args[1]->args[1];
         narg2  = form->args[0];
         var    = form->args[1]->args[0];
         assert(!TFormulaVarIsFree(terms, narg2, var));
         handle = TFormulaFCodeAlloc(terms, form->f_code, narg1, narg2);
         newform = TFormulaQuantorAlloc(terms, terms->sig->qall_code, var, handle);
         form = TFormulaShiftQuantors(terms, newform);
      }
   }
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
// Side Effects    : May destroy formula
//
/----------------------------------------------------------------------*/

TFormula_p TFormulaDistributeDisjunctions(TB_p terms, TFormula_p form)
{
   TFormula_p handle, narg1=NULL, narg2=NULL;
   bool change = false;

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
      if(form->args[0]->f_code == terms->sig->and_code)
      {  /* or(and(f1,f2), f3) -> and(or(f1,f3), or(f2, f3) */
         narg1 = TFormulaFCodeAlloc(terms, terms->sig->or_code, 
                                    form->args[0]->args[0], form->args[1]);
         narg2 = TFormulaFCodeAlloc(terms, terms->sig->or_code, 
                                    form->args[0]->args[1], form->args[1]);
         handle = TFormulaFCodeAlloc(terms, terms->sig->and_code,
                                     narg1, narg2);
         form = TFormulaDistributeDisjunctions(terms, handle);
      }
      else if(form->args[1]->f_code == terms->sig->and_code)
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
   FunCode   max_var;

   /* printf("Start: ");
   WFormulaPrint(GlobalOut, form, true);
   printf("\n"); */

   handle = TFormulaSimplify(terms, form->tformula);
   
   if(handle!=form->tformula)
   {
      form->tformula = handle;
      DocFormulaModificationDefault(form, inf_fof_simpl);
   }

   handle = TFormulaNNF(terms, form->tformula, 1);
   if(handle!=form->tformula)
   {
      form->tformula = handle;
      DocFormulaModificationDefault(form, inf_fof_nnf);
   }

  
   handle = TFormulaMiniScope(terms, form->tformula);
   if(handle!=form->tformula)
   {
      form->tformula = handle;
      DocFormulaModificationDefault(form, inf_shift_quantors);
   }
   max_var = TFormulaFindMaxVarCode(form->tformula);
   VarBankSetVCount(terms->vars, -max_var);
   handle = TFormulaVarRename(terms, form->tformula);
   
   if(handle!=form->tformula)
   {
      form->tformula = handle;  
      DocFormulaModificationDefault(form, inf_var_rename);
   }
   VarBankVarsSetProp(terms->vars, TPIsFreeVar);
   handle = TFormulaSkolemizeOutermost(terms, form->tformula);
   if(handle!=form->tformula)
   {
      form->tformula = handle;  
      DocFormulaModificationDefault(form, inf_skolemize_out);
   }
   handle = TFormulaShiftQuantors(terms, form->tformula);
   if(handle!=form->tformula)
   {
      form->tformula = handle;  
      DocFormulaModificationDefault(form, inf_shift_quantors);
   }   
   
   handle = TFormulaDistributeDisjunctions(terms, form->tformula);

   if(handle!=form->tformula)
   {
      form->tformula = handle;  
      DocFormulaModificationDefault(form, inf_fof_distrib);
   }
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


