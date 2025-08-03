/*-----------------------------------------------------------------------

File  : ccl_f_generality.c

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Code for determining function symbol distributions.

  Copyright 2010 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Thu Jul  1 01:10:42 CEST 2010
    New

-----------------------------------------------------------------------*/

#include "ccl_f_generality.h"



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
// Function: init_fun_gen_cell()
//
//   Initi a FunGenCell for keeping track of occurrences of f.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void init_fun_gen_cell(FunGen_p cell, FunCode f)
{
   cell->f_code    = f;
   cell->term_freq = 0;
   cell->fc_freq   = 0;
}


/*-----------------------------------------------------------------------
//
// Function: gd_merge_single_res()
//
//   Merge the new f-counts in dist_array into dist.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void gd_merge_single_res(GenDistrib_p dist,
                                PStack_p symbol_stack,
                                short factor)
{
   FunCode i;
   PStackPointer sp;

   for(sp=0; sp < PStackGetSP(symbol_stack); sp++)
   {
      i = PStackElementInt(symbol_stack, sp);
      dist->dist_array[i].term_freq += factor*dist->f_distrib[i];
      dist->dist_array[i].fc_freq+=factor;
   }
}




/*-----------------------------------------------------------------------
//
// Function: fun_gen_tg_cmp_wrapper()
//
//   Wrapper around FunGenTGCmp() to sort stacks of pointers.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static int fun_gen_tg_cmp_wrapper(const void *fg1, const void *fg2)
{
   const IntOrP* f1 = (const IntOrP*) fg1;
   const IntOrP* f2 = (const IntOrP*) fg2;

   return FunGenTGCmp(f1->p_val, f2->p_val);
}


/*-----------------------------------------------------------------------
//
// Function: fun_gen_cg_cmp_wrapper()
//
//   Wrapper around FunGenCGCmp() to sort stacks of pointers.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static int fun_gen_cg_cmp_wrapper(const void *fg1, const void *fg2)
{
   const IntOrP* f1 = (const IntOrP*) fg1;
   const IntOrP* f2 = (const IntOrP*) fg2;

   return FunGenCGCmp(f1->p_val, f2->p_val);
}


/*-----------------------------------------------------------------------
//
// Function: extract_generality()
//
//   Given a FunGen_p and a gentype, return the proper generality
//   measure.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static long extract_generality(FunGen_p gen,  GeneralityMeasure gentype)
{
   long res = 0;

   switch(gentype)
   {
   case GMTerms:
         res = gen->term_freq;
         break;
   case GMFormulas:
         res = gen->fc_freq;
         break;
   default:
         assert(false && "Unknown generality type");
         break;
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// function: compute_d_rel()
//
//   Find the least general of the function symbols in dist_array and
//   push them onto res.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void compute_d_rel(GenDistrib_p      generality,
                          GeneralityMeasure gentype,
                          double            benevolence,
                          long              generosity,
                          PStack_p          symbol_stack,
                          PStack_p          res)
{
   FunCode       i;
   FunGen_p      gen;
   PStack_p      sort_stack;
   long          least_gen, gen_limit, aux_gen_limit;
   PStackPointer sp;

   sort_stack = PStackAlloc();

   for(sp=0; sp < PStackGetSP(symbol_stack); sp++)
   {
      i = PStackElementInt(symbol_stack, sp);
      if(i >= generality->sig->internal_symbols)
      {
         PStackPushP(sort_stack,
                     &(generality->dist_array[i]));
      }
   }
   if(!PStackEmpty(sort_stack))
   {
      switch(gentype)
      {
      case GMTerms:
            PStackSort(sort_stack, fun_gen_tg_cmp_wrapper);
            break;
      case GMFormulas:
            PStackSort(sort_stack, fun_gen_cg_cmp_wrapper);
            break;
      default:
            assert(false && "Unknown generality type");
            break;
      }
      gen = PStackElementP(sort_stack, 0);
      least_gen = extract_generality(gen, gentype);
      gen_limit = least_gen*benevolence;

      if(generosity>=PStackGetSP(sort_stack))
      {
         generosity = PStackGetTopSP(sort_stack);
      }
      gen = PStackElementP(sort_stack, generosity);
      aux_gen_limit = extract_generality(gen, gentype);
      //printf("generosity = %ld, gen_limit = %ld aux_gen_limit = %ld\n",
      //generosity, gen_limit, aux_gen_limit);
      if(aux_gen_limit <  gen_limit)
      {
         gen_limit = aux_gen_limit;
      }

      for(i=0; i<PStackGetSP(sort_stack); i++)
      {
         gen = PStackElementP(sort_stack, i);
         // fprintf(stderr, "generality(% ld)=%s: %ld\n",
         //    i,
         //    SigFindName(generality->sig, gen->f_code),
         //    extract_generality(gen, gentype));

         if(extract_generality(gen, gentype)>gen_limit)
         {
            break;
         }
         PStackPushInt(res, gen->f_code);
      }
      //printf("Selected %ld symbols including %ld\n", PStackGetSP(res), PStackTopInt(res));
   }
   PStackFree(sort_stack);
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: GenDistribAlloc()
//
//   Allocate an initialized GenDistribCell.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

GenDistrib_p GenDistribAlloc(Sig_p sig)
{
   GenDistrib_p handle = GenDistribCellAlloc();
   FunCode i;

   handle->sig        = sig;
   handle->size       = sig->f_count+1;
   handle->dist_array = SecureMalloc(handle->size*sizeof(FunGenCell));

   for(i=0; i<handle->size; i++)
   {
      init_fun_gen_cell(&(handle->dist_array[i]), i);
   }
   handle->f_distrib = SizeMalloc(handle->size*sizeof(long));
   memset(handle->f_distrib, 0, handle->size*sizeof(long));

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: GenDistribFree()
//
//   Free a GenDistrib cell. The signature is external!
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void GenDistribFree(GenDistrib_p junk)
{
   FREE(junk->dist_array);
   SizeFree(junk->f_distrib, junk->size*sizeof(long));
   GenDistribCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: GenDistribSizeAdjust()
//
//   Ensure that GenDistrib is large enough to accomodate all symbols
//   in sig.
//
// Global Variables: -
//
// Side Effects    : Memory management.
//
/----------------------------------------------------------------------*/

