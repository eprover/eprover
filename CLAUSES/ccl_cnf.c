/*-----------------------------------------------------------------------

File  : ccl_cnf.c

Author: Stephan Schulz

Contents
 
  Functions for CNF conversion of a FOF formula.

  Copyright 1998-2003 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Thu Feb 26 00:21:17 CET 2004
    New

-----------------------------------------------------------------------*/

#include "ccl_cnf.h"



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
// Function: prop_arg_return_other()
//
//   If one of the args is a propositional formula of the desired
//   type, return the other one, else return NULL.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Formula_p prop_arg_return_other(Formula_p arg1, Formula_p arg2, bool positive)
{
   if(FormulaIsPropConst(arg1, positive))
   {
      return arg2;
   }
   else if (FormulaIsPropConst(arg2, positive))
   {
      return arg1;
   }
   return NULL;
}

/*-----------------------------------------------------------------------
//
// Function: prop_arg_return()
//
//   If one of the args is a propositional formula of the desired
//   type, return it, else return NULL.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Formula_p prop_arg_return(Formula_p arg1, Formula_p arg2, bool positive)
{
   if(FormulaIsPropConst(arg1, positive))
   {
      return arg1;
   }
   else if (FormulaIsPropConst(arg2, positive))
   {
      return arg2;
   }
   return NULL;
}


/*-----------------------------------------------------------------------
//
// Function: root_nnf()
//
//   Apply all NNF-transformation rules that can be applied at the
//   root level to *form.  Return true, if formula changed, false
//   otherwise.
//
// Global Variables: 
//
// Side Effects    : 
//
/----------------------------------------------------------------------*/

bool root_nnf(Formula_p *form, TB_p terms, int polarity)
{
   bool modified = false;
   Formula_p handle = *form, arg1, arg2, arg21, arg22;
   Eqn_p tmp;

   while(handle)
   {
      handle = NULL;

      if((*form)->op == OpUNot)
      {
         switch((*form)->arg1->op)
         {
         case OpIsLit:
               tmp = EqnAlloc((*form)->arg1->special.literal->lterm, 
                              (*form)->arg1->special.literal->rterm, 
                              terms,
                              !EqnIsPositive((*form)->arg1->special.literal));
               handle = FormulaLitAlloc(tmp);
               break;
         case OpUNot:
               handle = (*form)->arg1->arg1;
               break;
         case OpBOr:
               arg1 = FormulaOpAlloc(OpUNot, (*form)->arg1->arg1, NULL);
               arg2 = FormulaOpAlloc(OpUNot, (*form)->arg1->arg2, NULL);
               handle =  FormulaOpAlloc(OpBAnd, arg1, arg2);
               break;
         case OpBAnd:
               arg1 = FormulaOpAlloc(OpUNot, (*form)->arg1->arg1, NULL);
               arg2 = FormulaOpAlloc(OpUNot, (*form)->arg1->arg2, NULL);
               handle =  FormulaOpAlloc(OpBOr, arg1, arg2);
               break;
         case OpQAll:
               arg1 = FormulaOpAlloc(OpUNot, (*form)->arg1->arg1, NULL);
               handle = FormulaQuantorAlloc(OpQEx, (*form)->arg1->special.var, arg1);
               break;
         case OpQEx:
               arg1 = FormulaOpAlloc(OpUNot, (*form)->arg1->arg1, NULL);
               handle = FormulaQuantorAlloc(OpQAll, (*form)->arg1->special.var, arg1);
               break;               
         default:
               break;               
         }
      }
      else if ((*form)->op == OpBImpl)
      {
         arg1 = FormulaOpAlloc(OpUNot, (*form)->arg1, NULL);
         handle = FormulaOpAlloc(OpBOr, arg1, (*form)->arg2);
      }           
      else if ((*form)->op == OpBEquiv)
      {
         assert((polarity == 1) || (polarity == -1));
         if(polarity == 1)
         {
            arg1 = FormulaOpAlloc(OpBImpl, (*form)->arg1, (*form)->arg2);
            arg2 = FormulaOpAlloc(OpBImpl, (*form)->arg2, (*form)->arg1);
            handle = FormulaOpAlloc(OpBAnd, arg1, arg2);
         }
         else
         {
            arg21 = FormulaOpAlloc(OpUNot, (*form)->arg1, NULL);
            arg22 = FormulaOpAlloc(OpUNot, (*form)->arg2, NULL);
            arg2 = FormulaOpAlloc(OpBAnd, arg21, arg22);
            arg1 = FormulaOpAlloc(OpBAnd, (*form)->arg1, (*form)->arg2);
            handle = FormulaOpAlloc(OpBOr, arg1, arg2);                  
         }
      }
      if(handle)
      {
         FormulaGetRef(handle);
         FormulaRelRef(*form);         
         FormulaFree(*form);
         *form = handle;
         modified = true;
      }
   }
   return modified;
}


