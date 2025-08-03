/*-----------------------------------------------------------------------

File  : pcl_propanalysis.c

Author: Stephan Schulz

Contents

  Functions for collecting various amounts of statistical information
  about a PCL protocol.

Copyright 1998-2011 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Fri Mar  8 15:56:49 CET 2002
    New

-----------------------------------------------------------------------*/

#include "pcl_propanalysis.h"



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
// Function: pcl_weight_compare()
//
//   Compare two PCL steps by standard weight of the clause.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static int pcl_weight_compare(PCLStep_p step1, PCLStep_p step2)
{
   double w1, w2;

   if(PCLStepIsFOF(step1) && PCLStepIsFOF(step2))
   {
      return 0;
   }
   else if(PCLStepIsFOF(step1))
   {
      return -1;
   }
   else if(PCLStepIsFOF(step2))
   {
      return 1;
   }

   w1 = ClauseStandardWeight(step1->logic.clause);
   w2 = ClauseStandardWeight(step2->logic.clause);

   if(w1 < w2)
   {
      return -1;
   }
   if(w1 > w2)
   {
      return 1;
   }
   return 0;
}


/*-----------------------------------------------------------------------
//
// Function: pcl_sc_compare()
//
//   Compare two clause PCL steps by strict symbol count of the
//   clause. FOF steps are always smaller and equivalent.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static int pcl_sc_compare(PCLStep_p step1, PCLStep_p step2)
{
   double w1, w2;

   if(PCLStepIsFOF(step1) && PCLStepIsFOF(step2))
   {
      return 0;
   }
   else if(PCLStepIsFOF(step1))
   {
      return -1;
   }
   else if(PCLStepIsFOF(step2))
   {
      return 1;
   }

   w1 = ClauseSymTypeWeight(step1->logic.clause, 1,1,1,1,1,1,1,1);
   w2 = ClauseSymTypeWeight(step2->logic.clause, 1,1,1,1,1,1,1,1);

   if(w1 < w2)
   {
      return -1;
   }
   if(w1 > w2)
   {
      return 1;
   }
   return 0;
}


/*-----------------------------------------------------------------------
//
// Function: pcl_litno_compare()
//
//   Compare two PCL steps by literal number.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static int pcl_litno_compare(PCLStep_p step1, PCLStep_p step2)
{
   int w1, w2;

   if(PCLStepIsFOF(step1) && PCLStepIsFOF(step2))
   {
      return 0;
   }
   else if(PCLStepIsFOF(step1))
   {
      return -1;
   }
   else if(PCLStepIsFOF(step2))
   {
      return 1;
   }
   w1 = ClauseLiteralNumber(step1->logic.clause);
   w2 = ClauseLiteralNumber(step2->logic.clause);

   if(w1 < w2)
   {
      return -1;
   }
   if(w1 > w2)
   {
      return 1;
   }
   return 0;
}

/*-----------------------------------------------------------------------
//
// Function: pcl_depth_compare()
//
//   Compare two PCL steps by clause depth.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static int pcl_depth_compare(PCLStep_p step1, PCLStep_p step2)
{
   int w1, w2;

   if(PCLStepIsFOF(step1) && PCLStepIsFOF(step2))
   {
      return 0;
   }
   else if(PCLStepIsFOF(step1))
   {
      return -1;
   }
   else if(PCLStepIsFOF(step2))
   {
      return 1;
   }
   w1 = ClauseDepth(step1->logic.clause);
   w2 = ClauseDepth(step2->logic.clause);

   if(w1 < w2)
   {
      return -1;
   }
   if(w1 > w2)
   {
      return 1;
   }
   return 0;
}


/*-----------------------------------------------------------------------
//
// Function: pcl_prot_global_count()
//
//   Determine the global properties of the PCL listing.
//
// Global Variables: -
//
// Side Effects    : Only temporary memory operations.
//
/----------------------------------------------------------------------*/

static void pcl_prot_global_count(PCLProt_p prot, PCLPropData_p data)
{
   PCLStep_p tmp;
   Clause_p  clause;
   PStackPointer i;

   assert(prot && data);

   data->fof_formulae        = 0;
   data->pos_clauses         = 0;
   data->neg_clauses         = 0;
   data->mix_clauses         = 0;
   data->pos_clause_literals = 0;
   data->neg_clause_literals = 0;
   data->mix_clause_literals = 0;
   data->pos_literals        = 0;
   data->neg_literals        = 0;
   data->const_count         = 0;
   data->func_count          = 0;
   data->pred_count          = 0;
   data->var_count           = 0;

   PCLProtSerialize(prot);

   for(i=0; i<PStackGetSP(prot->in_order); i++)
   {
      tmp = PStackElementP(prot->in_order, i);
      if(PCLStepIsFOF(tmp))
      {
         data->fof_formulae++;
      }
      else
      {
         clause = tmp->logic.clause;

         if(!ClauseIsEmpty(clause))
         {
            if(ClauseIsPositive(clause))
            {
       data->pos_clauses++;
       data->pos_clause_literals += ClauseLiteralNumber(clause);
            }
            else if(ClauseIsNegative(clause))
            {
               data->neg_clauses++;
               data->neg_clause_literals += ClauseLiteralNumber(clause);
            }
            else
            {
               data->mix_clauses++;
               data->mix_clause_literals += ClauseLiteralNumber(clause);
            }
            data->pos_literals += clause->pos_lit_no;
            data->neg_literals += clause->neg_lit_no;
            data->const_count  += ClauseSymTypeWeight(clause,
                                                      1,1,1,0,0,1,0,1);
            data->func_count   += ClauseSymTypeWeight(clause,
                                                      1,1,1,0,1,0,0,1);
            data->pred_count   += ClauseSymTypeWeight(clause,
                                                      1,1,1,0,0,0,1,1);
            data->var_count   += ClauseSymTypeWeight(clause,
                                                     1,1,1,1,0,0,0,1);
         }
      }
   }
}



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: PCLProtFindMaxStep()
//
//   Find and return the first PCL step from the protocol that is
//   maximal with respect to cmp, NULL if prot is empty.
//
// Global Variables: -
//
// Side Effects    : Only temporary memory operations.
//
/----------------------------------------------------------------------*/

