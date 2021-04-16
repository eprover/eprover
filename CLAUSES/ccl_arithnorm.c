/*-----------------------------------------------------------------------

File  : clb_pstacks.h

Author: Florian Knoch and Lukas Naatz

Contents
  rewriting functions for arithmetic functions

  Copyright 2021 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes
<1> Tue Mar 23 09:14:36 MET 2021
    New

-----------------------------------------------------------------------*/

#include "ccl_arithnorm.h"


/*-----------------------------------------------------------------------
//
// Function: FormulaSetArithNorm()
//     Normalizes all Terms in the formulaset and handles documentation.
//
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void FormulaSetArithNorm(FormulaSet_p set, TB_p terms, GCAdmin_p gc) {
   WFormula_p handle, anchor;
   anchor = set->anchor;
   handle = anchor;

   for(handle = anchor->succ; handle != anchor; handle = handle->succ) {
      printf("------\n");
      PrintTermsDebug(handle->tformula, terms, 0);
   }
   printf("======\n");
   while((handle = handle->succ) != anchor)
   {
      handle->tformula = TFormulaArithNormalize(terms, handle->tformula);
      // Derivations maybe push to much, since the formula might not get rewritten
      DocFormulaModificationDefault(handle, inf_minimize);
      WFormulaPushDerivation(handle, DCArithNormalize, NULL, NULL);
   }
   
   while((handle = handle->succ) != anchor)
   {
      ACNormalize(handle->tformula, terms);
      // Derivations maybe push to much, since the formula might not get rewritten
      DocFormulaModificationDefault(handle, inf_minimize);
      WFormulaPushDerivation(handle, DCArithNormalize, NULL, NULL);
   }

   for(handle = anchor->succ; handle != anchor; handle = handle->succ) {
      printf("------\n");
      PrintTermsDebug(handle->tformula, terms, 0);
   }
}

/*-----------------------------------------------------------------------
//
// Function: PrintTermsDebug()
//     Prints the termstructure with f_code.
//
//
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void PrintTermsDebug(TFormula_p form, TB_p terms, int depth)
{
   for(int i = 0; i < depth; i++) printf("\t");
   printf("fcode:%d, properties:%x, type = %d, arity=%d)", form->f_code, form->properties, form->type->f_code, form->arity);
   for ( int i = 0; i < form->type->arity; i++) printf(" arg%d:%d",i,form->args[i]->f_code);
   printf("\n");
   for( int i = 0; i < form->arity; i++) PrintTermsDebug(form->args[i], terms, depth +1);

}

/*-----------------------------------------------------------------------
//
// Function: TFomulaArithNormalize()
//     Rewrites certain arithmetic functions so we dont have to deal with
//     them later.
//
// Global Variables: -
//
// Side Effects    : Memory Operations
//
/----------------------------------------------------------------------*/