/*-----------------------------------------------------------------------
//
// Function: formula_rec_skolemize()
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
// Side Effects    : Destroys form, adds Skolem terms to term bank,
//                   modifies signature.
//
/----------------------------------------------------------------------*/

Formula_p formula_rek_skolemize(Formula_p form, TB_p terms, bool *modified, 
                                PStack_p free_vars)
{
   Term_p sk_term, var;   
   Formula_p handle;

   switch(form->op)
   {
   case OpIsLit:
         handle = FormulaCopy(form, terms);
         FormulaRelRef(form);
         FormulaGetRef(handle);
         FormulaFree(form);
         form = handle;
         break;
   case OpQEx:
         var = form->special.var;
         assert(TermIsVar(var));
         assert(!var->binding);
         sk_term = TBAllocNewSkolem(terms,free_vars);
         var->binding = sk_term;
         handle = formula_rek_skolemize(form->arg1, terms, 
                                        modified, free_vars);         
         /* Reference is inerited from down below */
         FormulaCellFree(form);
         form = handle;
         var->binding = NULL;
         *modified = true;
         break;
   case OpQAll:
         var = form->special.var;
         assert(TermIsVar(var));
         assert(!var->binding);
         PStackPushP(free_vars, var);
         form->arg1 = formula_rek_skolemize(form->arg1, terms, 
                                            modified, free_vars);
         PStackPopP(free_vars);
         break;
   default:
         if(FormulaHasSubForm1(form))
         {
            form->arg1 = formula_rek_skolemize(form->arg1, terms, 
                                               modified, free_vars);
         }
         if(FormulaHasSubForm2(form))
         {
            form->arg2 = formula_rek_skolemize(form->arg2, terms, modified, 
                                               free_vars);
         }         
         break;
   }
   return form;
}



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: FormulaSimplify()
//
//   Maximally (and destructively) simplify a formula using (primarily)
//   the simplification rules (from [NW:SmallCNF-2001]). 
//
//   P | P => P    P | T => T     P | F => P
//   P & P => F    P & T => P     P & F -> F
//   ~T = F        ~F = T
//   P <-> P => T  P <-> F => ~P  P <-> T => P
//   P <~> P => ~(P<->P)
//   P -> P => T   P -> T => T    P -> F => 
//   ...
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool FormulaSimplify(Formula_p *form, TB_p terms)
{
   Formula_p handle, newform = *form;
   bool modified = false, tmpmod = false;
   

   /* printf("Simplifying: ");
   FormulaTPTPPrint(stdout, form, true);
   printf("\n");*/

   if(FormulaHasSubForm1(*form))
   {
      modified = FormulaSimplify(&((*form)->arg1), terms);
   }
   if(FormulaHasSubForm2(*form))
   {
      tmpmod = FormulaSimplify(&((*form)->arg2), terms);
      modified = modified || tmpmod;
   }
   switch((*form)->op)
   {
   case OpUNot:
         if(FormulaIsLiteral((*form)->arg1))
         {
            Eqn_p tmp;
            tmp = EqnAlloc((*form)->arg1->special.literal->lterm, 
                           (*form)->arg1->special.literal->rterm, 
                           terms,
                           !EqnIsPositive((*form)->arg1->special.literal));
            newform = FormulaLitAlloc(tmp);
         }
         break;
   case OpBOr:
         if((handle = prop_arg_return_other((*form)->arg1, (*form)->arg2, false)))
         {
            newform = handle;
         }
         else if((handle = prop_arg_return((*form)->arg1, (*form)->arg2, true)))
         {
            newform = handle;
         }
         else if(FormulaEqual((*form)->arg1,(*form)->arg2))
         {
            newform = (*form)->arg1;
         }
         break;
   case OpBAnd:
         if((handle = prop_arg_return_other((*form)->arg1, (*form)->arg2, true)))
         {
            newform = handle;
         }
         else if((handle = prop_arg_return((*form)->arg1, (*form)->arg2, false)))
         {
            newform = handle;
         }
         else if(FormulaEqual((*form)->arg1,(*form)->arg2))
         {
            newform = (*form)->arg1;
         }
         break;         
   case OpBEquiv:
         if((handle = prop_arg_return_other((*form)->arg1, (*form)->arg2, true)))
         {
            newform = handle;
         }
         else if((handle = prop_arg_return_other((*form)->arg1, (*form)->arg2, false)))
         {
            newform = FormulaOpAlloc(OpUNot, handle, NULL);
            FormulaGetRef(newform);
            tmpmod = FormulaSimplify(&newform, terms);
            FormulaRelRef(newform);
            modified = tmpmod || modified;
         }
         else if(FormulaEqual((*form)->arg1,(*form)->arg2))
         {
            newform = FormulaPropConstantAlloc(terms, true);
         }
         break;
   case OpBImpl: 
         if(FormulaIsPropTrue((*form)->arg1))
         {
            newform = (*form)->arg2;
         }
         else if(FormulaIsPropFalse((*form)->arg1))
         {
            newform = FormulaPropConstantAlloc(terms, true);
         }
         else if(FormulaIsPropFalse((*form)->arg2))
         {
            newform = FormulaOpAlloc(OpUNot, (*form)->arg1, NULL);
            FormulaGetRef(newform);
            tmpmod = FormulaSimplify(&newform, terms);
            FormulaRelRef(newform);
            modified = tmpmod || modified;
         }
         else if(FormulaIsPropTrue((*form)->arg2))
         {
            newform = FormulaPropConstantAlloc(terms, true);
         }
         else if(FormulaEqual((*form)->arg1,(*form)->arg2))
         {
            newform = FormulaPropConstantAlloc(terms, true);
         }
         break;
   case OpBXor:
         handle = FormulaOpAlloc(OpBEquiv, (*form)->arg1, (*form)->arg2);
         newform = FormulaOpAlloc(OpUNot, handle, NULL);
         FormulaGetRef(newform);
         tmpmod = FormulaSimplify(&newform, terms);
         FormulaRelRef(newform);
         modified = tmpmod || modified;
         break;
   case OpBNor:
         handle = FormulaOpAlloc(OpBOr, (*form)->arg1, (*form)->arg2);
         newform = FormulaOpAlloc(OpUNot, handle, NULL);
         FormulaGetRef(newform);
         tmpmod = FormulaSimplify(&newform, terms);
         FormulaRelRef(newform);
         modified = tmpmod || modified;
         break;
   case OpBNand:
         handle = FormulaOpAlloc(OpBAnd, (*form)->arg1, (*form)->arg2);
         newform = FormulaOpAlloc(OpUNot, handle, NULL);
         FormulaGetRef(newform);
         tmpmod = FormulaSimplify(&newform, terms);
         FormulaRelRef(newform);
         modified = tmpmod || modified;
         break;         
   case OpBNImpl:
         handle = FormulaOpAlloc(OpBImpl, (*form)->arg2, (*form)->arg1);
         newform = FormulaOpAlloc(OpUNot, handle, NULL);
         FormulaGetRef(newform);
         tmpmod = FormulaSimplify(&newform, terms);
         FormulaRelRef(newform);
         modified = tmpmod || modified;
         break;
   case OpQEx:
   case OpQAll:
         if(!FormulaVarIsFree((*form)->arg1, (*form)->special.var))
         {
            newform = (*form)->arg1;
         }
         break;
   default:
         break;
   }
   if(newform != *form)
   {
      tmpmod = true;
      FormulaRelRef(*form);
      FormulaGetRef(newform);
      FormulaFree(*form);
      *form = newform;
   }
   /* printf("...done: ");
   FormulaTPTPPrint(stdout, form, true);
   printf("\n");*/
   return modified || tmpmod;
}


