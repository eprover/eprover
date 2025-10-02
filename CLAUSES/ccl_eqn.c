/*-----------------------------------------------------------------------

  File  : ccl_eqn.c

  Author: Stephan Schulz

  Contents

  The termpair datatype: Rules, Equations, positive and negative
  literals.

  Copyright 1998-2017 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Fri Mar 13 17:23:31 MET 1998

  -----------------------------------------------------------------------*/

#include "ccl_eqn.h"
#include "cte_typecheck.h"
#include "ccl_tformulae.h"

/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

bool EqnUseInfix = true;
bool EqnFullEquationalRep = false;
IOFormat OutputFormat =LOPFormat;


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: compare_pos_eqns()
//
//   Compare two positive equations l1=r1 and l2=r2:
//
//     (1) {l1,r1} == {l2,r2}  <==>  (l1=l2 & r1=r2) v (l1=r2 & r1=l2)
//
//     Assume that {l1,r1} =/= {l2,r2}. Then,
//
//     (2) {l1,r1} >> {l2,r2}  <==>  (l1>l2 & l1>r2) v
//                                   (l1>=l2 & r1>=r2) v
//                                   (r1>=l2 & l1>=r2) v
//                                   (r1>l2 & r1>r2)
//
//     (3) {l1,r1} << {l2,r2}  <==>  (l1<l2 & r1<l2) v
//                                   (l1<=l2 & r1<=r2) v
//                                   (r1<=l2 & l1<=r2) v
//                                   (l1<r2 & r1<r2)
//
//     (4) Otherwise, {l1,r1} and {l2,r2} are incomparable.
//
//   Assume that l1>r1 holds. Then (2) and (3) of the above comparison
//   can be simplified in the following way:
//
//     (5) {l1,r1} >> {l2,r2}  <==>  (l1>l2 & l1>r2) v
//                                   (l1>=l2 & r1>=r2)
//
//     (6) {l1,r1} << {l2,r2}  <==>  l1<l2 v
//                                   (l1<=l2 & r1<=r2) v
//                                   (r1<=l2 & l1<=r2) v
//                                   l1<r2
//
//   Assume that l2>r2 holds. Then (2) and (3) of the above comparison
//   can be simplified in the following way:
//
//     (7) {l1,r1} >> {l2,r2}  <==>  l1>l2 v
//                                   (l1>=l2 & r1>=r2) v
//                                   (r1>=l2 & l1>=r2) v
//                                   r1>l2
//
//     (8) {l1,r1} << {l2,r2}  <==>  (l1<l2 & r1<l2) v
//                                   (r1<=l2 & l1<=r2)
//
//   Assume that l1>r1 and l2>r2 hold. Then (1), (2) and (3) of the
//   above comparison can be simplified in the following way:
//
//     (9)  {l1,r1} == {l2,r2}  <==>  l1=l2 & r1=r2
//
//     Assume that {l1,r1} =/= {l2,r2}. Then,
//
//     (10) {l1,r1} >> {l2,r2}  <==>  l1>l2 v (l1=l2 & r1>r2)
//     (11) {l1,r1} << {l2,r2}  <==>  l1<l2 v (l1=l2 & r1<r2)
//
//   The simplified versions (5)-(11) are not yet implemented!
//
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static CompareResult compare_pos_eqns(OCB_p ocb, Eqn_p eq1, Eqn_p eq2)
{
   assert(PropsAreEquiv(eq1, eq2, EPIsPositive));

   CompareResult l1l2 = TOCompare(ocb, eq1->lterm, eq2->lterm, DEREF_ALWAYS, DEREF_ALWAYS);
   CompareResult r1r2 = TOCompare(ocb, eq1->rterm, eq2->rterm, DEREF_ALWAYS, DEREF_ALWAYS);

   if((l1l2 == to_equal) && (r1r2 == to_equal))
   {
      return to_equal; /* Case (1) */
   }

   if(    ((l1l2 == to_greater) || (l1l2 == to_equal))
          && ((r1r2 == to_greater) || (r1r2 == to_equal)))
   {
      return to_greater; /* Case (2) */
   }

   if(    ((l1l2 == to_lesser) || (l1l2 == to_equal))
          && ((r1r2 == to_lesser) || (r1r2 == to_equal)))
   {
      return to_lesser; /* Case (3) */
   }


   CompareResult l1r2 = TOCompare(ocb, eq1->lterm, eq2->rterm, DEREF_ALWAYS, DEREF_ALWAYS);

   if((l1l2 == to_greater) && (l1r2 == to_greater))
   {
      return to_greater; /* Case (2) */
   }

   if((l1r2 == to_lesser) && (r1r2 == to_lesser))
   {
      return to_lesser; /* Case (3) */
   }


   CompareResult r1l2 = TOCompare(ocb, eq1->rterm, eq2->lterm, DEREF_ALWAYS, DEREF_ALWAYS);

   if((l1r2 == to_equal) && (r1l2 == to_equal))
   {
      return to_equal; /* Case (1) */
   }

   if(    ((r1l2 == to_greater) || (r1l2 == to_equal))
          && ((l1r2 == to_greater) || (l1r2 == to_equal)))
   {
      return to_greater; /* Case (2) */
   }


   if((r1l2 == to_greater) && (r1r2 == to_greater))
   {
      return to_greater; /* Case (2) */
   }

   if((l1l2 == to_lesser) && (r1l2 == to_lesser))
   {
      return to_lesser; /* Case (3) */
   }

   if(    ((r1l2 == to_lesser) || (r1l2 == to_equal))
          && ((l1r2 == to_lesser) || (l1r2 == to_equal)))
   {
      return to_lesser; /* Case (3) */
   }

   return to_uncomparable;
}


/*-----------------------------------------------------------------------
//
// Function: compare_poseqn_negeqn()
//
//   Compare a positive equations l1=r1 and a negative equation
//   l2=/=r2:
//
//   (1)  {{l1},{r1}} == {{l2,r2}}:   This case is impossible!
//
//   (2)  {{l1},{r1}} >> {{l2,r2}}  <==>  (l1>l2 & l1>r2) v
//                                                     (r1>l2 & r1>r2)
//   (3)  {{l1},{r1}} << {{l2,r2}}  <==>  (l1<=l2 v l1<=r2) &
//                                                   (r1<=l2 v r1<=r2)
//
//   (4)  Otherwise, {{l1},{r1}} and {{l2,r2}} are incomparable.
//
//   Assume that l1>r1 holds. Then the above comparison can be
//   simplified in the following way:
//
//   (5)  {{l1},{r1}} >> {{l2,r2}}  <==>  l1>l2 & l1>r2
//   (6)  {{l1},{r1}} << {{l2,r2}}  <==>  l1<=l2 v l1<=r2
//
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static CompareResult compare_poseqn_negeqn(OCB_p ocb, Eqn_p eq1, Eqn_p eq2)
{
   assert(EqnIsPositive(eq1));
   assert(!EqnIsPositive(eq2));

   CompareResult l1l2 = TOCompare(ocb, eq1->lterm, eq2->lterm, DEREF_ALWAYS, DEREF_ALWAYS);

   if(EqnIsOriented(eq1))
   {
      if((l1l2 == to_lesser) || (l1l2 == to_equal))
      {
         return to_lesser; /* Case (6) */
      }

      CompareResult l1r2 = TOCompare(ocb, eq1->lterm, eq2->rterm, DEREF_ALWAYS, DEREF_ALWAYS);

      if((l1r2 == to_lesser) || (l1r2 == to_equal))
      {
         return to_lesser; /* Case (6) */
      }

      if((l1l2 == to_greater) && (l1r2 == to_greater))
      {
         return to_greater; /* Case (5) */
      }
   }
   else
   {
      assert(!EqnIsOriented(eq1));

      CompareResult l1r2 = TOCompare(ocb, eq1->lterm, eq2->rterm, DEREF_ALWAYS, DEREF_ALWAYS);

      if((l1l2 == to_greater) && (l1r2 == to_greater))
      {
         return to_greater; /* Case (2) */
      }

      CompareResult r1l2 = TOCompare(ocb, eq1->rterm, eq2->lterm, DEREF_ALWAYS, DEREF_ALWAYS);
      CompareResult r1r2 = TOCompare(ocb, eq1->rterm, eq2->rterm, DEREF_ALWAYS, DEREF_ALWAYS);

      if((r1l2 == to_greater) && (r1r2 == to_greater))
      {
         return to_greater; /* Case (2) */
      }

      if(    ((l1l2 == to_lesser) || (l1l2 == to_equal) || (l1r2 == to_lesser) || (l1r2 == to_equal))
             && ((r1l2 == to_lesser) || (r1l2 == to_equal) || (r1r2 == to_lesser) || (r1r2 == to_equal)))
      {
         return to_lesser; /* Case (3) Buggy, changed by StS */
      }
   }

   return to_uncomparable;
   /* Note that the `equal'-case is impossible */
}


/*-----------------------------------------------------------------------
//
// MACRO:  BOOL_TERM_NORMALIZE()
//
//   Internal, local maxro to simplify handling of $false.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

#define BOOL_TERM_NORMALIZE(t)                  \
   if(t == bank->false_term)                    \
   {                                            \
      t =  bank->true_term;                     \
      positive = !positive;                     \
   }                                            \


/*-----------------------------------------------------------------------
//
// Function: eqn_parse_prefix()
//
//   Parse a literal without external sign assuming that _all_
//   equational literals are prefix. Return sign. This is for TPTP
//   format and old-style LOP.
//
// Global Variables: -
//
// Side Effects    : Input, memory management.
//
/----------------------------------------------------------------------*/

static bool eqn_parse_prefix(Scanner_p in, TB_p bank, Term_p *lref,
                             Term_p *rref)
{
   Term_p  lterm;
   Term_p  rterm;
   bool    positive = true;



   if(TestInpId(in, EQUAL_PREDICATE))
   {
      NextToken(in);
      AcceptInpTok(in, OpenBracket);
      lterm = TBTermParse(in, bank);
      BOOL_TERM_NORMALIZE(lterm);
      AcceptInpTok(in, Comma);
      rterm = TBTermParse(in, bank);
      BOOL_TERM_NORMALIZE(rterm);
      AcceptInpTok(in, CloseBracket);
   }
   else
   {
      lterm = TBTermParse(in, bank);
      TypeDeclareIsPredicate(bank->sig, lterm);
      BOOL_TERM_NORMALIZE(lterm);
      rterm = bank->true_term; /* Non-Equational literal */
   }
   if(rterm == bank->true_term)
   {
      if(TermIsFreeVar(lterm))
      {
         AktTokenError(in, "Individual variable "
                     "used at predicate position", false);

      }
      assert(!(TermIsDBVar(lterm)));
      SigDeclareIsPredicate(bank->sig, lterm->f_code);
   }
   *lref = lterm;
   *rref = rterm;

   return positive;
}


