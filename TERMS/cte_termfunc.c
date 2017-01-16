/*-----------------------------------------------------------------------

File  : cte_termfunc.c

Author: Stephan Schulz

Contents

  Most of the user-level functionality for unshared terms.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed Feb 25 16:50:36 MET 1998
    Ripped from cte_terms.c (should be obsolete by now)

-----------------------------------------------------------------------*/


#include "cte_termfunc.h"
#include "cte_typecheck.h"
#include "clb_plocalstacks.h"


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

bool      TermPrintLists = true;
bool      TermPrintTypes = false;

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: print_cons_list()
//
//   Print a list of $cons'ed terms, terminated with $nil. Abort on
//   not well-formed lists (no cons pairs!).
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void print_cons_list(FILE* out, Term_p list, Sig_p sig, DerefType deref)
{
   assert(SigSupportLists);
   putc('[', out);
   if(list->f_code == SIG_CONS_CODE)
   {
      assert(list->args);
      TermPrint(out, list->args[0], sig, deref);
      list = list->args[1];
      while(list->f_code == SIG_CONS_CODE)
      {
         putc(',', out);
         /* putc(' ', out); */
         assert(list->args);
         TermPrint(out, list->args[0], sig, deref);
         list = list->args[1];
      }
      assert(list->f_code == SIG_NIL_CODE);
   }
   putc(']', out);
}


/*-----------------------------------------------------------------------
//
// Function: parse_cons_list()
//
//   Parse a LOP list into an internal $cons list.
//
// Global Variables: -
//
// Side Effects    : Input, Memory operations
//
/----------------------------------------------------------------------*/

static Term_p parse_cons_list(Scanner_p in, Sig_p sig, VarBank_p vars)
{
   Term_p handle;
   Term_p current;

   assert(SigSupportLists);

   AcceptInpTok(in, OpenSquare);

   handle = TermDefaultCellAlloc();

   current = handle;

   if(!TestInpTok(in, CloseSquare))
   {

      current->f_code = SIG_CONS_CODE;
      current->arity = 2;
      current->sort = SigDefaultSort(sig);
      current->args = TermArgArrayAlloc(2);
      current->args[0] = TermParse(in, sig, vars);
      current->args[1] = TermDefaultCellAlloc();
      current = current->args[1];

      while(TestInpTok(in, Comma))
      {
         NextToken(in);
         current->f_code = SIG_CONS_CODE;
         current->arity = 2;
         current->sort = SigDefaultSort(sig);
         current->args = TermArgArrayAlloc(2);
         current->args[0] = TermParse(in, sig, vars);
         TermCellDelProp(current->args[0], TPTopPos);
         current->args[1] = TermDefaultCellAlloc();
         current = current->args[1];
      }
   }
   AcceptInpTok(in, CloseSquare);
   current->f_code = SIG_NIL_CODE;

   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: term_check_consistency_rek()
//
//   Traverse a tree and check if any one term cell occurs more than
//   once on any branch (which would make the term cyclic). Return the
//   first inconsistency found or NULL.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static Term_p term_check_consistency_rek(Term_p term, PTree_p *branch,
                                         DerefType deref)
{
   int      i;
   Term_p   res = NULL;

   term = TermDeref(term, &deref);
   putc('.', stdout);

   if(!PTreeStore(branch, term))
   {
      return term;
   }
   for(i=0; i<term->arity; i++)
   {
      if((res = term_check_consistency_rek(term->args[i], branch, deref)))
      {
         break;
      }
   }
   PTreeDeleteEntry(branch, term);
   return res;
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: VarPrint()
//
//   Print a variable with FunCode var out.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void VarPrint(FILE* out, FunCode var)
{
   char id;

   assert(var<0);

   id = 'X';
   if(var%2)
   {
      id = 'Y';
   }
   fprintf(out, "%c%ld", id, -((var-1)/2));
}


/*-----------------------------------------------------------------------
//
// Function: TermPrint()
//
//   Print a term to the given stream.
//
// Global Variables: TermPrintLists
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void TermPrint(FILE* out, Term_p term, Sig_p sig, DerefType deref)
{
   assert(term);
   assert(sig||TermIsVar(term));

   term = TermDeref(term, &deref);

#ifdef NEVER_DEFINED
   if(TermCellQueryProp(term, TPRestricted))
   {
      fprintf(out, "*");
   }
   if(TermCellQueryProp(term, TPIsRewritten))
   {
      if(TermIsTopRewritten(term))
      {
         fprintf(out, "=");
      }
      else
      {
         fprintf(out, "+");
      }
   }
#endif
   if(SigSupportLists && TermPrintLists &&
      ((term->f_code == SIG_NIL_CODE)||
       (term->f_code == SIG_CONS_CODE)))
   {
      print_cons_list(out, term, sig, deref);
   }
   else
   {
      if(TermIsVar(term))
      {
         VarPrint(out, term->f_code);
      }
      else
      {
         fputs(SigFindName(sig, term->f_code), out);
         if(!TermIsConst(term))
         {
            assert(term->args);
            TermPrintArgList(out, term->args, term->arity, sig, deref);
         }
      }
   }

   if(TermPrintTypes)
   {
      fputc(':', out);
      SortPrintTSTP(out, sig->sort_table, term->sort);
   }
}


/*--------------------------------------------------------------------
//
// Function: TermPrintArgList()
//
//   Print an argument list (i.e. an array with at least one term
//   element) to the given stream.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void TermPrintArgList(FILE* out, Term_p *args, int arity, Sig_p sig,
                      DerefType deref)
{
   int i;

   assert(arity>=1);
   putc('(', out);

   TermPrint(out, args[0], sig, deref);

   for(i=1; i<arity; i++)
   {
      putc(',', out);
      /* putc(' ', out); */
      TermPrint(out, args[i], sig, deref);
   }
   putc(')', out);
}


