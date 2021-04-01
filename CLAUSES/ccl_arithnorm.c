/*-----------------------------------------------------------------------

File  : clb_pstacks.h

Author: Florian Knoch and Lukas Naatz

Contents

  Soemwhat efficient unlimited growth stacks for pointers/long ints.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed Dec  3 16:22:48 MET 1997
    New

-----------------------------------------------------------------------*/

#include "ccl_arithnorm.h"

void FormulaSetArithNorm(FormulaSet_p set, TB_p terms, GCAdmin_p gc) {
   WFormula_p handle, anchor, form;
   TFormula_p normalized;
   anchor = set->anchor;
   handle = anchor;
   while((handle = handle->succ) != anchor)
   {
      handle->tformula = TFormulaArithNormalize(terms, handle->tformula);
      // Derivations maybe push to much, since the formula might not get rewritten 
      DocFormulaModificationDefault(handle, inf_minimize);
      WFormulaPushDerivation(handle, DCArithNormalize, NULL, NULL);
   }
   for(handle = anchor->succ; handle != anchor; handle = handle->succ) {
      printf("------\n");
      PrintTermsDebug(handle->tformula, terms, 0);
   }
}

void PrintTermsDebug(TFormula_p form, TB_p terms, int depth)
{
   for(int i = 0; i < depth; i++) printf("\t");
   printf("fcode:%d, properties:%x\n", form->f_code, form->properties);
   for( int i = 0; i < form->arity; i++) PrintTermsDebug(form->args[i], terms, depth +1);

}

