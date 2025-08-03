/*-----------------------------------------------------------------------

File  : cco_interpreted.c

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Code for dealing with interpreted symbols. At the moment, this
  mostly consists of preliminary ad-hoc implementations. Once enough
  substance has accumulated, this may (or may not, given how reality
  deals with running code) be refactored into something nice and
  general.

  Copyright 2011 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Mon Jun  6 11:53:02 CEST 2011
    New

-----------------------------------------------------------------------*/

#include "cco_interpreted.h"



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
// Function: answer_lit_print()
//
//   Print the answer in an answer literal nicely. At the moment, we
//   print proper answer literals as tuples of answers, all others
//   as-is.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void answer_lit_print(FILE* out, Eqn_p lit)
{
   Term_p answer_term;
   int i;
   char *sep="";

   fprintf(out, "[");
   if(EqnQueryProp(lit, EPPseudoLit)
      &&
      SigIsSimpleAnswerPred(lit->bank->sig, lit->lterm->f_code)
      &&
      lit->lterm->args[0]->f_code > 0)
   {
      /* It's a "proper" answer literal */
      answer_term = lit->lterm->args[0];
      for(i=0; i<answer_term->arity; i++)
      {
         fprintf(out, "%s", sep);
         TBPrintTermFull(out, lit->bank, answer_term->args[i]);
         sep = ", ";
      }
   }
   else
   {
      /* It's something weird -> Just print it */
      EqnTSTPPrint(out, lit, true);
   }
   fprintf(out, "]");
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/



/*-----------------------------------------------------------------------
//
// Function: ClausePrintAnswer()
//
//   If the clause has only answer literals, print the answer.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void ClausePrintAnswer(FILE* out, Clause_p clause, ProofState_p state)
{
   Eqn_p handle;

   if(ClauseIsSemFalse(clause)&&!ClauseIsEmpty(clause))
   {
      if(!state->status_reported)
      {
         TSTPOUT(GlobalOut, "Theorem");
         state->status_reported = true;
      }
      fprintf(out, COMCHAR" SZS answers Tuple [");
      fprintf(out, "%s", ClauseLiteralNumber(clause)>1?"(":"");
      handle = clause->literals;
      if(handle)
      {
         answer_lit_print(out, handle);
         handle = handle->next;
         while(handle)
         {
            fprintf(out, "|");
            answer_lit_print(out, handle);
            handle = handle->next;
         }
      }
      fprintf(out, "%s", ClauseLiteralNumber(clause)>1?")":"");
      fprintf(out, "|_]\n");
   }
}


/*-----------------------------------------------------------------------
//
// Function: ClauseEvaluateAnswerLits()
//
//   "Evaluate" the answer literals, i.e. remove all single-answer
//   lits if the clause is otherwise empty. Return number of removed
//   literals.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

int ClauseEvaluateAnswerLits(Clause_p clause)
{
   Eqn_p handle;
   int res = 0;

   assert(!ClauseIsAnyPropSet(clause, CPIsDIndexed|CPIsSIndexed));

   if(ClauseIsSemFalse(clause))
   {
      res = EqnListRemoveSimpleAnswers(&(clause->literals));

      if(res)
      {
         clause->neg_lit_no = 0;
         clause->pos_lit_no = 0;
         handle = clause->literals;

         while(handle)
         {
            if(EqnIsPositive(handle))
            {
               clause->pos_lit_no++;
            }
            else
            {
               clause->neg_lit_no++;
            }
            handle = handle->next;
         }

         if(clause->set)
         {
            clause->set->literals-=res;
         }
         DocClauseModificationDefault(clause, inf_eval_answers, NULL);
         ClausePushDerivation(clause, DCEvalAnswers, NULL, NULL);
      }
   }
   return res;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
