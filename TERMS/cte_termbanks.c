/*-----------------------------------------------------------------------

  File  : cte_termbanks.c

  Author: Stephan Schulz

  Contents

  Functions for term banks (efficient dag-representations for term
  sets).

  Bindings (i.e. values in term->binding) make only sense in dynamical
  contexts, not in (mostly) static term banks. Thus, bindings are
  followed whenever a term is treated as an individual term, but not
  when they form part of a term bank and are manipulated as such.

  Copyright 1998-2017 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: Sat Nov 15 16:26:30 MET 1997

  -----------------------------------------------------------------------*/

#include "cte_termbanks.h"
#include "cte_typecheck.h"
#include <cte_typebanks.h>
#include <ccl_tformulae.h>



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

bool TBPrintInternalInfo = false;  /* Print internal information about
                                      term nodes, e.g. the flag field */
bool TBPrintDetails = false;      /* Collect potentially expensive
                                     information (number of nodes,
                                     number of unshared nodes, size of
                                     various sub-data structures) and
                                     print them if required */


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/

typedef Term_p (*TermParseFun)(Scanner_p in, TB_p bank);

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: tb_print_dag()
//
//   Print the terms as a dag in the order of insertion.
//
// Global Variables: TBPrintInternalInfo
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void tb_print_dag(FILE *out, NumTree_p in_index, Sig_p sig)
{
   Term_p term;

   if(!in_index)
   {
      return;
   }
   tb_print_dag(out, in_index->lson, sig);
   term = in_index->val1.p_val;
   fprintf(out, "*%ld : ", term->entry_no);

   if(TermIsVar(term))
   {
      VarPrint(out, term->f_code);
   }
   else
   {
      fputs(SigFindName(sig, term->f_code), out);
      if(!TermIsConst(term))
      {
         int i;

         assert(term->arity>=1);
         assert(term->args);
         putc('(', out);

         fprintf(out, "*%ld", TBCellIdent(term->args[0]));
         for(i=1; i<term->arity; i++)
         {
            putc(',', out);
            fprintf(out, "*%ld", TBCellIdent(term->args[i]));
         }
         putc(')', out);
      }
      fprintf(out, "   =   ");
      TermPrint(out, term, sig, DEREF_NEVER);
   }
   if(TBPrintInternalInfo)
   {
      fprintf(out, "\t/*  Properties: %10d */",
              term->properties);
   }
   fprintf(out, "\n");
   tb_print_dag(out, in_index->rson, sig);
}

/*-----------------------------------------------------------------------
//
// Function: tb_termtop_insert()
//
//   Insert a term into the term bank for which the subterms are
//   already in the term bank. Will reuse or destroy the top cell!
//
// Global Variables: TBSupportReplace
//
// Side Effects    : Changes term bank
//
/----------------------------------------------------------------------*/

static Term_p tb_termtop_insert(TB_p bank, Term_p t)
{
   Term_p new;

   assert(t);
   assert(!TermIsVar(t));
   assert(!TermIsAppliedVar(t) || TermIsVar(t->args[0]));

#ifndef NDEBUG
   for(int i=0; i<t->arity; i++)
   {
      assert(TermIsShared(t->args[i]));
   }
#endif

   /* Infer the sort of this term (may be temporary) */
   if(t->type == NULL)
   {
      TypeInferSort(bank->sig, t, NULL);
      assert(t->type != NULL);
   }
   bank->insertions++;

   new = TermCellStoreInsert(&(bank->term_store), t);

   if(new) /* Term node already existed, just add properties */
   {
      assert(!TermIsShared(t));
      new->properties = (new->properties | t->properties)/*& bank->prop_mask*/;
      TermTopFree(t);
      t = new;
   }
   else
   {
      t->entry_no     = ++(bank->in_count);
      TermCellAssignProp(t,TPGarbageFlag, bank->garbage_state);
      TermCellSetProp(t, TPIsShared); /* Groundness may change below */
      t->v_count = 0;
      t->f_count = !TermIsAppliedVar(t) ? 1 : 0;
      t->weight = DEFAULT_FWEIGHT*t->f_count;
      for(int i=0; i<t->arity; i++)
      {
         assert(TermIsShared(t->args[i])||TermIsVar(t->args[i]));
         if(TermIsVar(t->args[i]))
         {
            t->v_count += 1;
            t->weight  += DEFAULT_VWEIGHT;
         }
         else
         {
            t->v_count +=t->args[i]->v_count;
            t->f_count +=t->args[i]->f_count;
            t->weight  +=t->args[i]->weight;
         }
      }

      if(t->v_count == 0)
      {
         TermCellSetProp(t, TPIsGround);
      }

      assert(TermWeight(t, DEFAULT_VWEIGHT, DEFAULT_FWEIGHT) == TermWeightCompute(t, DEFAULT_VWEIGHT, DEFAULT_FWEIGHT));
      assert((t->v_count == 0) == TermIsGround(t));
      assert(TBFind(bank, t));
      //assert(TermIsGround(t) == TermIsGroundCompute(t));
   }

   TermSetBank(t, bank);
   return t;
}


/*-----------------------------------------------------------------------
//
// Function: tb_parse_cons_list()
//
//   Parse a LOP list into an (shared) internal $cons list.
//
// Global Variables: -
//
// Side Effects    : Input, Memory operations
//
/----------------------------------------------------------------------*/