/*-----------------------------------------------------------------------
//
// Function: eqn_parse_mixfix()
//
//   Parse a literal without external sign, allowing both infix and
//   prefix notations (this is for mixed LOP).
//
// Global Variables: -
//
// Side Effects    : Input, memory management.
//
/----------------------------------------------------------------------*/

static bool eqn_parse_mixfix(Scanner_p in, TB_p bank, Term_p *lref,
                             Term_p *rref)
{
   if(TestInpId(in, EQUAL_PREDICATE))
   {
      return eqn_parse_prefix(in, bank, lref, rref);
   }
   return EqnParseInfix(in, bank, lref, rref);
}

/*-----------------------------------------------------------------------
//
// Function: eqn_parse_real()
//
//   Parse an equation with optional external sign and depending on
//   wether FOF or CNF is being parsed.
//
// Global Variables: -
//
// Side Effects    : Input, memory management.
//
/----------------------------------------------------------------------*/


bool eqn_parse_real(Scanner_p in, TB_p bank, Term_p *lref,
                    Term_p *rref, bool fof)
{
   bool    positive = true;
   bool    negate = false;

   switch(ScannerGetFormat(in))
   {
   case LOPFormat:
         if(TestInpTok(in, TildeSign))
         {
            negate = true;
            NextToken(in);
         }
         positive = eqn_parse_mixfix(in, bank, lref, rref);
         break;
   case TPTPFormat:
         if(fof)
         {
            if(TestInpTok(in, TildeSign))
            {
               negate = true;
               NextToken(in);
            }
         }
         else
         {
            CheckInpTok(in, Plus|Hyphen);
            if(TestInpTok(in, Hyphen))
            {
               negate = true;
               NextToken(in);
               AcceptInpTokNoSkip(in, Hyphen);
            }
            else
            {
               NextToken(in);
               AcceptInpTokNoSkip(in, Plus);
            }
         }
         positive = eqn_parse_prefix(in, bank,  lref, rref);
         break;
   case TSTPFormat:
         if(TestInpTok(in, TildeSign))
         {
            negate = true;
            NextToken(in);
         }
         positive = EqnParseInfix(in, bank,  lref, rref);
         break;
   default:
         assert(false && "Format not supported");
   }
   if(negate)
   {
      positive = !positive;
   }
   return positive;
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: EqnParseInfix()
//
//   Parse a literal without external sign assuming that _all_
//   equational literals are infix. Return sign. This is for TSTP
//   syntax and E-LOP style.
//
// Global Variables: -
//
// Side Effects    : Input, memory management.
//
/----------------------------------------------------------------------*/

bool EqnParseInfix(Scanner_p in, TB_p bank, Term_p *lref, Term_p *rref)
{
   Term_p  lterm;
   Term_p  rterm;
   bool    positive = true;

   bool in_parens = false;
   if(problemType == PROBLEM_HO && TestInpTok(in, OpenBracket))
   {
      AcceptInpTok(in, OpenBracket);
      in_parens = true;
   }

   lterm = TBTermParse(in, bank);

   BOOL_TERM_NORMALIZE(lterm);

   /* Shortcut not to check for equality --
         !TermIsFreeVar guards calls against negative f_code */
   if(problemType == PROBLEM_FO && !TermIsFreeVar(lterm) &&
      SigIsPredicate(bank->sig, lterm->f_code) &&
      SigIsFixedType(bank->sig, lterm->f_code))
   {
      rterm = bank->true_term; /* Non-Equational literal */
   }
   else
   {
      /* If we have a predicate variable then = might not come */
      if((TermIsAnyVar(lterm) && !TypeIsPredicate(lterm->type))
              /* guarding SigIsFunction */
          || (!TermIsAnyVar(lterm) && SigIsFunction(bank->sig, lterm->f_code)))
      {
         if(in_parens && TestInpTok(in, CloseBracket))
         {
            AcceptInpTok(in, CloseBracket);
            in_parens = false;
         }

         if(!TestInpTok(in, NegEqualSign|EqualSign))
         {
#ifdef ENABLE_LFHO
            // type is known but it is inside $let
            // or $ite and is not in an equation
            // (StS):I would think this is an actual parse
            // error...Petar set this to NULL, which crashes on some
            // garbage input in LOP format.
            rterm = bank->true_term;
#else
            CheckInpTok(in, NegEqualSign|EqualSign);
            rterm = NULL; //Stiffle warning
#endif

         }
         else
         {
            if(TestInpTok(in, NegEqualSign))
            {
               positive = !positive;
            }
            AcceptInpTok(in, NegEqualSign|EqualSign);

            rterm = TBTermParse(in, bank);

            if(!TermIsTopLevelAnyVar(rterm))
            {
               TypeDeclareIsNotPredicate(bank->sig, rterm, in);
            }
         }
      }
      else if(TestInpTok(in, NegEqualSign|EqualSign) && !TypeIsPredicate(lterm->type))
      { /* Now both sides must be terms */
         if(in_parens && TestInpTok(in, CloseBracket))
         {
            AcceptInpTok(in, CloseBracket);
            in_parens = false;
         }

         if(lterm->f_code > bank->sig->internal_symbols && problemType == PROBLEM_FO)
         {
            TypeDeclareIsNotPredicate(bank->sig, lterm, in);
         }
         if(TestInpTok(in, NegEqualSign))
         {
            positive = !positive;
         }
         AcceptInpTok(in, NegEqualSign|EqualSign);

         rterm = TBTermParse(in, bank);

         // We have to make those declarations only for FO problems
         if(lterm->f_code > bank->sig->internal_symbols && problemType == PROBLEM_FO)
         {
            TypeDeclareIsNotPredicate(bank->sig, lterm, in);
         }
         if(rterm->f_code > bank->sig->internal_symbols && problemType == PROBLEM_FO)
         {
            TypeDeclareIsNotPredicate(bank->sig, rterm, in);
         }
      }
      else
      {  /* It's a predicate */
         if(problemType == PROBLEM_HO
            && !TermIsAnyVar(lterm)
            && !TermIsPhonyApp(lterm)
            && SigIsFunction(bank->sig, lterm->f_code))
         {
            DStr_p err = DStrAlloc();
            DStrAppendStr(err, "Symbol ");
            DStrAppendStr(err, SigFindName(bank->sig, lterm->f_code));
            DStrAppendStr(err, " interpreted both as function and predicate (check parentheses).");
            AktTokenError(in, DStrView(err), false);
         }
         if(TypeIsBool(lterm->type) ||
            (!TermIsFreeVar(lterm) &&
             lterm->f_code > bank->sig->internal_symbols &&
             !SigIsFixedType(bank->sig, lterm->f_code))) // can be special symbol like $ite and $let
         {
            rterm = bank->true_term; /* Non-Equational literal */
            if(lterm->f_code > bank->sig->internal_symbols)
            {
               TypeDeclareIsPredicate(bank->sig, lterm);
            }
         }
         else
         {
            rterm = NULL;
         }
      }
   }
   *lref = lterm;
   *rref = rterm;

   if(in_parens)
   {
      AcceptInpTok(in, CloseBracket);
   }
   return positive;
}


/*-----------------------------------------------------------------------
//
// Function: EqnAlloc()
//
//   Allocate a literal with the given (shared terms). References for
//   the terms will be added to the bank.
//
// Global Variables: -
//
// Side Effects    : Adds references
//
/----------------------------------------------------------------------*/

Eqn_p EqnAlloc(Term_p lterm, Term_p rterm, TB_p bank,  bool positive)
{
   Eqn_p handle = EqnCellAlloc();
   if(lterm == bank->false_term)
   {
      lterm = bank->true_term;
      positive = !positive;
   }
   if(rterm == bank->false_term)
   {
      rterm = bank->true_term;
      positive = !positive;
   }
   if(lterm == bank->true_term)
   {
      SWAP(lterm, rterm);
   }

   /* printf("Handle = %p\n", handle); */
   handle->properties = EPNoProps;
   if(positive)
   {
      EqnSetProp(handle, EPIsPositive);
   }
   if(rterm != bank->true_term)
   {
      assert(rterm->f_code!=SIG_TRUE_CODE);
      EqnSetProp(handle, EPIsEquLiteral);
   }
   else
   {
      assert(TermCellQueryProp(rterm,TPPredPos));
      /*printf(COMCHAR" lterm->f_code: %ld <%s>\n", lterm->f_code,
               SigFindName(bank->sig,lterm->f_code));
               SigPrint(stdout,bank->sig);
               fflush(stdout); */
#ifndef ENABLE_LFHO
      //assert(!TermIsFreeVar(lterm));
#endif
      if(lterm->f_code > bank->sig->internal_symbols)
      {
         assert(!TermIsDBVar(lterm));
         SigDeclareIsPredicate(bank->sig, lterm->f_code);
      }

      TermCellSetProp(lterm, TPPredPos);
      if(!TermIsAnyVar(lterm) && SigQueryFuncProp(bank->sig, lterm->f_code, FPPseudoPred))
      {
         EqnSetProp(handle, EPPseudoLit);
      }
   }

   /* Allowing predicate variables to be stored in equation */
   if(lterm->type != rterm->type &&
      !(TypeIsPredicate(lterm->type) && rterm == bank->true_term))
   {
      TermAssertSameSort(bank->sig, lterm, rterm);
   }
   handle->bank = bank;
   handle->next = NULL;

   handle->lterm = lterm;
   handle->rterm = rterm;


   /* EqnPrint(stdout, handle, false, true);
      printf("\n"); */

   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: EqnAllocFlatten()
//
//   Allocates a predicate literal but makes sure that if it is
//   a formula of the form (~)$eq(s,t) then s and t are lifted
//   to the literal level.
//
// Global Variables: -
//
// Side Effects    : Adds references
//
/----------------------------------------------------------------------*/

Eqn_p EqnAllocFlatten(Term_p lterm, TB_p bank, bool sign)
{
   assert(TypeIsBool(lterm->type));
   Eqn_p res;
   if(lterm->f_code == bank->sig->eqn_code ||
      lterm->f_code == bank->sig->neqn_code)
   {
      if(lterm->f_code == bank->sig->neqn_code)
      {
         sign = !sign;
      }
      res = EqnAlloc(lterm->args[0], lterm->args[1], bank, sign);
   }
   else
   {
      res = EqnAlloc(lterm, bank->true_term, bank, sign);
   }
   return res;
}




/*-----------------------------------------------------------------------
//
// Function: EqnFree()
//
//   Free the space taken by an equation. Does not free the terms any
//   more - this is left to GC.
//
// Global Variables: -
//
// Side Effects    : May change term bank, removes references
//
/----------------------------------------------------------------------*/

void EqnFree(Eqn_p junk)
{
   /* Note that terms are no longer freed at all (GC) and that
      references have vanished! */
   /* TermReleaseRef(&(junk->lterm));
   TBDelete(junk->bank, junk->lterm);
   TermReleaseRef(&(junk->rterm));
   TBDelete(junk->bank, junk->rterm); */
   assert(TermIsShared(junk->lterm)); // added assertions to check whether
   assert(TermIsShared(junk->rterm)); // previous comment is true
   EqnCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: EqnParse()
//
//   Parse a CNF style equation according to the current input format
//   and return a pointer to the resulting cell.
//
// Global Variables: EqnUseInfix
//
// Side Effects    : Input, memory management.
//
/----------------------------------------------------------------------*/

Eqn_p EqnParse(Scanner_p in, TB_p bank)
{
   bool positive;
   Term_p lterm, rterm;
   Eqn_p handle;

   positive = eqn_parse_real(in, bank, &lterm, &rterm, false);
   handle = EqnAlloc(lterm, rterm, bank, positive);

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: EqnFOFParse()
//
//   Parse a literal in FOF format (changes syntax for TPTP literals).
//
// Global Variables: -
//
// Side Effects    : Input
//
/----------------------------------------------------------------------*/

Eqn_p EqnFOFParse(Scanner_p in, TB_p bank)
{
   bool positive;
   Term_p lterm, rterm;
   Eqn_p handle;


   positive = eqn_parse_real(in, bank, &lterm, &rterm, true);
   handle = EqnAlloc(lterm, rterm, bank, positive);

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: EqnHOFParse()
//
//   Parse a literal in THF format. Because of many peculiarities with
//   parentheses in THF, we might have to continue on parsing the formula
//   from the point where the function has been called and then read
//   the closing bracket :(
//
// Global Variables: -
//
// Side Effects    : Changes the value continue_parsing points to
//
/----------------------------------------------------------------------*/

Eqn_p EqnHOFParse(Scanner_p in, TB_p bank, bool* continue_parsing)
{
   Term_p  lterm;
   Term_p  rterm;
   bool    positive = true;
   bool    pure_eq  = false;

   lterm = TBTermParse(in, bank);
   BOOL_TERM_NORMALIZE(lterm);

   *continue_parsing = true;
   if(TestInpTok(in, CloseBracket) && TestTok(LookToken(in,1), NegEqualSign|EqualSign))
   {
      AcceptInpTok(in, CloseBracket);
      *continue_parsing = false; // saw closing bracket.
   }

   if(TestInpTok(in, NegEqualSign|EqualSign))
   {
      if(TestInpTok(in, EqualSign))
      {
         AcceptInpTok(in, EqualSign);
      }
      else
      {
         AcceptInpTok(in, NegEqualSign);
         positive = !positive;
      }
      pure_eq = true; // saw equal
   }
   else if(TestInpTok(in, CloseBracket))
   {
      AcceptInpTok(in, CloseBracket);
      *continue_parsing = false; // saw closed bracket
   }

   if(pure_eq)
   {
      rterm = TypeIsBool(lterm->type) ?
         TFormulaTSTPParse(in, bank) : TBTermParse(in, bank);

      if(TypeIsBool(lterm->type))
      {
         lterm = EqnTermsTBTermEncode(bank, lterm, bank->true_term, true, PENormal);
      }
   }
   else
   {
      if(!TermIsAnyVar(lterm) && !TermIsPhonyApp(lterm)
         && SigIsFunction(bank->sig, lterm->f_code))
      {
         DStr_p err = DStrAlloc();
         DStrAppendStr(err, "Symbol ");
         DStrAppendStr(err, SigFindName(bank->sig, lterm->f_code));
         DStrAppendStr(err, " interpreted both as function and"
                       " predicate (check parentheses).");
         AktTokenError(in, DStrView(err), false);
      }
      rterm = bank->true_term;
   }

   return EqnAlloc(lterm, rterm, bank, positive);
}

/*-----------------------------------------------------------------------
//
// Function: EqnTBTermEncode()
//
//   Take two terms (from bank) and a positive value and return a
//   pointer to a TermBank-Term corresponding to the term encoding of
//   the equation.
//
// Global Variables: -
//
// Side Effects    : Changes term bank
//
/----------------------------------------------------------------------*/

Term_p EqnTermsTBTermEncode(TB_p bank, Term_p lterm, Term_p rterm,
                            bool positive, PatEqnDirection dir)
{
   Term_p  handle;

   assert(bank);
   assert(TBFind(bank, lterm));
   assert(TBFind(bank, rterm));

   handle = TermDefaultCellArityAlloc(2);
   handle->arity = 2;
   handle->f_code = SigGetEqnCode(bank->sig, positive);
   handle->type = bank->sig->type_bank->bool_type;
   assert(handle->f_code);
   if(dir == PENormal)
   {
      handle->args[0] = lterm;
      handle->args[1] = rterm;
   }
   else
   {
      handle->args[0] = rterm;
      handle->args[1] = lterm;
   }
   handle = TBTermTopInsert(bank, handle);

   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: EqnTBTermDecode()
//
//   Given a term encoding of an equation, create and return a
//   suitable Equation.
//
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Eqn_p EqnTBTermDecode(TB_p terms, Term_p eqn)
{
   Eqn_p res;
   bool positive = false;

   assert((eqn->f_code == terms->sig->eqn_code)||
          (eqn->f_code == terms->sig->neqn_code));
   if(eqn->f_code == terms->sig->eqn_code)
   {
      positive = true;
   }
   res = EqnAlloc(eqn->args[0], eqn->args[1], terms, positive);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: EqnTBTermParse()
//
//   Parse an equation, encode it as a term bank term and return a
//   pointer to it.
//
// Global Variables: -
//
// Side Effects    : Input, memory management.
//
/----------------------------------------------------------------------*/

Term_p EqnTBTermParse(Scanner_p in, TB_p bank)
{
   Term_p  lterm = NULL;
   Term_p  rterm = NULL;
   bool    positive;

   positive = eqn_parse_real(in, bank, &lterm, &rterm, false);

   return EqnTermsTBTermEncode(bank, lterm, rterm, positive,
                               PENormal);
}


/*-----------------------------------------------------------------------
//
// Function: EqnPrint()
//
//   Print an equation. If negated is true, print the negated
//   equation. If TPTPFormatPrint is true, print TPTPFormat.
//
// Global Variables: EqnUseInfix, EqnFullEquationalRep,
//                   TPTPFormatPrint
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

/* #define MARK_MAX_EQNS  */

void EqnPrint(FILE* out, Eqn_p eq, bool negated,  bool fullterms)
{
   bool positive = XOR(EqnIsPositive(eq), negated);

   /* TermPrintAllCPos(out, eq->bank, eq->lterm);*/
#ifdef MARK_MAX_EQNS
   if(EqnIsMaximal(eq))
   {
      fprintf(out, "{");
   }
#endif
   /* if(EqnIsSelected(eq))
   {
      fprintf(out, "+");
   } */
   /* if(EqnIsXTypePred(eq))
   {
      fprintf(out, "*");
   }*/
   if(OutputFormat == TPTPFormat)
   {
      if(positive)
      {
         fprintf(out,"++");
      }
      else
      {
         fprintf(out,"--");
      }
      if(EqnIsEquLit(eq))
      {
         fputs(EQUAL_PREDICATE"(", out);
         TBPrintTerm(out, eq->bank, eq->lterm, fullterms);
         fprintf(out, ", ");
         TBPrintTerm(out, eq->bank, eq->rterm, fullterms);
         fputc(')', out);
      }
      else
      {
         TBPrintTerm(out, eq->bank, eq->lterm, fullterms);
      }
   }
   else
   {
      if(EqnUseInfix &&
         (EqnFullEquationalRep ||
          eq->rterm!=eq->bank->true_term
          /* || eq->lterm==eq->bank->true_term*/
            ))
      {
         PRINT_HO_PAREN(out, '(');
         TBPrintTerm(out, eq->bank, eq->lterm, fullterms);

         if(!positive)
         {
            fputc('!', out);
         }
         /* fprintf(out, EqnIsOriented(eq)?"=>":"="); */
         fprintf(out, "=");
         TBPrintTerm(out, eq->bank, eq->rterm, fullterms);
         PRINT_HO_PAREN(out, ')');
      }
      else
      {
         if(!positive)
         {
            fputc('~', out);
         }
         if((eq->rterm!=eq->bank->true_term) ||
            EqnFullEquationalRep/* ||
                                   eq->lterm==eq->bank->true_term*/)
         {
            fputs(EQUAL_PREDICATE"(", out);
            TBPrintTerm(out, eq->bank, eq->lterm, fullterms);
            fprintf(out, ", ");
            TBPrintTerm(out, eq->bank, eq->rterm, fullterms);
            fputc(')', out);
         }
         else
         {
            PRINT_HO_PAREN(out, '(');
            TBPrintTerm(out, eq->bank, eq->lterm, fullterms);
            PRINT_HO_PAREN(out, ')');
         }
      }
   }
#ifdef MARK_MAX_EQNS
   if(EqnIsMaximal(eq))
   {
      fprintf(out, "}");
   }
#endif
}

/*-----------------------------------------------------------------------
//
// Function: EqnPrintDBG()
//
//   Debug printing of the equation.
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/
void EqnPrintDBG(FILE* out, Eqn_p eq)
{
   TermPrintDbg(out, eq->lterm, eq->bank->sig, DEREF_NEVER);
   fprintf(out, "%s=", EqnIsPositive(eq)?"":"!");
   TermPrintDbg(out, eq->rterm, eq->bank->sig, DEREF_NEVER);
   fprintf(out, "%s", EqnIsMaximal(eq) ? "*" : "");
   fprintf(out, "%s", EqnIsOriented(eq) ? ">" : "");
   fprintf(out, "%s", EqnQueryProp(eq, EPIsEquLiteral) ? COMCHAR : "");
}

/*-----------------------------------------------------------------------
//
// Function: EqnPrintDeref()
//
//   Print a (potentially instantiated) equation (in standard infix).
//
// Global Variables: -
//
// Side Effects    : Oputput
//
/----------------------------------------------------------------------*/

void EqnPrintDeref(FILE* out, Eqn_p eq, DerefType deref)
{
   TermPrint(out, eq->lterm, eq->bank->sig, deref);
   fprintf(out, "%s", EqnIsPositive(eq)?"=":"!=");
   TermPrint(out, eq->rterm, eq->bank->sig, deref);
}


/*-----------------------------------------------------------------------
//
// Function: EqnFOFPrint()
//
//   Print an equation in FOF format. For LOP/TSTP that is infix, for
//   TPTP/PCL it is prefix.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void EqnFOFPrint(FILE* out, Eqn_p eq, bool negated,  bool fullterms, bool pcl)
{
   bool positive = XOR(EqnIsPositive(eq), negated);
   bool infix = false;

   switch(OutputFormat)
   {
   case TPTPFormat:
         break;
   case TSTPFormat:
         infix = true;
         break;
   case LOPFormat:
         infix = !pcl;
         break;
   default:
         assert(false && "Format nor supported.");
         break;
   }
   if(infix)
   {
      if(EqnIsEquLit(eq))
      {
         PRINT_HO_PAREN(out, '(');
         PRINT_HO_PAREN(out, '(');
         TBPrintTerm(out, eq->bank, eq->lterm, fullterms);
         PRINT_HO_PAREN(out, ')');
         if(!positive)
         {
            fputc('!', out);
         }
         fputc('=', out);
         PRINT_HO_PAREN(out, '(');
         TBPrintTerm(out, eq->bank, eq->rterm, fullterms);
         PRINT_HO_PAREN(out, ')');
         PRINT_HO_PAREN(out, ')');
      }
      else
      {
         if(!positive)
         {
            fputc('~', out);
         }
         PRINT_HO_PAREN(out, '(');
         TBPrintTerm(out, eq->bank, eq->lterm, fullterms);
         PRINT_HO_PAREN(out, ')');
      }
   }
   else
   {
      if(!positive)
      {
         fprintf(out,"~");
      }
      if(EqnIsEquLit(eq))
      {
         fputs(EQUAL_PREDICATE"(", out);
         TBPrintTerm(out, eq->bank, eq->lterm, fullterms);
         fprintf(out, ", ");
         TBPrintTerm(out, eq->bank, eq->rterm, fullterms);
         fputc(')', out);
      }
      else
      {
         TBPrintTerm(out, eq->bank, eq->lterm, fullterms);
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: EqnAppEncode()
//
//   Encodes both sides of the equation using applicative encoding.
//   Does not change original equation.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void EqnAppEncode(FILE* out, Eqn_p eq, bool negated)
{
   bool positive = XOR(EqnIsPositive(eq), negated);

   Term_p l_app_enc = TermAppEncode(eq->lterm, eq->bank->sig);
   if(EqnIsEquLit(eq))
   {
      Term_p r_app_enc = TermAppEncode(eq->rterm, eq->bank->sig);

      TermPrintFO(out, l_app_enc, eq->bank->sig, DEREF_NEVER);
      if(!positive)
      {
         fputc('!', out);
      }
      fputc('=', out);
      TermPrintFO(out, r_app_enc, eq->bank->sig, DEREF_NEVER);

      TermFree(r_app_enc);
   }
   else
   {
      if(!positive)
      {
         fputc('~', out);
      }
      TermPrintFO(out, l_app_enc, eq->bank->sig, DEREF_NEVER);
   }
   TermFree(l_app_enc);
}


/*-----------------------------------------------------------------------
//
// Function: EqnTSTPPrint()
//
//   Print a literal in TSTP format.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void EqnTSTPPrint(FILE* out, Eqn_p eq, bool fullterms)
{
   if(EqnIsPropFalse(eq))
   {
      fputs("$false", out);
   }
   else
   {
      if(EqnIsEquLit(eq))
      {
         TBPrintTerm(out, eq->bank, eq->lterm, fullterms);
         fprintf(out, "%s", EqnIsNegative(eq)?"!=":"=");
         TBPrintTerm(out, eq->bank, eq->rterm, fullterms);
      }
      else
      {
         if(EqnIsNegative(eq))
         {
            fputc('~', out);
         }
         TBPrintTerm(out, eq->bank, eq->lterm, fullterms);
      }
   }
}



/*-----------------------------------------------------------------------
//
// Function: EqnSwapSidesSimple()
//
//   Exchange the two sides of the equation. This will lead to
//   inconsistent states if not used carefully (i.e. only temporary or
//   via EqnSwapSides() (which takes care of the attached strings).
//
// Global Variables: -
//
// Side Effects    : Exchanges sides, !!! Causes inconsistent state!!!
//
/----------------------------------------------------------------------*/

void EqnSwapSidesSimple(Eqn_p eq)
{
   Term_p term;

   term = eq->lterm;
   eq->lterm = eq->rterm;
   eq->rterm = term;
}


/*-----------------------------------------------------------------------
//
// Function: EqnSwapSides()
//
//   Exchange the two sides of an equation. Will update type and
//   references.
//
// Global Variables: -
//
// Side Effects    : Exchanges the sides, changes type.
//
/----------------------------------------------------------------------*/

void EqnSwapSides(Eqn_p eq)
{
   EqnDelProp(eq, EPIsOriented);
   EqnDelProp(eq, EPMaxIsUpToDate);
   /* TermReleaseRef(&(eq->lterm));
      TermReleaseRef(&(eq->rterm)); */
   EqnSwapSidesSimple(eq);
   /* TermGetRef(&(eq->lterm), eq->lterm);
      TermGetRef(&(eq->rterm), eq->rterm); */
}


/*-----------------------------------------------------------------------
//
// Function: EqnCopy()
//
//   Create a copy of eq with terms from bank. Does not copy the
//   next pointer. Properties of the original terms are not copied.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Eqn_p EqnCopy(Eqn_p eq, TB_p bank)
{
   Eqn_p  handle;
   Term_p lterm, rterm;

   lterm = TBInsertNoPropsCached(bank, eq->lterm, DEREF_ALWAYS);
   rterm = TBInsertNoPropsCached(bank, eq->rterm, DEREF_ALWAYS);

   /* Properties will be taken care of later! */
   handle = EqnAlloc(lterm, rterm, bank, EqnIsPositive(eq));
   // in HO version positivity is computed in relatively complex way,
   // thus EPIsPositive must be preserved!
   handle->properties =
      (handle->properties & EPIsPositive)
      | (eq->properties & ~EPIsPositive);
   if(!EqnIsOriented(handle))
   {
      EqnDelProp(handle, EPMaxIsUpToDate);
   }
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: EqnFlatCopy()
//
//   Create a flat copy of eq.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Eqn_p EqnFlatCopy(Eqn_p eq)
{
   Eqn_p  handle;
   Term_p lterm, rterm;

   lterm = eq->lterm;
   rterm = eq->rterm;

   /* Properties will be taken care of later! */
   handle = EqnAlloc(lterm, rterm, eq->bank, EqnIsPositive(eq));
   // in HO version positivity is computed in relatively complex way,
   // thus EPIsPositive must be preserved!
   handle->properties =
      (handle->properties & EPIsPositive)
      | (eq->properties & ~EPIsPositive);
   if(!EqnIsOriented(handle))
   {
      EqnDelProp(handle, EPMaxIsUpToDate);
   }
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: EqnCopyRepl()
//
//   As EqnCopy(), but replace occurrences of old with repl.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Eqn_p EqnCopyRepl(Eqn_p eq, TB_p bank, Term_p old, Term_p repl)
{
   Eqn_p  handle;
   Term_p lterm, rterm;

   lterm = TBInsertRepl(bank, eq->lterm, DEREF_ALWAYS, old, repl);
   rterm = TBInsertRepl(bank, eq->rterm, DEREF_ALWAYS, old, repl);

   /* Properties will be taken care of later! */
   handle = EqnAlloc(lterm, rterm, bank, EqnIsPositive(eq));
   // in HO version positivity is computed in relatively complex way,
   // thus EPIsPositive must be preserved!
   handle->properties =
      (handle->properties & EPIsPositive)
      | (eq->properties & ~EPIsPositive);
   EqnDelProp(handle, EPMaxIsUpToDate);
   EqnDelProp(handle, EPIsOriented);

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: EqnCopyReplPlain()
//
//   As EqnCopyRepl(), but copy terms uninstantiated.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Eqn_p EqnCopyReplPlain(Eqn_p eq, TB_p bank, Term_p old, Term_p repl)
{
   Eqn_p  handle;
   Term_p lterm, rterm;

   /* This cam be optimize for uninstantiated terms */
   lterm = TBInsertReplPlain(bank, eq->lterm, old, repl);
   rterm = TBInsertReplPlain(bank, eq->rterm, old, repl);

   /* Properties will be taken care of later! */
   handle = EqnAlloc(lterm, rterm, bank, EqnIsPositive(eq));
   // in HO version positivity is computed in relatively complex way,
   // thus EPIsPositive must be preserved!
   handle->properties =
      (handle->properties & EPIsPositive)
      | (eq->properties & ~EPIsPositive);
   EqnDelProp(handle, EPMaxIsUpToDate);
   EqnDelProp(handle, EPIsOriented);

   return handle;
}



/*-----------------------------------------------------------------------
//
// Function: EqnCopyOpt()
//
//   Copy an instantiated equation into the same term bank (using the
//   common optimizations possible in that case).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Eqn_p EqnCopyOpt(Eqn_p eq)
{
   Eqn_p  handle;
   Term_p lterm, rterm;

   lterm = TBInsertOpt(eq->bank, eq->lterm, DEREF_ALWAYS);
   rterm = TBInsertOpt(eq->bank, eq->rterm, DEREF_ALWAYS);

   /* Properties will be taken care of later! */
   handle = EqnAlloc(lterm, rterm, eq->bank, EqnIsPositive(eq));
   // in HO version positivity is computed in relatively complex way,
   // thus EPIsPositive must be preserved!
   handle->properties =
      (handle->properties & EPIsPositive)
      | (eq->properties & ~EPIsPositive);
   EqnDelProp(handle, EPMaxIsUpToDate);
   EqnDelProp(handle, EPIsOriented);

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: EqnCopyDisjoint()
//
//   Copy an equation into the same term bank, but with disjoint
//   (odd->even or vice versa) variable.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

Eqn_p EqnCopyDisjoint(Eqn_p eq)
{
   Eqn_p  handle;
   Term_p lterm, rterm;

   lterm = TBInsertDisjoint(eq->bank, eq->lterm);
   rterm = TBInsertDisjoint(eq->bank, eq->rterm);

   /* Properties will be taken care of later! */
   handle = EqnAlloc(lterm, rterm, eq->bank, EqnIsPositive(eq));
   // in HO version positivity is computed in relatively complex way,
   // thus EPIsPositive must be preserved!
   handle->properties =
      (handle->properties & EPIsPositive)
      | (eq->properties & ~EPIsPositive);

   return handle;
}




/*-----------------------------------------------------------------------
//
// Function: EqnIsACTrivial()
//
//   Return true iff the two terms are AC-equal (with respect to the
//   AC symbols specified in the signatrue).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

bool EqnIsACTrivial(Eqn_p eq)
{
   return TermACEqual(eq->bank->sig, eq->lterm, eq->rterm);
}


/*-----------------------------------------------------------------------
//
// Function: EqnTermsAreDistinct()
//
//   Return true if terms are forced distinct by built-in
//   semi-interpretation of numbers and objects.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool EqnTermsAreDistinct(Eqn_p eq)
{
   if(TermIsConst(eq->lterm)  &&
      TermIsConst(eq->rterm)  &&
      SigIsAnyFuncPropSet(eq->bank->sig, eq->lterm->f_code,
                          eq->bank->sig->distinct_props)   &&
      SigIsAnyFuncPropSet(eq->bank->sig, eq->rterm->f_code,
                          eq->bank->sig->distinct_props)   &&
      (eq->lterm->f_code!=eq->rterm->f_code))
   {
      return true;
   }
   return false;
}

/*-----------------------------------------------------------------------
//
// Function: EqnIsTrue()
//
//   Return true if the equation is guranteed to evaluate to true (s=s
//   or s!=t where s and t are objects/numbers)
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool EqnIsTrue(Eqn_p eq)
{
   if(EqnIsPositive(eq))
   {
      return EqnIsTrivial(eq);
   }
   return EqnTermsAreDistinct(eq);
}


/*-----------------------------------------------------------------------
//
// Function: EqnIsFalse()
//
//   Return true if the equation is guaranteed to evaluate to false.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool EqnIsFalse(Eqn_p eq)
{
   if(EqnIsNegative(eq))
   {
      return EqnIsTrivial(eq);
   }
   return EqnTermsAreDistinct(eq);
}




/*-----------------------------------------------------------------------
//
// Function: EqnHasUnboundVars()
//
//   Return false if Vars(dom_side) is a superset of var(other_side),
//   true otherwise.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

bool EqnHasUnboundVars(Eqn_p eq, EqnSide domside)
{
   Term_p llside, lrside;

   /* printf("EqnHasUnboundVars(): ");
      EqnPrintOriginal(stdout, eq); */

   llside = (domside == LeftSide) ? eq->lterm : eq->rterm;
   lrside = (domside == LeftSide) ? eq->rterm : eq->lterm;

   TermVarSetProp(lrside, DEREF_NEVER, TPOpFlag);
   TermVarDelProp(llside, DEREF_NEVER, TPOpFlag);
   if(TermVarSearchProp(lrside, DEREF_NEVER, TPOpFlag))
   {
      return true;
   }
   return false;
}


/*-----------------------------------------------------------------------
//
// Function: EqnIsDefinition()
//
//   Return true if eqn is a definition, i.e. positive, and of the
//   form f(X1....Xn)=t with f not occuring in t and no other
//   variables in t.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

EqnSide EqnIsDefinition(Eqn_p eq, int min_arity)
{
   if(EqnIsNegative(eq))
   {
      return NoSide;
   }
   if(TermIsDefTerm(eq->lterm, min_arity) &&
      !SigQueryFuncProp(eq->bank->sig, eq->lterm->f_code, FPPseudoPred)&&
      !TermHasFCode(eq->rterm, eq->lterm->f_code) &&
      !EqnHasUnboundVars(eq, LeftSide))
   {
      return LeftSide;
   }
   if(TermIsDefTerm(eq->rterm, min_arity) &&
      !SigQueryFuncProp(eq->bank->sig, eq->rterm->f_code, FPPseudoPred)&&
      !TermHasFCode(eq->lterm, eq->rterm->f_code) &&
      !EqnHasUnboundVars(eq, RightSide))
   {
      return RightSide;
   }
   return NoSide;
}


/*-----------------------------------------------------------------------
//
// Function: EqnSubsumeQOrderCompare()
//
//   Compare two equations with a quasi-ordering that
//   ensures that only equivalent equations can subsume each other.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int EqnSubsumeQOrderCompare(const void* lit1, const void* lit2)
{
   int res;
   const Eqn_p l1 = (const Eqn_p) lit1;
   const Eqn_p l2 = (const Eqn_p) lit2;

   res = EqnIsPositive(l1) - EqnIsPositive(l2);
   if(res)
   {
      return res;
   }
   res = EqnIsEquLit(l1) - EqnIsEquLit(l2);
   if(res)
   {
      return res;
   }

   if(problemType == PROBLEM_FO && !EqnIsEquLit(l1))
   {
      // because variables might appear at predicate positions,
      // all nonequational literals belong to the same class in full HOL
      res = CMP(l1->lterm->f_code, l2->lterm->f_code);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: EqnSubsumeInverseCompareRef()
//
//   Compute a refinement of the inverse of the previous ordering such
//   that a smaller literal can never subsume a larger ond
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int EqnSubsumeInverseCompareRef(const void* lit1ref, const void* lit2ref)
{
   const Eqn_p *l1 = lit1ref;
   const Eqn_p *l2 = lit2ref;

   int res = EqnSubsumeQOrderCompare(*l2, *l1);

   if(!res)
   {
      res = CMP(EqnStandardWeight(*l2), EqnStandardWeight(*l1));
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: EqnSubsumeInverseRefinedCompareRef()
//
//   A refined version of the above, made total for search control
//   purposes, but not longer strictly compatible with subsumption!
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int EqnSubsumeInverseRefinedCompareRef(const void* lit1ref, const void* lit2ref)
{
   const Eqn_p *l1 = lit1ref;
   const Eqn_p *l2 = lit2ref;
   int res = EqnSubsumeInverseCompareRef(lit1ref, lit2ref);

   /*
   // The following tries to bring a bit more determinism into
   // subsumption and literal order. For reasons I don't quite grasp,
   // this plays a major role and leads into very bad case behaviour if
   // overdone...
   */
   /*if(!res)
     {
     res = (*l2)->lterm->entry_no%2 - (*l1)->lterm->entry_no%2;
     }
     if(!res)
     {
     res = (*l1)->rterm->entry_no%2 - (*l2)->rterm->entry_no%2;
     }*/

   /* This hack makes the ordering stable */
   if(!res)
   {
      res = (*l1)->pos - (*l2)->pos;
   }
   return res;
}




/*-----------------------------------------------------------------------
//
// Function: EqnSubsumeCompare()
//
//   Compute the inverse of the previous order, taking pointers as
//   arguments.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int EqnSubsumeCompare(Eqn_p l1, Eqn_p l2)
{
   return EqnSubsumeInverseCompareRef(&l2, &l1);
}



/*-----------------------------------------------------------------------
//
// Function: EqnCanonize(eq)
//
//   Bring equation into canonical form: If there is at least one
//   $true-term, RHS is a true term. Otherwise, the bigger term (by
//   standard weight) is the LHS. If they are equal, the one with the
//   smaller top symbol arity is LHS. Otherwise, compare
//   lexicographically.
//
// Global Variables: -
//
// Side Effects    : Changes orientation.
//
/----------------------------------------------------------------------*/

Eqn_p EqnCanonize(Eqn_p eq)
{
   if(!TermStructWeightCompare(eq->lterm, eq->rterm)
      && TermLexCompare(eq->lterm, eq->rterm) < 0)
   {
      EqnSwapSides(eq);
   }
   /* printf("Canonical form (eq): ");
      EqnPrintOriginal(stdout, eq);
      printf("\n"); */
   return eq;
}


/*-----------------------------------------------------------------------
//
// Function: EqnStructWeightCompare()
//
//   Compare two equation (literals) based on structural criteria
//   only: Sign, Equality, Size, LHS structure, RHS
//   structure. Assumes that the literals are in canonical form (see
//   above).
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

long EqnStructWeightCompare(Eqn_p l1, Eqn_p l2)
{
   long res;

   if(EqnIsPositive(l1) && !EqnIsPositive(l2))
   {
      return -1;
   }
   if(EqnIsPositive(l2) && !EqnIsPositive(l1))
   {
      return 1;
   }
   if(EqnIsEquLit(l1) && !EqnIsEquLit(l2))
   {
      return -1;
   }
   if(EqnIsEquLit(l2) &&  !EqnIsEquLit(l1))
   {
      return 1;
   }
   res = EqnStandardWeight(l1) - EqnStandardWeight(l2);
   if(res)
   {
      return res;
   }
   res = TermStructWeightCompare(l1->lterm, l2->lterm);
   if(res)
   {
      return res;
   }
   res = TermStructWeightCompare(l1->rterm, l2->rterm);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: EqnCanonCompareRef()
//
//   Compare two pointed to equations with EqnStructWeightLexCompare().
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int EqnCanonCompareRef(const void* lit1ref, const void* lit2ref)
{
   const Eqn_p *l1 = lit1ref;
   const Eqn_p *l2 = lit2ref;

   return CMP(EqnStructWeightLexCompare(*l1, *l2), 0);
}




/*-----------------------------------------------------------------------
//
// Function: EqnStructWeightLexCompare()
//
//   Compare equations first by structure, then by lexical f_codes.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long EqnStructWeightLexCompare(Eqn_p l1, Eqn_p l2)
{
   long res = EqnStructWeightCompare(l1, l2);

   if(res)
   {
      return res;
   }
   res = TermLexCompare(l1->lterm, l2->lterm);
   if(res)
   {
      return res;
   }
   res = TermLexCompare(l1->rterm, l2->rterm);
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: EqnEqualDeref()
//
//   Test wether two equations are equivalent (modulo commutativity).
//   Treats equations as _unsigned_ term sets. Follows variable binding
//   pointers as denoted by d1 and d2.
//
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool EqnEqualDeref(Eqn_p eq1,  Eqn_p eq2, DerefType d1, DerefType d2)
{
   bool res;

   res = EqnEqualDirectedDeref(eq1, eq2, d1, d2);

   if(res || (EqnIsOriented(eq1) && EqnIsOriented(eq2)))
   {
      return res;
   }
   EqnSwapSidesSimple(eq2);
   res = EqnEqualDirectedDeref(eq1, eq2, d1, d2);
   EqnSwapSidesSimple(eq2);
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: EqnSubsumeDirected()
//
//   Test wether an equation subsumes another one. If yes, return true
//   and extend subst to give the substitution, otherwise just return
//   false and let subst unmodified. Don't deal with commutativity of
//   equality.
//
// Global Variables: -
//
// Side Effects    : -
//
//----------------------------------------------------------------------*/

bool EqnSubsumeDirected(Eqn_p subsumer, Eqn_p subsumed, Subst_p subst)
{
   PStackPointer backtrack = PStackGetSP(subst);
   bool res;

   res = SubstMatchComplete(subsumer->lterm, subsumed->lterm, subst);
   if(res)
   {
      res = SubstMatchComplete(subsumer->rterm, subsumed->rterm, subst);
   }
   if(!res)
   {
      SubstBacktrackToPos(subst, backtrack);
   }
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: EqnSubsume()
//
//   Test wether an equation subsumes another one. If yes, return true
//   and extend subst to give the substitution, otherwise just return
//   true and let subst unmodifies. Equations are treated as 2-sets of
//   terms unless both are oriented.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool EqnSubsume(Eqn_p subsumer, Eqn_p subsumed, Subst_p subst)
{
   bool res;

   if(EqnIsOriented(subsumer) && !EqnIsOriented(subsumed))
   {
      return false;
   }
   res = EqnSubsumeDirected(subsumer, subsumed, subst);

   if(res || EqnIsOriented(subsumer))
   {
      return res;
   }
   EqnSwapSidesSimple(subsumer);
   res = EqnSubsumeDirected(subsumer, subsumed, subst);
   EqnSwapSidesSimple(subsumer);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: EqnSubsumeP()
//
//   Test wether subsumer subsumes subsumed, undo all side effects.
//
// Global Variables: -
//
// Side Effects    : - (I hope)
//
/----------------------------------------------------------------------*/

bool EqnSubsumeP(Eqn_p subsumer, Eqn_p subsumed)
{
   Subst_p subst = SubstAlloc();
   bool    res = EqnSubsume(subsumer, subsumed, subst);

   SubstDelete(subst);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: LiteralSubsumeP()
//
//   Return true if subsumer subsumes subsumed, false
///  otherwise. Checks signs!
//
// Global Variables: -
//
// Side Effects    : Only temporary
//
/----------------------------------------------------------------------*/

bool LiteralSubsumeP(Eqn_p subsumer, Eqn_p subsumed)
{
   if(PropsAreEquiv(subsumer, subsumed, EPIsPositive))
   {
      return EqnSubsumeP(subsumer, subsumed);
   }
   return false;
}



/*-----------------------------------------------------------------------
//
// Function: EqnUnifyDirected()
//
//   Test wether two equations can be unified. If yes, return true
//   and extend subst to give the substitution, otherwise just return
//   false and let subst unmodified. Don't deal with commutativity of
//   equality.
//
// Global Variables: -
//
// Side Effects    : -
//
//----------------------------------------------------------------------*/

bool EqnUnifyDirected(Eqn_p eq1, Eqn_p eq2, Subst_p subst)
{
   assert(eq1->bank == eq2->bank);

   PStackPointer backtrack = PStackGetSP(subst);
   bool res;

   res = SubstMguComplete(eq1->lterm, eq2->lterm, subst);
   if(res)
   {
      res = SubstMguComplete(eq1->rterm, eq2->rterm, subst);
   }
   if(!res)
   {
      SubstBacktrackToPos(subst, backtrack);
   }
   return res;
}




/*-----------------------------------------------------------------------
//
// Function: EqnUnify()
//
//   Test wether two equations are unifyable. If yes, return true
//   and extend subst to give the substitution, otherwise just return
//   true and let subst unmodifies. Equations are treated as 2-sets of
//   terms unless both are oriented.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool EqnUnify(Eqn_p eq1, Eqn_p eq2, Subst_p subst)
{
   bool   res = EqnUnifyDirected(eq1, eq2, subst);

   if(res || (EqnIsOriented(eq1) && EqnIsOriented(eq2)))
   {
      return res;
   }
   EqnSwapSidesSimple(eq1);
   res = EqnUnifyDirected(eq1, eq2, subst);
   EqnSwapSidesSimple(eq1);
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: EqnUnifyP()
//
//   Test wether two equations are unifiable, undo all side effects.
//
// Global Variables: -
//
// Side Effects    : - (I hope)
//
/----------------------------------------------------------------------*/

bool  EqnUnifyP(Eqn_p eq1, Eqn_p eq2)
{
   Subst_p subst = SubstAlloc();
   bool    res = EqnUnify(eq1, eq2, subst);

   SubstDelete(subst);

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: LiteralUnifyOneWay()
//
//   Test wether two equations are unifyable, taking into account sign
//   and direction. If yes, return true and extend subst to give the
//   substitution, otherwise just return false and let subst
//   unmodifies.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

bool LiteralUnifyOneWay(Eqn_p eq1, Eqn_p eq2, Subst_p subst, bool swapped)
{
   PStackPointer backtrack = PStackGetSP(subst);
   bool res = false;

   if(!EQUIV(EqnIsPositive(eq1), EqnIsPositive(eq2)))
   {
      return res;
   }
   if(swapped)
   {
      EqnSwapSides(eq2);
   }
   res = SubstMguComplete(eq1->lterm, eq2->lterm, subst);
   if(res)
   {
      res = SubstMguComplete(eq1->rterm, eq2->rterm, subst);
   }
   if(!res)
   {
      SubstBacktrackToPos(subst, backtrack);
   }
   if(swapped)
   {
      EqnSwapSides(eq2);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: EqnSyntaxCompare()
//
//   Induce a total ordering on equations (modulo  commutativity, but
//   ignoring properties, including polarity). Assumes that terms are
//   perfectly shared. Equality literals are smaller than
//   non-equational literals, the rest is done by comparing term bank
//   entry_no.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

int EqnSyntaxCompare(const void* l1, const void* l2)
{
   const Eqn_p eq1 = (const Eqn_p) l1;
   const Eqn_p eq2 = (const Eqn_p) l2;
   long e1, e2;

   //printf("EqnSyntaxCompare()\n");
   //EqnPrint(stdout, eq1, false, true);
   //printf("\n");
   //EqnPrint(stdout, eq2, false, true);
   //printf("\n");

   if(EqnIsEquLit(eq1) && !EqnIsEquLit(eq2))
   {
      return -1;
   }
   if(EqnIsEquLit(eq2) &&  !EqnIsEquLit(eq1))
   {
      return 1;
   }
   e1 = MAX(eq1->lterm->entry_no,eq1->rterm->entry_no);
   e2 = MAX(eq2->lterm->entry_no,eq2->rterm->entry_no);
   if(e1<e2)
   {
      return -1;
   }
   if(e1>e2)
   {
      return 1;
   }
   e1 = MIN(eq1->lterm->entry_no,eq1->rterm->entry_no);
   e2 = MIN(eq2->lterm->entry_no,eq2->rterm->entry_no);
   if(e1<e2)
   {
      return -1;
   }
   if(e1>e2)
   {
      return 1;
   }
   return 0;
}



/*-----------------------------------------------------------------------
//
// Function: LiteralSyntaxCompare()
//
//   Induce a total ordering on literals (modulo
//   commutativity). Assumes that terms are perfectly shared. Negative
//   literals are bigger than positive ones, equality literals are
//   smaller than non-equational literals, the rest is done by
//   comparing term bank entry_no.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

int LiteralSyntaxCompare(const void* l1, const void* l2)
{
   const Eqn_p eq1 = (const Eqn_p) l1;
   const Eqn_p eq2 = (const Eqn_p) l2;

   if(EqnIsPositive(eq1) && !EqnIsPositive(eq2))
   {
      return -1;
   }
   if(EqnIsPositive(eq2) && !EqnIsPositive(eq1))
   {
      return 1;
   }
   return EqnSyntaxCompare(l1, l2);
}





/*-----------------------------------------------------------------------
//
// Function: EqnOrient()
//
//   Orient an equation. Return true, if sides are exchanged, false
//   otherwise.
//
// Global Variables: -
//
// Side Effects    : May change term references
//
/----------------------------------------------------------------------*/

bool EqnOrient(OCB_p ocb, Eqn_p eq)
{
   CompareResult relation = to_uncomparable;
   bool res = false;

   if(EqnQueryProp(eq, EPMaxIsUpToDate))
   {
      return false;
   }
   if(eq->lterm == eq->rterm)
   {
      relation = to_equal;
   }
   else if(eq->lterm == eq->bank->true_term)
   {
      relation = to_lesser;
   }
   else if(eq->rterm == eq->bank->true_term)
   {
      relation = to_greater;
   }
   else
   {
      /*printf("EqnOrient: ");
        TermPrint(stdout, eq->lterm, eq->bank->sig, DEREF_ALWAYS);
        printf(" "COMCHAR" ");
        TermPrint(stdout, eq->rterm, eq->bank->sig, DEREF_ALWAYS);
        printf("\n");*/
      relation = TOCompare(ocb, eq->lterm, eq->rterm, DEREF_ALWAYS, DEREF_ALWAYS);
   }
   switch(relation)
   {
   case to_uncomparable:
   case to_equal:
         EqnDelProp(eq, EPIsOriented);
         break;
   case to_greater:
         EqnSetProp(eq, EPIsOriented);
         break;
   case to_lesser:
         EqnSwapSides(eq);
         res = true;
         EqnSetProp(eq, EPIsOriented);
         break;
   default:
         assert(false);
         break;
   }
   EqnSetProp(eq, EPMaxIsUpToDate);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: EqnCompare()
//
//   Compare two equations (as multisets of terms) and return the
//   result.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

CompareResult EqnCompare(OCB_p ocb, Eqn_p eq1, Eqn_p eq2)
{
   CompareResult res;

   res = compare_pos_eqns(ocb, eq1, eq2);

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: EqnGreater()
//
//   Return true if eq1 is greater than eq2, false otherwise.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool EqnGreater(OCB_p ocb, Eqn_p eq1, Eqn_p eq2)
{
   bool res;

   res = EqnCompare(ocb, eq1, eq2) == to_greater;

   return res;
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

CompareResult tfo_literal_compare(OCB_p ocb, Eqn_p eq1, Eqn_p eq2)
{
   if(ocb->lit_cmp==LCTFOEqMax)
   {
      if(EqnIsEquLit(eq1) && !EqnIsEquLit(eq2))
      {
         return to_greater;
      }
      if(!EqnIsEquLit(eq1) && EqnIsEquLit(eq2))
      {
         return to_lesser;
      }
      if(!EqnIsEquLit(eq1) && !TermIsTopLevelFreeVar(eq1->lterm)
                           && !TermIsTopLevelFreeVar(eq2->lterm)) /* both non-equational! */
      {
         return OCBFunCompare(ocb, eq1->lterm->f_code, eq2->lterm->f_code);
      }
   }
   else if(ocb->lit_cmp==LCTFOEqMin)
   {
      if(EqnIsEquLit(eq1) && !EqnIsEquLit(eq2))
      {
         return to_lesser;
      }
      if(!EqnIsEquLit(eq1) && EqnIsEquLit(eq2))
      {
         return to_greater;
      }
      if(!EqnIsEquLit(eq1) && !TermIsTopLevelFreeVar(eq1->lterm)
                           && !TermIsTopLevelFreeVar(eq2->lterm)) /* both non-equational! */
      {
         return OCBFunCompare(ocb, eq1->lterm->f_code, eq2->lterm->f_code);
      }
   }
   return to_unknown;
}

/*-----------------------------------------------------------------------
//
// Function: LiteralCompare()
//
//   Compare two signed literals L1 and L2:
//
//     L1 > L2  <==>  rep(L1) >> rep(L2)
//
//     where >> stands for the extension of an ordering on terms to
//     multisets of (multisets of) terms
//     and rep is a function mapping (negative or positive) equations
//     to multisets (of multisets) of terms in the following way:
//
//     o rep(s=t) = rep(s=/=t) = {s,t}
//       if L1 and L2 both are positive or both are negative equations
//
//     o rep(s=t) = {{s},{t}}  and  rep(s=/=t) = {{s,t}}
//       otherwise
//
//     Additionally, selected equations are bigger than unselected
//     ones, and selected positive and negative equations are
//     uncomparable.
//
//     Finally, pseudo-literals are smaller than all other literals.
//
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

CompareResult LiteralCompare(OCB_p ocb, Eqn_p eq1, Eqn_p eq2)
{
   CompareResult res;

   if(EqnQueryProp(eq1, EPPseudoLit) &&
      !(EqnQueryProp(eq2, EPPseudoLit)))
   {
      return to_lesser;
   }
   if(EqnQueryProp(eq2, EPPseudoLit) &&
      !(EqnQueryProp(eq1, EPPseudoLit)))
   {
      return to_greater;
   }
   if(!EqnIsSelected(eq1))
   {
      if(EqnIsSelected(eq2))
      {
         return to_lesser;
      }
   }
   else if(!EqnIsSelected(eq2))
   {
      if(EqnIsSelected(eq1))
      {
         return to_greater;
      }
   }
   else
   {
      assert(EqnIsSelected(eq1) && EqnIsSelected(eq2));
      if(!PropsAreEquiv(eq1, eq2, EPIsPositive))
      {
         return to_uncomparable;
      }
   }
   if(ocb->lit_cmp==LCNoCmp)
   {
      return to_uncomparable;
   }

   res = tfo_literal_compare(ocb, eq1, eq2);
   if((res == to_greater) || (res == to_lesser))
   {
      return res;
   }

   if(PropsAreEquiv(eq1, eq2, EPIsPositive))
   {
      return compare_pos_eqns(ocb, eq1, eq2);
   }
   else if(EqnIsPositive(eq1))
   {   /* Exactly one of the equations is negative */
      return compare_poseqn_negeqn(ocb, eq1, eq2);
   }
   else
   {
      /* eq1 is negative and eq2 is positive */
      return POInverseRelation(compare_poseqn_negeqn(ocb, eq2,
                                                     eq1));
   }
}



/*-----------------------------------------------------------------------
//
// Function: LiteralGreater()
//
//   Return true if eq1 is greater than eq2, takes as (signed)
//   literals, false otherwise.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool LiteralGreater(OCB_p ocb, Eqn_p eq1, Eqn_p eq2)
{
   bool res;

   res = LiteralCompare(ocb, eq1, eq2) == to_greater;

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: SubstNormEqn()
//
//   Instantiate all variables in eq with normed variables. Returns
//   the previous value of vars->v_count, i.e. the number of the first
//   fresh variable used.
//
// Global Variables: -
//
// Side Effects    : By SubstNormTerm()
//
/----------------------------------------------------------------------*/

PStackPointer SubstNormEqn(Eqn_p eq, Subst_p subst, VarBank_p vars)
{
   PStackPointer res;

   res = SubstNormTerm(eq->lterm, subst, vars, eq->bank->sig);
   SubstNormTerm(eq->rterm, subst, vars, eq->bank->sig);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: EqnWeight()
//
//   Compute the weight of an equation. Weights of potentially maximal
//   sides are multiplied by max_multiplier.
//   Weight of applied variables is multiplied with app_var_mult.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double EqnWeight(Eqn_p eq, double max_multiplier, long vweight, long
                 fweight, double app_var_mult)
{
   double res;

   if(EqnIsOriented(eq))
   {
      res = (double)TermWeight(eq->rterm, vweight, fweight);
   }
   else
   {
      res = (double)TermWeight(eq->rterm, vweight, fweight) *
         max_multiplier;
   }
   res = TERM_APPLY_APP_VAR_MULT(res, eq->rterm, app_var_mult);
   res += TERM_APPLY_APP_VAR_MULT((double)TermWeight(eq->lterm, vweight, fweight)
                                  * max_multiplier,
                                  eq->lterm, app_var_mult);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: EqnDAGWeight()
//
//   Compute the DAG weight of an equation. Weights of potentially
//   maximal sides are cpmputed first, and are multiplied by
//   max_multiplier. If new_eqn is set, the equation is treated as a
//   stand-alone structure. If new_terms is set, the two terms are
//   treated as stand-alone structures.
//
// Global Variables: -
//
// Side Effects    : Sets TPOpFlag
//
/----------------------------------------------------------------------*/

double  EqnDAGWeight(Eqn_p eq, double uniqmax_multiplier,
                     double max_multiplier, long vweight, long fweight,
                     long dup_weight, bool new_eqn, bool new_terms)
{
   double res;
   long lweight, rweight;

   if(new_eqn)
   {
      EqnTermDelProp(eq, TPOpFlag);
   }
   else if(new_terms)
   {
      TermDelPropOpt(eq->lterm, TPOpFlag);
   }
   lweight = TermDAGWeight(eq->lterm, fweight, vweight, dup_weight, false);
   rweight = TermDAGWeight(eq->rterm, fweight, vweight, dup_weight, new_terms);
   //printf("(%ld/%ld)\n", lweight, rweight);

   if(EqnIsOriented(eq))
   {
      res = uniqmax_multiplier*max_multiplier*(double)lweight;
      res += (double)rweight;
   }
   else
   {
      res = max_multiplier*(double)lweight;
      res += max_multiplier*(double)rweight;
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: EqnDAGWeight2()
//
//   Alternative DAG weight of an equation, inspired by Twee
//   (Smallbone:CADE-202, but details via personal email): Terms are
//   treated as individual DAGs, the bigger weight of both terms is
//   boosted by a multiplier. Term order is not considered.
//
// Global Variables: -
//
// Side Effects    : Sets TPOpFlag
//
/----------------------------------------------------------------------*/

double EqnDAGWeight2(Eqn_p eq, double maxw_multiplier,
                     long vweight, long fweight, long dup_weight)
{
   double res;
   long lweight, rweight;

   lweight = TermDAGWeight(eq->lterm, fweight, vweight, dup_weight, true);
   rweight = TermDAGWeight(eq->rterm, fweight, vweight, dup_weight, true);
   //printf("(%ld/%ld)\n", lweight, rweight);

   if(rweight > lweight)
   {
      SWAP(lweight, rweight);
   }
   res = maxw_multiplier*lweight+rweight;

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: EqnFunWeight()
//
//   As EqnWeight(), but use weighted FSum instead of plain term
//   weight.
//   Weight of applied variables is multiplied with app_var_mult.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

double EqnFunWeight(Eqn_p eq, double max_multiplier, long vweight,
                    long flimit, long *fweights, long default_fweight,
                    double app_var_mult, long* typefreqs)
{
   double res;

   res = (double)TermFsumWeight(eq->rterm, vweight, flimit, fweights, default_fweight,
                                typefreqs);
   res = TERM_APPLY_APP_VAR_MULT(res, eq->rterm, app_var_mult);

   if(!EqnIsOriented(eq))
   {
      res *= max_multiplier;
   }

   res += TERM_APPLY_APP_VAR_MULT(
      (double)TermFsumWeight(eq->lterm, vweight, flimit, fweights,
                             default_fweight, typefreqs) * max_multiplier,
      eq->lterm, app_var_mult);

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: EqnTermExtWeight()
//
//   Compute the weight of a literal as an extension of an arbitrary term
//   weight function. Modifiers are applied, several extensions are
//   supported (standard - sum literal/term weights, subterms - sum
//   weights of all subterms, or take the maximum subterm weight).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double EqnTermExtWeight(Eqn_p eq, TermWeightExtension_p twe)
{
   double res;

   if (EqnIsOriented(eq))
   {
      res = TermExtWeight(eq->rterm, twe);
   }
   else
   {
      res = TermExtWeight(eq->rterm, twe) * twe->max_term_multiplier;
   }
   res += (TermExtWeight(eq->lterm, twe) * twe->max_term_multiplier);

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: EqnNonLinearWeight()
//
//   Compute the non-linear weight of an equation. Weights of
//   potentially maximal sides are multiplied by max_multiplier.
//   Weight of applied variables is multiplied with app_var_mult.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double EqnNonLinearWeight(Eqn_p eq, double max_multiplier, long
                          vlweight, long vweight, long fweight,
                          double app_var_mult)
{
   double res;

   if(EqnIsOriented(eq))
   {
      res = (double)TermNonLinearWeight(eq->rterm, vlweight, vweight, fweight);
   }
   else
   {
      res = (double)TermNonLinearWeight(eq->rterm, vlweight, vweight,
                                        fweight) * max_multiplier;
   }
   res = TERM_APPLY_APP_VAR_MULT(res, eq->rterm, app_var_mult);
   res += TERM_APPLY_APP_VAR_MULT(
            (double)(TermNonLinearWeight(eq->lterm, vlweight, vweight,
                                       fweight) * max_multiplier),
            eq->lterm, app_var_mult);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: EqnSymTypeWeight()
//
//   Compute the symbol type weight of an equation.
//   Weight of applied variables is multiplied with app_var_mult.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double  EqnSymTypeWeight(Eqn_p eq, double max_multiplier, long
                         vweight, long fweight, long cweight, long
                         pweight, double app_var_mult)
{
   double res;

   if(EqnIsOriented(eq))
   {
      res = (double)TermSymTypeWeight(eq->rterm, vweight, fweight,
                                      cweight, pweight);
   }
   else
   {
      res = (double)TermSymTypeWeight(eq->rterm, vweight, fweight,
                                      cweight, pweight) * max_multiplier;
   }
   res = TERM_APPLY_APP_VAR_MULT(res, eq->rterm, app_var_mult);
   res += TERM_APPLY_APP_VAR_MULT((double)(TermSymTypeWeight(eq->lterm, vweight, fweight,
                                      cweight, pweight) * max_multiplier),
                                      eq->lterm, app_var_mult);

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: EqnMaxWeight()
//
//   Compute the maximum of the weighs of the terms of an equation.
//   Weight of applied variables is multiplied with app_var_mult.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double EqnMaxWeight(Eqn_p eq, long vweight, long fweight, double app_var_mult)
{
   double lweight = TermWeight(eq->lterm, vweight, fweight);
   double rweight = TermWeight(eq->rterm, vweight, fweight);

   return MAX(TERM_APPLY_APP_VAR_MULT(lweight, eq->lterm, app_var_mult),
              TERM_APPLY_APP_VAR_MULT(rweight, eq->rterm, app_var_mult));
}


/*-----------------------------------------------------------------------
//
// Function: EqnCorrectedWeight()
//
//   Compute the weight of an equation. Weights of potentially maximal
//   sides are multiplied by max_multiplier. The equal-Predicate is
//   counted with weight fweight, $true is not counted. Applied variable
//   terms are multiplied with app_var_mult.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double EqnCorrectedWeight(Eqn_p eq, double max_multiplier, long
                          vweight, long fweight, double app_var_mult)
{
   double res;

   if(EqnIsEquLit(eq))
   {
      if(EqnIsOriented(eq))
      {
         res = (double)TermWeight(eq->rterm, vweight, fweight);
      }
      else
      {
         res = (double)TermWeight(eq->rterm, vweight, fweight) *
            max_multiplier;
      }
      res += fweight; /* Count the equal-predicate */
      res = TERM_APPLY_APP_VAR_MULT(res, eq->rterm, app_var_mult);
   }
   else
   {
      res = 0;
   }
   res += TERM_APPLY_APP_VAR_MULT((double)TermWeight(eq->lterm, vweight, fweight) * max_multiplier,
                                     eq->lterm, app_var_mult);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: EqnCorrectedNonLinearWeight()
//
//   Compute the weight of an equation. Weights of potentially maximal
//   sides are multiplied by max_multiplier. The equal-Predicate is
//   counted with weight fweight, $true is not counted. Applied variable's
//   weight is multiplied with app_var_mult.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double EqnCorrectedNonLinearWeight(Eqn_p eq, double max_multiplier,
                                   long vlweight, long vweight, long
                                   fweight, double app_var_mult)
{
   double res;

   if(EqnIsEquLit(eq))
   {
      if(EqnIsOriented(eq))
      {
         res = (double)TermNonLinearWeight(eq->rterm, vlweight,
                                           vweight, fweight);
      }
      else
      {
         res = (double)TermNonLinearWeight(eq->rterm, vlweight,
                                           vweight, fweight) *
            max_multiplier;
      }
      res = TERM_APPLY_APP_VAR_MULT(res, eq->rterm, app_var_mult);
      res += fweight; /* Count the equal-predicate */
   }
   else
   {
      res = 0;
   }
   res += TERM_APPLY_APP_VAR_MULT(((double)TermNonLinearWeight(eq->lterm, vlweight, vweight,
                                       fweight) * max_multiplier), eq->lterm, app_var_mult);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: EqnMaxTermPositions()
//
//   Return the number of positions in maximal terms of eqn.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long EqnMaxTermPositions(Eqn_p eqn)
{
   long res = TermWeight(eqn->lterm, 1, 1);

   if(!EqnIsOriented(eqn))
   {
      res += TermWeight(eqn->rterm, 1, 1);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: EqnInferencePositions()
//
//   Return the number of potential inference positions in maximal
//   terms of eqn. Variables are not inference positions.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

long EqnInferencePositions(Eqn_p eqn)
{
   long res = TermWeight(eqn->lterm, 0, 1);

   if(EqnIsOriented(eqn))
   {
      res += TermWeight(eqn->rterm, 0, 1);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: LiteralWeight()
//
//   Return weight of a literal. max_term_multipler is applied to maximal
//   sides, max_literal_multiplier to maxinal literals, pos_multiplier
//   is applied to positive literals. If count_eq_encoding is true,
//   count $true and ignore the equal-predicate, otherwise ignore
//   $true and count the equal-predicate for equations only.
//   Applied variable's weights are multiplied by app_var_mult.
//
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double  LiteralWeight(Eqn_p eq, double max_term_multiplier, double
                      max_literal_multiplier, double
                      pos_multiplier, long vweight, long fweight,
                      double app_var_mult, bool count_eq_encoding)
{
   double res;

   res = count_eq_encoding?
      EqnWeight(eq, max_term_multiplier, vweight, fweight, app_var_mult):
      EqnCorrectedWeight(eq, max_term_multiplier, vweight, fweight, app_var_mult);


   if(EqnIsMaximal(eq))
   {
      res = res*max_literal_multiplier;
   }
   if(EqnIsPositive(eq))
   {
      res = res*pos_multiplier;
   }

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: LiteralFunWeight()
//
//   As LiteralWeight(), but use individual functgion symbol
//   weights. The eq encoding is always counted.
//   Weight of applied variables is multiplied with app_var_mult.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double  LiteralFunWeight(Eqn_p eq,
                         double max_term_multiplier,
                         double max_literal_multiplier,
                         double pos_multiplier,
                         long vweight,
                         long flimit,
                         long *fweights,
                         long default_fweight,
                         double app_var_mult,
                         long* typefreqs)
{
   double res;

   res = EqnFunWeight(eq, max_term_multiplier, vweight, flimit,
                      fweights, default_fweight, app_var_mult,
                      typefreqs);

   if(EqnIsMaximal(eq))
   {
      res = res*max_literal_multiplier;
   }
   if(EqnIsPositive(eq))
   {
      res = res*pos_multiplier;
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: LiteralTermExtWeight()
//
//   Compute the weight of a literal as an extension of an arbitrary term
//   weight function. Modifiers are applied, several extensions are
//   supported (standard - sum literal/term weights, subterms - sum
//   weights of all subterms, or take the maximum subterm weight).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double  LiteralTermExtWeight(Eqn_p eq, TermWeightExtension_p twe)
{
   double res;

   res = EqnTermExtWeight(eq, twe);

   if (EqnIsMaximal(eq))
   {
      res *= twe->max_literal_multiplier;
   }
   if (EqnIsPositive(eq))
   {
      res *= twe->pos_eq_multiplier;
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: LiteralNonLinearWeight()
//
//   Return weight of a literal. max_term_multipler is applied to maximal
//   sides, max_literal_multiplier to maxinal literals, pos_multiplier
//   is applied to positive literals. If count_eq_encoding is true,
//   count $true and ignore the equal-predicate, otherwise ignore
//   $true and count the equal-predicate for equations only.
//   Weight of applied variables is multiplied with app_var_mult.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double  LiteralNonLinearWeight(Eqn_p eq, double max_term_multiplier,
                               double max_literal_multiplier, double
                               pos_multiplier, long vlweight, long
                               vweight, long fweight, double app_var_mult, bool
                               count_eq_encoding)
{
   double res;

   res = count_eq_encoding?
      EqnNonLinearWeight(eq, max_term_multiplier, vlweight, vweight,
                         fweight, app_var_mult):
      EqnCorrectedNonLinearWeight(eq, max_term_multiplier, vlweight,
                                  vweight, fweight, app_var_mult);


   if(EqnIsMaximal(eq))
   {
      res = res*max_literal_multiplier;
   }
   if(EqnIsPositive(eq))
   {
      res = res*pos_multiplier;
   }

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: LiteralSymTypeWeight()
//
//   Return weight of a literal. max_term_multipler is applied to maximal
//   sides, max_literal_multiplier to maxinal literals, pos_multiplier
//   is applied to positive literals. Different weights are used for
//   predicate symbols, constants, function symbols and variables.
//   Weight of applied variables is multiplied with app_var_mult.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double LiteralSymTypeWeight(Eqn_p eq, double max_term_multiplier,
                            double max_literal_multiplier, double
                            pos_multiplier, long vweight, long
                            fweight, long cweight, long pweight,
                            double app_var_mult)
{
   double res;

   res = EqnSymTypeWeight(eq, max_term_multiplier, vweight, fweight,
                          cweight, pweight, app_var_mult);


   if(EqnIsMaximal(eq))
   {
      res = res*max_literal_multiplier;
   }
   if(EqnIsPositive(eq))
   {
      res = res*pos_multiplier;
   }

   return res;
}



/*-----------------------------------------------------------------------
//
// Function: LiteralCompareFun()
//
//   Comparison function for technical stuff, i.e. trees and so on.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int LiteralCompareFun(Eqn_p lit1, Eqn_p lit2)
{
   int cmpres;

   if(EqnIsPositive(lit1) && !EqnIsPositive(lit2))
   {
      return 1;
   }
   else if(!EqnIsPositive(lit1) && EqnIsPositive(lit2))
   {
      return -1;
   }
   cmpres = PCmp(MAX(lit1->lterm, lit1->rterm),
                 MAX(lit2->lterm, lit2->rterm));

   if(cmpres)
   {
      return cmpres;
   }
   return PCmp(MIN(lit1->lterm, lit1->rterm),
               MIN(lit2->lterm, lit2->rterm));
}


/*-----------------------------------------------------------------------
//
// Function: EqnAddSymbolFeatures()
//
//   Add symbol features to the feature array.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void EqnAddSymbolFeatures(Eqn_p eq, PStack_p mod_stack, long *feature_array)
{
   long offset = EqnIsNegative(eq)?2:0;

   TermAddSymbolFeatures(eq->lterm, mod_stack, 0, feature_array, offset);
   TermAddSymbolFeatures(eq->rterm, mod_stack, 0, feature_array, offset);
}


/*-----------------------------------------------------------------------
//
// Function: EqnCollectSubterms()
//
//   Collect all subterms of eqn onto collector. Assumes that
//   TPOpFlag is set if and only if the term is already in the
//   collection. Returns the number of new terms found.
//
// Global Variables: -
//
// Side Effects    : Sets the OpFlag of newly collected terms.
//
/----------------------------------------------------------------------*/

long EqnCollectSubterms(Eqn_p eqn, PStack_p collector)
{
   long res = 0;

   assert(eqn);
   assert(collector);

   res += TBTermCollectSubterms(eqn->lterm, collector);
   res += TBTermCollectSubterms(eqn->rterm, collector);

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: EqnHasAppVar()
//
//   Does eq have an applied variable at any side?
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool EqnHasAppVar(Eqn_p eq)
{
   assert(eq && eq->lterm && eq->rterm);
   return TermIsAppliedFreeVar(eq->lterm) || TermIsAppliedFreeVar(eq->rterm);
}

/*-----------------------------------------------------------------------
//
// Function: EqnListMapTerms()
//
//   Map all terms in the equation list using f.
//
// Global Variables: -
//
// Side Effects    : Sets the OpFlag of newly collected terms.
//
/----------------------------------------------------------------------*/

void EqnMap(Eqn_p lit, TermMapper_p f, void* arg)
{
   Term_p lterm = f(arg, lit->lterm);
   Term_p rterm = f(arg, lit->rterm);
   DBGTermCheckUnownedSubterm(stdout, lterm, "EqnMapL1");
   DBGTermCheckUnownedSubterm(stdout, rterm, "EqnMapL2");
   bool negate = false;
   TB_p bank = lit->bank;

   if(lterm == bank->false_term)
   {
      lterm = bank->true_term;
      negate = !negate;
   }
   if(rterm == bank->false_term)
   {
      rterm = bank->true_term;
      negate = !negate;
   }
   if(lterm == bank->true_term)
   {
      SWAP(lterm, rterm);
   }
   if(rterm != bank->true_term)
   {
      EqnSetProp(lit, EPIsEquLiteral);
   }
   else
   {
      EqnDelProp(lit, EPIsEquLiteral);
   }

   if(negate)
   {
      EqnFlipProp(lit, EPIsPositive);
   }

   if(lterm != lit->lterm)
   {
      EqnDelProp(lit, EPMaxIsUpToDate);
      EqnDelProp(lit, EPIsOriented);
   }

   lit->lterm = lterm;
   lit->rterm = rterm;
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
