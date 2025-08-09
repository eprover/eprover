/*-----------------------------------------------------------------------

File  : che_clausefeatures.c

Author: Stephan Schulz

Contents

  Functions for determining features of clauses.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Mon Sep 28 19:17:50 MET DST 1998
    New

-----------------------------------------------------------------------*/

#include "che_clausefeatures.h"



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
// Function: term_depth_info_add()
//
//   Change term depth to depthsum, adapt depthmax, increase count by
//   one. Return the new max.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static long term_depth_info_add(Term_p term, long* depthmax, long*
            depthsum, long* count)
{
   long depth = TermDepth(term);

   (*depthsum)+=depth;
   (*count)++;
   if(depth > *depthmax)
   {
      (*depthmax) = depth;
   }
   return *depthmax;
}


/*-----------------------------------------------------------------------
//
// Function: eqn_tptp_depth_info_add()
//
//   Add term depth info according to TPTP interpretation (all
//   literals are conventional, equations are interpreted as equal(t1,
//   t2)).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static long eqn_tptp_depth_info_add(Eqn_p eqn, long* depthmax, long*
                depthsum, long* count)
{
   if(EqnIsEquLit(eqn))
   {
      term_depth_info_add(eqn->lterm, depthmax, depthsum, count);
      term_depth_info_add(eqn->rterm, depthmax, depthsum, count);
   }
   else
   {
      int i;
      for(i=0; i<eqn->lterm->arity; i++)
      {
    term_depth_info_add(eqn->lterm->args[i], depthmax, depthsum,
              count);
      }
   }
   return *depthmax;
}



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: ClauseCountExtSymbols()
//
//   Return the number of different external function symbols in
//   clause.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int ClauseCountExtSymbols(Clause_p clause, Sig_p sig, long min_arity)
{
   long *dist_array = SizeMalloc((sig->f_count+1)*sizeof(long));
   FunCode i;
   int  res =0;

   for(i=0; i<=sig->f_count; i++)
   {
      dist_array[i] = 0;
   }
   ClauseAddSymbolDistribution(clause, dist_array);

   for(i=sig->internal_symbols+1; i<=sig->f_count; i++)
   {
      if((SigFindArity(sig, i)>=min_arity) && dist_array[i])
      {
    res++;
      }
   }
   SizeFree(dist_array, (sig->f_count+1)*sizeof(long));

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: TermAddVarDistribution()
//
//   Count the variable occurences in term. Return the largest
//   (negated) variable f_count.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

FunCode TermAddVarDistribution(Term_p term, PDArray_p dist_array)
{
   PStack_p stack = PStackAlloc();
   long     count;
   FunCode  max_var = 0;
   assert(term);

   PStackPushP(stack, term);

   while(!PStackEmpty(stack))
   {
      term = PStackPopP(stack);

      if(!TermIsFreeVar(term))
      {
         int i;

         assert(term->f_code > 0);

         for(i=0; i<term->arity; i++)
         {
            assert(term->args);
            PStackPushP(stack, term->args[i]);
         }
      }
      else
      {
         count = PDArrayElementInt(dist_array, -(term->f_code));
         count++;
         max_var = MAX(max_var, -(term->f_code));
         PDArrayAssignInt(dist_array, -(term->f_code), count);
      }
   }
   PStackFree(stack);

   return max_var;
}


/*-----------------------------------------------------------------------
//
// Function: EqnAddVarDistribution()
//
//   As TermAddVarDistribution(), but for equations.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

FunCode EqnAddVarDistribution(Eqn_p eqn, PDArray_p dist_array)
{
   FunCode lvars, rvars;

   lvars = TermAddVarDistribution(eqn->lterm, dist_array);
   rvars = TermAddVarDistribution(eqn->rterm, dist_array);

   return MAX(lvars, rvars);
}


/*-----------------------------------------------------------------------
//
// Function: EqnListAddVarDistribution()
//
//   As TernAddVarDistribution, for lists of equations.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

FunCode EqnListAddVarDistribution(Eqn_p list, PDArray_p dist_array)
{
   FunCode max_var = 0, local;

   while(list)
   {
      local = EqnAddVarDistribution(list, dist_array);
      max_var = MAX(max_var, local);
      list = list->next;
   }
   return max_var;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseCountVariableSet()
//
//   Return the number of different variables in
//   clause.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ClauseCountVariableSet(Clause_p clause)
{
   PDArray_p dist_array = PDIntArrayAlloc(20,20);
   FunCode max_var,i;
   long res = 0;

   max_var = ClauseAddVarDistribution(clause, dist_array);

   for(i=1; i<=max_var; i++)
   {
      if(PDArrayElementInt(dist_array, i))
      {
         res++;
      }
   }
   PDArrayFree(dist_array);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseCountSingletonSet()
//
//   Return the number of different singleton variables in
//   clause.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ClauseCountSingletonSet(Clause_p clause)
{
   PDArray_p dist_array = PDIntArrayAlloc(20,20);
   FunCode max_var,i;
   long res = 0;

   max_var = ClauseAddVarDistribution(clause, dist_array);

   for(i=1; i<=max_var; i++)
   {
      if(PDArrayElementInt(dist_array, i)==1)
      {
         res++;
      }
   }
   PDArrayFree(dist_array);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseCountMaximalTerms()
//
//   Given an clause, return the number of maximal terms in maximal
//   literals.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ClauseCountMaximalTerms(Clause_p clause)
{
   Eqn_p handle;
   long  res = 0;

   for(handle=clause->literals; handle; handle = handle->next)
   {
      if(EqnIsMaximal(handle))
      {
    res+=EqnCountMaximalLiterals(handle);
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseCountMaximalLiterals()
//
//   Given an clause, return the number of maximal literals.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ClauseCountMaximalLiterals(Clause_p clause)
{
   Eqn_p handle;
   long  res = 0;

   for(handle=clause->literals; handle; handle = handle->next)
   {
      if(EqnIsMaximal(handle))
      {
    res++;
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseCountUnorientableLiterals()
//
//   Given an clause, return the number of unorientable literals.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ClauseCountUnorientableLiterals(Clause_p clause)
{
   Eqn_p handle;
   long  res = 0;

   for(handle=clause->literals; handle; handle = handle->next)
   {
      if(!EqnIsOriented(handle))
      {
    res++;
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseTPTPDepthInfoAdd()
//
//   Add the term depth information according to TPTP interpretation
//   (see eqn_tptp_depth_info_add()).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ClauseTPTPDepthInfoAdd(Clause_p clause, long* depthmax, long*
             depthsum, long* count)
{
   Eqn_p handle;

   for(handle = clause->literals; handle; handle=handle->next)
   {
      eqn_tptp_depth_info_add(handle, depthmax, depthsum, count);
   }
   return *depthmax;
}

/*-----------------------------------------------------------------------
//
// Function: ClauseInfoPrint()
//
//   Print a lot of information about clause in the form
//   info(d0,...,dn) with
//
//   d0: Clause ident (number)
//   d1: Proof depth
//   d2: Proof length
//   d3: Symbol count
//   d4: Clause depth
//   d5: Literal number
//   d6: Number of variable occurences
//   d7: Number of different variables
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void ClauseInfoPrint(FILE* out, Clause_p clause)
{
   fprintf(out, "info(%ld, %ld, %ld, %ld, %ld, %d, %ld, %ld)",
      clause->ident,
      clause->proof_depth,
      clause->proof_size,
      (long)ClauseWeight(clause, 1, 1, 1, 1, 1, 1, false),
      ClauseDepth(clause),
      ClauseLiteralNumber(clause),
      (long)ClauseWeight(clause, 0, 1, 1, 1, 1, 1, false),
      ClauseCountVariableSet(clause));
}


/*-----------------------------------------------------------------------
//
// Function: ClauseLinePrint()
//
//   Print the clause and potential information on a single line.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void ClauseLinePrint(FILE* out, Clause_p clause, bool printinfo)
{
   ClausePrint(out, clause, true);
   if(printinfo)
   {
      fputs(" "COMCHARRAW" ", out);
      ClauseInfoPrint(out, clause);
   }
   fputc('\n', out);
}


/*-----------------------------------------------------------------------
//
// Function: ClausePropInfoPrint()
//
//   Print a clause and certain statistical information about it as a
//   comment.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void ClausePropInfoPrint(FILE* out, Clause_p clause)
{
   assert(clause);

   fprintf(out, COMCHAR" ");
   ClausePCLPrint(out, clause, true);
   fprintf(out,
      "\n"
      COMCHAR" Standardweight: %6ld\n"
      COMCHAR" Symbol count  : %6ld\n"
      COMCHAR"    F. symbols : %6ld\n"
      COMCHAR"    Variables  : %6ld\n"
      COMCHAR"    Constants  : %6ld\n"
      COMCHAR"    P. symbols : %6ld\n"
      COMCHAR" Depth         : %6ld\n"
           COMCHAR" Literals      : %6d\n"
           COMCHAR"    ...positive: %6d\n"
           COMCHAR"    ...negative: %6d\n",
      (long)ClauseStandardWeight(clause),
      (long)ClauseSymTypeWeight(clause, 1,1,1,1,1,1,1,1),
      (long)ClauseSymTypeWeight(clause, 1,1,1,0,1,0,0,1),
      (long)ClauseSymTypeWeight(clause, 1,1,1,1,0,0,0,1),
      (long)ClauseSymTypeWeight(clause, 1,1,1,0,0,1,0,1),
      (long)ClauseSymTypeWeight(clause, 1,1,1,0,0,0,1,1),
      ClauseDepth(clause),
      ClauseLiteralNumber(clause),
      clause->pos_lit_no,
      clause->neg_lit_no);
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