PCLStep_p PCLProtFindMaxStep(PCLProt_p prot, PCLCmpFunType cmp)
{
   PCLStep_p res = NULL, tmp;
   PStack_p stack;
   PTree_p  cell;

   assert(prot && cmp);
   if(!prot->steps)
   {
      return NULL;
   }
   stack = PTreeTraverseInit(prot->steps);
   cell = PTreeTraverseNext(stack);
   res = cell->key;

   while((cell=PTreeTraverseNext(stack)))
   {
      tmp = cell->key;
      if(cmp(tmp,res) > 0)
      {
    res = tmp;
      }
   }
   PStackFree(stack);

   return res;
}



/*-----------------------------------------------------------------------
//
// Function: PCLProtPropAnalyse()
//
//   Analyse the PCL protocol and put the relevant information into
//   data.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void PCLProtPropAnalyse(PCLProt_p prot, PCLPropData_p data)
{
   data->max_standard_weight_clause =
      PCLProtFindMaxStep(prot, pcl_weight_compare);
   data->longest_clause =
      PCLProtFindMaxStep(prot, pcl_litno_compare);
   data->max_symbol_clause =
      PCLProtFindMaxStep(prot, pcl_sc_compare);
   data->max_depth_clause =
      PCLProtFindMaxStep(prot, pcl_depth_compare);

   pcl_prot_global_count(prot, data);
}

/*-----------------------------------------------------------------------
//
// Function: PCLProtPropDataPrint()
//
//   Print the result of the property analysis in reasonably readable
//   form.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void PCLProtPropDataPrint(FILE* out, PCLPropData_p data)
{
   long clauses =
      data->pos_clauses+data->neg_clauses+data->mix_clauses;

   fprintf(out,
      COMCHAR" Protocol properties\n"
      COMCHAR" ===================\n"
      COMCHAR" Number of clauses                  : %6ld\n"
      COMCHAR" ...of those positive               : %6ld\n"
      COMCHAR" ...of those negative               : %6ld\n"
      COMCHAR" ...of those mixed                  : %6ld\n"
      COMCHAR" Average number of literals         : %6.4f\n"
      COMCHAR" ...in positive clauses             : %6.4f\n"
      COMCHAR" ...in negative clauses             : %6.4f\n"
      COMCHAR" ...in mixed clauses                : %6.4f\n"
      COMCHAR" ...positive literals only          : %6.4f\n"
      COMCHAR" ...negative literals only          : %6.4f\n"
      COMCHAR" Average number of function  symbols: %6.4f\n"
      COMCHAR" Average number of variable  symbols: %6.4f\n"
      COMCHAR" Average number of constant  symbols: %6.4f\n"
      COMCHAR" Average number of predicate symbols: %6.4f\n",
      clauses,
      data->pos_clauses,
      data->neg_clauses,
      data->mix_clauses,
      (double)(data->pos_literals+data->neg_literals)/clauses,
      (double)(data->pos_clause_literals)/data->pos_clauses,
      (double)(data->neg_clause_literals)/data->neg_clauses,
      (double)(data->mix_clause_literals)/data->mix_clauses,
      (double)(data->pos_literals)/clauses,
      (double)(data->neg_literals)/clauses,
      (double)(data->func_count)/clauses,
      (double)(data->var_count)/clauses,
      (double)(data->const_count)/clauses,
      (double)(data->pred_count)/clauses);
   fprintf(out, COMCHAR" Longest Clause (if any): \n");
   PCLStepPrint(out, data->longest_clause);
   fprintf(out, "\n"COMCHAR" Largest Clause (if any): \n");
   PCLStepPrint(out, data->max_symbol_clause);
   fprintf(out, "\n"COMCHAR" Heaviest Clause (if any): \n");
   ClausePropInfoPrint(out, data->max_standard_weight_clause->logic.clause);
   PCLStepPrint(out, data->max_standard_weight_clause);
   fprintf(out, "\n"COMCHAR" Deepest Clause (if any): \n");
   PCLStepPrint(out, data->max_depth_clause);
   fprintf(out, "\n");
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