static Term_p tb_parse_cons_list(Scanner_p in, TB_p bank, bool check_symb_prop)
{
   Term_p   handle;
   Term_p   current;
   PStack_p stack;

   assert(SigSupportLists);

   stack = PStackAlloc();
   AcceptInpTok(in, OpenSquare);

   handle = TermDefaultCellAlloc();
   current = handle;

   if(!TestInpTok(in, CloseSquare))
   {

      current->f_code = SIG_CONS_CODE;
      current->arity = 2;
      current->args = TermArgArrayAlloc(2);
      current->args[0] = TBTermParseReal(in, bank, check_symb_prop);
      current->args[1] = TermDefaultCellAlloc();
      current = current->args[1];
      PStackPushP(stack, current);

      while(TestInpTok(in, Comma))
      {
         NextToken(in);
         current->f_code = SIG_CONS_CODE;
         current->arity = 2;
         current->args = TermArgArrayAlloc(2);
         current->args[0] = TBTermParseReal(in, bank, check_symb_prop);
         current->args[1] = TermDefaultCellAlloc();
         current = current->args[1];
         PStackPushP(stack, current);
      }
      current = PStackPopP(stack);
   }
   AcceptInpTok(in, CloseSquare);
   current->f_code = SIG_NIL_CODE;

   /* Now insert the list into the bank */
   handle = TBInsert(bank, current, DEREF_NEVER);

   while(!PStackEmpty(stack))
   {
      current = PStackPopP(stack);
      current->args[1] = handle;
      handle = tb_termtop_insert(bank, current);
   }

   PStackFree(stack);
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: tb_subterm_parse()
//
//   Parse a subterm, i.e. a term which cannot start with a predicate
//   symbol.
//
// Global Variables: -
//
// Side Effects    : Input, memory operations, changes termbank.
//
/----------------------------------------------------------------------*/

static Term_p tb_subterm_parse(Scanner_p in, TB_p bank)
{
   Term_p res = TBTermParseReal(in, bank, true);

   if(!TermIsVar(res))
   {
      if(SigIsPredicate(bank->sig, res->f_code))
      {
         if(SigIsFixedType(bank->sig, res->f_code))
         {
            AktTokenError(in,
                          "Predicate used as function symbol in preceeding term",
                          false);
         }
         else
         {
            SigDeclareIsFunction(bank->sig, res->f_code);
            TypeInferSort(bank->sig, res, in);
            assert(res->type);
         }
      }
      else
      {
         SigFixType(bank->sig, res->f_code);
      }
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: choose_subterm_parse_fun()
//
//   If the argument to be parsed should be of boolean type, parse
//   the argument as a formula. Otherwise, parse it as before.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static Term_p choose_subterm_parse_fun(bool check_symb_prop,
                                              Type_p type, int arg,
                                              Scanner_p in, TB_p bank)
{
   Term_p res = NULL;

   if(type && arg < TypeGetMaxArity(type) && TypeIsBool(type->args[arg]))
   {
      res = TFormulaTSTPParse(in,bank);
   }
   else
   {
      res = check_symb_prop ? tb_subterm_parse(in, bank) : TBTermParse(in, bank);
   }

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: normalize_boolean_terms()
//
//   If term_ref points to an equation of type X=true that appears
//   under context, replace this equation by X.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void normalize_boolean_terms(Term_p* term_ref, TB_p bank)
{
   assert(term_ref && *term_ref);
   Term_p term = *term_ref;
   Sig_p  sig  = bank->sig;
   if(term->f_code == sig->eqn_code)
   {
      if(TermIsVar(term->args[0]) && term->args[1]->f_code == SIG_TRUE_CODE)
      {
         *term_ref = term->args[0]; // garbage collection will deal with parent
      }
      else if(term->args[0]->f_code == SIG_TRUE_CODE)
      {
         assert(term->args[1]->f_code == SIG_TRUE_CODE);
         assert(term->args[0] == bank->true_term);
         assert(term->args[1] == bank->true_term);
         *term_ref = term->args[0];
      }
   }
   else if(term->f_code == sig->neqn_code
            && term->args[0]->f_code == SIG_TRUE_CODE)
   {
      assert(term->args[1]->f_code == SIG_TRUE_CODE);
      assert(term->args[0] == bank->true_term);
      assert(term->args[1] == bank->true_term);
      *term_ref = bank->false_term;
   }
}

/*-----------------------------------------------------------------------
//
// Function: tb_term_parse_arglist()
//
//   Parse a list of terms (comma-separated and enclosed in brackets)
//   into an array of (shared) term pointers. See TermParseArgList()
//   in cte_terms.c for more.
//
// Global Variables: -
//
// Side Effects    : Input, memory operations, changes term bank
//
/----------------------------------------------------------------------*/

static int tb_term_parse_arglist(Scanner_p in, Term_p** arg_anchor,
                                 TB_p bank, bool check_symb_prop, Type_p type)
{
   Term_p   tmp;
   PStackPointer i=0, arity;
   PStack_p args;

   AcceptInpTok(in, OpenBracket);
   if(TestInpTok(in, CloseBracket))
   {
      NextToken(in);
      *arg_anchor = NULL;
      return 0;
   }
   args = PStackAlloc();

   tmp = choose_subterm_parse_fun(check_symb_prop, type, i, in, bank);
   normalize_boolean_terms(&tmp, bank);
   PStackPushP(args, tmp);
   i++;

   while(TestInpTok(in, Comma))
   {
      NextToken(in);
      tmp  = choose_subterm_parse_fun(check_symb_prop, type, i, in, bank);
      normalize_boolean_terms(&tmp, bank);
      PStackPushP(args, tmp);
      i++;
   }
   AcceptInpTok(in, CloseBracket);
   arity = PStackGetSP(args);
   *arg_anchor = TermArgArrayAlloc(arity);
   for(i=0;i<arity;i++)
   {
      (*arg_anchor)[i] = PStackElementP(args,i);
   }
   PStackFree(args);

   return arity;
}

/*-----------------------------------------------------------------------
//
// Function: normalize_head()
//
//   Makes sure that term is represented in a flattened representation.
//    NB: Term is unshared at this point!
//
// Global Variables: -
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

static Term_p normalize_head(Term_p head, Term_p* rest_args, int rest_arity)
{
   assert(problemType == PROBLEM_HO);
   Term_p res;
   if(rest_arity == 0)
   {
      res = head; // do not copy in case there is nothing to be copied
   }
   else
   {
      res = TermDefaultCellAlloc();
      int total_arity = head->arity + rest_arity;

      if(TermIsVar(head))
      {
         total_arity++; // var is going to be the first argument
         res->args = TermArgArrayAlloc(total_arity);
         res->f_code = SIG_APP_VAR_CODE;

         res->args[0] = head;
         for(int i=1; i<total_arity; i++)
         {
            res->args[i] = rest_args[i-1];
         }
      }
      else if(total_arity)
      {
         res->f_code = head->f_code;
         res->args = TermArgArrayAlloc(total_arity);
         int i;
         for(i=0; i < head->arity; i++)
         {
            res->args[i] = head->args[i];
         }

         for(i=0; i < rest_arity; i++)
         {
            res->args[head->arity + i] = rest_args[i];
         }
      }
      else
      {
         res->args = NULL;
      }

      res->arity = total_arity;
   }
   assert(TermIsShared(head));
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: make_head()
//
//   Makes term that has function code that corresponds to f_name
//   and no arguments.
//    NB:  Term is unshared at this point!
//
// Global Variables: -
//
// Side Effects    : Input, memory operations, changes term bank
//
/----------------------------------------------------------------------*/

static Term_p make_head(Sig_p sig, const char* f_name)
{
   Term_p head = TermDefaultCellAlloc();
   head->f_code = SigFindFCode(sig, f_name);
   if(!head->f_code)
   {
      DStr_p msg = DStrAlloc();
      DStrAppendStr(msg, "Function symbol ");
      DStrAppendStr(msg, (char*)f_name);
      DStrAppendStr(msg, " has not been defined previously.");
      Error(DStrView(msg), SYNTAX_ERROR);
   }
   head->arity = 0;
   head->args = NULL;
   head->type = SigGetType(sig, head->f_code);

   return head;
}

/*-----------------------------------------------------------------------
//
// Function: parse_one_ho()
//
//    Parses one HO symbol.
//
// Global Variables: -
//
// Side Effects    : Input, memory operations
//
/----------------------------------------------------------------------*/

static Term_p __inline__ parse_one_ho(Scanner_p in, TB_p bank)
{
   assert(problemType == PROBLEM_HO);

   FuncSymbType   id_type;
   DStr_p id      = DStrAlloc();
   Type_p type;
   Term_p head;

   if((id_type=TermParseOperator(in, id))==FSIdentVar)
   {
      /* A variable may be annotated with a sort */
      if(TestInpTok(in, Colon))
      {
         AcceptInpTok(in, Colon);
         type = TypeBankParseType(in, bank->sig->type_bank);
         head = VarBankExtNameAssertAllocSort(bank->vars, DStrView(id), type);
      }
      else
      {
         head = VarBankExtNameAssertAlloc(bank->vars, DStrView(id));
      }

      assert(TermIsVar(head));
   }
   else
   {
      head = tb_termtop_insert(bank, make_head(bank->sig, DStrView(id)));
   }

   DStrFree(id);
   assert(TermIsShared(head));
   return head;
}



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: TBAlloc()
//
//   Allocate an empty, initialized termbank.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

TB_p TBAlloc(Sig_p sig)
{
   TB_p handle;
   Term_p term;

   assert(sig);

   handle = TBCellAlloc();

   handle->in_count = 0;
   handle->insertions = 0;
   handle->rewrite_steps = 0;
   handle->ext_index = PDIntArrayAlloc(1,100000);
   handle->garbage_state = TPIgnoreProps;
   handle->sig = sig;
   handle->vars = VarBankAlloc(sig->type_bank);
   TermCellStoreInit(&(handle->term_store));

   term = TermConstCellAlloc(SIG_TRUE_CODE);
   term->type = sig->type_bank->bool_type;
   TermCellSetProp(term, TPPredPos);
   handle->true_term = TBInsert(handle, term, DEREF_NEVER);
   TermFree(term);
   term = TermConstCellAlloc(SIG_FALSE_CODE);
   term->type = sig->type_bank->bool_type;
   TermCellSetProp(term, TPPredPos);
   handle->false_term = TBInsert(handle, term, DEREF_NEVER);
   TermFree(term);
   handle->min_terms   = PDArrayAlloc(16, 0);
   //handle->freevarsets = NULL;
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: TBFree()
//
//   Free a term bank (if the signature alread has been
//   extracted). Voids all pointers to terms in the bank!
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

void TBFree(TB_p junk)
{
   assert(!junk->sig);

   /* printf("TBFree(): %ld\n", TermCellStoreNodes(&(junk->term_store)));
    */
   TermCellStoreExit(&(junk->term_store));
   PDArrayFree(junk->ext_index);
   VarBankFree(junk->vars);
   PDArrayFree(junk->min_terms);
   //assert(!junk->freevarsets);
   TBCellFree(junk);
}

/*-----------------------------------------------------------------------
//
// Function: TBVarSetStoreFree()
//
//   Free and reset the VarSetStore in bank.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void TBVarSetStoreFree(TB_p bank)
{
   // VarSetStoreFree(bank->freevarsets);
   // bank->freevarsets = NULL;
}


/*-----------------------------------------------------------------------
//
// Function: TBTermNodes()
//
//   Return the number of term nodes (variables and non-variables) in
//   the term bank.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long TBTermNodes(TB_p bank)
{
   assert(TermCellStoreNodes(&(bank->term_store))==
          TermCellStoreCountNodes(&(bank->term_store)));
   return TermCellStoreNodes(&(bank->term_store))+VarBankCardinality(bank->vars);
}


/*-----------------------------------------------------------------------
//
// Function: TBInsert()
//
//  Insert the term into the termbank. The original term will remain
//  untouched. The routine returns a pointer to a new, shared term of
//  the same structure.
//
//  TermProperties are masked with bank->prop_mask.
//
// Global Variables: -
//
// Side Effects    : Changes term bank
//
/----------------------------------------------------------------------*/

Term_p TBInsert(TB_p bank, Term_p term, DerefType deref)
{
   int    i;
   Term_p t;
   const int limit = DEREF_LIMIT(term, deref);

   assert(term);

   term = TermDeref(term, &deref);

   if(TermIsVar(term))
   {
      t = VarBankVarAssertAlloc(bank->vars, term->f_code, term->type);
      TermSetBank(t, bank);
   }
   else
   {
      t = TermTopCopyWithoutArgs(term); /* This is an unshared term cell at the moment */

      assert(SysDateIsCreationDate(t->rw_data.nf_date[0]));
      assert(SysDateIsCreationDate(t->rw_data.nf_date[1]));

      for(i=0; i<t->arity; i++)
      {
         t->args[i] = TBInsert(bank, term->args[i],
                               CONVERT_DEREF(i, limit, deref));
      }
      t = tb_termtop_insert(bank, t);
   }
   return t;
}

/*-----------------------------------------------------------------------
//
// Function: TBInsertNoProps()
//
//   As TBInsert, but will set all properties of the new term to 0
//   first.
//
// Global Variables: TBSupportReplace
//
// Side Effects    : Changes term bank
//
/----------------------------------------------------------------------*/

Term_p TBInsertNoProps(TB_p bank, Term_p term, DerefType deref)
{
   int    i;
   Term_p t;

   assert(term);

   const int limit = DEREF_LIMIT(term, deref);
   term = TermDeref(term, &deref);

   if(TermIsVar(term))
   {
      t = VarBankVarAssertAlloc(bank->vars, term->f_code, term->type);
      TermSetBank(t, bank);
   }
   else
   {
      t = TermTopCopyWithoutArgs(term); /* This is an unshared term cell at the moment */
      t->properties = TPIgnoreProps;

      assert(SysDateIsCreationDate(t->rw_data.nf_date[0]));
      assert(SysDateIsCreationDate(t->rw_data.nf_date[1]));

      for(i=0; i<t->arity; i++)
      {
         t->args[i] = TBInsertNoProps(bank, term->args[i],
                                      CONVERT_DEREF(i, limit, deref));
      }
      t = tb_termtop_insert(bank, t);
   }
   return t;
}


/*-----------------------------------------------------------------------
//
// Function: TBInsertRepl()
//
//   As TBInsertNoProps, but when old is encountered as a subterm
//   (regardless of instantiation), replace it with uninstantiated
//   repl (which _must_ be in bank).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p  TBInsertRepl(TB_p bank, Term_p term, DerefType deref, Term_p old, Term_p repl)
{
   int    i;
   Term_p t;

   assert(term);

   if(term == old)
   {
      assert(TBFind(bank, repl));
      return repl;
   }

   const int limit = DEREF_LIMIT(term, deref);
   term = TermDeref(term, &deref);

   if(TermIsVar(term))
   {
      t = VarBankVarAssertAlloc(bank->vars, term->f_code, term->type);
      TermSetBank(t, bank);
   }
   else
   {
      t = TermTopCopyWithoutArgs(term); /* This is an unshared term cell at the moment */
      t->properties = TPIgnoreProps;

      assert(SysDateIsCreationDate(t->rw_data.nf_date[0]));
      assert(SysDateIsCreationDate(t->rw_data.nf_date[1]));

      for(i=0; i<t->arity; i++)
      {
         t->args[i] = TBInsertRepl(bank, term->args[i],
                                   CONVERT_DEREF(i, limit, deref), old, repl);
      }
      t = tb_termtop_insert(bank, t);
   }
   return t;
}

/*-----------------------------------------------------------------------
//
// Function: TBInsertReplPlain()
//
//   As TBInsertReplPlain, but terms are not instantiated.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p TBInsertReplPlain(TB_p bank, Term_p term, Term_p old, Term_p repl)
{
   int    i;
   Term_p t;
   bool   changed = false;

   assert(term);

   if(term == old)
   {
      assert(TBFind(bank, repl));
      return repl;
   }
   if(TermStandardWeight(term) <= TermStandardWeight(old))
   {
      return term;
   }
   if(TermIsVar(term))
   {
      return term;
   }
   else
   {
      t = TermTopCopyWithoutArgs(term); /* This is an unshared term cell at the moment */
      t->properties = TPIgnoreProps;

      assert(SysDateIsCreationDate(t->rw_data.nf_date[0]));
      assert(SysDateIsCreationDate(t->rw_data.nf_date[1]));

      for(i=0; i<t->arity; i++)
      {
         t->args[i] = TBInsertReplPlain(bank, term->args[i],
                                        old, repl);
         if(t->args[i]!=term->args[i])
         {
            changed = true;
         }
      }
      if(changed)
      {
         t = tb_termtop_insert(bank, t);
      }
      else
      {
         TermTopFree(t);
         t = term;
      }
   }
   return t;
}