void GenDistribSizeAdjust(GenDistrib_p gd, Sig_p sig)
{
   long new_size;
   FunCode i;

   if(sig->f_count >= gd->size)
   {
      new_size = sig->f_count+1;
      gd->dist_array = SecureRealloc(gd->dist_array, new_size*sizeof(FunGenCell));
      for(i=gd->size; i<new_size; i++)
      {
         init_fun_gen_cell(&(gd->dist_array[i]), i);
      }
      SizeFree(gd->f_distrib, gd->size*sizeof(long));
      gd->f_distrib = SizeMalloc(new_size*sizeof(long));
      memset(gd->f_distrib, 0, new_size*sizeof(long));

      gd->size = new_size;
   }
}


/*-----------------------------------------------------------------------
//
// Function: GenDistribAddClause()
//
//   Add f_code occurrences to dist.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void GenDistribAddClause(GenDistrib_p dist,
                         Clause_p clause,
                         short factor)
{
   PStack_p symbol_stack = PStackAlloc();

   ClauseAddSymbolDistExist(clause,
                            dist->f_distrib,
                            symbol_stack);
   gd_merge_single_res(dist, symbol_stack, factor);

   while(!PStackEmpty(symbol_stack))
   {
      dist->f_distrib[PStackPopInt(symbol_stack)] = 0;
   }
   PStackFree(symbol_stack);
}



/*-----------------------------------------------------------------------
//
// Function: GenDistribAddClauseSet()
//
//   Add all clauses in set into the distribution.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void GenDistribAddClauseSet(GenDistrib_p dist,
                            ClauseSet_p set,
                            short factor)
{
   Clause_p handle;

   for(handle=set->anchor->succ;
       handle!=set->anchor;
       handle=handle->succ)
   {
      GenDistribAddClause(dist, handle, factor);
   }
}


/*-----------------------------------------------------------------------
//
// Function: GenDistribAddFormula()
//
//   Add a Formula to the distribution.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void GenDistribAddFormula(GenDistrib_p dist,
                          WFormula_p form,
                          bool trim,
                          short factor)
{
   PStack_p symbol_stack = PStackAlloc();
   Sig_p sig = form->terms->sig;

   TermAddSymbolDistExist((FormulaIsConjecture(form) && trim) ?
                          TermTrimImplications(sig, form->tformula) :
                          form->tformula,
                          dist->f_distrib,
                          symbol_stack);
   gd_merge_single_res(dist, symbol_stack, factor);

   while(!PStackEmpty(symbol_stack))
   {
      dist->f_distrib[PStackPopInt(symbol_stack)] = 0;
   }
   PStackFree(symbol_stack);
}


/*-----------------------------------------------------------------------
//
// Function: GenDistribAddFormulaSet()
//
//   Add all formulas in set into the distribution.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void GenDistribAddFormulaSet(GenDistrib_p dist,
                             FormulaSet_p set,
                             bool trim,
                             short factor)
{
   WFormula_p handle;

   for(handle=set->anchor->succ;
       handle!=set->anchor;
       handle=handle->succ)
   {
      GenDistribAddFormula(dist, handle, trim, factor);
   }
}


/*-----------------------------------------------------------------------
//
// Function: GenDistribAddClauseSetStack()
//
//   Add all clause sets on stack into dist.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void GenDistribAddClauseSetStack(GenDistrib_p dist,
                                 PStack_p stack,
                                 PStackPointer start,
                                 short factor)
{
   PStackPointer i;
   ClauseSet_p   handle;

   for(i=start; i<PStackGetSP(stack); i++)
   {
      handle = PStackElementP(stack, i);
      GenDistribAddClauseSet(dist, handle, factor);
   }
}

/*-----------------------------------------------------------------------
//
// Function: GenDistribAddFormulaSetStack)
//
//   Add all formula sets on stack into dist.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void GenDistribAddFormulaSetStack(GenDistrib_p dist, PStack_p stack,
                                  PStackPointer start, bool trim, short factor)
{
   PStackPointer i;
   FormulaSet_p   handle;

   for(i=start; i<PStackGetSP(stack); i++)
   {
      handle = PStackElementP(stack, i);
      GenDistribAddFormulaSet(dist, handle, trim, factor);
   }
}


/*-----------------------------------------------------------------------
//
// Function: GenDistribPrint()
//
//   Print the symbol distribution.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void GenDistribPrint(FILE* out, GenDistrib_p dist, long limit)
{
   FunCode i;
   long term_freq_total = 0,
      fc_freq_total = 0;

   for(i=dist->sig->internal_symbols+1; i<dist->size; i++)
   {
      term_freq_total+= dist->dist_array[i].term_freq;
      fc_freq_total  += dist->dist_array[i].fc_freq;
   }
   fprintf(out, COMCHAR" GenDist %p %ld %ld\n", dist, term_freq_total, fc_freq_total);

   for(i=dist->sig->internal_symbols+1; i<MIN(dist->size,dist->sig->internal_symbols+limit); i++)
   {
      fprintf(out, COMCHAR" %-30s (%8ld = %8ld): %8ld  %8ld\n",
              SigFindName(dist->sig, i),
              i,
              dist->dist_array[i].f_code,
              dist->dist_array[i].term_freq,
              dist->dist_array[i].fc_freq);
   }
}


/*-----------------------------------------------------------------------
//
// Function: FunGenTGCmp()
//
//   Compare function for FunGen cell pointers, by term-frequency,
//   tie-break by clause frequency, tie-break by f_code.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

int FunGenTGCmp(const FunGen_p fg1, const FunGen_p fg2)
{
   int res;

   res = CMP(fg1->term_freq, fg2->term_freq);
   if(res)
   {
      return res;
   }

   res = CMP(fg1->fc_freq, fg2->fc_freq);
   if(res)
   {
      return res;
   }

   res = CMP(fg1->f_code, fg2->f_code);

   return res;
}



/*-----------------------------------------------------------------------
//
// Function: FunGenCGCmp()
//
//   Compare function for FunGen cell pointers, by
//   clause/formula-frequency, tie-break by term frequency, tie-break
//   by f_code.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int FunGenCGCmp(const FunGen_p fg1, const FunGen_p fg2)
{
   int res;

   res = CMP(fg1->fc_freq, fg2->fc_freq);
   if(res)
   {
      return res;
   }

   res = CMP(fg1->term_freq, fg2->term_freq);
   if(res)
   {
      return res;
   }

   res = CMP(fg1->f_code, fg2->f_code);

   return res;
}




/*-----------------------------------------------------------------------
//
// Function: ClauseComputeDRel()
//
//   Push the FCodes of functions in D-relation with clause onto res.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void ClauseComputeDRel(GenDistrib_p generality,
                       GeneralityMeasure gentype,
                       double benevolence,
                       long generosity,
                       Clause_p clause,
                       PStack_p res)
{
   PStack_p      symbol_stack = PStackAlloc();

   /* memset(generality->f_distrib, 0, generality->size*sizeof(long)); */
   ClauseAddSymbolDistExist(clause,
                            generality->f_distrib,
                            symbol_stack);

   /* printf("Symbolstack has %d elements\n",
      PStackGetSP(symbol_stack)); */
   compute_d_rel(generality, gentype, benevolence, generosity, symbol_stack, res);

   while(!PStackEmpty(symbol_stack))
   {
      generality->f_distrib[PStackPopInt(symbol_stack)] = 0;
   }
   PStackFree(symbol_stack);
}


/*-----------------------------------------------------------------------
//
// Function: FormulaComputeDRel()
//
//   Push the FCodes of functions in D-relation with form onto res.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void FormulaComputeDRel(GenDistrib_p generality,
                        GeneralityMeasure gentype,
                        double benevolence,
                        long generosity,
                        WFormula_p form,
                        PStack_p res,
                        bool trim_impl)
{
   PStack_p      symbol_stack = PStackAlloc();
   Sig_p sig = form->terms->sig;

   // memset(generality->f_distrib, 0, generality->size*sizeof(long));
   TermAddSymbolDistExist(FormulaIsConjecture(form) && trim_impl ?
                          TermTrimImplications(sig,  form->tformula) : form->tformula,
                          generality->f_distrib,
                          symbol_stack);

   //printf("Symbolstack has %ld elements\n", PStackGetSP(symbol_stack));
   //fprintf(stdout, "sine(%s)=\n", WFormulaGetId(form));
   compute_d_rel(generality, gentype, benevolence, generosity, symbol_stack, res);

   while(!PStackEmpty(symbol_stack))
   {
      generality->f_distrib[PStackPopInt(symbol_stack)] = 0;
   }
   PStackFree(symbol_stack);
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
