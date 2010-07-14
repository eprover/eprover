/*-----------------------------------------------------------------------

File  : ccl_f_generality.c

Author: Stephan Schulz (schulz@eprover.org)

Contents
 
  Code for determining function symbol distributions.

  Copyright 2010 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
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
//   Initi a FunGenCell for keeping track of occurances of f.
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
                                long *dist_array, 
                                short factor)
{
   FunCode i;
   
   for(i=dist->sig->internal_symbols+1; i<dist->size; i++)
   {
      if(dist_array[i])
      {
         dist->dist_array[i].term_freq += factor*dist_array[i];
         dist->dist_array[i].fc_freq+=factor;
      }
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

static void compute_d_rel(GenDistrib_p generality, 
                          GeneralityMeasure gentype,
                          double benevolence,
                          long *f_occs, 
                          PStack_p res)
{
   FunCode i;
   FunGen_p gen;
   PStack_p sort_stack;
   long     least_gen, gen_limit;

   sort_stack = PStackAlloc();
   
   for(i=generality->sig->internal_symbols+1; 
       i<=generality->sig->f_count; 
       i++)
   {
      if(f_occs[i])
      {
         PStackPushP(sort_stack, &(generality->dist_array[i]));
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

      for(i=0; i<PStackGetSP(sort_stack); i++)
      {
         gen = PStackElementP(sort_stack, i);
         /* printf("generality(% ld)=%s: %ld\n", 
            i,
            SigFindName(generality->sig, gen->f_code),
            extract_generality(gen, gentype)); */
         if(extract_generality(gen, gentype)>gen_limit)
         {
            break;
         }
         PStackPushInt(res, gen->f_code);
      }
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
      gd->size = new_size;
   }
}


/*-----------------------------------------------------------------------
//
// Function: GenDistribAddClause()
//
//   Add f_code occurances to dist.
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
   long *dist_array = SizeMalloc(dist->size*sizeof(long));
   assert(clause);
   
   memset(dist_array, 0, dist->size*sizeof(long));
   ClauseAddSymbolDistribution(clause, dist_array);
   gd_merge_single_res(dist, dist_array, factor);
   
   SizeFree(dist_array, dist->size*sizeof(long));
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
                          short factor)
{
   long *dist_array = SizeMalloc(dist->size*sizeof(long));
   assert(form && form->tformula);
   
   memset(dist_array, 0, dist->size*sizeof(long));
   TermAddSymbolDistribution(form->tformula, dist_array);
   gd_merge_single_res(dist, dist_array, factor);
   
   SizeFree(dist_array, dist->size*sizeof(long));
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
                             short factor)
{
   WFormula_p handle;

   for(handle=set->anchor->succ; 
       handle!=set->anchor; 
       handle=handle->succ)
   {
      GenDistribAddFormula(dist, handle, factor);
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
                                  PStackPointer start, short factor)
{
   PStackPointer i;
   FormulaSet_p   handle;

   for(i=start; i<PStackGetSP(stack); i++)
   {
      handle = PStackElementP(stack, i);
      GenDistribAddFormulaSet(dist, handle, factor);
   }
}


/*-----------------------------------------------------------------------
//
// Function: GenDistPrint()
//
//   Print the symbol distribution.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void GenDistPrint(FILE* out, GenDistrib_p dist)
{
   FunCode i;
   
   for(i=dist->sig->internal_symbols+1; i<dist->size; i++)
   {
      fprintf(out, "# %-20s (%8ld = %8ld): %8ld  %8ld\n", 
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
   long res;

   res = fg1->term_freq - fg2->term_freq;
   if(!res)
   {
      res = fg1->fc_freq - fg2->fc_freq;
   }
   if(!res)
   {
      res = fg1->f_code - fg2->f_code;
   }
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
   long res;

   res = fg1->fc_freq - fg2->fc_freq;
   if(!res)
   {
      res = fg1->term_freq - fg2->term_freq;
   }
   if(!res)
   {
      res = fg1->f_code - fg2->f_code;
   }
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
                       Clause_p clause, 
                       PStack_p res)
{
   long *dist_array = SizeMalloc(generality->size*sizeof(long));
   
   memset(dist_array, 0, generality->size*sizeof(long));
   ClauseAddSymbolDistribution(clause, dist_array);
   
   compute_d_rel(generality, gentype, benevolence, dist_array, res);

   SizeFree(dist_array, generality->size*sizeof(long));
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
                        WFormula_p form, 
                        PStack_p res)
{   
   long *dist_array = SizeMalloc(generality->size*sizeof(long));  
  
   memset(dist_array, 0, generality->size*sizeof(long));
   TermAddSymbolDistribution(form->tformula, dist_array);
   
   compute_d_rel(generality, gentype, benevolence, dist_array, res);

   SizeFree(dist_array, generality->size*sizeof(long));
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


