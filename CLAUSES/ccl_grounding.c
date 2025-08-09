/*-----------------------------------------------------------------------

  File  : ccl_grounding.c

  Author: Stephan Schulz

  Contents

  Functions for implementing grounding of near-propositional clause
  sets. Note that all functons here require a fairly static term bank
  - don't do any freeing or rewriting intermingeled with grounding, it
  may well smoke and burn!

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created:  Tue May 29 02:25:12 CEST 2001

  -----------------------------------------------------------------------*/

#include "ccl_grounding.h"



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
// Function: clause_get_max_lit()
//
//   Return maximal propositional literal number in clause.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long clause_get_max_lit(Clause_p clause)
{
   long res = 0;
   Eqn_p handle;

   for(handle = clause->literals; handle; handle = handle->next)
   {
      res = MAX(res, handle->lterm->entry_no);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: varsetinstapply()
//
//   Given a complete VarSetInst data structure, instaniate the
//   variables apropriately.
//
// Global Variables: -
//
// Side Effects    : Changes instantiations of variables
//
/----------------------------------------------------------------------*/

void varsetinstapply(VarSetInst_p varinst)
{
   long   i;
   Term_p t;

   for(i=0; i< varinst->size; i++)
   {
      t = PStackElementP(varinst->cells[i].alternatives,
                         varinst->cells[i].position);
      /* printf("varsetinstapply: i=%ld, position = %ld\n",i,
         varinst->cells[i].position); */
      assert(!TermIsFreeVar(t));
      assert(TermIsFreeVar(varinst->cells[i].variable));
      varinst->cells[i].variable->binding = t;
   }
}

/*-----------------------------------------------------------------------
//
// Function: varsetinstclear()
//
//   Clear variables instatiated by a VarSetInst structure.
//
// Global Variables: -
//
// Side Effects    : Changes instantiations of variables
//
/----------------------------------------------------------------------*/

void varsetinstclear(VarSetInst_p varinst)
{
   long i;

   for(i=0; i< varinst->size; i++)
   {
      varinst->cells[i].variable->binding = NULL;
   }
}


/*-----------------------------------------------------------------------
//
// Function: varsetinstinitialize()
//
//   Initialize a VarSetInst structure to represent the initial ground
//   substitution (PStacks with alternatives have to be present!).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool varsetinstinitialize(VarSetInst_p varinst)
{
   long i,tmp;

   for(i=0; i< varinst->size; i++)
   {
      assert(varinst->cells[i].alternatives);
      tmp = PStackGetSP(varinst->cells[i].alternatives)-1;
      if(tmp<0)
      {
         return false;
      }
      varinst->cells[i].position = tmp;
   }
   return true;
}


/*-----------------------------------------------------------------------
//
// Function: varinstestimate()
//
//   Return the number of clauses induced by inst.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

double varinstestimate(VarSetInst_p inst)
{
   long i;
   double res = 1.0;

   for(i=0; i< inst->size; i++)
   {
      assert(inst->cells[i].alternatives);
      res = res * PStackGetSP(inst->cells[i].alternatives);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: varsetinstnext()
//
//   Switch to the next substitution, return false if substitution
//   space is exhausted (true otherwise).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool varsetinstnext(VarSetInst_p varinst)
{
   long i;

   for(i=0; i< varinst->size; i++)
   {
      if(varinst->cells[i].position)
      {
         varinst->cells[i].position--;
         return true;
      }
      varinst->cells[i].position =
         PStackGetSP(varinst->cells[i].alternatives)-1;
   }
   return false;
}


/*-----------------------------------------------------------------------
//
// Function: ground_set_print_unit()
//
//   Print a unit clause in standard E format.
//
// Global Variables: -
//
// Side Effects    : Output, memory operations
//
/----------------------------------------------------------------------*/

void ground_set_print_unit(FILE* out, GroundSet_p set, long unit, bool
                           pos)
{
   Eqn_p eqn = EqnAlloc(PDArrayElementP(set->unit_terms, unit),
                        set->lit_bank->true_term, set->lit_bank, pos);
   Clause_p clause = ClauseAlloc(eqn);

   ClausePrint(out, clause, true);
   ClauseFree(clause);
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: ClauseCmpByLen()
//
//   Compare two clauses. The shorter one is smaller. In clauses of
//   equal lenght, the one with more positive literals is smaller
//   (because I say so!). Otherwise, they are considered to be in the
//   same equivalence class.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int  ClauseCmpByLen(const void* clause1, const void* clause2)
{
   const Clause_p *c1 = clause1;
   const Clause_p *c2 = clause2;
   int res;

   res = ClauseLiteralNumber(*c1) - ClauseLiteralNumber(*c2);
   if(res)
   {
      return res;
   }
   return (*c1)->pos_lit_no - (*c2)->pos_lit_no;
}

/*-----------------------------------------------------------------------
//
// Function: EqnEqlitRecode()
//
//   Recode an equational literal as a non-equational one using
//   $eq(l,r)=T. Return true if recoded, false otherwise.
//
// Global Variables: -
//
// Side Effects    : Changes the literal in place!
//
/----------------------------------------------------------------------*/

bool EqnEqlitRecode(Eqn_p lit)
{
   if(EqnIsEquLit(lit))
   {
      Term_p t = TermDefaultCellArityAlloc(2);

      t->type = lit->bank->sig->type_bank->bool_type;
      t->f_code = SigGetEqnCode(lit->bank->sig, true);
      t->arity = 2;
      t->args[0] = lit->lterm;
      t->args[1] = lit->rterm;
      t = TBTermTopInsert(lit->bank, t);
      lit->lterm = t;
      lit->rterm = lit->bank->true_term;
      EqnDelProp(lit, EPIsEquLiteral);
      return true;
   }
   return false;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseEqlitRecode()
//
//   Recode a potential equational clause to a non-equational
//   one. Return true if conversion took place.
//
// Global Variables: -
//
// Side Effects    : Changes clause
//
/----------------------------------------------------------------------*/

bool ClauseEqlitRecode(Clause_p clause)
{
   Eqn_p handle;
   bool  res=false, tmp;

   for(handle = clause->literals; handle; handle = handle->next)
   {
      tmp = EqnEqlitRecode(handle);
      res = res | tmp;
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetEqlitRecode()
//
//   Recode all clauses in set, return number of conversions.
//
// Global Variables: -
//
// Side Effects    : Changes clauses
//
/----------------------------------------------------------------------*/

long ClauseSetEqlitRecode(ClauseSet_p set)
{
   long     res = 0;
   Clause_p handle;

   for(handle = set->anchor->succ; handle!= set->anchor; handle =
          handle->succ)
   {
      if(ClauseEqlitRecode(handle))
      {
         res++;
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: VarSetInstAlloc()
//
//   Create a VarSetInst for all variables occurring in clause. Does
//   not allocate the PStacks needed!
//
// Global Variables: -
//
// Side Effects    : Memory allocation
//
/----------------------------------------------------------------------*/

VarSetInst_p VarSetInstAlloc(Clause_p clause)
{
   PTree_p      tree = 0;
   VarSetInst_p handle = VarSetInstCellAlloc();
   long         i;
   Term_p       var;

   handle->size = ClauseCollectVariables(clause, &tree);
   if(handle->size)
   {
      handle->cells = SizeMalloc(handle->size*sizeof(VarInstCell));

      for(i=0; i<handle->size; i++)
      {
         assert(tree);

         var = PTreeExtractRootKey(&tree);
         assert(var);
         handle->cells[i].variable = var;
         handle->cells[i].alternatives = NULL;
      }
      assert(!tree);
   }
   else
   {
      handle->cells = NULL;
   }
   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: VarSetInstFree()
//
//   Free a VarSetInst. Does not free the PStacks()!
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void VarSetInstFree(VarSetInst_p junk)
{
   assert(junk);

   if(junk->size)
   {
      assert(junk->cells);
      SizeFree(junk->cells, junk->size*sizeof(VarInstCell));
   }
   else
   {
      assert(!junk->cells);
   }
   VarSetInstCellFree(junk);
}




/*-----------------------------------------------------------------------
//
// Function: VarSetConstrInstAlloc()
//
//   Create a VarSetInst for all variables occurring in clause,
//   constrained as much as possible. Does allocate the PStacks
//   needed!
//
// Global Variables: -
//
// Side Effects    : Memory allocation
//
/----------------------------------------------------------------------*/

VarSetInst_p VarSetConstrInstAlloc(LitOccTable_p p_table,
                                   LitOccTable_p n_table, Clause_p
                                   clause, PTree_p ground_terms)
{
   PTree_p      tree = NULL, tmp;
   VarSetInst_p handle = VarSetInstCellAlloc();
   long         i;
   Term_p       var;
   PDArray_p    var_constr;

   handle->size = ClauseCollectVariables(clause, &tree);
   if(handle->size)
   {
      handle->cells = SizeMalloc(handle->size*sizeof(VarInstCell));
      var_constr = PDArrayAlloc(DEFAULT_VARBANK_SIZE,
                                DEFAULT_VARBANK_SIZE);



      for(i=0; i<handle->size; i++)
      {
         assert(tree);

         var = PTreeExtractRootKey(&tree);
         assert(var);
         assert(TermIsFreeVar(var));
         tmp = PTreeCopy(ground_terms);
         PDArrayAssignP(var_constr, -var->f_code, tmp);

         handle->cells[i].variable = var;
         handle->cells[i].alternatives = PStackAlloc();
      }
      assert(!tree);

      ClauseCollectVarConstr(p_table, n_table, clause, ground_terms,
                             var_constr);
      for(i=0; i<handle->size; i++)
      {
         tmp = PDArrayElementP(var_constr,
                               -(handle->cells[i].variable->f_code));
         PTreeToPStack(handle->cells[i].alternatives, tmp);
         PTreeFree(tmp);
      }
      PDArrayFree(var_constr);
   }
   else
   {
      handle->cells = NULL;
   }
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: VarSetConstrInstFree()
//
//   Free a VarSetInst. Does free the PStacks() (and expects them to
//   be there).
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void VarSetConstrInstFree(VarSetInst_p junk)
{
   long i;

   assert(junk);

   for(i=0;  i < junk->size; i++)
   {
      assert(junk->cells[i].alternatives);
      PStackFree(junk->cells[i].alternatives);
   }
   if(junk->size)
   {
      assert(junk->cells);
      SizeFree(junk->cells, junk->size*sizeof(VarInstCell));
   }
   else
   {
      assert(!junk->cells);
   }
   VarSetInstCellFree(junk);
}



/*-----------------------------------------------------------------------
//
// Function: PrintDimacsHeader()
//
//   Print a Dimacs header with the given values.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void PrintDimacsHeader(FILE* out, long max_lit, long members)
{
   if(max_lit <= 0)
   {
      max_lit = 1;
   }
   fprintf(out, "p cnf %ld %ld\n", max_lit, members);
}


/*-----------------------------------------------------------------------
//
// Function: ClausePrintDimacs()
//
//   Print a clause in DIMACS format.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void ClausePrintDimacs(FILE* out, Clause_p clause)
{
   Eqn_p handle;

   if(ClauseIsEmpty(clause))
   {
      /* SATO is broken, and so may be lots of other provers...we just
         print a trivial problem that reduces to empty...*/
      fprintf(out, " -1 0\n  1 0\n");
   }
   else
   {
      for(handle = clause->literals; handle; handle = handle->next)
      {
         if(EqnIsPositive(handle))
         {
            fprintf(stdout, "  %ld", handle->lterm->entry_no);
         }
         else
         {
            fprintf(stdout, " -%ld", handle->lterm->entry_no);
         }
      }
      fputs(" 0\n", out);
   }
}

/*-----------------------------------------------------------------------
//
// Function: ClauseSetPrintDimacs()
//
//   Print a clause set in DIMACS format
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void ClauseSetPrintDimacs(FILE* out, ClauseSet_p set)
{
   Clause_p handle;

   for(handle = set->anchor->succ; handle != set->anchor; handle =
          handle->succ)
   {
      ClausePrintDimacs(out, handle);
   }
}


/*-----------------------------------------------------------------------
//
// Function: GroundSetAlloc()
//
//   Create a initialized GroundSet structure.
//
// Global Variables: -
//
// Side Effects    : Memory allocation
//
/----------------------------------------------------------------------*/

GroundSet_p GroundSetAlloc(TB_p bank)
{
   GroundSet_p set = GroundSetCellAlloc();

   assert(bank);

   set->lit_bank   = bank;
   set->unit_no    = 0;
   set->complete   = cpl_unknown;
   set->max_literal= 0;
   set->units      = PDIntArrayAlloc(DEFAULT_LIT_NO, DEFAULT_LIT_GROW);
   set->unit_terms = PDArrayAlloc(DEFAULT_LIT_NO, DEFAULT_LIT_GROW);
   set->non_units  = PropClauseSetAlloc();

   return set;
}


/*-----------------------------------------------------------------------
//
// Function: GroundSetFree()
//
//   Free a ground set.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void GroundSetFree(GroundSet_p junk)
{
   assert(junk);

   PDArrayFree(junk->units);
   PDArrayFree(junk->unit_terms);
   PropClauseSetFree(junk->non_units);
   /* Keep in mind that lit_bank is external! Strictly speaking, we
      would need to delete the dummy term references here - but we
      don't. This is just a hack, anyways! */
   GroundSetCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: GroundSetMaxVar()
//
//   Return the index of the largest variable in set.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long GroundSetMaxVar(GroundSet_p set)
{
   long         res=0, tmp, i;
   GCUEncoding  status;

   for(i=set->units->size;i>0; i--)
   {
      status = PDArrayElementInt(set->units, i);
      if(status)
      {
         res = i;
         break;
      }
   }
   tmp = PropClauseSetMaxVar(set->non_units);

   if(tmp > res)
   {
      res = tmp;
   }

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: GroundSetInsert()
//
//   Insert a (ground) clause into a GroundSet. Return false if clause
//   is already represented as a unit clause, true otherwise
//
// Global Variables: -
//
// Side Effects    : Changes set (duh!)
//
/----------------------------------------------------------------------*/

bool GroundSetInsert(GroundSet_p set, Clause_p clause)
{
   bool res;

   assert(set);
   assert(clause);

   /* printf("Inserting: ");
      ClausePrint(stdout, clause, true);
      printf("\n"); */

   if(!ClauseIsUnit(clause))
   {
      long tmp = clause_get_max_lit(clause);
      set->max_literal = MAX(set->max_literal,tmp);
      PropClauseSetInsertClause(set->non_units, clause);
      return true;
   }
   else
   {
      long        lit_no;
      GCUEncoding status;
      int         sign;

      assert(clause->literals);

      lit_no = EqnLitCode(clause->literals);
      sign   = EqnIsPositive(clause->literals)?GCUPos:GCUNeg;
      status = PDArrayElementInt(set->units, lit_no);

      if(status & sign)
      {
         res = false;
      }
      else
      {
         set->max_literal = MAX(set->max_literal, clause->literals->lterm->entry_no);
         PDArrayAssignInt(set->units, lit_no, status | sign);
         PDArrayAssignP(set->unit_terms, lit_no, clause->literals->lterm);
         set->unit_no++;
         res = true;
      }
      ClauseFree(clause);
      return res;
   }
}

/*-----------------------------------------------------------------------
//
// Function: GroundSetPrint()
//
//   Print a gound set to out.
//
// Global Variables: -
//
// Side Effects    : Output, memory operations
//
/----------------------------------------------------------------------*/

void GroundSetPrint(FILE* out, GroundSet_p set)
{
   long        i;
   GCUEncoding status;

   for(i=0; i<set->units->size; i++)
   {
      status = PDArrayElementInt(set->units, i);
      if(status&GCUPos)
      {
         ground_set_print_unit(out, set, i, true);
         fputc('\n', out);
      }
      if(status&GCUNeg)
      {
         ground_set_print_unit(out, set, i, false);
         fputc('\n', out);
      }
   }
   PropClauseSetPrint(out, set->lit_bank, set->non_units);
}

/*-----------------------------------------------------------------------
//
// Function: GroundSetPrintDimacs()
//
//   Print a gound set in DIMACS format to out (will not print
//   header!).
//
// Global Variables: -
//
// Side Effects    : Output, memory operations
//
/----------------------------------------------------------------------*/

void GroundSetPrintDimacs(FILE* out, GroundSet_p set)
{
   PropClause_p handle;
   Clause_p     tmp;
   long         i;
   GCUEncoding  status;

   for(i=0; i<set->units->size; i++)
   {
      status = PDArrayElementInt(set->units, i);
      if(status&GCUPos)
      {
         fprintf(out, "  %ld 0\n", i);
      }
      if(status&GCUNeg)
      {
         fprintf(out, " -%ld 0\n", i);
      }
   }
   for(handle = set->non_units->list; handle; handle = handle->next)
   {
      tmp = PropClauseToClause(set->lit_bank, handle);
      ClausePrintDimacs(out, tmp);
      ClauseFree(tmp);
   }
}

/*-----------------------------------------------------------------------
//
// Function: GroundSetUnitSimplifyClause()
//
//   Check if clause is subsumed by a unit clause from set. If yes,
//   return true. Otherwise, remove all units resolvable with units
//   from set and return false.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool GroundSetUnitSimplifyClause(GroundSet_p set, Clause_p clause,
                                 bool subsume, bool resolve)
{
   EqnRef      list = &(clause->literals);
   long        lit_enc;
   GCUEncoding status;

   while(*list)
   {
      lit_enc = EqnLitCode(*list);
      status  = PDArrayElementInt(set->units, lit_enc);

      if(EqnIsPositive(*list))
      {
         if(subsume&&(status&GCUPos))
         {
            return true;
         }
         if(resolve&&(status&GCUNeg))
         {
            EqnListDeleteElement(list);
            clause->pos_lit_no--;
         }
         else
         {
            list = &((*list)->next);
         }
      }
      else
      {
         if(subsume&&(status&GCUNeg))
         {
            return true;
         }
         if(resolve&&(status&GCUPos))
         {
            EqnListDeleteElement(list);
            clause->neg_lit_no--;
         }
         else
         {
            list = &((*list)->next);
         }
      }

   }
   return false;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseCreateGroundInstances()
//
//   Create all non-tautological ground instances of clause described
//   by inst. Return false if the empty clause has been created, true
//   otherwise.
//
// Global Variables: -
//
// Side Effects    : Memory operations, changes inst.
//
/----------------------------------------------------------------------*/

bool ClauseCreateGroundInstances(TB_p bank, Clause_p clause,
                                 VarSetInst_p inst, GroundSet_p
                                 groundset, bool subsume, bool
                                 resolve, bool taut_check)
{
   bool next = true;
   bool res = true;
   Eqn_p handle;
   Clause_p new;

   if(OutputLevel == 1)
   {
      fprintf(GlobalOut, COMCHAR);
      fflush(GlobalOut);
   }
   else if(OutputLevel >=2)
   {
      fputs(COMCHARRAW" ", GlobalOut);
      ClausePrint(GlobalOut, clause, true);
      fputc('\n', GlobalOut);
   }
   if(!varsetinstinitialize(inst))
   {
      return true;
   }
   while(next && res && !TimeIsUp && !MemIsLow)
   {
      varsetinstapply(inst);
      handle = EqnListCopy(clause->literals, bank);
      EqnListRemoveDuplicates(handle);
      if(taut_check && EqnListIsTrivial(handle))
      {
         EqnListFree(handle);
      }
      else
      {
         new = ClauseAlloc(handle);
         if(!GroundSetUnitSimplifyClause(groundset, new, subsume,
                                         resolve))
         {
            if(ClauseIsEmpty(new))
            {
               res = false;
               PropClauseSetFree(groundset->non_units);
               groundset->non_units = PropClauseSetAlloc();
               PDArrayFree(groundset->units);
               groundset->units = PDArrayAlloc(1,1);
               groundset->unit_no = 0;
            }
            GroundSetInsert(groundset, new);
         }
         else
         {
            ClauseFree(new);
         }
      }
      next = varsetinstnext(inst);
   }
   varsetinstclear(inst);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetCreateGroundInstances()
//
//   Create all ground instances of set and put them into
//   groundset. Return false if the empty clause has been detected,
//   true otherwise.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool ClauseSetCreateGroundInstances(TB_p bank, ClauseSet_p set,
                                    GroundSet_p groundset, bool
                                    subsume, bool resolve, bool
                                    taut_check, long give_up)
{
   PStack_p     default_terms = PStackAlloc();
   Clause_p     handle;
   VarSetInst_p inst;
   long         i;
   bool         res = true, tmp;

   SigCollectConstantTerms(bank, default_terms, 0);

   if(give_up)
   {
      long vars = ClauseSetMaxVarNumber(set);
      double est_inst=1;

      tmp = PStackGetSP(default_terms);
      for(i=0; i<vars; i++)
      {
         est_inst = est_inst*tmp;
         if(est_inst > give_up)
         {
            fprintf(GlobalOut, "\n"COMCHAR" Failure: User resource limit"
                    " exceeded (estimated number of instances)!\n");
            exit(NO_ERROR);
         }
      }
   }

   for(handle = set->anchor->succ; handle!= set->anchor && !TimeIsUp
          && !MemIsLow; handle = handle->succ)
   {
      inst = VarSetInstAlloc(handle);
      for(i=0; i<inst->size; i++)
      {
         inst->cells[i].alternatives = default_terms;
      }
      tmp = ClauseCreateGroundInstances(bank, handle, inst, groundset,
                                        subsume, resolve, taut_check);
      VarSetInstFree(inst);
      if(!tmp)
      {
         res = false;
         break;
      }
   }
   if(TimeIsUp)
   {
      groundset->complete = cpl_timeout;
   }
   else if(MemIsLow)
   {
      groundset->complete = cpl_lowmem;
   }
   else
   {
      groundset->complete = cpl_complete;
   }
   PStackFree(default_terms);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetCreateConstrGroundInstances()
//
//   Create ground instances of set using global instantiation
//   constraints. Return false if the empty clause has been found,
//   true otherwise. If
//   just_one_instance is set, just create a single instance (mapping
//   all variables to the most frequent symbol).
//
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool ClauseSetCreateConstrGroundInstances(TB_p bank, ClauseSet_p set,
                                          GroundSet_p groundset, bool
                                          subsume, bool resolve, bool
                                          taut_check, long give_up,
                                          long just_one_instance)
{
   PStack_p     default_terms = PStackAlloc();
   PTree_p      default_term_tree = NULL;
   Clause_p     handle;
   VarSetInst_p inst;
   LitOccTable_p p_table, n_table;
   bool         res = true, tmp;
   double       clause_estimate;

   if(just_one_instance)
   {
      SigCollectConstantTerms(bank, default_terms, just_one_instance);
   }
   else
   {
      SigCollectConstantTerms(bank, default_terms, 0);
   }
   PStackToPTree(&default_term_tree, default_terms);
   p_table = LitOccTableAlloc(bank->sig);
   n_table = LitOccTableAlloc(bank->sig);
   LitOccAddClauseSetAlt(p_table, n_table, set);

   for(handle = set->anchor->succ;
       handle!= set->anchor && !TimeIsUp && !MemIsLow;
       handle = handle->succ)
   {
      inst = VarSetConstrInstAlloc(p_table, n_table, handle,
                                   default_term_tree);

      if(give_up)
      {
         clause_estimate = varinstestimate(inst);
         if((GroundSetMembers(groundset)+clause_estimate) > give_up)
         {
            fprintf(GlobalOut, "\n"COMCHAR" Failure: User resource limit"
                    " exceeded (estimated number of instances)!\n");
            exit(NO_ERROR);
         }
      }

      tmp = ClauseCreateGroundInstances(bank, handle, inst, groundset,
                                        subsume, resolve, taut_check);
      VarSetConstrInstFree(inst);
      if(!tmp)
      {
         res = false;
         break;
      }
   }
   if(TimeIsUp)
   {
      groundset->complete = cpl_timeout;
   }
   else if(MemIsLow)
   {
      groundset->complete = cpl_lowmem;
   }
   else
   {
      groundset->complete = cpl_complete;
   }
   LitOccTableFree(p_table);
   LitOccTableFree(n_table);
   PStackFree(default_terms);
   PTreeFree(default_term_tree);

   return res;
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