/*-----------------------------------------------------------------------
//
// Function: TermParseOperator()
//
//   Parse an operator (i.e. an optional $, followed by an
//   identifier), store the representation into id and determine
//   the type.using the following rules:
//   - If it starts with a $, it's a TermIdentInterpreted (LOP global
//     variables are treated as interpreted constants).
//   - If it is a PosInt, it is a TermIdentNumber
//   - If its a String, it is a TermIdentObject
//   - If it is an upper-case or underscore Ident and no opening
//     bracket follows, its a TermIdentVariable
//   - Otherwise its a free function symbol (cases are SQString and
//     Identifier starting with lower-case letter.
//
//   Return value is the type
//
// Global Variables: SigIdentStartToken
//
// Side Effects    : Read input
//
/----------------------------------------------------------------------*/

FuncSymbType TermParseOperator(Scanner_p in, DStr_p id)
{
   FuncSymbType res = FuncSymbParse(in, id);

#ifndef STRICT_TPTP
   if((isupper(DStrView(id)[0])
       ||
       (DStrView(id)[0] == '_'))
      &&
      TestInpTok(in,OpenBracket))
   {
      res = FSIdentFreeFun;
   }
#endif

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: TermSigInsert()
//
//   Thin wrapper around SigInsertId that also sets corresponding
//   properties for different identifier types.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

FunCode TermSigInsert(Sig_p sig, const char* name, int arity, bool
                      special_id, FuncSymbType type)
{
   FunCode res;

   res = SigInsertId(sig, name, arity, special_id);
   switch(type)
   {
   case FSIdentInt:
         SigSetFuncProp(sig, res, FPIsInteger);
         break;
   case FSIdentFloat:
         SigSetFuncProp(sig, res, FPIsFloat);
         break;
   case FSIdentRational:
         SigSetFuncProp(sig, res, FPIsRational);
         break;
   case FSIdentObject:
         SigSetFuncProp(sig, res, FPIsObject);
         break;
   case FSIdentInterpreted:
         SigSetFuncProp(sig, res, FPInterpreted);
         break;
   default:
         /* Nothing */
         break;
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: TermParse()
//
//   Parse a term from the given scanner object into the internal term
//   representation.
//
// Global Variables: -
//
// Side Effects    : Input, memory operations, may extend signature
//
/----------------------------------------------------------------------*/

Term_p TermParse(Scanner_p in, Sig_p sig, VarBank_p vars)
{
   Term_p        handle;
   DStr_p        id;
   FuncSymbType id_type;
   DStr_p        source_name, errpos;
   SortType      sort;
   long          line, column;
   StreamType    type;

   if(SigSupportLists && TestInpTok(in, OpenSquare))
   {
      handle =  parse_cons_list(in, sig, vars);
   }
   else
   {
      id = DStrAlloc();
      line = AktToken(in)->line;
      column = AktToken(in)->column;
      source_name = DStrGetRef(AktToken(in)->source);
      type = AktToken(in)->stream_type;

      if((id_type = TermParseOperator(in, id))==FSIdentVar)
      {
         /* A variable may be annotated with a sort */
         if(TestInpTok(in, Colon))
         {
            AcceptInpTok(in, Colon);
            sort = SortParseTSTP(in, vars->sort_table);
            handle = VarBankExtNameAssertAllocSort(vars,
                                                   DStrView(id), sort);
         }
         else
         {
            handle = VarBankExtNameAssertAlloc(vars, DStrView(id));
         }
      }
      else
      {
         handle = TermDefaultCellAlloc();

         if(TestInpTok(in, OpenBracket))
         {
            if((id_type == FSIdentInt)
               &&(sig->distinct_props & FPIsInteger))
            {
               AktTokenError(in,
                             "Number cannot have argument list (consider --free-numbers)",
                             false);
            }
            if((id_type == FSIdentObject)
               &&(sig->distinct_props & FPIsObject))
            {
               AktTokenError(in,
                             "Object cannot have argument list (consider --free-objects)",
                             false);
            }

            handle->arity = TermParseArgList(in, &(handle->args), sig,
                                             vars);
         }
         else
         {
            handle->arity = 0;
         }
         handle->f_code = TermSigInsert(sig, DStrView(id),
                                        handle->arity, false, id_type);
         if(!handle->f_code)
         {
            errpos = DStrAlloc();

            DStrAppendStr(errpos, PosRep(type, source_name, line, column));
            DStrAppendChar(errpos, ' ');
            DStrAppendStr(errpos, DStrView(id));
            DStrAppendStr(errpos, " used with arity ");
            DStrAppendInt(errpos, (long)handle->arity);
            DStrAppendStr(errpos, " but registered with arity ");
            DStrAppendInt(errpos,
                          (long)SigFindArity(sig, SigFindFCode(sig, DStrView(id))));
            Error(DStrView(errpos), SYNTAX_ERROR);
            DStrFree(errpos);
         }
      }
      DStrReleaseRef(source_name);
      DStrFree(id);
   }
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: TermParseArgList()
//
//   Parse a list of terms (comma-separated and enclosed in brackets)
//   into an array of term pointers. Return the number of terms
//   parsed, and make arg_anchor point to the array. Note: The array
//   has to have exactly the right size, as it will be handled by
//   Size[Malloc|Free] for efficiency reasons and may otherwise lead
//   to a memory leak. This leads to some complexity...
//   If the arglist is empty, return 0 and use the NULL pointer as
//   anchor.
//
// Global Variables: -
//
// Side Effects    : Input, memory operations
//
/----------------------------------------------------------------------*/

int TermParseArgList(Scanner_p in, Term_p** arg_anchor, Sig_p sig,
                     VarBank_p vars)
{
   Term_p *handle;
   int    arity;
   int    size;
   int    i;

   AcceptInpTok(in, OpenBracket);
   if(TestInpTok(in, CloseBracket))
   {
      NextToken(in);
      *arg_anchor = NULL;
      return 0;
   }
   size = TERMS_INITIAL_ARGS;
   handle = (Term_p*)SizeMalloc(size*sizeof(Term_p));
   arity = 0;
   handle[arity] = TermParse(in, sig, vars);
   arity++;
   while(TestInpTok(in, Comma))
   {
      NextToken(in);
      if(arity==size)
      {
         size+=TERMS_INITIAL_ARGS;
         handle = (Term_p*)SecureRealloc(handle, size*sizeof(Term_p));
      }
      handle[arity] = TermParse(in, sig, vars);
      arity++;
   }
   AcceptInpTok(in, CloseBracket);
   *arg_anchor = TermArgArrayAlloc(arity);
   for(i=0;i<arity;i++)
   {
      (*arg_anchor)[i] = handle[i];
   }
   SizeFree(handle, size*sizeof(Term_p));

   return arity;
}


/*-----------------------------------------------------------------------
//
// Function: TermCopy()
//
//   Return a copy of a given term. The new term will be unshared
//   (except, of coure, for the variables) even if the original term
//   was shared. Variable cells will be allocated from the VarBank
//   given to the function.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Term_p TermCopy(Term_p source, VarBank_p vars, DerefType deref)
{
   Term_p handle;
   int i;

   assert(source);

   source = TermDeref(source, &deref);

   if(TermIsVar(source))
   {
      handle = VarBankVarAssertAlloc(vars, source->f_code, source->sort);
   }
   else
   {
      handle = TermTopCopyWithoutArgs(source);

      for(i=0; i<handle->arity; i++)
      {
         handle->args[i] = TermCopy(source->args[i], vars, deref);
      }
   }

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: TermCopyKeepVars()
//
//   Return a copy of a given term. The new term will be unshared
//   (except, of coure, for the variables) even if the original term
//   was shared. Variable cells will not be copied. Note that printing
//   such a term might be confusing, since two variables with the same
//   f_code may indeed be different!
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Term_p TermCopyKeepVars(Term_p source, DerefType deref)
{
   Term_p handle;
   int i;

   assert(source);

   source = TermDeref(source, &deref);

   if(TermIsVar(source))
   {
      return source;
   }

   handle = TermTopCopy(source);

   for(i=0; i<handle->arity; i++) /* Hack: Loop will not be entered if
                                     arity = 0 */
   {
      handle->args[i] = TermCopyKeepVars(handle->args[i], deref);
   }
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: TermStructEqual()
//
//   Return true if the two terms have the same structure. Follows
//   bindings.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/


bool TermStructEqual(Term_p t1, Term_p t2)
{
   t1 = TermDerefAlways(t1);
   t2 = TermDerefAlways(t2);

   if(t1==t2)
   {
      return true;
   }

   if(t1->f_code != t2->f_code)
   {
      return false;
   }

   assert(t1->sort == t2->sort);
   assert(t1->arity == t2->arity);
   for(int i=0; i<t1->arity; i++)
   {
      if(!TermStructEqual(t1->args[i], t2->args[i]))
      {
         return false;
      }
   }
   return true;
}



/*-----------------------------------------------------------------------
//
// Function: TermStructEqualNoDeref()
//
//   Return true if the two terms have the same structures. Ignores
//   bindings.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/


bool TermStructEqualNoDeref(Term_p t1, Term_p t2)
{
   if(t1==t2)
   {
      return true;
   }

   if(t1->f_code != t2->f_code)
   {
      return false;
   }

   assert(t1->sort == t2->sort);
   assert(t1->arity == t2->arity);
   for(int i=0; i<t1->arity; i++)
   {
      if(!TermStructEqualNoDeref(t1->args[i], t2->args[i]))
      {
         return false;
      }
   }
   return true;
}

/*-----------------------------------------------------------------------
//
// Function: TermStructEqualDeref()
//
//   Return true if the two terms have the same
//   structures. Dereference both terms as designated by deref_1,
//   deref_2.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool TermStructEqualDeref(Term_p t1, Term_p t2, DerefType deref_1, DerefType deref_2)
{

   t1 = TermDeref(t1, &deref_1);
   t2 = TermDeref(t2, &deref_2);

   if((t1==t2) && (deref_1==deref_2))
   {
      return true;
   }

   if(t1->f_code != t2->f_code)
   {
      return false;
   }

   assert(t1->sort == t2->sort);
   assert(t1->arity == t2->arity);
   for(int i=0; i<t1->arity; i++)
   {
      if(!TermStructEqualDeref(t1->args[i], t2->args[i], deref_1, deref_2))
      {
         return false;
      }
   }
   return true;
}

/*-----------------------------------------------------------------------
//
// Function: TermStructWeightCompare()
//
//   Compare two terms based on just structural criteria: First
//   compare standard-weight, then compare top-symbol arity, then
//   compare subterms lexicographically. $true is always minimal.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long TermStructWeightCompare(Term_p t1, Term_p t2)
{
   assert(t1);
   assert(t2);

   if(t1->f_code == SIG_TRUE_CODE)
   {
      assert(t1->arity == 0);
      if(t2->f_code == SIG_TRUE_CODE)
      {
         assert(t2->arity == 0);
         return 0;
      }
      return -1;
   }

   if(t2->f_code == SIG_TRUE_CODE)
   {
      assert(t2->arity == 0);
      return 1;
   }

   long res = TermStandardWeight(t1) - TermStandardWeight(t2);
   if(res)
   {
      return res;
   }

   if(TermIsVar(t1))
   { /* Then t2 also is a variable due to equal weights! */
      assert(TermIsVar(t2));
      return t1->sort - t2->sort;
   }

   res = t1->arity - t2->arity;
   if(res)
   {
      return res;
   }

   assert(t1->sort == t2->sort);
   assert(t1->arity == t2->arity);
   for(int i=0; i<t1->arity; i++)
   {
      res = TermStructWeightCompare(t1->args[i], t2->args[i]);
      if(res)
      {
         return res;
      }
   }

   return 0;
}


/*-----------------------------------------------------------------------
//
// Function: TermLexCompare()
//
//   Compare two terms lexicographically by f_codes.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long TermLexCompare(Term_p t1, Term_p t2)
{
   int i;

   long res = t1->f_code - t2->f_code;
   if(res)
   {
      return res;
   }

   assert(t1->sort == t2->sort);
   assert(t1->arity == t2->arity);
   for(i=0; i<t1->arity; i++)
   {
      res = TermLexCompare(t1->args[i], t2->args[i]);
      if(res)
      {
         return res;
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: TermIsSubterm()
//
//   Return true if test is a subterm to super.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool TermIsSubterm(Term_p super, Term_p test, DerefType deref)
{
   int i;

   super = TermDeref(super, &deref);

   if(super == test)
   {
      return true;
   }
   for(i=0; i<super->arity; i++)
   {
      if(TermIsSubterm(super->args[i], test, deref))
      {
         return true;
      }
   }
   return false;
}


/*-----------------------------------------------------------------------
//
// Function: TermIsSubtermDeref()
//
//   Return true if test is a subterm to super. Uses
//   TermStructEqualDeref() for equal test.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool TermIsSubtermDeref(Term_p super, Term_p test, DerefType
                        deref_super, DerefType deref_test)
{
   int i;

   super = TermDeref(super, &deref_super);
   if(TermStructEqualDeref(super, test, deref_super, deref_test))
   {
      return true;
   }
   for(i=0; i<super->arity; i++)
   {
      if(TermIsSubtermDeref(super->args[i], test, deref_super, deref_test))
      {
         return true;
      }
   }
   return false;
}


/*-----------------------------------------------------------------------
//
// Function: TermWeightCompute()
//
//   Compute the weight of a term, counting variables as vweight and
//   function symbols as fweight.
//
// Global Variables: -
//
// Side Effects    : Memory operations for the stack used.
//
/----------------------------------------------------------------------*/

long TermWeightCompute(Term_p term, long vweight, long fweight)
{
   long res = 0;

   if(TermIsVar(term))
   {
      res += vweight;
   }
   else
   {
      res += fweight;
      for(int i=0; i<term->arity; i++)
      {
         res += TermWeight(term->args[i], vweight, fweight);
      }
   }

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: TermFsumWeight()
//
//   Return a weighted sum of the function symbols weights (and
//   variable weights) in the term.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

long TermFsumWeight(Term_p term, long vweight, long flimit,
                    long *fweights, long default_fweight)
{
   long res = 0;

   if(TermIsVar(term))
   {
      res += vweight;
   }
   else
   {
      if(term->f_code < flimit)
      {
         res += fweights[term->f_code];
      }
      else
      {
         res += default_fweight;
      }

      for(int i = 0; i < term->arity; i++)
      {
         res += TermFsumWeight(term->args[i], vweight, flimit, fweights, default_fweight);
      }
   }

   return res;

}



/*-----------------------------------------------------------------------
//
// Function: TermNonLinearWeight()
//
//   Compute the weight of a term, counting variables that occur for
//   the first time as vlweight, varaibes that reoccur as vweight, and
//   function symbols as fweight.
//
// Global Variables: -
//
// Side Effects    : Memory operations for the stack used.
//
/----------------------------------------------------------------------*/

long TermNonLinearWeight(Term_p term, long vlweight, long vweight,
                         long fweight)
{
   long     res = 0;
   PStack_p stack = PStackAlloc();
   Term_p   handle;

   assert(term);

   TermDelProp(term, DEREF_NEVER, TPOpFlag);

   PStackPushP(stack, term);

   while(!PStackEmpty(stack))
   {
      handle = PStackPopP(stack);
      if(TermIsVar(handle))
      {
         if(TermCellQueryProp(handle, TPOpFlag))
         {
            res += vweight;
         }
         else
         {
            TermCellSetProp(handle, TPOpFlag);
            res += vlweight;
         }
      }
      else
      {
         int i;

         res += fweight;

         for(i=0; i<handle->arity; i++)
         {
            PStackPushP(stack, handle->args[i]);
         }
      }
   }
   PStackFree(stack);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: TermSymTypeWeight()
//
//   Compute the weight of a term, giving different weight to
//   variables, constants, function symbols and predicates.
//
// Global Variables: -
//
// Side Effects    : Memory operations for the stack.
//
/----------------------------------------------------------------------*/

long TermSymTypeWeight(Term_p term, long vweight, long fweight, long
                       cweight, long pweight)
{
   long     res = 0;
   PStack_p stack = PStackAlloc();
   Term_p   handle;

   assert(term);

   PStackPushP(stack, term);

   while(!PStackEmpty(stack))
   {
      handle = PStackPopP(stack);
      if(TermIsVar(handle))
      {
         res += vweight;
      }
      else
      {
         int i;

         if(TermCellQueryProp(handle,TPPredPos))
         {
            res += pweight;
         }
         else if(handle->arity==0)
         {
            res += cweight;
         }
         else
         {
            res += fweight;
         }
         for(i=0; i<handle->arity; i++)
         {
            PStackPushP(stack, handle->args[i]);
         }
      }
   }
   PStackFree(stack);

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: TermDepth()
//
//   Return the depth of a term.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long TermDepth(Term_p term)
{
   long maxdepth = 0, ldepth;
   int  i;

   for(i=0; i<term->arity; i++)
   {
      ldepth = TermDepth(term->args[i]);
      maxdepth = MAX(maxdepth, ldepth);
   }
   return maxdepth+1;
}

/*-----------------------------------------------------------------------
//
// Function: TermIsDefTerm()
//
//   Return true if t is of the form f(X1...Xn) with n>=arity.
//
// Global Variables: -
//
// Side Effects    : Sets TPOpFlag
//
/----------------------------------------------------------------------*/

bool TermIsDefTerm(Term_p term, int min_arity)
{
   int i;

   assert(term);

   if(TermIsVar(term))
   {
      return false;
   }
   if(term->arity<min_arity)
   {
      return false;
   }
   if(TermStandardWeight(term)!=(DEFAULT_FWEIGHT+term->arity*DEFAULT_VWEIGHT))
   {
      return false;
   }
   for(i=0; i<term->arity; i++)
   {
      TermCellDelProp(term->args[i], TPOpFlag);
   }
   for(i=0; i<term->arity; i++)
   {
      if(TermCellQueryProp(term->args[i], TPOpFlag))
      {
         return false;
      }
      TermCellSetProp(term->args[i], TPOpFlag);
   }
   return true;
}


/*-----------------------------------------------------------------------
//
// Function: TermHasFCode()
//
//   Return true if f occurs in term, false otherwise.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool TermHasFCode(Term_p term, FunCode f)
{
   int i;

   assert(term);

   if(term->f_code == f)
   {
      return true;
   }
   for(i=0; i<term->arity; i++)
   {
      if(TermHasFCode(term->args[i], f))
      {
         return true;
      }
   }
   return false;
}

/*-----------------------------------------------------------------------
//
// Function: TermHasUnboundVariables()
//
//   Return if the term contains unbound variables.
//   Does not follow bindings.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool TermHasUnboundVariables(Term_p term)
{
   bool res = false;

   if(TermIsVar(term))
   {
      if(!term->binding)
      {
         res = true;
      }
   }
   else
   {
      for(int i=0; i < term->arity; i++)
      {
         if(TermHasUnboundVariables(term->args[i]))
         {
            res = true;
            break;
         }
      }
   }

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: TermIsGroundCompute()
//
//   Return if the term contains no variables.
//   Does not follow bindings.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool TermIsGroundCompute(Term_p term)
{
   bool res = true;

   if(TermIsVar(term))
   {
      res = false;
   }
   else
   {
      for(int i=0; i < term->arity; i++)
      {
         if(!TermIsGroundCompute(term->args[i]))
         {
            res = false;
            break;
         }
      }
   }

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: TermFindMaxVarCode()
//
//   Return largest (absolute, i.e. largest negative) f_code of any
//   variable in term.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

FunCode TermFindMaxVarCode(Term_p term)
{
   int i;
   long res, tmp;

   if(TermIsVar(term))
   {
      return term->f_code;
   }
   else
   {
      res = 0;
      for(i=0; i<term->arity; i++)
      {
         tmp = TermFindMaxVarCode(term->args[i]);
         res = MIN(res, tmp);
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: VarBankCheckBindings()
//
//   For all variables in bank, check if they are bound. If sig!=0,
//   print the variable and binding as a comment, otherwise just print
//   variable number. Return number of bound variables.
//
// Global Variables: -
//
// Side Effects    : Output, Memory
//
/----------------------------------------------------------------------*/

FunCode VarBankCheckBindings(FILE* out, VarBank_p bank, Sig_p sig)
{
   Term_p    term;
   VarBankStack_p stack;
   long      res = 0;
   int       i,j;

   fprintf(out, "#  VarBankCheckBindings() started...\n");
   for(i=0; i<PDArraySize(bank->stacks); i++)
   {
      stack = (VarBankStack_p) PDArrayElementP(bank->stacks, i);
      if (!stack)
      {
         continue;
      }

      for(j=0; j < PDArraySize(stack); j++)
      {
         term = PDArrayElementP(stack, j);
         if(term)
         {
            assert(TermIsVar(term));
            if(term->binding)
            {
               res++;
               if(sig)
               {
                  fprintf(out, "# %ld: ", term->f_code);
                  TermPrint(out, term, sig, DEREF_NEVER);
                  fprintf(out, " <--- ");
                  TermPrint(out, term, sig, DEREF_ONCE);
                  fprintf(out, "\n");
               }
               else
               {
                  fprintf(out, "# Var%ld <---- %p\n",
                          term->f_code,
                          (void*)term->binding);
               }
            }
         }
      }
   }
   fprintf(out, "#  ...VarBankCheckBindings() completed\n");
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: TermAddSymbolDistributionLimited()
//
//   Count occurences of function symbols with f_code<limit in
//   dist_array. Terms are not dereferenced!
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void TermAddSymbolDistributionLimited(Term_p term, long *dist_array, long limit)
{
   PStack_p stack = PStackAlloc();
   assert(term);

   PStackPushP(stack, term);

   while(!PStackEmpty(stack))
   {
      term = PStackPopP(stack);
      assert(term);

      if(!TermIsVar(term))
      {
         int i;

         assert(term->f_code > 0);
         if(term->f_code < limit)
         {
            dist_array[term->f_code]++;
         }
         for(i=0; i<term->arity; i++)
         {
            assert(term->args);
            PStackPushP(stack, term->args[i]);
         }
      }
   }
   PStackFree(stack);
}

/*-----------------------------------------------------------------------
//
// Function: TermAddSymbolDistribExist()
//
//   Compute the distribution of symbols in term. Push all occuring
//   symbols onto exists (once ;-).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void TermAddSymbolDistExist(Term_p term, long *dist_array,
                            PStack_p exists)
{
   PStack_p stack = PStackAlloc();
   assert(term);

   PStackPushP(stack, term);

   while(!PStackEmpty(stack))
   {
      term = PStackPopP(stack);
      assert(term);

      if(!TermIsVar(term))
      {
         int i;

         assert(term->f_code > 0);
         if(!dist_array[term->f_code])
         {
            PStackPushInt(exists, term->f_code);
         }
         dist_array[term->f_code]++;

         for(i=0; i<term->arity; i++)
         {
            assert(term->args);
            PStackPushP(stack, term->args[i]);
         }
      }
   }
   PStackFree(stack);
}


/*-----------------------------------------------------------------------
//
// Function: TermAddSymbolFeaturesLimited()
//
//   Add function symbol frequencies and deepest depth of a function
//   symbol to the two arrays. This is an extension of the function
//   above, this one does the extendet task in a single term
//   traversal. Note that function symbols >=limit are counted in
//   array[0] for both depth and frequency.
//
// Global Variables: -
//
// Side Effects    : Changes the arrays.
//
/----------------------------------------------------------------------*/

void TermAddSymbolFeaturesLimited(Term_p term, long depth,
                                  long *freq_array, long* depth_array,
                                  long limit)
{
   if(!TermIsVar(term))
   {
      int i;

      if(term->f_code < limit)
      {
         freq_array[term->f_code]++;
         depth_array[term->f_code] = MAX(depth, depth_array[term->f_code]);
      }
      else
      {
         freq_array[0]++;
         depth_array[0] = MAX(depth, depth_array[0]);
      }
      for(i=0; i<term->arity; i++)
      {
         TermAddSymbolFeaturesLimited(term->args[i], depth+1,
                                      freq_array, depth_array,
                                      limit);
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: TermAddSymbolFeatures()
//
//   Add function symbol frequencies and deepest depth of a function
//   symbol to the array. offset should be 0 for positive literals, 2
//   for negative literals. Thus, the 4 features for a given f
//   are stored at indices follows:
//   - 4*f_code:   |C^+|_f
//   - 4*f_code+1: d_f(C^+)
//   - 4*f_code+2: |C^-|_f
//   - 4*f_code+3: d_f(C^-)
//
// Global Variables: -
//
// Side Effects    : Changes the arrays.
//
/----------------------------------------------------------------------*/

void TermAddSymbolFeatures(Term_p term, PStack_p mod_stack, long depth,
                           long *feature_array, long offset)
{
   if(!TermIsVar(term))
   {
      int i;
      long findex = 4*term->f_code+offset;

      if(feature_array[findex] == 0)
      {
         PStackPushInt(mod_stack, findex);
      }

      feature_array[findex]++;
      feature_array[findex+1] = MAX(depth, feature_array[findex+1]);
      for(i=0; i<term->arity; i++)
      {
         TermAddSymbolFeatures(term->args[i], mod_stack, depth+1,
                               feature_array, offset);
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: TermComputeFunctionRanks()
//
//   Assign an occurance rank to each symbol in term.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void TermComputeFunctionRanks(Term_p term, long *rank_array, long *count)
{
   int i;

   if(TermIsVar(term))
   {
      return;
   }
   for(i=0; i < term->arity; i++)
   {
      TermComputeFunctionRanks(term->args[i], rank_array, count);
   }
   if(!rank_array[term->f_code])
   {
      rank_array[term->f_code] = (*count)++;
   }
}


/*-----------------------------------------------------------------------
//
// Function: TermCollectPropVariables()
//
//   Insert all variables with properties prop in term into
//   tree. Return number of new variables.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

long TermCollectPropVariables(Term_p term, PTree_p *tree,
                              TermProperties prop)
{
   long res = 0;
   PStack_p stack = PStackAlloc();
   int      i;

   PStackPushP(stack,term);

   while(!PStackEmpty(stack))
   {
      term = PStackPopP(stack);
      if(TermIsVar(term) &&
         TermCellQueryProp(term,prop))
      {
         if(PTreeStore(tree, term))
         {
            res++;
         }
      }
      else
      {
         for(i=0; i<term->arity; i++)
         {
            PStackPushP(stack,term->args[i]);
         }
      }
   }
   PStackFree(stack);

   return res;
}



/*-----------------------------------------------------------------------
//
// Function: TermAddFunOcc()
//
//  Add all new occurences of function symbol to res_stack and mark
//  them as no-longer-new in f_occur. Return number of new function
//  symbols added.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

long TermAddFunOcc(Term_p term, PDArray_p f_occur, PStack_p res_stack)
{
   long res = 0;
   PStack_p stack = PStackAlloc();
   int      i;

   PStackPushP(stack,term);

   while(!PStackEmpty(stack))
   {
      term = PStackPopP(stack);
      if(!TermIsVar(term))
      {
         if(!PDArrayElementInt(f_occur, term->f_code))
         {
            res++;
            PStackPushInt(res_stack, term->f_code);
            PDArrayAssignInt(f_occur, term->f_code, 1);
         }
         for(i=0; i<term->arity; i++)
         {
            PStackPushP(stack,term->args[i]);
         }
      }
   }
   PStackFree(stack);

   return res;
}




/*-----------------------------------------------------------------------
//
// Function: TermLinearize()
//
//   Put all subterms of term onto PStack in left-right preorder. Note
//   that for an empty stack, that makes the index of s on the stack
//   equal to its TermCPos. Returns number of subterms.
//
// Global Variables: -
//
// Side Effects    : Changes stack
//
/----------------------------------------------------------------------*/

long TermLinearize(PStack_p stack, Term_p term)
{
   long res = 1;
   int i;

   PStackPushP(stack, term);
   for(i = 0; i<term->arity; i++)
   {
      res += TermLinearize(stack, term->args[i]);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: TermCheckConsistency()
//
//   Traverse a tree and check if any one term cell occurs more than
//   once on any branch (which would make the term cyclic). Return the
//   first inconsistency found or NULL.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Term_p TermCheckConsistency(Term_p term, DerefType deref)
{
   Term_p   res;
   PTree_p  branch = NULL;

   printf("TermCheckConsistency...\n");
   res = term_check_consistency_rek(term, &branch, deref);
   assert(branch == 0);
   printf("...TermCheckConsistency\n");
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: TermAssertSameSort
//
//  checks whether the two terms have the same sort. Prints a (verbose)
//  message if it's not the case and exit.
//
// Global Variables: -
//
// Side Effects    : May exit
//
/----------------------------------------------------------------------*/
void TermAssertSameSort(Sig_p sig, Term_p t1, Term_p t2)
{
   if(t1->sort != t2->sort)
   {
      fprintf(stderr, "# Error: terms ");
      TermPrint(stderr, t1, sig, DEREF_NEVER);
      fprintf(stderr, ": ");
      SortPrintTSTP(stderr, sig->sort_table, t1->sort);
      fprintf(stderr, " and ");
      TermPrint(stderr, t2, sig, DEREF_NEVER);
      fprintf(stderr, ": ");
      SortPrintTSTP(stderr, sig->sort_table, t2->sort);
      fprintf(stderr, " should have same sort\n");
      Error("Type error", SYNTAX_ERROR);
   }
}


/*-----------------------------------------------------------------------
//
// Function: TermIsUntyped
//
//   check whether all sorts occurring in the term are either
//   individual or boolean. In other words, whether the term
//   belongs to regular untyped logic.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/
bool TermIsUntyped(Term_p term)
{
   bool res = true;

   PLocalStackInit(stack);

   PLocalStackPush(stack, term);

   while(!PLocalStackEmpty(stack))
   {
      term = PLocalStackPop(stack);

      if(term->sort == STIndividuals || term->sort == STBool)
      {
         PLocalStackPushTermArgs(stack, term);
      }
      else
      {
         res = false;
         break;
      }
   }

   PLocalStackFree(stack);

   return res;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