/*-----------------------------------------------------------------------
//
// Function: FormulaNNF()
//
//   Destructively transform a (simpified) formula into NNF.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool FormulaNNF(Formula_p *form, TB_p terms, int polarity)
{
   bool normalform = false, modified = false, tmpmod;

   assert(*form);
   assert((*form)->ref_count > 0);
      
   while(!normalform)
   {
      normalform = true;
      tmpmod = root_nnf(form, terms, polarity);      
      modified = modified||tmpmod;
            
      switch((*form)->op)
      {
      case OpIsLit:
            break;
      case OpUNot:
            tmpmod = FormulaNNF(&((*form)->arg1), terms, -polarity);
            modified = modified || tmpmod;
            normalform = normalform && !tmpmod;
            break;
      case OpQEx:
      case OpQAll:
            tmpmod = FormulaNNF(&((*form)->arg1), terms, polarity);
            modified = modified || tmpmod;
            normalform = normalform && !tmpmod;
            break;
      case OpBAnd:
      case OpBOr:
            tmpmod = FormulaNNF(&((*form)->arg1), terms, polarity);
            modified = modified || tmpmod;
            normalform = normalform && !tmpmod;
            tmpmod = FormulaNNF(&((*form)->arg2), terms, polarity);
            modified = modified || tmpmod;
            normalform = normalform && !tmpmod;
            break;            
      default:
            assert(false && "Top level term not in normal form");
            break;
      }
   }
   return modified;
}


/*-----------------------------------------------------------------------
//
// Function: FormulaMiniScope()
//
//   Perform mini-scoping, i.e. move quantors inward as far as
//   possible. Return true if the formula changed, false otherwise. 
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool FormulaMiniScope(Formula_p *form)
{
   bool modified = false, tmpmod;
   Formula_p newform = NULL, arg1,arg2;
   FOFOperatorType op, quant;   

   if(FormulaIsQuantified(*form))
   {
      op = (*form)->arg1->op;
      quant = (*form)->op;
      if((op == OpBAnd) || (op == OpBOr))
      {
         if(!FormulaVarIsFree((*form)->arg1->arg1, 
                              (*form)->special.var))
         {
            arg2 = FormulaQuantorAlloc(quant, (*form)->special.var,
                                       (*form)->arg1->arg2);
            arg1 = (*form)->arg1->arg1;
            newform = FormulaOpAlloc(op, arg1, arg2);
         }
         else if(!FormulaVarIsFree((*form)->arg1->arg2, 
                                   (*form)->special.var))
         {
            arg1 = FormulaQuantorAlloc(quant, (*form)->special.var,
                                       (*form)->arg1->arg1);
            arg2 = (*form)->arg1->arg2;
            newform = FormulaOpAlloc(op, arg1, arg2);
         }
         else
         {
            if((op == OpBAnd) && (quant == OpQAll))
            {
               arg1 = FormulaQuantorAlloc(OpQAll, (*form)->special.var,
                                          (*form)->arg1->arg1);
               arg2 = FormulaQuantorAlloc(OpQAll, (*form)->special.var,
                                          (*form)->arg1->arg2);
               newform = FormulaOpAlloc(OpBAnd, arg1, arg2);
            }
            else if((op == OpBOr) && (quant == OpQEx))
            {
               arg1 = FormulaQuantorAlloc(OpQEx, (*form)->special.var,
                                          (*form)->arg1->arg1);
               arg2 = FormulaQuantorAlloc(OpQEx, (*form)->special.var,
                                          (*form)->arg1->arg2);
               newform = FormulaOpAlloc(OpBOr, arg1, arg2);
            } 
         }
         if(newform)
         {
            modified = true;
            FormulaRelRef(*form);
            FormulaGetRef(newform);
            FormulaFree(*form);
            *form = newform;
         }
      }
   }
   if(FormulaHasSubForm1(*form))
   {
      tmpmod = FormulaMiniScope(&((*form)->arg1));
      modified = modified || tmpmod;
   }
   if(FormulaHasSubForm2(*form))
   {
      tmpmod = FormulaMiniScope(&((*form)->arg2));
      modified = modified || tmpmod;
   }
   if(modified)
   {
      FormulaMiniScope(form);
   }
   return modified;
}



/*-----------------------------------------------------------------------
//
// Function: FormulaVarRename()
//
//   Convert the formula into one where all the bound variables have
//   been replaced by fresh one. Does not free the old formula!
//
//   IMPORTANT PRECONDITION: terms->vars->f_count _must_ point to a
//   variable bigger than all in form.
//
// Global Variables: -
//
// Side Effects    : Consumes fresh variables from the term bank.
//
/----------------------------------------------------------------------*/

