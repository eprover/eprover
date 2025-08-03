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
#include <cte_lambda.h>



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

   if(TermIsFreeVar(term))
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
   assert(!TermIsAnyVar(t));
   assert(t->f_code != SIG_NAMED_LAMBDA_CODE ||
          (t->arity == 2 && TermIsFreeVar(t->args[0])));
   assert(t->f_code != SIG_DB_LAMBDA_CODE ||
          (t->arity == 2 && TermIsDBVar(t->args[0])));
   assert(!TermIsPhonyApp(t) || TermIsAnyVar(t->args[0]) || TermIsLambda(t->args[0]));
   assert(!TermIsPhonyApp(t) || t->arity > 1);

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
      TermSetBank(t, bank);
      t = new;
   }
   else
   {
      t->entry_no     = ++(bank->in_count);
      TermSetBank(t, bank);
      TermCellAssignProp(t,TPGarbageFlag, bank->garbage_state);
      TermCellSetProp(t, TPIsShared); /* Groundness may change below */
      if(TermIsDBVar(t))
      {
         TermCellSetProp(t, TPHasDBSubterm);
      }
      if(TypeIsBool(t->type))
      {
         TermCellSetProp(t, TPHasBoolSubterm);
      }
      if(TermIsPhonyApp(t) && TermIsLambda(t->args[0]))
      {
         TermCellSetProp(t, TPIsBetaReducible);
      }
      if(TermIsLambda(t))
      {
         TermCellSetProp(t, TPHasLambdaSubterm);
      }
      if(TypeIsArrow(t->type) && !TermIsLambda(t))
      {
         TermCellSetProp(t, TPHasEtaExpandableSubterm);
      }
      if(t->f_code == bank->sig->eqn_code || t->f_code == bank->sig->neqn_code)
      {
         TermCellSetProp(t, TPHasEqNeqSym);
      }
      t->v_count = 0;
      t->f_count = !TermIsPhonyApp(t) ? 1 : 0;
      t->weight = DEFAULT_FWEIGHT*t->f_count;
      for(int i=0; i<t->arity; i++)
      {
         assert(TermIsShared(t->args[i])||TermIsFreeVar(t->args[i]));
         TermCellSetProp(t, TermCellGiveProps(t->args[i], TPIsBetaReducible));
         TermCellSetProp(t, TermCellGiveProps(t->args[i], TPHasDBSubterm));
         TermCellSetProp(t, TermCellGiveProps(t->args[i], TPHasEqNeqSym));
         TermCellSetProp(t, TermCellGiveProps(t->args[i], TPHasBoolSubterm));
         if(TypeIsBool(t->args[i]))
         {
            TermCellSetProp(t, TPHasBoolSubterm); // vars and dbvars are sometimes not shared.
         }
         TermCellSetProp(t, TermCellGiveProps(t->args[i], TPHasLambdaSubterm));
         if(!(TermIsPhonyApp(t) || TermIsLambda(t)) || i!=0)
         {
            TermCellSetProp(t, TermCellGiveProps(t->args[i], TPHasEtaExpandableSubterm));
         }
         TermCellSetProp(t, TermCellGiveProps(t->args[i], TPHasNonPatternVar));
         TermCellSetProp(t, TermCellGiveProps(t->args[i], TPHasAppVar));
         if(TermIsFreeVar(t->args[i]))
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

      if(t->f_code == SIG_DB_LAMBDA_CODE)
      {
         t->f_count -= 2; // adjusting for db var and lambda sign
         t->weight  -= 2*DEFAULT_FWEIGHT; // adjusting  for db var and lambda sign
      }
      if(TermIsAppliedFreeVar(t))
      {
         TermCellSetProp(t, TPHasAppVar);
         // counting applied pattern free var as a single var.
         if(NormalizePatternAppVar(bank, t))
         {
            t->f_count = 0;
            t->v_count = 1;
            t->weight  = DEFAULT_VWEIGHT;
         }
         else
         {
            TermCellSetProp(t, TPHasNonPatternVar);
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

   handle = TermDefaultCellArityAlloc(2);
   current = handle;

   if(!TestInpTok(in, CloseSquare))
   {

      current->f_code = SIG_CONS_CODE;
      current->args[0] = TBTermParseReal(in, bank, check_symb_prop);
      current->args[1] = TermDefaultCellArityAlloc(2);
      current = current->args[1];

      PStackPushP(stack, current);

      while(TestInpTok(in, Comma))
      {
         NextToken(in);
         current->f_code = SIG_CONS_CODE;
         current->args[0] = TBTermParseReal(in, bank, check_symb_prop);
         current->args[1] = TermDefaultCellArityAlloc(2);
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
// Function: parse_let_typedecl()
//
//   Parses a single type declaration that constitutes of the first
//   part of a let term. For each parsed symbol, on type_decl it stores
//   symbol name (DStr), fresh symbol ID (regardless of whether
//   the symbol is already in the signature), and symbol type
//   ACHTUNG: Dynamically allocated DStr is put on the stack.
//
// Global Variables: -
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

static void parse_let_typedecl(Scanner_p in, TB_p bank, PStack_p type_decls)
{
   DStr_p name = DStrAlloc();
   FuncSymbType sym_type = FuncSymbParse(in, name);
   if (sym_type == FSIdentFreeFun)
   {
      AcceptInpTok(in, Colon);
      Type_p type = TypeBankParseType(in, bank->sig->type_bank);

      PStackPushP(type_decls, name);
      PStackPushInt(type_decls, SigInsertLetId(bank->sig, DStrView(name), type));
   }
   else
   {
      AktTokenError(in, "let declaration expects a function symbol",
                    true);
   }
}

/*-----------------------------------------------------------------------
//
// Function: parse_let_definition()
//
//   Parses a single type declaration that constitutes of the first
//   part of a let term. For each parsed symbol, on type_decl it stores
//   symbol name (DStr), fresh symbol ID (regardless of whether
//   the symbol is already in the signature), and symbol type
//   ACHTUNG: Dynamically allocated DStr is put on the stack.
//
// Global Variables: -
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

static Term_p parse_let_sym_def(Scanner_p in, TB_p bank, PStack_p type_decls)
{
   DStr_p name = DStrAlloc();
   FuncSymbType sym_type = FuncSymbParse(in, name);
   FunCode id = 0;
   for(PStackPointer i = 0; !id && i<PStackGetSP(type_decls); i += 2)
   {
      if(!strcmp(DStrView(name), DStrView(PStackElementP(type_decls, i))))
      {
         id = PStackElementInt(type_decls, i+1);
      }
   }

   if(id)
   {
      Type_p type = SigGetType(bank->sig, id);
      int arity = TypeGetMaxArity(type);
      Term_p vars[arity]; // Variable length array
      IntOrP dummy = {0};
      if(arity)
      {
         AcceptInpTok(in, OpenBracket);
         StrTree_p prev_vars = NULL;

         for(int i=0; i<arity; i++)
         {
            DStrReset(name);
            sym_type = FuncSymbParse(in, name);
            if(StrTreeFind(&prev_vars, DStrView(name)))
            {
               AktTokenError(in, "variables must be distinct", true);
            }

            StrTreeStore(&prev_vars, DStrView(name), dummy, dummy);
            if(sym_type != FSIdentVar)
            {
               AktTokenError(in, "variable is expected", true);
            }

            Type_p arg_type = type->args[i];

            VarBankPushEnv(bank->vars);
            vars[i] = VarBankExtNameAssertAllocSort(bank->vars,
                                                    DStrView(name), arg_type);
            if(i!=arity-1)
            {
               AcceptInpTok(in, Comma);
            }
         }
         AcceptInpTok(in, CloseBracket);
         StrTreeFree(prev_vars);
      }

      AcceptInpTok(in, Colon);
      AcceptInpTok(in, EqualSign);

      Term_p rhs = TFormulaTSTPParse(in, bank);
      Term_p lhs = TermTopAlloc(id, arity);
      for(int i=0; i<arity; i++)
      {
         lhs->args[i] = vars[i];
         VarBankPopEnv(bank->vars);
      }
      lhs = TBTermTopInsert(bank, lhs);

      DStrFree(name);
      return EqnTermsTBTermEncode(bank, lhs, rhs, true, PENormal);
   }
   else
   {
      AktTokenError(in, "symbol not in let declaration list", true);
      return NULL;
   }
}

/*-----------------------------------------------------------------------
//
// Function: make_let()
//
//   Bulids the variable-arity let term.
//
// Global Variables: -
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

static Term_p make_let(TB_p bank, PStack_p definitions, Term_p body)
{
   Term_p let_t = TermTopAlloc(SIG_LET_CODE, PStackGetSP(definitions) + 1);
   for(PStackPointer i=0; i<PStackGetSP(definitions); i++)
   {
      let_t->args[i] = PStackElementP(definitions, i);
   }
   let_t->args[PStackGetSP(definitions)] = body;
   let_t->type = body->type;
   return TBTermTopInsert(bank, let_t);
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

   if(!TermIsFreeVar(res))
   {
      if(SigIsPredicate(bank->sig, res->f_code))
      {
         if(SigIsFixedType(bank->sig, res->f_code))
         {
            AktTokenError(in,
                          "Predicate used as function symbol in preceding term",
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
Term_p do_tb_insert_no_props_cached(TB_p bank, Term_p term,
                                    DerefType deref, PObjMap_p* cache)
{
   int    i;
   Term_p t;

   assert(term);

   term = problemType == PROBLEM_HO && deref == DEREF_ALWAYS ?
            WHNF_deref(term) : TermDeref(term, &deref);
   Term_p cached = PObjMapFind(cache, term, PCmpFun);
   if(cached)
   {
      t = cached;
   }
   else
   {
      const int limit = DEREF_LIMIT(term, deref);

      if(TermIsFreeVar(term))
      {
         t = VarBankVarAssertAlloc(bank->vars, term->f_code, term->type);
         TermSetBank(t, bank);
      }
      else if (TermIsDBVar(term))
      {
         t = TBRequestDBVar(bank, term->type, term->f_code);
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
            t->args[i] = do_tb_insert_no_props_cached(bank, term->args[i],
                                                      CONVERT_DEREF(i, limit, deref), cache);
         }
         t = tb_termtop_insert(bank, t);
      }
      PObjMapStore(cache, term, t, PCmpFun);
   }
   return t;
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
      if(TermIsFreeVar(term->args[0]) && term->args[1]->f_code == SIG_TRUE_CODE)
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

static Term_p tb_term_parse_arglist(Scanner_p in, TB_p bank,
                                    bool check_symb_prop, Type_p type)
{
   Term_p   tmp;
   Term_p result;
   PStackPointer i=0, arity;
   PStack_p args;

   AcceptInpTok(in, OpenBracket);
   if(TestInpTok(in, CloseBracket))
   {
      NextToken(in);
      result = TermDefaultCellAlloc();
      return result;
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
   result = TermDefaultCellArityAlloc(arity);

   for(i=0;i<arity;i++)
   {
      result->args[i] = PStackElementP(args,i);
   }

   PStackFree(args);

   return result;
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
   handle->recovered = 0;
   handle->ext_index = PDIntArrayAlloc(1,100000);
   handle->garbage_state = TPIgnoreProps;
   handle->gc = GCAdminAlloc();
   handle->sig = sig;
   handle->vars = VarBankAlloc(sig->type_bank);
   handle->vars->term_bank = handle;
   handle->db_vars = DBVarBankAlloc();
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
   GCAdminFree(junk->gc);
   VarBankFree(junk->vars);
   PDArrayFree(junk->min_terms);
   DBVarBankFree(junk->db_vars);
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

   term = problemType == PROBLEM_HO && deref == DEREF_ALWAYS ?
            WHNF_deref(term) : TermDeref(term, &deref);

   if(TermIsFreeVar(term))
   {
      t = VarBankVarAssertAlloc(bank->vars, term->f_code, term->type);
   }
   else if (TermIsDBVar(term))
   {
      t = TBRequestDBVar(bank, term->type, term->f_code);
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
// Function: TBInsertInstantiatedDeref()
//
//  Insert the term, following the bindings of the variables
//  according to DerefType.
//
// Global Variables: -
//
// Side Effects    : Changes term bank
//
/----------------------------------------------------------------------*/

Term_p TBInsertInstantiatedDeref(TB_p bank, Term_p term, DerefType deref)
{
   if(deref == DEREF_NEVER)
   {
      return term;
   }
   int    i;
   Term_p t;
   const int limit = DEREF_LIMIT(term, deref);

   assert(term);
   term = problemType == PROBLEM_HO && deref == DEREF_ALWAYS ?
            WHNF_deref(term) : TermDeref(term, &deref);

   if(TermIsAnyVar(term) || TermIsGround(term))
   {
      t = term;
   }
   else
   {
      t = TermTopCopyWithoutArgs(term); /* This is an unshared term cell at the moment */

      for(i=0; i<t->arity; i++)
      {
         t->args[i] =
            TBInsertInstantiatedDeref(bank ? bank : TermGetBank(term),
                                      term->args[i], CONVERT_DEREF(i, limit, deref));
      }
      t = tb_termtop_insert(bank, t);
   }
   return t;
}

/*-----------------------------------------------------------------------
//
// Function: TBInsertIgnoreVar()
//
//  As TBInsert, but does instead of using variables from the term bank,
//  uses the ones already present in the temr.
//
//  TermProperties are masked with bank->prop_mask.
//
// Global Variables: -
//
// Side Effects    : Changes term bank
//
/----------------------------------------------------------------------*/

Term_p TBInsertIgnoreVar(TB_p bank, Term_p term, DerefType deref)
{
   int    i;
   Term_p t;
   const int limit = DEREF_LIMIT(term, deref);

   assert(term);

   term = problemType == PROBLEM_HO && deref == DEREF_ALWAYS ?
            WHNF_deref(term) : TermDeref(term, &deref);

   if(TermIsAnyVar(term))
   {
      return term;
   }
   else
   {
      t = TermTopCopyWithoutArgs(term); /* This is an unshared term cell at the moment */

      assert(SysDateIsCreationDate(t->rw_data.nf_date[0]));
      assert(SysDateIsCreationDate(t->rw_data.nf_date[1]));

      for(i=0; i<t->arity; i++)
      {
         t->args[i] = TBInsertIgnoreVar(bank, term->args[i],
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
   TB_p tmp_bank = TermGetBank(term);

   assert(term);

   TermSetBank(term, bank); //Cheat because WHNF_deref() needs it, God
                            //knows for what!
   const int limit = DEREF_LIMIT(term, deref);
   t = problemType == PROBLEM_HO && deref == DEREF_ALWAYS ?
      WHNF_deref(term) : TermDeref(term, &deref);
   TermSetBank(term, tmp_bank);
   term = t;

   if(TermIsFreeVar(term))
   {
      t = VarBankVarAssertAlloc(bank->vars, term->f_code, term->type);
   }
   else if (TermIsDBVar(term))
   {
      t = TBRequestDBVar(bank, term->type, term->f_code);
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
// Function: TBInsertNoPropsCached()
//
//   As TBInsert, but will set all properties of the new term to 0
//   first. Also, use a cache so that work is not repeated for the
//   same terms.
//
// Global Variables: TBSupportReplace
//
// Side Effects    : Changes term bank
//
/----------------------------------------------------------------------*/
#define CACHE_THRESHOLD 8096
Term_p TBInsertNoPropsCached(TB_p bank, Term_p term, DerefType deref)
{
   Term_p res;
   if(term->f_count > CACHE_THRESHOLD)
   {
      PObjMap_p cache = NULL;
      res = do_tb_insert_no_props_cached(bank, term, deref, &cache);
      PObjMapFree(cache);
   }
   else
   {
      res = TBInsertNoProps(bank, term, deref);
   }
   return res;
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
   term = problemType == PROBLEM_HO && deref == DEREF_ALWAYS ?
            WHNF_deref(term) : TermDeref(term, &deref);

   if(TermIsFreeVar(term))
   {
      t = VarBankVarAssertAlloc(bank->vars, term->f_code, term->type);
   }
   else if (TermIsDBVar(term))
   {
      t = TBRequestDBVar(bank, term->type, term->f_code);
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
   if(TermIsAnyVar(term))
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

   if(TermIsFreeVar(term))
   {
      t = VarBankVarAssertAlloc(bank->vars, term->f_code, term->type);
   }
   else if (TermIsDBVar(term))
   {
      t = TBRequestDBVar(bank, term->type, term->f_code);
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

   if(TermIsGround(term) && TermIsShared(term))
   {
      return term;
   }

   if(TermIsFreeVar(term) && term->binding)
   {
      t = follow_bind ? TBInsert(bank, term->binding, DEREF_NEVER) : term;
      TermSetBank(t, bank);
      return t;
   }
   else if (TermIsDBVar(term))
   {
      t = TBRequestDBVar(bank, term->type, term->f_code);
      return t;
   }

   int ignore_args = 0;
   if(TermIsAppliedFreeVar(term) && term->args[0]->binding && follow_bind)
   {
      Term_p binding = term->args[0]->binding;
      ignore_args =
         TermIsLambda(binding) ? 1 : (binding->arity + (TermIsFreeVar(binding) ? 1 : 0));
      DerefType d = DEREF_ONCE;
      term = TermDeref(term, &d);
   }

   if(TermIsFreeVar(term))
   {
      t = VarBankVarAssertAlloc(bank->vars, term->f_code, term->type);
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
   term = problemType == PROBLEM_HO && deref == DEREF_ALWAYS ?
            WHNF_deref(term) : TermDeref(term, &deref);

   if(TermIsGround(term))
   {
      assert(TermIsShared(term));
      return term;
   }

   if(TermIsFreeVar(term))
   {
      t = VarBankVarAssertAlloc(bank->vars, term->f_code, term->type);
   }
   else if (TermIsDBVar(term))
   {
      t = TBRequestDBVar(bank, term->type, term->f_code);
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

   if(TermIsFreeVar(term))
   {
      t = VarBankGetAltVar(bank->vars, term);
      // t = VarBankVarAssertAlloc(bank->vars, term->f_code+1, term->sort);
   }
   else if (TermIsDBVar(term))
   {
      t = TBRequestDBVar(bank, term->type, term->f_code);
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
   if(TermIsFreeVar(term))
   {
      return VarBankFCodeFind(bank->vars, term->f_code);
   }
   else if (TermIsDBVar(term))
   {
      return TBRequestDBVar(bank, term->type, term->f_code);
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
      if(TermIsFreeVar(term))
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
            assert(term->arity>0);
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
   DStr_p        source_name;
   Type_p        type;
   long          line, column;
   StreamType    type_stream;

   source_name = DStrGetRef(AktToken(in)->source);
   type_stream = AktToken(in)->stream_type;
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

      if(TestInpTok(in, LetToken))
      {
         handle = ParseLet(in, bank);
      }
      else if(TestInpTok(in, IteToken))
      {
         handle = ParseIte(in, bank);
      }
      else if((id_type=TermParseOperator(in, id))==FSIdentVar)
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
         handle = NULL;

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

            handle = tb_term_parse_arglist(in, bank,
                                           check_symb_prop, sym_type);
         }
         else
         {
            handle = TermDefaultCellAlloc();
            handle->arity = 0;
         }
         handle->f_code = TermSigInsert(bank->sig, DStrView(id),
                                        handle->arity, false, id_type);
         if(!handle->f_code)
         {
            Error("%s %s used with arity %d but registered with arity %d",
                  TYPE_ERROR,
                  PosRep(type_stream, source_name, line, column),
                  DStrView(id),
                  handle->arity,
                  SigFindArity(bank->sig, SigFindFCode(bank->sig, DStrView(id))));
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
// Function: TBTermParseSimple()
//
//   Parses terms without giving any special semantics to symbols.
//   Input variant of TermPrintSimple().
//
// Global Variables: -
//
// Side Effects    : Input, memory operations, changes termbank.
//
/----------------------------------------------------------------------*/

Term_p TBTermParseSimple(Scanner_p in, TB_p bank)
{
   Term_p        handle;
   DStr_p        id;
   FuncSymbType  id_type;
   DStr_p        source_name;
   Type_p        type;
   long          line, column;
   StreamType    type_stream;

   source_name = DStrGetRef(AktToken(in)->source);
   type_stream = AktToken(in)->stream_type;
   line = AktToken(in)->line;
   column = AktToken(in)->column;

   /* Normal term stuff, bloated because of the nonsensical SETHEO
      syntax */
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
      if(TestInpTok(in, OpenBracket))
      {
         AcceptInpTok(in, OpenBracket);
         if(TestInpTok(in, CloseBracket))
         {
            NextToken(in);
            handle = TermDefaultCellAlloc();
         }
         else
         {
            PStack_p args = PStackAlloc();
            int i=0;

            PStackPushP(args, TBTermParseSimple(in, bank));
            i++;

            while(TestInpTok(in, Comma))
            {
               NextToken(in);
               PStackPushP(args, TBTermParseSimple(in, bank));
               i++;
            }

            AcceptInpTok(in, CloseBracket);
            int arity = PStackGetSP(args);
            handle = TermDefaultCellArityAlloc(arity);

            for(i=0;i<arity;i++)
            {
               handle->args[i] = PStackElementP(args,i);
            }

            PStackFree(args);
         }
      }
      else
      {
         handle = TermDefaultCellAlloc();
         handle->arity = 0;
      }
      handle->f_code = TermSigInsert(bank->sig, DStrView(id),
                                       handle->arity, false, id_type);
      if(!handle->f_code)
      {
         Error("%s %s used with arity %d but registered with arity %d",
               TYPE_ERROR,
               PosRep(type_stream, source_name, line, column),
               DStrView(id),
               handle->arity,
               SigFindArity(bank->sig, SigFindFCode(bank->sig, DStrView(id))));

      }
      handle = tb_termtop_insert(bank, handle);
   }
   DStrFree(id);
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

   assert(!TermIsAnyVar(*ref));

   term = *ref;
   if(TermCellQueryProp(term, prop)||TermIsAnyVar(term))
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
   if(!TermCellIsAnyPropSet(term, prop)||TermIsAnyVar(term))
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

         if(TermIsAppliedFreeVar(term) && TermGetCache(term))
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
              COMCHAR" Garbage collection reclaimed %ld unused term cells.\n",
              recovered);
   }
#endif
   bank->garbage_state =
      bank->garbage_state?TPIgnoreProps:TPGarbageFlag;
   bank->recovered+=recovered;
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
   assert(type);
   Term_p t = PDArrayElementP(bank->min_terms, type->type_uid);

   if(!t)
   {
      t = TBCreateConstTerm(bank, min_const);
      assert(t->type);
      PDArrayAssignP(bank->min_terms, type->type_uid, t);
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

   if (TermIsAnyVar(term) || TermIsConst(term))
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
//   occurrences for each symbol in conjecture clauses, dist_array the
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
/*-----------------------------------------------------------------------
//
// Function: TermMap()
//
//   Applies the function f to term t to obtain t'. If t' != t,
//   it continues mapping t'. Else, it recursively applies f to
//   arguments of t. Result term is guaranteed to be shared.
//   Term mapper must also return shared term of the same type
//   as the original one. IMPORTANT: If f returns NULL this signifies
//   that recursion should stop and the term is unaltered: this is used
//   to painlessly implement optimizations.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Term_p TermMap(TB_p bank, Term_p t, TermMapper f)
{
   assert(t);
   Term_p s = f(bank, t);
   assert(!s || (TermIsShared(s) && s->type == t->type));

   if(!s)
   {
      s = t;
   }
   else if (s!=t)
   {
      s = TermMap(bank, s, f);
   }
   else
   {
      bool changed = false;
      s = TermTopCopyWithoutArgs(t);
      for(int i=0; i < t->arity; i++)
      {
         s->args[i] = TermMap(bank, t->args[i], f);
         assert(TermIsShared(s->args[i]) && s->args[i]->type == t->args[i]->type);
         changed = changed || (s->args[i] != t->args[i]);
      }

      if(changed)
      {
         s = TBTermTopInsert(bank, s);
      }
      else
      {
         TermTopFree(s);
         s = t;
      }
   }
   return s;
}

/*-----------------------------------------------------------------------
//
// Function: ParseLet()
//
//   Parses let according to the TPTP description:
//    http://ceur-ws.org/Vol-2162/paper-07.pdf. If top_level is true,
//   let appears at the formula level and its body must be Bool.
//   Otherwise, its body is parsed as a non-Bool.
//
// Global Variables: -
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

Term_p ParseLet(Scanner_p in, TB_p bank)
{
   AcceptInpTok(in, LetToken);
   AcceptInpTok(in, OpenBracket);

   PStack_p type_decls = PStackAlloc();
   /* parsing type declarations */
   if(TestInpTok(in, OpenSquare))
   {
      AcceptInpTok(in, OpenSquare);
      parse_let_typedecl(in, bank, type_decls);
      while(TestInpTok(in, Comma))
      {
         AcceptInpTok(in, Comma);
         parse_let_typedecl(in, bank, type_decls);
      }
      AcceptInpTok(in, CloseSquare);
   }
   else
   {
      parse_let_typedecl(in, bank, type_decls);
   }

   AcceptInpTok(in, Comma);

   /* parsing symbol definitions */
   PStack_p definitions = PStackAlloc();
   if(TestInpTok(in, OpenSquare))
   {
      AcceptInpTok(in, OpenSquare);
      PStackPushP(definitions, parse_let_sym_def(in, bank, type_decls));
      while(TestInpTok(in, Comma))
      {
         AcceptInpTok(in, Comma);
         PStackPushP(definitions, parse_let_sym_def(in, bank, type_decls));
      }
      AcceptInpTok(in, CloseSquare);
   }
   else
   {
      PStackPushP(definitions, parse_let_sym_def(in, bank, type_decls));
   }

   AcceptInpTok(in, Comma);

   SigEnterLetScope(bank->sig, type_decls);
   Term_p body = TFormulaTSTPParse(in, bank);
   SigExitLetScope(bank->sig);

   Term_p let_term = make_let(bank, definitions, body);

   while(!(PStackEmpty(type_decls)))
   {
      UNUSED(PStackPopP(type_decls));
      DStrFree(PStackPopP(type_decls));
   }
   AcceptInpTok(in, CloseBracket);

   PStackFree(type_decls);
   PStackFree(definitions);

   return let_term;
}

/*-----------------------------------------------------------------------
//
// Function: ParseIte()
//
//   Parses ite according to the TPTP description:
//    http://ceur-ws.org/Vol-2162/paper-07.pdf. If top_level is true,
//   ite appears at the formula level and its body must be Bool.
//   Otherwise, its body is parsed as a non-Bool.
//
// Global Variables: -
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

Term_p ParseIte(Scanner_p in, TB_p bank)
{
   AcceptInpTok(in, IteToken);
   AcceptInpTok(in, OpenBracket);
   Term_p cond = TFormulaTSTPParse(in, bank);
   AcceptInpTok(in, Comma);
   Term_p if_true = TFormulaTSTPParse(in, bank);
   AcceptInpTok(in, Comma);
   Term_p if_false = TFormulaTSTPParse(in, bank);
   AcceptInpTok(in, CloseBracket);

   Term_p res = TermTopAlloc(SIG_ITE_CODE, 3);
   res->args[0] = cond;
   res->args[1] = if_true;
   res->args[2] = if_false;

   TermAssertSameSort(bank->sig, cond, bank->true_term);
   TermAssertSameSort(bank->sig, if_true, if_false);

   res->type = if_true->type;
   return TBTermTopInsert(bank, res);
}

/*-----------------------------------------------------------------------
//
// Function: NormalizePatternAppVar()
//
//   Tries to normalize applied variable so that all of its arguments
//   are (?!?)
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p NormalizePatternAppVar(TB_p bank, Term_p s)
{
   if(TermIsFreeVar(s))
   {
      return s;
   }

   assert(TermIsAppliedFreeVar(s));
   assert(bank);

   s = LambdaEtaReduceDB(bank, s);
   bool all_dbs = true;
   for(long i=1; i<s->arity && all_dbs; i++)
   {
      all_dbs = TermIsDBVar(s->args[i]);
   }

   if(all_dbs && TermArrayNoDuplicates(s->args, s->arity))
   {
      return s;
   }
   else
   {
      return NULL;
   }
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