/*-----------------------------------------------------------------------
//
// Function: TBInsertInstantiatedFO()
//
//   Insert a term into the termbank under the assumption that it is a
//   right side of a rule (or equation) composed of terms from bank,
//   and (possibly) instantiated with terms from bank - i.e. don't
//   insert terms that are bound to variables and ground terms, but
//   assume that they are in the term bank. Properties in newly
//   created nodes are deleted.
//
// Global Variables: TBSupportReplace
//
// Side Effects    : Changes term bank
//
/----------------------------------------------------------------------*/

Term_p TBInsertInstantiatedFO(TB_p bank, Term_p term)
{
   int    i;
   Term_p t;

   assert(term);

   if(TermIsGround(term))
   {
      assert(TBFind(bank, term));
      return term;
   }

   if(term->binding)
   {
      assert(TBFind(bank, term->binding));
      return term->binding;
   }

   if(TermIsVar(term))
   {
      t = VarBankVarAssertAlloc(bank->vars, term->f_code, term->type);
      TermSetBank(t, bank);
   }
   else
   {
      t = TermTopCopyWithoutArgs(term); /* This is an unshared term cell at the moment */
      t->properties    = TPIgnoreProps;

      assert(SysDateIsCreationDate(t->rw_data.nf_date[0]));
      assert(SysDateIsCreationDate(t->rw_data.nf_date[1]));

      for(i=0; i<t->arity; i++)
      {
         t->args[i] = TBInsertInstantiatedFO(bank, term->args[i]);
      }
      t = tb_termtop_insert(bank, t);
   }
   return t;
}