Formula_p FormulaVarRename(Formula_p form, bool *modified, TB_p terms)
{
   Term_p old_var = NULL, new_var = NULL;
   Formula_p handle = NULL, arg1=NULL, arg2=NULL;

   if(FormulaIsQuantified(form))
   {
      old_var = form->special.var->binding;
      new_var = VarBankGetFreshVar(terms->vars);
      assert(new_var != form->special.var);
      form->special.var->binding = new_var;
      *modified = true;
   }
   if(FormulaIsLiteral(form))
   {      
      handle = FormulaCopy(form, terms);
   }
   if(FormulaHasSubForm1(form))
   {
      arg1 = FormulaVarRename(form->arg1, modified, terms);
   }
   if(FormulaHasSubForm2(form))
   {
      arg2 = FormulaVarRename(form->arg2, modified, terms);
   }   
   if(FormulaIsQuantified(form))
   {      
      handle = FormulaQuantorAlloc(form->op, new_var, arg1);
      form->special.var->binding = old_var;
   }
   else if(!FormulaIsLiteral(form))
   {
      handle = FormulaOpAlloc(form->op, arg1, arg2);
   }
   return handle;   
}


/*-----------------------------------------------------------------------
//
// Function: FormulaSkolemizeOutermost()
//
//   Destructively Skolemize a formula in an outermost
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

Formula_p FormulaSkolemizeOutermost(Formula_p form, bool* modified, TB_p terms)
{
   Formula_p res = NULL;
   PTree_p   free_vars = NULL;
   PStack_p  var_stack = PStackAlloc();
   Term_p    var;

   FormulaCollectFreeVars(form, &free_vars);
   while(free_vars)
   {
      var = PTreeExtractRootKey(&free_vars);
      PStackPushP(var_stack, var);
   }
   res = formula_rek_skolemize(form, terms, modified, var_stack);
   PStackFree(var_stack);   
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: FormulaShiftQuantors()
//
//   Shift all remaining all-quantors outward.
//
// Global Variables: -
//
// Side Effects    : Destroys original formula.
//
/----------------------------------------------------------------------*/

