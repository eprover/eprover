/*-----------------------------------------------------------------------

  File  : cle_clauseenc.c

  Author: Stephan Schulz

  Contents

  Functions for dealing with term representations of clauses.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Tue Jul 13 12:52:35 MET DST 1999

  -----------------------------------------------------------------------*/

#include "cle_clauseenc.h"



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
// Function: FlatEncodeClauseListRep()
//
//   Take a PStack wich describes a clause in a given order and
//   compute the flat term representation of it.
//
// Global Variables: -
//
// Side Effects    : Changes termbank
//
/----------------------------------------------------------------------*/

Term_p FlatEncodeClauseListRep(TB_p bank, PStack_p list)
{
   int arity;
   PStackPointer i;
   Term_p handle;
   Eqn_p current;
   PatEqnDirection dir;

   arity = PStackGetSP(list)/2;

   handle = TermDefaultCellArityAlloc(arity);
   handle->arity = arity;
   handle->f_code = SigGetOrNCode(bank->sig, arity);
   assert(handle->f_code);
   if(arity)
   {
      for(i=0; i<2*arity; i+=2)
      {
         current = PStackElementP(list, i);
         dir     = PStackElementInt(list, i+1);
         handle->args[i/2] = EqnTBTermEncode(current,dir);
      }
   }
   handle = TBTermTopInsert(bank, handle);
   assert(handle->weight ==
          TermWeight(handle,DEFAULT_VWEIGHT,DEFAULT_FWEIGHT));

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: RecEncodeClauseListRep()
//
//   Take a PStack wich describes a clause in a given order and
//   compute the recursive term representation of it.
//
// Global Variables: -
//
// Side Effects    : Changes termbank
//
/----------------------------------------------------------------------*/

Term_p   RecEncodeClauseListRep(TB_p bank, PStack_p list)
{
   int arity;
   PStackPointer i;
   Term_p handle, rest;
   Eqn_p current;
   PatEqnDirection dir;

   arity = PStackGetSP(list)/2;

   rest = TermDefaultCellAlloc();
   rest->arity = 0;
   rest->f_code = SigGetCNilCode(bank->sig);
   assert(rest->f_code);
   rest = TBTermTopInsert(bank, rest);

   while(arity)
   {
      arity--;
      i = 2* arity;
      current = PStackElementP(list, i);
      dir     = PStackElementInt(list, i+1);
      handle = TermDefaultCellArityAlloc(2);
      handle->arity = 2;
      handle->f_code = SigGetOrCode(bank->sig);
      handle->args[1] = rest;
      handle->args[0] = EqnTBTermEncode(current,dir);
      rest = TBTermTopInsert(bank, handle);

   }
   assert(rest->weight ==
          TermWeight(rest,DEFAULT_VWEIGHT,DEFAULT_FWEIGHT));
   return rest;
}


/*-----------------------------------------------------------------------
//
// Function: TermEncodeEqnList()
//
//   Encode an eqnlist (as might be parsed with EqnListParse()) as a
//   term.
//
// Global Variables: -
//
// Side Effects    : As for FlatEncodeClauseListRep(), +Memory
//                   operations.
//
/----------------------------------------------------------------------*/

Term_p TermEncodeEqnList(TB_p bank, Eqn_p list, bool flat)
{
   PStack_p stack = PStackAlloc();
   Term_p   res;

   while(list)
   {
      PStackPushP(stack, list);
      PStackPushInt(stack, PENormal);
      list = list->next;
   }
   res = flat?
      FlatEncodeClauseListRep(bank, stack):
      RecEncodeClauseListRep(bank, stack);
   PStackFree(stack);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: FlatRecodeRecClauseRep()
//
//   Take a recursive clause encoding and generate a corresponding
//   flat one. This is a simple, not a particularly efficient
//   implementation.
//
// Global Variables: -
//
// Side Effects    : Creates new term bank.
//
/----------------------------------------------------------------------*/

Term_p FlatRecodeRecClauseRep(TB_p bank,Term_p clauserep)
{
   PStack_p stack = PStackAlloc();
   Term_p   res;
   Eqn_p    eqn;
   bool     positive = true;

   assert(bank);
   assert(clauserep);

   while(clauserep->f_code == SigGetOrCode(bank->sig))
   {
      if(clauserep->args[0]->f_code == SigGetEqnCode(bank->sig, true))
      {
         positive = true;
      }
      else if(clauserep->args[0]->f_code == SigGetEqnCode(bank->sig,
                                                          false))
      {
         positive = false;
      }
      else
      {
         Error("Term is not a correct recursive clause encoding!",
               SYNTAX_ERROR);
      }
      eqn = EqnAlloc(clauserep->args[0]->args[0],
                     clauserep->args[0]->args[1],
                     bank, positive);
      PStackPushP(stack, eqn);
      PStackPushInt(stack, PENormal);
      clauserep = clauserep->args[1];
   }
   if(clauserep->f_code != bank->sig->cnil_code)
   {
      Error("Term is not a correct recursive clause encoding!",
            SYNTAX_ERROR);
   }
   res = FlatEncodeClauseListRep(bank, stack);
   while(!PStackEmpty(stack))
   {
      PStackDiscardTop(stack);
      assert(!PStackEmpty(stack));
      EqnFree(PStackPopP(stack));
   }
   PStackFree(stack);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ParseClauseTermRep()
//
//   Parse a clause representation in literal list format (other
//   formats are unsuitable because literal order matters!) and return
//   a term representation of it.
//
// Global Variables: -
//
// Side Effects    : Read input, Memory operations, changes term
//                   bank.
//
/----------------------------------------------------------------------*/

Term_p ParseClauseTermRep(Scanner_p in, TB_p bank, bool flat)
{
   Eqn_p list;
   Term_p res;

   assert(ScannerGetFormat(in)==LOPFormat);
   list = EqnListParse(in, bank, Semicolon);
   AcceptInpTok(in, LesserSign);
   AcceptInpTokNoSkip(in, Hyphen);
   AcceptInpTok(in, Fullstop);
   res = TermEncodeEqnList(bank, list, flat);
   EqnListFree(list);
   return res;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
