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


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: FormulaSimplify()
//
//   Maximally (and destructively) simplify a formula using (primarily)
//   the simplification rules (from [NW:SmallCNF-2001]). Note that
//   polarity is a dummy parameter neither used nor correctly
//   computed. It is ony there to get a consistent interface for all
//   the NNF functions.
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

Formula_p FormulaSimplify(Formula_p form, TB_p terms, 
                          int polarity /* dummy!*/)
{
   Formula_p handle;

   /* printf("Simplifying: ");
   FormulaTPTPPrint(stdout, form, true);
   printf("\n");*/

   if(FormulaHasSubForm1(form))
   {
      FormulaRefSimplify(form->arg1, terms, polarity);
   }
   if(FormulaHasSubForm2(form))
   {
      FormulaRefSimplify(form->arg2, terms, polarity);
   }
   switch(form->op)
   {
   case OpUNot:
         if(FormulaIsLiteral(form->arg1))
         {
            Eqn_p tmp;
            tmp = EqnAlloc(form->arg1->special.literal->lterm, 
                           form->arg1->special.literal->rterm, 
                           terms,
                           !EqnIsPositive(form->arg1->special.literal));
            form = FormulaLitAlloc(tmp);
         }
         break;
   case OpBOr:
         if((handle = prop_arg_return_other(form->arg1, form->arg2, false)))
         {
            form = handle;
         }
         else if((handle = prop_arg_return(form->arg1, form->arg2, true)))
         {
            form = handle;
         }
         else if(FormulaEqual(form->arg1,form->arg2))
         {
            form = form->arg1;
         }
         break;
   case OpBAnd:
         if((handle = prop_arg_return_other(form->arg1, form->arg2, true)))
         {
            form = handle;
         }
         else if((handle = prop_arg_return(form->arg1, form->arg2, false)))
         {
            form = handle;
         }
         else if(FormulaEqual(form->arg1,form->arg2))
         {
            form = form->arg1;
         }
         break;         
   case OpBEquiv:
         if((handle = prop_arg_return_other(form->arg1, form->arg2, true)))
         {
            form = handle;
         }
         else if((handle = prop_arg_return_other(form->arg1, form->arg2, false)))
         {
            form = FormulaOpAlloc(OpUNot, handle, NULL);
            handle = FormulaGetRef(form);
            FormulaRefSimplify(handle, terms, polarity);
            form = FormulaRelRef(handle);            
         }
         else if(FormulaEqual(form->arg1,form->arg2))
         {
            form = FormulaPropConstantAlloc(terms, true);
         }
         break;
   case OpBImpl: 
         if(FormulaIsPropTrue(form->arg1))
         {
            form = form->arg2;
         }
         else if(FormulaIsPropFalse(form->arg1))
         {
            form = FormulaPropConstantAlloc(terms, true);
         }
         else if(FormulaIsPropFalse(form->arg2))
         {
            form = FormulaOpAlloc(OpUNot, form->arg1, NULL);
            handle = FormulaGetRef(form);
            FormulaRefSimplify(handle, terms, polarity);
            form = FormulaRelRef(handle); 
         }
         else if(FormulaIsPropTrue(form->arg2))
         {
            form = FormulaPropConstantAlloc(terms, true);
         }
         else if(FormulaEqual(form->arg1,form->arg2))
         {
            form = FormulaPropConstantAlloc(terms, true);
         }
         break;
   case OpBXor:
         form = FormulaOpAlloc(OpBEquiv, form->arg1, form->arg2);
         form = FormulaOpAlloc(OpUNot, form, NULL);
         handle = FormulaGetRef(form);
         FormulaRefSimplify(handle, terms, polarity);
         form = FormulaRelRef(handle);                    
         break;
   case OpBNor:
         form = FormulaOpAlloc(OpBOr, form->arg1, form->arg2);
         form = FormulaOpAlloc(OpUNot, form, NULL);
         handle = FormulaGetRef(form);
         FormulaRefSimplify(handle, terms, polarity);
         form = FormulaRelRef(handle);                    
         break;
   case OpBNand:
         form = FormulaOpAlloc(OpBAnd, form->arg1, form->arg2);
         form = FormulaOpAlloc(OpUNot, form, NULL);
         handle = FormulaGetRef(form);
         FormulaRefSimplify(handle, terms, polarity);
         form = FormulaRelRef(handle);                    
         break;         
   case OpBNImpl:
         form = FormulaOpAlloc(OpBImpl, form->arg2, form->arg1);
         handle = FormulaGetRef(form);
         FormulaRefSimplify(handle, terms, polarity);
         form = FormulaRelRef(handle);                    
         break;
   case OpQEx:
   case OpQAll:
         if(!FormulaVarIsFree(form->arg1, form->special.var))
         {
            form = form->arg1;
         }
         break;
   default:
         break;
   }
   /* printf("...done: ");
   FormulaTPTPPrint(stdout, form, true);
   printf("\n");*/
   return form;
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
// Function: FormulaCNF()
//
//   Transform a formula into CNF.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Formula_p FormulaCNF(Formula_p form, TB_p terms, int polarity)
{
   printf("FormulaCNF()...\n");
   printf("CNFing: ");
   FormulaTPTPPrint(stdout, form, true);
   printf("\n");
   FormulaRefSimplify(form, terms,1);
   printf("...FormulaCNF()...\n");
   FormulaNNF(&form, terms,1);
   printf("...FormulaCNF()\n");
   return form;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