Formula_p FormulaShiftQuantors(Formula_p form)
{
   Formula_p handle, narg1, narg2, newform;
   Term_p var;

   if(FormulaHasSubForm1(form))
   {
      form->arg1 = FormulaShiftQuantors(form->arg1);
   }
   if(FormulaHasSubForm2(form))
   {
      form->arg2 = FormulaShiftQuantors(form->arg2);
   }   
   switch(form->op)
   {
   case OpBAnd:
   case OpBOr:
         if(form->arg1->op == OpQAll)
         {
            narg1  = form->arg1->arg1;
            narg2  = form->arg2;
            var    = form->arg1->special.var;
            assert(!FormulaVarIsFree(narg2, var));
            handle = FormulaOpAlloc(form->op, narg1, narg2);
            newform = FormulaQuantorAlloc(OpQAll, var, handle);
            FormulaGetRef(newform);
            FormulaRelRef(form);
            FormulaFree(form);
            form = FormulaShiftQuantors(newform);            
         }
         else if(form->arg2->op == OpQAll)
         {
            narg2  = form->arg2->arg1;
            narg1  = form->arg1;
            var    = form->arg2->special.var;
            assert(!FormulaVarIsFree(narg1, var));
            handle = FormulaOpAlloc(form->op, narg1, narg2);
            newform = FormulaQuantorAlloc(OpQAll, var, handle);
            FormulaGetRef(newform);
            FormulaRelRef(form);
            FormulaFree(form);
            form = FormulaShiftQuantors(newform);            
         }       
   case OpQAll:
   case OpIsLit:
         break;
   default:
         assert(false && "Wrong formula operator in FormulaShiftQuantors()");
         break;
   }
   return form;
}