/*-----------------------------------------------------------------------
//
// Function: TBInsertInstantiatedHO()
//
//    Differs from TBInsertInstantiatedFO by inserting every binding in
//    the termbank. The reason is that bindings might be unshared terms,
//    so we need to make sure we share them.
//
// Global Variables: TBSupportReplace
//
// Side Effects    : Changes term bank
//
/----------------------------------------------------------------------*/

Term_p TBInsertInstantiatedHO(TB_p bank, Term_p term, bool follow_bind)
{
   int    i;
   Term_p t;

   assert(term);

   if(TermIsVar(term) && term->binding)
   {
      t = follow_bind ? TBInsert(bank, term->binding, DEREF_NEVER) : term;
      TermSetBank(t, bank);
      return t;
   }

   int ignore_args = 0;
   if(TermIsAppliedVar(term) && term->args[0]->binding && follow_bind)
   {
      ignore_args = term->args[0]->binding->arity + (TermIsVar(term->args[0]->binding) ? 1 : 0);
      DerefType d = DEREF_ONCE;
      term = TermDeref(term, &d);
   }

   if(TermIsVar(term))
   {
      t = VarBankVarAssertAlloc(bank->vars, term->f_code, term->type);
      TermSetBank(t, bank);
   }
   else
   {
      t = TermTopCopyWithoutArgs(term); /* This is an unshared term cell at the moment */
      t->properties    = TPIgnoreProps;

      assert(SysDateIsCreationDate(t->rw_data.nf_date[0]));
      assert(SysDateIsCreationDate(t->rw_data.nf_date[1]));

      for(i=0; i<t->arity; i++)
      {
         t->args[i] = TBInsertInstantiatedHO(bank, term->args[i], follow_bind && (i >= ignore_args));
      }
      t = tb_termtop_insert(bank, t);
   }
   return t;
}