TFormula_p TFormulaArithNormalize(TB_p terms, TFormula_p form)
{
   assert(terms);
   assert(form);

   TFormula_p newform=NULL, arg1=NULL, arg2=NULL;
   if(form->arity == 0 || form->arity > 2) {
      return form;
   }

   if(form->arity >= 1)
   {
      arg1 = TFormulaArithNormalize(terms, form->args[0]);
      //form->args[0] = arg1;
   }
   if(form->arity == 2)
   {
      arg2 = TFormulaArithNormalize(terms, form->args[1]);
      //form->args[1] = arg2;

   }
   
   if(form->f_code == terms->sig->eqn_code && 
         (form->args[0]->f_code == terms->sig->lesseq_code || form->args[0]->f_code == terms->sig->greatereq_code) )
      // for $lesseq and $greatereq we need to add a ~ which needs to be outside of the
      // equationblock (the fool-normalization would otherwise remove the $eqn, which breaks the definition search)
      // ~a=b ---> (~a | ~b) & (a | b)
      // ~(less(1,2)) = $true
      // (~less(1,2) | ~($true)) & (less(1,2) | $true)
   {
      TFormula_p tmp1 = TFormulaArithFCodeAlloc(terms, terms->sig->eqn_code, form->type, arg1,arg2); // new eqn
      newform = TFormulaArithFCodeAlloc(terms, terms->sig->not_code, form->type, tmp1, NULL);
   
   } else if(form->f_code == terms->sig->greater_code) {
      // $less(Y,X)
      newform = TFormulaArithFCodeAlloc(terms, terms->sig->less_code,
                                        form->type, arg2, arg1);
   }
   else if(form->f_code == terms->sig->lesseq_code) {
      // ~ $less(Y,X)
      // since we cant add the ~ here, the parent node (always $eqn) will add it
      newform = TFormulaArithFCodeAlloc(terms, terms->sig->less_code,
                                               form->type, arg2, arg1);
   }
   else if(form->f_code == terms->sig->greatereq_code) {
      // ~ $less(X,Y)
      // since we cant add the ~ here, the parent node (always $eqn) will add it
      newform = TFormulaArithFCodeAlloc(terms, terms->sig->less_code,
                                               form->type, arg1, arg2);
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
   assert(newform);
   return newform;
}

/*-----------------------------------------------------------------------
//
// Function: TFormulaArithFCodeAlloc()
//    Pretty similar to TFormulaFCodeAlloc() but for arithmetic
//    functions (type).
//    Creates a new Termcell and inserts into the termbank.
//
//
// Global Variables: -
//
// Side Effects    : Memory Operations
//
/----------------------------------------------------------------------*/

TFormula_p TFormulaArithFCodeAlloc(TB_p bank, FunCode op, Type_p FunType, TFormula_p arg1, TFormula_p arg2)
{
   int arity = SigFindArity(bank->sig, op);
   TFormula_p res;

   assert(bank);
   assert((arity == 1) || (arity == 2));
   assert(EQUIV((arity==2), arg2));

   res = TermTopAlloc(op,arity);

   if(SigIsPredicate(bank->sig, op))
   {
      // I dont think SigIsPredicate does the right job, $sum would be a predicate
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
   
   res->type = FunType;
   
   assert(bank);
   
   res = TBTermTopInsert(bank, res);

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: ACNormalize()
//    Finds AC Subterms ( $sum / $product) and rewrites the termtree so
//    groundterms are on the left.
//
// Global Variables: -
//
// Side Effects    : Memory Operations
//
/----------------------------------------------------------------------*/

bool ACNormalize(TFormula_p acterm, TB_p bank)
{
   assert(acterm);
   assert(bank);
  
   
   bool is_ground = true;
   if(acterm->arity == 0) {
      // check if Element is Variable or constant
      return TermCellQueryProp(acterm, TPIsGround);
   }

   if((acterm->f_code != bank->sig->product_code) &&
      (acterm->f_code != bank->sig->sum_code))
   {
      for(int i = 0; i < acterm->arity; i++) {
         is_ground &= ACNormalize(acterm->args[i], bank);
      }
      return is_ground;
   }
   
   ACStruct_p head = AllocNormalizeStruct();
   collect_ac_leafes(acterm, bank, acterm->f_code, head);
   ACNorm_p children = head->groundterms;

   if(children == NULL) {
      children = head->nongroundterms;
   } else {
      ACCellAppend(children, head->nongroundterms);
   }

   assert(children && children->succ);

   while(children->succ->succ != NULL)
   {
      ACNorm_p arg1 = children;
      ACNorm_p arg2 = children->succ;
      TFormula_p new = TFormulaArithFCodeAlloc(bank, acterm->f_code, acterm->type, arg1->acterm, arg2->acterm);
      
      ACNorm_p newcell = AllocNormalizeCell(new, arg1->isground & arg2->isground);
      newcell->succ = arg2->succ;
      children = newcell;

      free(arg1);
      free(arg2);
   }
   ACNorm_p arg1 = children;
   ACNorm_p arg2 = children->succ;
   acterm->args[0] = arg1->acterm;
   acterm->args[1] = arg2->acterm;
   is_ground = arg1->isground & arg2->isground;

   free(arg1);
   free(arg2);
   free(head);
   assert(acterm->args[0] && acterm->args[1]);
         
   return is_ground;
}

/*-----------------------------------------------------------------------
//
// Function: collect_ac_leafes()
//    collects all nodes, that are the arguments of the current AC Subterm
//
// Global Variables: -
//
// Side Effects    : Memory Operations
//
/----------------------------------------------------------------------*/

void collect_ac_leafes(TFormula_p acterm, TB_p bank, FunCode rootcode, ACStruct_p head) {
   
   if(acterm->f_code != rootcode) {
      bool isground = ACNormalize(acterm, bank);
      ACNorm_p leaf = AllocNormalizeCell(acterm, isground);

      if(isground){
         if(head->groundterms == NULL) {
            head->groundterms = leaf;
         } else {
            leaf->succ = head->groundterms;
            head->groundterms = leaf;
         }
      } else {
         if(head->nongroundterms == NULL) {
            head->nongroundterms = leaf;
         } else {
            leaf->succ = head->nongroundterms;
            head->nongroundterms = leaf;
         }
      }
   } else {
      for(int i = 0; i < acterm->arity; i++) {
         collect_ac_leafes(acterm->args[i], bank, rootcode, head);
      }
   }
}

/*-----------------------------------------------------------------------
//
// Function: ACCellAppend()
//    Appends a listelement at the end of a list
//
// Global Variables: -
//
// Side Effects    : Memory Operations
//
/----------------------------------------------------------------------*/

void ACCellAppend(ACNorm_p head, ACNorm_p tail) {
   ACNorm_p current;
   for(current = head; current->succ != NULL; current = current->succ);
   current->succ = tail;
}

/*-----------------------------------------------------------------------
//
// Function: AllocNormalizeStruct()
//    Creates the headstruct for ground- and nongroundterm lists
//
// Global Variables: -
//
// Side Effects    : Memory Operations
//
/----------------------------------------------------------------------*/

ACStruct_p AllocNormalizeStruct() {
   ACStruct_p newstruct = SizeMalloc(sizeof(ACNormalizeStruct));
   newstruct->groundterms = NULL;
   newstruct->nongroundterms = NULL;
   return newstruct;
}

/*-----------------------------------------------------------------------
//
// Function: AllocNormalizeCell()
//    Creates a Normalisation Cell, which holds a leaf of the termtree
//
// Global Variables: -
//
// Side Effects    : Memory Operations
//
/----------------------------------------------------------------------*/

ACNorm_p AllocNormalizeCell(TFormula_p leaf, bool isground) {
   ACNorm_p newcell = SizeMalloc(sizeof(ACNormalizeCell));
   newcell->acterm = leaf;
   newcell->isground = isground;
   newcell->succ = NULL;

   return newcell;
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