/*-----------------------------------------------------------------------
//
// Function: FormulaDistributeDisjunctions()
//
//   Apply distributivity law to transform a suitably preprocessed
//   formula into conjunctive normal form.
//
// Global Variables: -
//
// Side Effects    : May destroy formula
//
/----------------------------------------------------------------------*/

Formula_p FormulaDistributeDisjunctions(Formula_p form, bool* modified)
{
   Formula_p handle, narg1, narg2;

   if(FormulaHasSubForm1(form))
   {
      form->arg1 = FormulaDistributeDisjunctions(form->arg1, modified);
   }
   if(FormulaHasSubForm2(form))
   {
      form->arg2 = FormulaDistributeDisjunctions(form->arg2, modified);
   }   
   switch(form->op)
   {
   case OpQAll:
   case OpBAnd:
   case OpIsLit:
         break;
   case OpBOr:
         if(form->arg1->op == OpBAnd)
         {            
            narg1 = FormulaOpAlloc(OpBOr, form->arg1->arg1, form->arg2);
            narg2 = FormulaOpAlloc(OpBOr, form->arg1->arg2, form->arg2);
            handle = FormulaOpAlloc(OpBAnd, narg1, narg2);
            FormulaGetRef(handle);
            FormulaRelRef(form);
            FormulaFree(form);
            form = FormulaDistributeDisjunctions(handle, modified);
            *modified = true;
         }
         else if(form->arg2->op == OpBAnd)
         {
            narg2 = FormulaOpAlloc(OpBOr, form->arg2->arg2, form->arg1);
            narg1 = FormulaOpAlloc(OpBOr, form->arg2->arg1, form->arg1);
            handle = FormulaOpAlloc(OpBAnd, narg1, narg2);
            FormulaGetRef(handle);
            FormulaRelRef(form);
            FormulaFree(form);
            form = FormulaDistributeDisjunctions(handle, modified);
            *modified = true;
         }
         break;
   default:
         assert(false && "Wrong formula operator in FormulaDistributeDisjunctions()");
         break;
   }   
   return form;
}



/*-----------------------------------------------------------------------
//
// Function: WFormulaConjunctiveNF()
//
//   Transform a formula into Conjunctive Normal Form.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool WFormulaConjunctiveNF(WFormula_p form, TB_p terms)
{
   bool res, tmp;
   Formula_p handle;
   FunCode   max_var;

   res = FormulaSimplify(&(form->formula), terms);
   if(res)
   {
      DocFormulaModificationDefault(form, inf_fof_simpl);
   }
   tmp = FormulaNNF(&(form->formula), terms,1);
   if(tmp)
   {
      DocFormulaModificationDefault(form, inf_fof_nnf);
   }
   
   res = res || tmp;
   tmp = FormulaMiniScope(&(form->formula));
   if(tmp)
   {
      DocFormulaModificationDefault(form, inf_shift_quantors);
   }
   res = res || tmp;
   tmp = false;   
   max_var = FormulaFindMaxVarCode(form->formula);
   VarBankSetVCount(terms->vars, -max_var);
   handle = FormulaVarRename(form->formula, &tmp, terms);
   FormulaRelRef(form->formula);
   FormulaGetRef(handle);
   FormulaFree(form->formula);
   form->formula = handle;  
   if(tmp)
   {
      DocFormulaModificationDefault(form, inf_var_rename);
   }
   VarBankVarsSetProp(terms->vars, TPIsFreeVar);
   tmp = false;
   form->formula = FormulaSkolemizeOutermost(form->formula, &tmp, terms);
   if(tmp)
   {
      DocFormulaModificationDefault(form, inf_skolemize_out);
   }
   tmp = false;
   form->formula = FormulaShiftQuantors(form->formula);
   if(tmp)
   {
      DocFormulaModificationDefault(form, inf_shift_quantors);
   }   
   tmp = false;
   form->formula = FormulaDistributeDisjunctions(form->formula, &tmp);
   if(tmp)
   {
      DocFormulaModificationDefault(form, inf_fof_distrib);
   }
   return res;
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