#ifdef ENABLE_LFHO
/*-----------------------------------------------------------------------
//
// Function: TBInsertInstantiated()
//
//    Wrapper that chooses which function to call based on the
//    problem type.
//
// Global Variables: TBSupportReplace
//
// Side Effects    : Changes term bank
//
/----------------------------------------------------------------------*/

__inline__ Term_p TBInsertInstantiated(TB_p bank, Term_p term)
{
   if(problemType == PROBLEM_HO)
   {
      return TBInsertInstantiatedHO(bank, term, true);
   }
   else
   {
      return TBInsertInstantiatedFO(bank, term);
   }
}
#endif


/*-----------------------------------------------------------------------
//
// Function: TBInsertOpt()
//
//   Insert term into bank under the assumption that it it already is
//   in the bank (except possibly for variables appearing as
//   bindings). This allows us to just return term for ground terms.
//
// Global Variables: -
//
// Side Effects    : Changes term bank.
//
/----------------------------------------------------------------------*/

Term_p TBInsertOpt(TB_p bank, Term_p term, DerefType deref)
{
   int    i;
   Term_p t;

   assert(term);

   const int limit = DEREF_LIMIT(term, deref);
   term = TermDeref(term, &deref);

   if(TermIsGround(term))
   {
      assert(TermIsShared(term));
      return term;
   }

   if(TermIsVar(term))
   {
      t = VarBankVarAssertAlloc(bank->vars, term->f_code, term->type);
      TermSetBank(t, bank);
   }
   else
   {
      t = TermTopCopyWithoutArgs(term); /* This is an unshared term cell at the moment */

      assert(SysDateIsCreationDate(t->rw_data.nf_date[0]));
      assert(SysDateIsCreationDate(t->rw_data.nf_date[1]));

      for(i=0; i<t->arity; i++)
      {
         t->args[i] = TBInsertOpt(bank, term->args[i], CONVERT_DEREF(i, limit, deref));
      }
      t = tb_termtop_insert(bank, t);
   }
   return t;
}


/*-----------------------------------------------------------------------
//
// Function: TBInsertDisjoint()
//
//   Create a copy of (uninstantiated) term with disjoint
//   variables. This assumes that all variables in term are odd or
//   even, the returned copy will have variable ids shifted by -1.
//
// Global Variables: -
//
// Side Effects    : Changes bank.
//
/----------------------------------------------------------------------*/

Term_p  TBInsertDisjoint(TB_p bank, Term_p term)
{
   int    i;
   Term_p t;

   assert(term);


   if(TermIsGround(term))
   {
      return term;
   }

   if(TermIsVar(term))
   {
      t = VarBankGetAltVar(bank->vars, term);
      TermSetBank(t, bank);
      // t = VarBankVarAssertAlloc(bank->vars, term->f_code+1, term->sort);
   }
   else
   {
      t = TermTopCopyWithoutArgs(term); /* This is an unshared term cell at the moment */

      assert(SysDateIsCreationDate(t->rw_data.nf_date[0]));
      assert(SysDateIsCreationDate(t->rw_data.nf_date[1]));

      for(i=0; i<t->arity; i++)
      {
         t->args[i] = TBInsertDisjoint(bank, term->args[i]);
      }
      t = tb_termtop_insert(bank, t);
   }
   return t;
}



/*-----------------------------------------------------------------------
//
// Function: TBTermTopInsert()
//
//   See tb_termtop_insert, for export without hurting inlining
//   capabilities.
//
// Global Variables: -
//
// Side Effects    : Changes bank
//
/----------------------------------------------------------------------*/

Term_p TBTermTopInsert(TB_p bank, Term_p t)
{
   return tb_termtop_insert(bank,t);
}


/*-----------------------------------------------------------------------
//
// Function: TBAllocNewSkolem()
//
//   Create a news Skolem term (or definition atom) with the given
//   variables in the term bank and return the pointer to it.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

Term_p TBAllocNewSkolem(TB_p bank, PStack_p variables, Type_p ret_type)
{
   Term_p handle, res;

   handle = TermAllocNewSkolem(bank->sig, variables, ret_type);
   res = TBInsert(bank, handle, DEREF_NEVER);
   TermFree(handle);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: TBFind()
//
//   Find a term in the term cell bank and return it.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p TBFind(TB_p bank, Term_p term)
{
   if(TermIsVar(term))
   {
      return VarBankFCodeFind(bank->vars, term->f_code);
   }
   return TermCellStoreFind(&(bank->term_store), term);
}


/*-----------------------------------------------------------------------
//
// Function: TBPrintBankInOrder()
//
//   Print the DAG in the order of ascending entry_no.
//
// Global Variables: TBPrintTermsFlat
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void TBPrintBankInOrder(FILE* out, TB_p bank)
{
   NumTree_p tree = NULL;
   long i;
   PStack_p stack;
   Term_p   cell;
   IntOrP   dummy;

   for(i=0; i<TERM_STORE_HASH_SIZE; i++)
   {
      stack = TermTreeTraverseInit(bank->term_store.store[i]);
      while((cell = TermTreeTraverseNext(stack)))
      {
         dummy.p_val = cell;
         NumTreeStore(&tree, cell->entry_no,dummy, dummy);
      }
      TermTreeTraverseExit(stack);
   }
   tb_print_dag(out, tree, bank->sig);
   NumTreeFree(tree);
}

/*-----------------------------------------------------------------------
//
// Function: TBPrintTermCompact()
//
//   Print a term bank term. Introduce abbreviations for all subterms
//   encountered. Subterms with TPOutputFlag are not
//   printed, but are assumed to be known. Does _not_ follow bindings
//   (they are temporary and as such make little sense in the term
//   bank context)
//
// Global Variables: -
//
// Side Effects    : Output, set TPOutputFlag in subterms printed with
//                   abbreviation.
//
/----------------------------------------------------------------------*/