TFormula_p TFormulaArithNormalize(TB_p terms, TFormula_p form)
{
   TFormula_p newform=NULL, arg1=NULL, arg2=NULL;
   
   if(form->arity == 0) {
      
      return form;
   }
   if(form->arity >= 1)
   {
      arg1 = TFormulaArithNormalize(terms, form->args[0]);
   }
   if(form->arity == 2)
   {
      arg2 = TFormulaArithNormalize(terms, form->args[1]);

   }
	if(form->f_code == 3) // eqn
   {
      TFormula_p tmp1,tmp2;
      tmp1 = TFormulaArithFCodeAlloc(terms, terms->sig->eqn_code, form->type, arg1->args[0],arg2); // new eqn
      tmp2 = TFormulaArithFCodeAlloc(terms, terms->sig->not_code, form->type, tmp1, NULL);
      return tmp2;
   }

   if(form->f_code == terms->sig->greater_code) { 
      // $less(Y,X)
      newform = TFormulaArithFCodeAlloc(terms, terms->sig->less_code,
                                        form->type, arg2, arg1);
   }
   else if(form->f_code == terms->sig->lesseq_code) { 
      // ~ $less(Y,X)
      TFormula_p tmp = TFormulaArithFCodeAlloc(terms, terms->sig->less_code,
                                               form->type, arg2, arg1); 
      newform = TFormulaArithFCodeAlloc(terms, terms->sig->not_code,
                                        form->type, tmp, NULL);
   }
   else if(form->f_code == terms->sig->greatereq_code) { 
      // ~ $less(X,Y)
      TFormula_p tmp = TFormulaArithFCodeAlloc(terms, terms->sig->less_code,
                                               form->type, arg1, arg2);
      newform = TFormulaArithFCodeAlloc(terms, terms->sig->not_code,
                                        form->type, tmp, NULL);
   }
   else if(form->f_code == terms->sig->difference_code) { 
      // $sum(X, $uminus(Y))
      TFormula_p tmp = TFormulaArithFCodeAlloc(terms, terms->sig->uminus_code,
                                               form->type, arg2, NULL);
      newform = TFormulaArithFCodeAlloc(terms, terms->sig->sum_code,
                                        form->type, arg1, tmp);
   }
   else if(form->f_code == terms->sig->quotient_t_code) { 
      // $truncate($quotient(X,Y))
      TFormula_p tmp = TFormulaArithFCodeAlloc(terms, terms->sig->quotient_code,
                                               form->type, arg1, arg2);
      newform = TFormulaArithFCodeAlloc(terms, terms->sig->truncate_code,
                                        form->type, tmp, NULL);
   }
   else if(form->f_code == terms->sig->quotient_f_code ) { 
      // $floor($quotient(X,Y))
      TFormula_p tmp = TFormulaArithFCodeAlloc(terms, terms->sig->quotient_code,
                                               form->type, arg1, arg2);
      newform = TFormulaArithFCodeAlloc(terms, terms->sig->floor_code,
                                        form->type, tmp, NULL);
   }
   else if(form->f_code == terms->sig->remainder_e_code) { 
      // $product(X, Y) - product(quotient_e(X, Y), Y)
      TFormula_p tmp1 = TFormulaArithFCodeAlloc(terms, terms->sig->product_code,
                                                form->type, arg1, arg2);
      TFormula_p tmp2 = TFormulaArithFCodeAlloc(terms, terms->sig->quotient_e_code,
                                                form->type, arg1, arg2);
      TFormula_p tmp3 = TFormulaArithFCodeAlloc(terms, terms->sig->product_code,
                                                form->type, tmp2, arg2);
      TFormula_p tmp4 = TFormulaArithFCodeAlloc(terms, terms->sig->uminus_code,
                                                form->type, tmp3, NULL);
      newform = TFormulaArithFCodeAlloc(terms, terms->sig->sum_code,
                                        form->type, tmp1, tmp4);
   }
   else if(form->f_code == terms->sig->remainder_t_code) { 
      // $product(X, Y) - product($truncate($quotient(X, Y)), Y)
      TFormula_p tmp1 = TFormulaArithFCodeAlloc(terms, terms->sig->product_code,
                                                form->type, arg1, arg2);
      TFormula_p tmp2 = TFormulaArithFCodeAlloc(terms, terms->sig->quotient_code,
                                                form->type, arg1, arg2);
      TFormula_p tmp3 = TFormulaArithFCodeAlloc(terms, terms->sig->truncate_code,
                                                form->type, tmp2, NULL);
      TFormula_p tmp4 = TFormulaArithFCodeAlloc(terms, terms->sig->product_code,
                                                form->type, tmp3, arg2);
      TFormula_p tmp5 = TFormulaArithFCodeAlloc(terms, terms->sig->uminus_code,
                                                form->type, tmp4, NULL);
      newform = TFormulaArithFCodeAlloc(terms, terms->sig->sum_code,
                                        form->type, tmp1, tmp5);
   }
   else if(form->f_code == terms->sig->remainder_f_code) { 
      // $product(X, Y) - product($floor($quotient(X, Y)), Y)
      TFormula_p tmp1 = TFormulaArithFCodeAlloc(terms, terms->sig->product_code,
                                                form->type, arg1, arg2);
      TFormula_p tmp2 = TFormulaArithFCodeAlloc(terms, terms->sig->quotient_code,
                                                form->type, arg1, arg2);
      TFormula_p tmp3 = TFormulaArithFCodeAlloc(terms, terms->sig->floor_code,
                                                form->type, tmp2, NULL);
      TFormula_p tmp4 = TFormulaArithFCodeAlloc(terms, terms->sig->product_code,
                                                form->type, tmp3, arg2);
      TFormula_p tmp5 = TFormulaArithFCodeAlloc(terms, terms->sig->uminus_code,
                                                form->type, tmp4, NULL);
      newform = TFormulaArithFCodeAlloc(terms, terms->sig->sum_code,
                                        form->type, tmp1, tmp5);
   }
   else if(form->f_code == terms->sig->ceiling_code) { 
      // $uminus($floor($uminus(X)))
      TFormula_p tmp1 = TFormulaArithFCodeAlloc(terms, terms->sig->uminus_code,
                                                form->type, arg1, NULL);
      TFormula_p tmp2 = TFormulaArithFCodeAlloc(terms, terms->sig->floor_code,
                                                form->type, tmp1, NULL);
      newform = TFormulaArithFCodeAlloc(terms, terms->sig->uminus_code,
                                        form->type, tmp2, NULL);
   }
   else {
      newform = TFormulaArithFCodeAlloc(terms, form->f_code, form->type, arg1, arg2);
   }

   return newform;
}

TFormula_p TFormulaArithFCodeAlloc(TB_p bank, FunCode op, Type_p FunType, TFormula_p arg1, TFormula_p arg2)
{   
   int arity = SigFindArity(bank->sig, op);
   TFormula_p res;

   assert(bank);
   assert((arity == 1) || (arity == 2));
   assert(EQUIV((arity==2), arg2));

   res = TermTopAlloc(op,arity);
   
   // vvv is this ok?  
   res->type = FunType;
   
   if(SigIsPredicate(bank->sig, op))
   {
      TermCellSetProp(res, TPPredPos);
   }
   if(arity > 0)
   {
      res->args[0] = arg1;
      if(arity > 1)
      {
         res->args[1] = arg2;
      }
   }
   assert(bank);
   res = TBTermTopInsert(bank, res);

   return res;
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
