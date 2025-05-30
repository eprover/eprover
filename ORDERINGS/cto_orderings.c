/*-----------------------------------------------------------------------

  File  : cto_orderings.c

  Author: Stephan Schulz

  Contents

  Interface to the ordering module

  Copyright 1998, 1999, 2024 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Mon May  4 23:24:41 MET DST 1998

-----------------------------------------------------------------------*/

#include "cto_orderings.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: TOGreater()
//
//   Test wether t1 is greater that t2 in the ordering described by
//   the ocb.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool TOGreater(OCB_p ocb, Term_p s, Term_p t, DerefType deref_s,
               DerefType deref_t)
{
   bool res = false;
   /* Term_p tmp; */

   assert(ocb);
   assert(s);
   assert(t);
   /* OCBDebugPrint(stdout, ocb); */
   /* printf("TOGreater...\n");
      TermPrint(stdout, s, ocb->sig, deref_s);
      printf(" -|- ");
      TermPrint(stdout, t, ocb->sig, deref_t);
      printf("\n");*/

   switch(ocb->type)
   {
   case LPO:
         res = LPOGreater(ocb, s, t, deref_s, deref_t);
         break;
   case LPOCopy:
         res = LPOGreaterCopy(ocb, s, t, deref_s, deref_t);
         break;
   case LPO4:
         res = LPO4Greater(ocb, s, t, deref_s, deref_t);
         break;
   case LPO4Copy:
         res = LPO4GreaterCopy(ocb, s, t, deref_s, deref_t);
         break;
   case RPO:
         assert(false && "RPO not yet implemented!");
         break;
   case KBO:
         res = KBOGreater(ocb, s, t, deref_s, deref_t);
         break;
   case KBO6:
         res = KBO6Greater(ocb, s, t, deref_s, deref_t);
         break;
   default:
         assert(false);
         break;
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: TOCompare()
//
//   Compare t1 and t2 in the ordering described by the ocb.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

CompareResult TOCompare(OCB_p ocb, Term_p s, Term_p t, DerefType deref_s,
                        DerefType deref_t)
{
   CompareResult res = to_uncomparable /* , res1 = to_uncomparable*/;
   /* Term_p tmp; */

   assert(ocb);
   assert(s);
   assert(t);

   /* printf("TOCompare...\n");
      TermPrint(stdout, s, ocb->sig, deref_s);
      printf(" -|- ");
      TermPrint(stdout, t, ocb->sig, deref_t);
      printf("\n"); */

   switch(ocb->type)
   {
   case LPO:
         res = LPOCompare(ocb, s, t, deref_s, deref_t);
         break;
   case LPOCopy:
         res = LPOCompareCopy(ocb, s, t, deref_s, deref_t);
         break;
   case LPO4:
         res = LPO4Compare(ocb, s, t, deref_s, deref_t);
         break;
   case LPO4Copy:
         res = LPO4CompareCopy(ocb, s, t, deref_s, deref_t);
         break;
   case RPO:
         assert(false && "RPO not yet implemented!");
         break;
   case KBO:
         assert(problemType != PROBLEM_HO);
         res = KBOCompare(ocb, s, t, deref_s, deref_t);
         break;
   case KBO6:
         res = KBO6Compare(ocb, s, t, deref_s, deref_t);
         break;
   case EMPTY:
         res  = to_uncomparable;
         break;
   default:
         assert(false);
         break;
   }
   /* printf("...TOCompare (%d)\n", res);  */
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: TOCompareSymbolParse()
//
//   Parse a symbol (>, <, =) and return the corresponding comparion
//   result code.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

CompareResult TOCompareSymbolParse(Scanner_p in)
{
   CompareResult res;

   CheckInpTok(in, LesserSign|GreaterSign|EqualSign);
   switch(AktToken(in)->tok)
   {
   case LesserSign:
         res = to_lesser;
         break;
   case GreaterSign:
         res = to_greater;
         break;
   case EqualSign:
         res = to_equal;
         break;
   default:
         assert(false);
         res = to_uncomparable;
         break;
   }
   NextToken(in);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: TOSymbolComparisonChainParse()
//
//   Parse a chain of precedence constraints (e.g. f > g = h < a) and
//   insert the constraints into ocb. Return new OCB status pointer.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

PStackPointer TOSymbolComparisonChainParse(Scanner_p in, OCB_p ocb)
{
   FunCode       f1, f2;
   int           line1, column1, line2, column2;
   StreamType    type1, type2;
   DStr_p        source1, source2;
   CompareResult relation;
   PStackPointer ocb_state = OCBPrecedenceGetState(ocb);

   line1   = AktToken(in)->line;
   column1 = AktToken(in)->column;
   source1 = DStrGetRef(AktToken(in)->source);
   type1   = AktToken(in)->stream_type;
   f1      = SigParseKnownOperator(in, ocb->sig);

   while(TestInpTok(in,LesserSign|GreaterSign|EqualSign))
   {
      relation = TOCompareSymbolParse(in);

      line2    = AktToken(in)->line;
      column2 = AktToken(in)->column;
      source2 = DStrGetRef(AktToken(in)->source);
      type2   = AktToken(in)->stream_type;
      f2      = SigParseKnownOperator(in, ocb->sig);

      ocb_state = OCBPrecedenceAddTuple(ocb, f1, f2, relation);

      if(!ocb_state)
      {
         Error("%s Precedence incompatible with previous ordering",
               INPUT_SEMANTIC_ERROR,
               PosRep(type1, source1, line1, column1));
      }
      DStrReleaseRef(source1);
      line1   = line2;
      column1 = column2;
      source1 = DStrGetRef(source2);
      DStrReleaseRef(source2);
      type1   = type2;
      f1      = f2;
   }
   DStrReleaseRef(source1);

   return ocb_state;
}


/*-----------------------------------------------------------------------
//
// Function: TOPrecedenceParse()
//
//   Parse a precedence (list of precedence chains).
//
// Global Variables: -
//
// Side Effects    : Changes OCB, reads input
//
/----------------------------------------------------------------------*/

PStackPointer TOPrecedenceParse(Scanner_p in, OCB_p ocb)
{
   PStackPointer res;

   assert(ocb);
   assert(ocb->sig_size == ocb->sig->f_count);
   assert(ocb->precedence);

   res = OCBPrecedenceGetState(ocb);
   if(TestInpTok(in, Identifier))
   {
      res = TOSymbolComparisonChainParse(in, ocb);
      while(TestInpTok(in, Comma))
      {
         AcceptInpTok(in, Comma);
         res = TOSymbolComparisonChainParse(in, ocb);
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: TOSymbolWeightParse()
//
//   Parse a f:w declaration.
//
// Global Variables: -
//
// Side Effects    : Input, changes OCB, may generate error.
//
/----------------------------------------------------------------------*/

void TOSymbolWeightParse(Scanner_p in, OCB_p ocb)
{
   FunCode       f;
   long          weight;

   f = SigParseKnownOperator(in, ocb->sig);
   AcceptInpTok(in, Colon);
   weight = AktToken(in)->numval;
   AcceptInpTok(in, PosInt);

   *OCBFunWeightPos(ocb, f) = weight*W_DEFAULT_WEIGHT;
}


/*-----------------------------------------------------------------------
//
// Function: TOWeightsParse()
//
//   Parse a list of weight assignments. Return number of assignments
//   parsed.
//
// Global Variables: -
//
// Side Effects    : Changes OCB, reads input
//
/----------------------------------------------------------------------*/

long TOWeightsParse(Scanner_p in, OCB_p ocb)
{
   long res = 0;

   assert(ocb);
   assert(ocb->sig_size == ocb->sig->f_count);

   if(TestInpTok(in, Identifier))
   {
      TOSymbolWeightParse(in, ocb);
      res++;
      while(TestInpTok(in, Comma))
      {
         AcceptInpTok(in, Comma);
         TOSymbolWeightParse(in, ocb);
         res++;
      }
   }
   return res;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