void TBPrintTermCompact(FILE* out, TB_p bank, Term_p term)
{
   int i;

   if(TermCellQueryProp(term, TPOutputFlag))
   {
      fprintf(out, "*%ld", term->entry_no);
   }
   else
   {
      if(TermIsVar(term))
      {
         VarPrint(out, term->f_code);
      }
      else
      {
         fprintf(out, "*%ld:", term->entry_no);
         TermCellSetProp(term, TPOutputFlag);
         fputs(SigFindName(bank->sig, term->f_code), out);
         if(!TermIsConst(term))
         {
            fputc('(',out);
            assert(term->args && (term->arity>0));
            TBPrintTermCompact(out, bank, term->args[0]);
            for(i=1;i<term->arity;i++)
            {
               fputc(',', out);
               TBPrintTermCompact(out, bank, term->args[i]);
            }
            fputc(')',out);
         }
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: TBPrintTerm()
//
//   Print a term from a term bank either in compact form (with
//   abbreviations) or as a conventional term.
//
// Global Variables: -
//
// Side Effects    : By the called functions
//
/----------------------------------------------------------------------*/

void TBPrintTerm(FILE* out, TB_p bank, Term_p term, bool fullterms)
{
   if(fullterms)
   {
      TBPrintTermFull(out, bank, term);
   }
   else
   {
      TBPrintTermCompact(out, bank, term);
   }
}


/*-----------------------------------------------------------------------
//
// Function:  TBPrintBankTerms()
//
//   Print the terms inserted into the term bank with abbreviations.
//
// Global Variables: -
//
// Side Effects    : Output, changes by TBPrintTermCompact()
//
/----------------------------------------------------------------------*/

void TBPrintBankTerms(FILE* out, TB_p bank)
{
   PStack_p stack = PStackAlloc();
   Term_p term;
   int i;

   for(i=0; i<TERM_STORE_HASH_SIZE; i++)
   {
      PStackPushP(stack, bank->term_store.store[i]);

      while(!PStackEmpty(stack))
      {
         term = PStackPopP(stack);
         if(term)
         {
            PStackPushP(stack, term->lson);
            PStackPushP(stack, term->rson);
            if(TermCellQueryProp(term, TPTopPos))
            {
               TBPrintTermCompact(out, bank, term);
               fprintf(out, "\n");
            }
         }
      }
   }
   PStackFree(stack);
}


/*-----------------------------------------------------------------------
//
// Function: TBTermParseReal()
//
//   Parse a term from the given scanner object directly into the
//   termbank. Supports abbreviations. This function will _not_ set
//   the TPTopPos property on top terms while parsing. It will or will
//   not check and set symbol properties (function symbol, predicate
//   symbol), depending on the check_symb_prop parameter.
//
// Global Variables: -
//
// Side Effects    : Input, memory operations, changes termbank.
//
/----------------------------------------------------------------------*/

Term_p TBTermParseReal(Scanner_p in, TB_p bank, bool check_symb_prop)
{
   Term_p        handle;
   DStr_p        id;
   FuncSymbType  id_type;
   DStr_p        source_name, errpos;
   Type_p        type;
   long          line, column;
   StreamType    type_stream;

   source_name = DStrGetRef(AktToken(in)->source);
   type_stream        = AktToken(in)->stream_type;
   line = AktToken(in)->line;
   column = AktToken(in)->column;

   /* Normal term stuff, bloated because of the nonsensical SETHEO
      syntax */

   if(SigSupportLists && TestInpTok(in, OpenSquare))
   {
      handle =  tb_parse_cons_list(in, bank, check_symb_prop);
   }
   else
   {
      id = DStrAlloc();

      if((id_type=TermParseOperator(in, id))==FSIdentVar)
      {
         /* A variable may be annotated with a sort */
         if(TestInpTok(in, Colon))
         {
            AcceptInpTok(in, Colon);
            type = TypeBankParseType(in, bank->sig->type_bank);
            handle = VarBankExtNameAssertAllocSort(bank->vars,
                                                   DStrView(id), type);
         }
         else
         {
            handle = VarBankExtNameAssertAlloc(bank->vars, DStrView(id));
         }
      }
      else
      {
         handle = TermDefaultCellAlloc();

         if(TestInpTok(in, OpenBracket))
         {
            if((id_type == FSIdentInt)
               &&(bank->sig->distinct_props & FPIsInteger))
            {
               AktTokenError(in,
                             "Number cannot have argument list "
                             "(consider --free-numbers)",
                             false);
            }
            if((id_type == FSIdentFloat)
               &&(bank->sig->distinct_props & FPIsFloat))
            {
               AktTokenError(in,
                             "Floating point number cannot have argument list "
                             "(consider --free-numbers)",
                             false);
            }
            if((id_type == FSIdentRational)
               &&(bank->sig->distinct_props & FPIsRational))
            {
               AktTokenError(in,
                             "Rational number cannot have argument list "
                             "(consider --free-numbers)",
                             false);
            }
            if((id_type == FSIdentObject)
               &&(bank->sig->distinct_props & FPIsObject))
            {
               AktTokenError(in,
                             "Object cannot have argument list "
                             "(consider --free-objects)",
                             false);
            }

            // in TFX all symbols must be declared beforehand
            // thus, if we have a formula at the argument, symbol with name
            // id already has a type declared with $o in appropriate places
            FunCode sym_code = SigFindFCode(bank->sig, DStrView(id));
            Type_p  sym_type = sym_code ? SigGetType(bank->sig, sym_code) : NULL;

            handle->arity = tb_term_parse_arglist(in, &(handle->args),
                                                  bank, check_symb_prop, sym_type);
         }
         else
         {
            handle->arity = 0;
         }
         handle->f_code = TermSigInsert(bank->sig, DStrView(id),
                                        handle->arity, false, id_type);
         if(!handle->f_code)
         {
            errpos = DStrAlloc();
            DStrAppendStr(errpos, PosRep(type_stream, source_name, line, column));
            DStrAppendStr(errpos, DStrView(id));
            DStrAppendStr(errpos, " used with arity ");
            DStrAppendInt(errpos, (long)handle->arity);
            DStrAppendStr(errpos, ", but registered with arity ");
            DStrAppendInt(errpos,
                          (long)(bank->sig)->
                          f_info[SigFindFCode(bank->sig, DStrView(id))].arity);
            Error(DStrView(errpos), SYNTAX_ERROR);
            DStrFree(errpos);
         }
         handle = tb_termtop_insert(bank, handle);
         }
      DStrFree(id);
   }
   DStrReleaseRef(source_name);

   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: TBTermParseRealHO()
//
//   Parse a term from the given scanner object directly into the
//   termbank. Supports TPTP thf syntax except for:
//       - lambdas
//       - formulas as arguments to predicates
//       - !! and ~ @ syntax (or in general @ with no lhs argument)
//       - ... anything else for what you get error message :)
//
// Global Variables: -
//
// Side Effects    : Input, memory operations, changes termbank.
//
/----------------------------------------------------------------------*/

Term_p  TBTermParseRealHO(Scanner_p in, TB_p bank, bool check_symb_prop)
{
   Term_p  head    = NULL;
   Term_p  arg     = NULL;
   Term_p* rest_args    = NULL;
   Term_p  res     = NULL;
   int     rest_arity   = 0;
   int     allocated    = 0;
   int     head_arity   = 0;

   if(TestInpTok(in, OpenBracket))
   {
      AcceptInpTok(in, OpenBracket);
      head = TBTermParseRealHO(in, bank, check_symb_prop);
      AcceptInpTok(in, CloseBracket);
   }
   else
   {
      head = parse_one_ho(in, bank);
   }

   head_arity = head->arity;

   if(!TermIsVar(head) && !TermIsAppliedVar(head) && !SigGetType(bank->sig, head->f_code))
   {
      DStr_p msg = DStrAlloc();
      if(head->f_code > 0)
      {
         DStrAppendStr(msg, SigFindName(bank->sig, head->f_code));
         DStrAppendStr(msg, " with id ");
      }
      DStrAppendInt(msg, (int)head->f_code);
      DStrAppendStr(msg, " has not been declared previously. This needs to change.");
      AktTokenError(in, DStrView(msg), false);
   }

   allocated = TERMS_INITIAL_ARGS;
   rest_args = (Term_p*)SecureMalloc(allocated*sizeof(Term_p));
   rest_arity = 0;

   while(TestInpTok(in, Application))
   {
      AcceptInpTok(in, Application);

      if(head_arity + rest_arity >= TypeGetMaxArity(GetHeadType(bank->sig, head)))
      {
         AktTokenError(in, "Too many arguments supplied to symbol.", false);
      }
      if(TypeIsBool(GetHeadType(bank->sig, head)->args[head_arity+rest_arity]))
      {
         arg = (TestInpTok(in, Name|SemIdent) ? parse_one_ho :  TFormulaTSTPParse)(in, bank);
      }
      else
      {
         if(TestInpTok(in, OpenBracket))
         {
            AcceptInpTok(in, OpenBracket);
            arg = TBTermParseRealHO(in, bank, check_symb_prop);
            AcceptInpTok(in, CloseBracket);
         }
         else
         {
            arg = parse_one_ho(in, bank);
         }
      }

      if(rest_arity == allocated)
      {
         allocated += TERMS_INITIAL_ARGS;
         rest_args = (Term_p*)SecureRealloc(rest_args, allocated*sizeof(Term_p));
      }

      rest_args[rest_arity++] = arg;
   }

   res = normalize_head(head, rest_args, rest_arity);

   if(!TermIsVar(res) && !TermIsShared(res))
   {
      res = tb_termtop_insert(bank, res);
   }
   else
   {
      assert(TermIsVar(res) || TBFind(bank, res));
   }

   if(allocated)
   {
      FREE(rest_args);
   }
   assert(TermIsShared(res));
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: TBRefSetProp()
//
//   Make ref point to a term of the same structure as *ref, but with
//   properties prop set. Properties do not work for variables!
//
// Global Variables: -
//
// Side Effects    : Changes bank, *ref
//
/----------------------------------------------------------------------*/

void TBRefSetProp(TB_p bank, TermRef ref, TermProperties prop)
{
   Term_p term, new;

   assert(!TermIsVar(*ref));

   term = *ref;
   if(TermCellQueryProp(term, prop)||TermIsVar(term))
   {
      return;
   }

   new = TermTopCopy(term);
   TermCellSetProp(new, prop);
   new = tb_termtop_insert(bank, new);
   *ref = new;
   /* Old term will be garbage-collected eventually */
}



/*-----------------------------------------------------------------------
//
// Function: TBRefDelProp()
//
//   Make ref point to a term of the same structure as *ref, but with
//   properties prop deleted. If the term is a variable, do nothing!
//
// Global Variables: -
//
// Side Effects    : Changes bank, *ref
//
/----------------------------------------------------------------------*/

void TBRefDelProp(TB_p bank, TermRef ref, TermProperties prop)
{
   Term_p term, new;

   term = *ref;
   if(!TermCellIsAnyPropSet(term, prop)||TermIsVar(term))
   {
      return;
   }
   new = TermTopCopy(term);
   TermCellDelProp(new, prop);
   new = tb_termtop_insert(bank, new);
   *ref = new;
}


/*-----------------------------------------------------------------------
//
// Function: TBTermDelPropCount()
//
//   Delete properties prop in term, return number of term cells with
//   this property. Does assume that all subterms of a term without
//   this property also do not carry it!
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

long TBTermDelPropCount(Term_p term, TermProperties prop)
{
   long count = 0;
   int i;
   PStack_p stack = PStackAlloc();

   PStackPushP(stack, term);
   while(!PStackEmpty(stack))
   {
      term = PStackPopP(stack);
      if(TermCellQueryProp(term, prop))
      {
         TermCellDelProp(term, prop);
         count++;
         for(i=0; i<term->arity; i++)
         {
            PStackPushP(stack, term->args[i]);
         }
      }
   }
   PStackFree(stack);
   return count;
}


/*-----------------------------------------------------------------------
//
// Function: TBGCMarkTerm()
//
//   Mark a term as used for the garbage collector.
//
// Global Variables: -
//
// Side Effects    : Marks the term, memory operations
//
/----------------------------------------------------------------------*/

void TBGCMarkTerm(TB_p bank, Term_p term)
{
   PStack_p stack = PStackAlloc();
   int i;

   assert(bank);
   assert(term);

   PStackPushP(stack, term);
   while(!PStackEmpty(stack))
   {
      term = PStackPopP(stack);
      if(!TBTermCellIsMarked(bank,term))
      {
         TermCellFlipProp(term, TPGarbageFlag);
         for(i=0; i<term->arity; i++)
         {
            PStackPushP(stack, term->args[i]);
         }
         if(TermIsRewritten(term))
         {
            PStackPushP(stack, TermRWReplaceField(term));
         }

         if(TermIsAppliedVar(term) && TermGetCache(term))
         {
            PStackPushP(stack, TermGetCache(term));
         }
      }
   }
   PStackFree(stack);
}


/*-----------------------------------------------------------------------
//
// Function: TBGCSweep()
//
//   Sweep the term bank and free all unmarked term
//   cells. bank->true_term will be marked automatically. Returns the
//   number of term cells recovered.
//
// Global Variables: -
//
// Side Effects    : Memory operations, flips bank->garbage_state
//
/----------------------------------------------------------------------*/

long TBGCSweep(TB_p bank)
{
   long recovered = 0, i;
   Term_p t;

   assert(bank);
   assert(!TermIsRewritten(bank->true_term));
   TBGCMarkTerm(bank, bank->true_term);
   TBGCMarkTerm(bank, bank->false_term);

   for(i=0; i< bank->min_terms->size; i++)
   {
      t = PDArrayElementP(bank->min_terms, i);

      if(t)
      {
         TBGCMarkTerm(bank, t);
      }
   }
   VERBOUT("Garbage collection started.\n");
   recovered = TermCellStoreGCSweep(&(bank->term_store),
                                    bank->garbage_state);
   VERBOSE(fprintf(stderr, "Garbage collection reclaimed %ld unused term cells.\n",recovered););
/* #ifdef PRINT_SOMEERRORS_STDOUT */
#ifdef NEVER_DEFINED
   if(OutputLevel)
   {
      fprintf(GlobalOut,
              "# Garbage collection reclaimed %ld unused term cells.\n",
              recovered);
   }
#endif
   bank->garbage_state =
      bank->garbage_state?TPIgnoreProps:TPGarbageFlag;

   return recovered;
}


/*-----------------------------------------------------------------------
//
// Function: TBCreateConstTerm()
//
//   Create constant term for a given symbol.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p TBCreateConstTerm(TB_p bank, FunCode fconst)
{
   Term_p res,
      t = TermConstCellAlloc(fconst);

   res = TBInsert(bank, t, DEREF_NEVER);
   TermFree(t);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: TBCreateMinTerm()
//
//   If bank->min_term exists, return it. Otherwise create and return
//   it.
//
// Global Variables: -
//
// Side Effects    : Might set bank->min_term
//
/----------------------------------------------------------------------*/

Term_p TBCreateMinTerm(TB_p bank, FunCode min_const)
{
   Type_p type = SigGetType(bank->sig, min_const);
   long sort = GetReturnSort(type)->type_uid;
   Term_p t = PDArrayElementP(bank->min_terms, sort);

   if(!t)
   {
      t = TBCreateConstTerm(bank, min_const);
      PDArrayAssignP(bank->min_terms, sort, t);
   }
   return t;
}


/*-----------------------------------------------------------------------
//
// Function: TBTermCollectSubterms()
//
//   Collect all subterms of term onto collector. Assumes that
//   TPOpFlag is set if and only if the term is already in the
//   collection. Returns the number of new terms found.
//
// Global Variables: -
//
// Side Effects    : Sets the OpFlag of newly collected terms.
//
/----------------------------------------------------------------------*/

long TBTermCollectSubterms(Term_p term, PStack_p collector)
{
   long res = 0;
   int i;

   assert(term);
   assert(TermIsShared(term));

   if(!TermCellQueryProp(term, TPOpFlag))
   {
      res = 1;
      TermCellSetProp(term, TPOpFlag);
      PStackPushP(collector, term);
      for(i=0; i<term->arity; i++)
      {
         res += TBTermCollectSubterms(term->args[i], collector);
      }
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: TBFindRepr()
//
//   Find the representation of a term from another (or none) bank in
//   this bank.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p TBFindRepr(TB_p bank, Term_p term)
{
   int i;
   Term_p work;
   Term_p repr;

   if (TermIsVar(term) || TermIsConst(term))
   {
      return TBFind(bank, term);
   }

   work = TermTopCopy(term);
   for (i=0; i<work->arity; i++)
   {
      work->args[i] = TBFindRepr(bank, term->args[i]);
   }
   repr = TBFind(bank, work);
   TermTopFree(work);
   return repr;
}


/*-----------------------------------------------------------------------
//
// Function: TBGetFirstConstTerm()
//
//    Return a constant term with the first constant of the proper
//    sort in sig.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Term_p TBGetFirstConstTerm(TB_p bank, Type_p type)
{
   PStack_p cand_stack = PStackAlloc();
   Term_p   res = NULL;

   SigCollectSortConsts(bank->sig, type, cand_stack);
   if(!PStackEmpty(cand_stack))
   {
      res = TBCreateConstTerm(bank, PStackElementInt(cand_stack, 0));
   }
   PStackFree(cand_stack);
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: TBGetFreqConstTerm()
//
//   Find the best (according to is_better) constant of the give sort,
//   and return a shared term with this constant. If no suitable
//   constant exists, returns NULL. conj_dist_array contains number of
//   occurances for each symbol in conjecture clauses, dist_array the
//   same for all clauses.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p TBGetFreqConstTerm(TB_p terms, Type_p type,
                          long* conj_dist_array,
                          long* dist_array, FunConstCmpFunType is_better)
{
   Term_p res = NULL;
   long   cand_no;
   PStackPointer i;
   FunCode f = 0, cand;

   PStack_p candidates = PStackAlloc();
   cand_no = SigCollectSortConsts(terms->sig, type, candidates);

   if(cand_no)
   {
      f = PStackElementInt(candidates, 0);
      for(i=1; i<PStackGetSP(candidates); i++)
      {
         cand = PStackElementInt(candidates, i);

         if(is_better(cand, f, conj_dist_array, dist_array))
         {
            f = cand;
         }
      }
      res = TBCreateConstTerm(terms, f);
   }
   PStackFree(candidates);
   return res;
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
