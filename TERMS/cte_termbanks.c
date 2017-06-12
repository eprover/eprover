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

Copyright 1998-2011 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sat Nov 15 16:26:30 MET 1997
    New
<2> Fri Oct  2 20:35:41 MET DST 1998
    Variables are now longer in the term trees and have lost superterm
    information -> 20% Speedup!
<3> You cannot take references for variables anymore. This requires
    that no variables are ever replaced (but this is a requirement
    anyways), but deals with a strange bug (and may speed up the code
    even more).
<4> Sat Apr  6 21:42:35 CEST 2002
    Changed for new rewriting. No Superterms anymore! References are
    fully passive. Deletion does not work any more, I'm working on a
    mark-and-sweep garbage collector for terms instead.

-----------------------------------------------------------------------*/

#include "cte_termbanks.h"
#include "cte_typecheck.h"



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
      printf("   =   ");
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

   /* Infer the sort of this term (may be temporary) */
   if(t->sort == STNoSort)
   {
      TypeInferSort(bank->sig, t);
      assert(t->sort != STNoSort);
   }
   bank->insertions++;

   new = TermCellStoreInsert(&(bank->term_store), t);

   if(new) /* Term node already existed, just add properties */
   {
      new->properties = (new->properties | t->properties)/*& bank->prop_mask*/;
      TermTopFree(t);
      return new;
   }
   else
   {
      t->entry_no     = ++(bank->in_count);
      TermCellAssignProp(t,TPGarbageFlag, bank->garbage_state);
      TermCellSetProp(t, TPIsShared); /* Groundness may change below */
      t->v_count = 0;
      t->f_count = 1;
      t->weight = DEFAULT_FWEIGHT;
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
      //assert(TermIsGround(t) == TermIsGroundCompute(t));
   }
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
                          SYNTAX_ERROR);
         }
         else
         {
            SigDeclareIsFunction(bank->sig, res->f_code);
            TypeInferSort(bank->sig, res);
            assert(res->sort != STNoSort);
         }
      }
   }
   return res;
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
                          TB_p bank, bool check_symb_prop)
{
   Term_p *handle, tmp;
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
   tmp = check_symb_prop?
      tb_subterm_parse(in, bank):
      TBRawTermParse(in,bank);

   handle[arity] = tmp;
   arity++;
   while(TestInpTok(in, Comma))
   {
      NextToken(in);
      if(arity==size)
      {
         size+=TERMS_INITIAL_ARGS;
         handle = (Term_p*)SecureRealloc(handle, size*sizeof(Term_p));
      }
      handle[arity] = check_symb_prop?
         tb_subterm_parse(in, bank):
         TBRawTermParse(in,bank);
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
   handle->vars = VarBankAlloc(sig->sort_table);
   TermCellStoreInit(&(handle->term_store));

   term = TermConstCellAlloc(SIG_TRUE_CODE);
   term->sort = STBool;
   TermCellSetProp(term, TPPredPos);
   handle->true_term = TBInsert(handle, term, DEREF_NEVER);
   TermFree(term);
   term = TermConstCellAlloc(SIG_FALSE_CODE);
   term->sort = STBool;
   TermCellSetProp(term, TPPredPos);
   handle->false_term = TBInsert(handle, term, DEREF_NEVER);
   TermFree(term);
   handle->min_term    = NULL;
   handle->freevarsets = NULL;
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

   assert(!junk->freevarsets);
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
   VarSetStoreFree(bank->freevarsets);
   bank->freevarsets = NULL;
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
   return TermCellStoreNodes(&(bank->term_store))+VarBankCardinal(bank->vars);
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

   assert(term);

   term = TermDeref(term, &deref);

   if(TermIsVar(term))
   {
      t = VarBankVarAssertAlloc(bank->vars, term->f_code, term->sort);
   }
   else
   {
      t = TermTopCopyWithoutArgs(term); /* This is an unshared term cell at the moment */

      assert(SysDateIsCreationDate(t->rw_data.nf_date[0]));
      assert(SysDateIsCreationDate(t->rw_data.nf_date[1]));

      for(i=0; i<t->arity; i++)
      {
         t->args[i] = TBInsert(bank, term->args[i], deref);
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

   term = TermDeref(term, &deref);

   if(TermIsVar(term))
   {
      t = VarBankVarAssertAlloc(bank->vars, term->f_code, term->sort);
   }
   else
   {
      t = TermTopCopyWithoutArgs(term); /* This is an unshared term cell at the moment */
      t->properties = TPIgnoreProps;

      assert(SysDateIsCreationDate(t->rw_data.nf_date[0]));
      assert(SysDateIsCreationDate(t->rw_data.nf_date[1]));

      for(i=0; i<t->arity; i++)
      {
         t->args[i] = TBInsertNoProps(bank, term->args[i], deref);
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

   term = TermDeref(term, &deref);

   if(TermIsVar(term))
   {
      t = VarBankVarAssertAlloc(bank->vars, term->f_code, term->sort);
   }
   else
   {
      t = TermTopCopyWithoutArgs(term); /* This is an unshared term cell at the moment */
      t->properties = TPIgnoreProps;

      assert(SysDateIsCreationDate(t->rw_data.nf_date[0]));
      assert(SysDateIsCreationDate(t->rw_data.nf_date[1]));

      for(i=0; i<t->arity; i++)
      {
         t->args[i] = TBInsertRepl(bank, term->args[i], deref, old, repl);
      }
      t = tb_termtop_insert(bank, t);
   }
   return t;
}


/*-----------------------------------------------------------------------
//
// Function: TBInsertInstantiated()
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

Term_p TBInsertInstantiated(TB_p bank, Term_p term)
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
      t = VarBankVarAssertAlloc(bank->vars, term->f_code, term->sort);
   }
   else
   {
      t = TermTopCopyWithoutArgs(term); /* This is an unshared term cell at the moment */
      t->properties    = TPIgnoreProps;

      assert(SysDateIsCreationDate(t->rw_data.nf_date[0]));
      assert(SysDateIsCreationDate(t->rw_data.nf_date[1]));

      for(i=0; i<t->arity; i++)
      {
         t->args[i] = TBInsertInstantiated(bank, term->args[i]);
      }
      t = tb_termtop_insert(bank, t);
   }
   return t;
}



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

   term = TermDeref(term, &deref);

   if(TermIsGround(term))
   {
      return term;
   }

   if(TermIsVar(term))
   {
      t = VarBankVarAssertAlloc(bank->vars, term->f_code, term->sort);
   }
   else
   {
      t = TermTopCopyWithoutArgs(term); /* This is an unshared term cell at the moment */

      assert(SysDateIsCreationDate(t->rw_data.nf_date[0]));
      assert(SysDateIsCreationDate(t->rw_data.nf_date[1]));

      for(i=0; i<t->arity; i++)
      {
         t->args[i] = TBInsertOpt(bank, term->args[i], deref);
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
      t = VarBankVarAssertAlloc(bank->vars, term->f_code+1, term->sort);
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

Term_p TBAllocNewSkolem(TB_p bank, PStack_p variables, SortType sort)
{
   Term_p handle, res;

   handle = TermAllocNewSkolem(bank->sig, variables, sort);
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
      return VarBankFCodeFind(bank->vars, term->f_code, term->sort);
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
   FuncSymbType id_type;
   DStr_p        source_name, errpos;
   SortType      sort;
   long          line, column;
   StreamType    type;

   source_name = DStrGetRef(AktToken(in)->source);
   type        = AktToken(in)->stream_type;
   line = AktToken(in)->line;
   column = AktToken(in)->column;

   /* Test for abbreviation */
   if(TestInpTok(in, Mult))
   {
      long           abbrev;
      TermProperties properties = TPIgnoreProps;

      NextToken(in);
      abbrev = ParseInt(in);

      if(TestInpTok(in, Colon|Slash))
      { /* This _defines_ the abbrev! */
    if(PDArrayElementP(bank->ext_index, abbrev))
    {
       /* Error: Abbreviation defined twice */
       errpos = DStrAlloc();
       DStrAppendStr(errpos, PosRep(type, source_name, line, column));
       DStrAppendStr(errpos, "Abbreviation *");
       DStrAppendInt(errpos, abbrev);
       DStrAppendStr(errpos, " already defined");
       Error(DStrView(errpos), SYNTAX_ERROR);
       DStrFree(errpos);
    }
    if(TestInpTok(in, Slash))
    {
       NextToken(in);
       properties = ParseInt(in);
    }
    NextToken(in);
    handle = TBTermParseReal(in, bank, check_symb_prop); /* Elegant, aint it? */

    if(properties)
    {
       TBRefSetProp(bank, &handle, properties);
    }
    /* printf("# Term %ld = %ld\n", abbrev, handle->entry_no); */
    PDArrayAssignP(bank->ext_index, abbrev, handle);
      }
      else
      { /* This references the abbrev */

    handle = PDArrayElementP(bank->ext_index, abbrev);
    if(!handle)
    {
       /* Error: Undefined abbrev */
       errpos = DStrAlloc();
       DStrAppendStr(errpos, PosRep(type, source_name, line, column));
       DStrAppendStr(errpos, "Abbreviation *");
       DStrAppendInt(errpos, abbrev);
       DStrAppendStr(errpos, " undefined");
       Error(DStrView(errpos), SYNTAX_ERROR);
       DStrFree(errpos);
    }
      }
   }
   else
   {
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
               sort = SortParseTSTP(in, bank->sig->sort_table);
               handle = VarBankExtNameAssertAllocSort(bank->vars,
                                                      DStrView(id), sort);
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

          handle->arity = tb_term_parse_arglist(in, &(handle->args),
                                                     bank, check_symb_prop);
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
          DStrAppendStr(errpos, PosRep(type, source_name, line, column));
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
   }
   DStrReleaseRef(source_name);

   return handle;
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
   long recovered = 0;

   assert(bank);
   assert(!TermIsRewritten(bank->true_term));
   TBGCMarkTerm(bank, bank->true_term);
   TBGCMarkTerm(bank, bank->false_term);
   if(bank->min_term)
   {
      TBGCMarkTerm(bank, bank->min_term);
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
   if(!bank->min_term)
   {
      Term_p t = TermConstCellAlloc(min_const);
      bank->min_term = TBInsert(bank, t, DEREF_NEVER);
      TermFree(t);
   }
   assert(bank->min_term->f_code == min_const);
   return bank->min_term;
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


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
