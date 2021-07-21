/*-----------------------------------------------------------------------

  File  : ctr_idx_fp.c

  Author: Stephan Schulz (schulz@eprover.org)

  Contents

  Compute and handle term fingerprints for indexing.

  Copyright 2010, 2018 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created:  Wed Feb 24 01:28:18 EET 2010

-----------------------------------------------------------------------*/

#include "cte_idx_fp.h"
#include "cte_simpletypes.h"


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

char* FPIndexNames[] =
{
   "FP0",
   "FPfp",
   "FP1",
   "FP2",
   "FP3D",
   "FP3W",
   "FP4D",
   "FP4W",
   "FP4M",
   "FP5M",
   "FP6M",
   "FP7",
   "FP7M",
   "FP4X2_2",
   "FP3DFlex",
   "NPDT",
   "NoIndex",
   NULL
};

static FPIndexFunction fp_index_funs[] =
{
   IndexFP0Create,
   IndexFPfpCreate,
   IndexFP1Create,
   IndexFP2Create,
   IndexFP3DCreate,
   IndexFP3WCreate,
   IndexFP4DCreate,
   IndexFP4WCreate,
   IndexFP4MCreate,
   IndexFP5MCreate,
   IndexFP6MCreate,
   IndexFP7Create,
   IndexFP7MCreate,
   IndexFP4X2_2Create,
   IndexFP3DFlexCreate,
   IndexDTCreate,
   NULL,
   NULL
};



/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: push_fcodes()
//
//   Push the f_codes of the term (in depth first, LR order) onto the
//   stack.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void push_fcodes(PStack_p stack, Term_p t)
{
   if(TermIsFreeVar(t))
   {
      PStackPushInt(stack, ANY_VAR);
   }
   else
   {
      int i;
      if(!TermIsPhonyApp(t))
      {
         PStackPushInt(stack, t->f_code);
      }

      for(i=0; i<t->arity; i++)
      {
         push_fcodes(stack, t->args[i]);
      }
   }
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: TermFPSampleFO()
//
//   Sample the term at the position described by the optional
//   arguments (encoding a (-1)-terminated position.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

FunCode TermFPSampleFO(Term_p term, va_list ap)
{
  int pos = 0;
  FunCode res = 0;

  for(pos = va_arg(ap, int); pos != -1;  pos = va_arg(ap, int))
  {
     if(TermIsFreeVar(term))
     {
        res = BELOW_VAR;
        break;
     }
     if(pos >= term->arity)
     {
        res = NOT_IN_TERM;
        break;
     }
     term = term->args[pos];
  }
  if(pos == -1)
  {
     res = TermIsFreeVar(term)?ANY_VAR:term->f_code;
  }
  va_end(ap);

  return res;
}

#ifdef ENABLE_LFHO


/*-----------------------------------------------------------------------
//
// Function: TermFPSampleHO()
//
//  For details see TermFPSampleFO(). It differs by supporting
//  prefix matching/unification, where terms can have trailing arguments.
//
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/
FunCode TermFPSampleHO(Term_p term, va_list ap)
{
  int pos = 0;
  FunCode res = 0;

  for(pos = va_arg(ap, int); pos != -1;  pos = va_arg(ap, int))
  {
    while(TermIsLambda(term))
    {
      term = term->args[1];
    }

    if(TermIsTopLevelFreeVar(term))
    {
      res = BELOW_VAR;
      break;
    }

    if(pos < term->arity)
    {
      term = term->args[pos];
    }
    else if(pos < term->arity + TypeGetMaxArity(term->type))
    {
      res = SIG_DB_LAMBDA_CODE;
      break;
    }
    else
    {
      res = NOT_IN_TERM;
      break;
    }
  }

  if(pos == -1)
  {
     res = TermIsTopLevelFreeVar(term) ? ANY_VAR:
            (TermIsTopLevelAnyVar(term) ? SIG_DB_LAMBDA_CODE :
            term->f_code);
  }
  va_end(ap);

  return res;
}
#endif

/*-----------------------------------------------------------------------
//
// Function: TermFPSample()
//
//   Based on problem type, chooses appropriate fingerprinting function.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/
FunCode  TermFPSample(Term_p term, ...)
{
   va_list args;
   va_start(args, term);

   FunCode res;
#ifdef ENABLE_LFHO
   if(problemType == PROBLEM_HO)
   {
      res = TermFPSampleHO(term, args);
   }
   else
   {
#endif
      res = TermFPSampleFO(term, args);
#ifdef ENABLE_LFHO
   }
#endif

   va_end(args);

   return res;
}



/*-----------------------------------------------------------------------
//
// Function: TermFPFlexSample()
//
//   Sample the term at the position described by the array at
//   pos. Update pos to point behind the end of the (-1)-terminated
//   position.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

FunCode TermFPFlexSampleFO(Term_p term, IntOrP* *seq)
{
  FunCode res = 0;
  long pos;

  while((pos=(*seq)->i_val)!=-1)
  {
     if(TermIsFreeVar(term))
     {
        res = BELOW_VAR;
        break;
     }
     if(pos >= term->arity)
     {
        res = NOT_IN_TERM;
        break;
     }
     term = term->args[pos];
     (*seq)++;
  }
  if(pos == -1)
  {
     res = TermIsFreeVar(term)?ANY_VAR:term->f_code;
  }
  else
  {
     /* Find the end of the position */
     while((pos=(*seq)->i_val)!=-1)
     {
        (*seq)++;
     }
  }
  /* We want to point beyond the end */
  (*seq)++;
  return res;
}

#ifdef ENABLE_LFHO
/*-----------------------------------------------------------------------
//
// Function: TermFPFlexSampleHO()
//
//   Similar to TermFPFlexSample(), but supports HO fingerprinting.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/
FunCode TermFPFlexSampleHO(Term_p term, IntOrP* *seq)
{
  FunCode res = 0;
  long pos;

  while((pos=(*seq)->i_val)!=-1)
  {
    while(TermIsLambda(term))
    {
      term = term->args[1];
    }

    if(TermIsTopLevelFreeVar(term))
    {
      res = BELOW_VAR;
      break;
    }
    if(pos < term->arity)
    {
      term = term->args[pos];
    }
    else if(pos < term->arity + TypeGetMaxArity(term->type))
    {
      res = SIG_DB_LAMBDA_CODE;
      break;
    }
    else
    {
      res = NOT_IN_TERM;
      break;
    }
    (*seq)++;
  }
  
  if(pos == -1)
  {
     res = TermIsTopLevelFreeVar(term) ? ANY_VAR:
            (TermIsTopLevelAnyVar(term) ? SIG_DB_LAMBDA_CODE :
            term->f_code);
  }
  else
  {
     /* Find the end of the position */
     while((pos=(*seq)->i_val)!=-1)
     {
        (*seq)++;
     }
  }
  /* We want to point beyond the end */
  (*seq)++;
  return res;
}

/*-----------------------------------------------------------------------
//
// Function: TermFPFlexSample()
//
//   Based on problem type, chooses appropriate fingerprinting function.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/
inline FunCode TermFPFlexSample(Term_p term, IntOrP* *seq)
{
   if(problemType == PROBLEM_HO)
   {
      return TermFPFlexSampleHO(term, seq);
   }
   else
   {
      return TermFPFlexSampleFO(term, seq);
   }
}

#endif

/*-----------------------------------------------------------------------
//
// Function: IndexFP0Create()
//
//   Create a dummy fingerprint structure.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

IndexFP_p IndexFP0Create(Term_p t)
{
   IndexFP_p res = SizeMalloc(sizeof(FunCode)*1);

   res[0] = 1;

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: IndexFPfpCreate()
//
//   Create a fingerprint structure using an abstraction to just avoid
//   function/predicate unifications/matches.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

IndexFP_p IndexFPfpCreate(Term_p t)
{
   IndexFP_p res = SizeMalloc(sizeof(FunCode)*2);
   static FunCode f_rep = 0, p_rep = 0;

   res[0] = 2;
   res[1] = TermFPSample(t, -1);
   if(res[1] > 0)
   {
      if(TermCellQueryProp((t), TPPredPos))
      {
         if(!p_rep)
         {
            p_rep = res[1];
         }
         res[1] = p_rep;
      }
      else
      {
         if(!f_rep)
         {
            f_rep = res[1];
         }
         res[1] = f_rep;
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: IndexFP1Create()
//
//   Create a fingerprint structure representing top symbol hashing.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

IndexFP_p IndexFP1Create(Term_p t)
{
   IndexFP_p res = SizeMalloc(sizeof(FunCode)*2);

   res[0] = 2;
   res[1] = TermFPSample(t, -1);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: IndexFP2Create()
//
//   Create a fingerprint structure representing sampling at epsilon,
//   0.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

IndexFP_p IndexFP2Create(Term_p t)
{
   IndexFP_p res = SizeMalloc(sizeof(FunCode)*3);

   res[0] = 3;
   res[1] = TermFPSample(t, -1);
   res[2] = TermFPSample(t, 0, -1);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: IndexFP3DCreate()
//
//   Create a fingerprint structure representing sampling at epsilon,
//   0, 0.0.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

IndexFP_p IndexFP3DCreate(Term_p t)
{
   IndexFP_p res = SizeMalloc(sizeof(FunCode)*4);

   res[0] = 4;
   res[1] = TermFPSample(t, -1);
   res[2] = TermFPSample(t, 0, -1);
   res[3] = TermFPSample(t, 0, 0, -1);

   return res;
}



/*-----------------------------------------------------------------------
//
// Function: IndexFP3WCreate()
//
//   Create a fingerprint structure representing sampling at epsilon,
//   0, 1.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

IndexFP_p IndexFP3WCreate(Term_p t)
{
   IndexFP_p res = SizeMalloc(sizeof(FunCode)*4);

   res[0] = 4;
   res[1] = TermFPSample(t, -1);
   res[2] = TermFPSample(t, 0, -1);
   res[3] = TermFPSample(t, 1, -1);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: IndexFP4DCreate()
//
//   Create a fingerprint structure representing sampling at epsilon,
//   0, 0.0, 0.0.0
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

IndexFP_p IndexFP4DCreate(Term_p t)
{
   IndexFP_p res = SizeMalloc(sizeof(FunCode)*5);

   res[0] = 5;
   res[1] = TermFPSample(t, -1);
   res[2] = TermFPSample(t, 0, -1);
   res[3] = TermFPSample(t, 0, 0, -1);
   res[4] = TermFPSample(t, 0, 0,0, -1);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: IndexFP4WCreate()
//
//   Create a fingerprint structure representing sampling at epsilon,
//   0, 1, 2
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

IndexFP_p IndexFP4WCreate(Term_p t)
{
   IndexFP_p res = SizeMalloc(sizeof(FunCode)*5);

   res[0] = 5;
   res[1] = TermFPSample(t, -1);
   res[2] = TermFPSample(t, 0, -1);
   res[3] = TermFPSample(t, 1, -1);
   res[4] = TermFPSample(t, 2, -1);

   return res;
}



/*-----------------------------------------------------------------------
//
// Function: IndexFP4MCreate()
//
//   Create a fingerprint structure representing sampling at epsilon,
//   0, 1, 0.0
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

IndexFP_p IndexFP4MCreate(Term_p t)
{
   IndexFP_p res = SizeMalloc(sizeof(FunCode)*5);

   res[0] = 5;
   res[1] = TermFPSample(t, -1);
   res[2] = TermFPSample(t, 0, -1);
   res[3] = TermFPSample(t, 1, -1);
   res[4] = TermFPSample(t, 0, 0, -1);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: IndexFP5MCreate()
//
//   Create a fingerprint structure representing sampling at epsilon,
//   0, 1, 2, 0.0
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

IndexFP_p IndexFP5MCreate(Term_p t)
{
   IndexFP_p res = SizeMalloc(sizeof(FunCode)*6);

   res[0] = 6;
   res[1] = TermFPSample(t, -1);
   res[2] = TermFPSample(t, 0, -1);
   res[3] = TermFPSample(t, 1, -1);
   res[4] = TermFPSample(t, 2, -1);
   res[5] = TermFPSample(t, 0, 0, -1);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: IndexFP6MCreate()
//
//   Create a fingerprint structure representing sampling at epsilon,
//   0, 1, 2, 0.0, 0.1
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

IndexFP_p IndexFP6MCreate(Term_p t)
{
   IndexFP_p res = SizeMalloc(sizeof(FunCode)*7);

   res[0] = 7;
   res[1] = TermFPSample(t, -1);
   res[2] = TermFPSample(t, 0, -1);
   res[3] = TermFPSample(t, 1, -1);
   res[4] = TermFPSample(t, 2, -1);
   res[5] = TermFPSample(t, 0, 0, -1);
   res[6] = TermFPSample(t, 0, 1, -1);

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: IndexFP7Create()
//
//   Create a fingerprint structure with samples at positions epsilon,
//   0, 1, 0.0, 0.1, 1.0, 1.1 (using E's internal numbering).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

IndexFP_p IndexFP7Create(Term_p t)
{
   IndexFP_p res = SizeMalloc(sizeof(FunCode)*8);

   res[0] = 8;
   res[1] = TermFPSample(t, -1);
   res[2] = TermFPSample(t, 0, -1);
   res[3] = TermFPSample(t, 1, -1);
   res[4] = TermFPSample(t, 0, 0, -1);
   res[5] = TermFPSample(t, 0, 1, -1);
   res[6] = TermFPSample(t, 1, 0, -1);
   res[7] = TermFPSample(t, 1, 1, -1);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: IndexFP7MCreate()
//
//   Create a fingerprint structure representing sampling at epsilon,
//   0, 1, 2, 3, 0.0, 0.1
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

IndexFP_p IndexFP7MCreate(Term_p t)
{
   IndexFP_p res = SizeMalloc(sizeof(FunCode)*8);

   res[0] = 8;
   res[1] = TermFPSample(t, -1);
   res[2] = TermFPSample(t, 0, -1);
   res[3] = TermFPSample(t, 1, -1);
   res[4] = TermFPSample(t, 2, -1);
   res[5] = TermFPSample(t, 0, 0, -1);
   res[6] = TermFPSample(t, 3, -1);
   res[7] = TermFPSample(t, 0, 1, -1);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: IndexFP4X2_2Create()
//
//   Create a fingerprint structure with samples at positions as
//   specified below.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

IndexFP_p IndexFP4X2_2Create(Term_p t)
{
   IndexFP_p res = SizeMalloc(sizeof(FunCode)*17);

   res[ 0] = 17;
   res[ 1] = TermFPSample(t, -1);
   res[ 2] = TermFPSample(t, 0, -1);
   res[ 3] = TermFPSample(t, 1, -1);
   res[ 4] = TermFPSample(t, 2, -1);
   res[ 5] = TermFPSample(t, 3, -1);

   res[ 6] = TermFPSample(t, 0, 0, -1);
   res[ 7] = TermFPSample(t, 0, 1, -1);
   res[ 8] = TermFPSample(t, 0, 2, -1);

   res[9] = TermFPSample(t, 1, 0, -1);
   res[10] = TermFPSample(t, 1, 1, -1);
   res[11] = TermFPSample(t, 1, 2, -1);

   res[12] = TermFPSample(t, 2, 0, -1);
   res[13] = TermFPSample(t, 2, 1, -1);
   res[14] = TermFPSample(t, 2, 2, -1);

   res[15] = TermFPSample(t, 0, 0, 0, -1);
   res[16] = TermFPSample(t, 1, 0, 0, -1);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: IndexFPFlexCreate()
//
//   Create a fingerprint of len elments, with the positions in pos.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

IndexFP_p IndexFPFlexCreate(Term_p t, PStack_p pos, int len)
{
   IndexFP_p res = SizeMalloc(sizeof(FunCode)*(len+1));
   IntOrP *p = PStackBaseAddress(pos);
   int     i;

   res[0] = (len+1);

   i=1;
   while((*p).i_val != -2)
   {
      res[i] = TermFPFlexSample(t, &p);
      i++;
   }

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: IndexFP3DFlexCreate()
//
//   Testfunction, equivalent to IndexFP3DCreate()
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

IndexFP_p IndexFP3DFlexCreate(Term_p t)
{
   IndexFP_p res;
   PStack_p pos = PStackAlloc();

   PStackPushInt(pos, -1);

   PStackPushInt(pos, 0);
   PStackPushInt(pos, -1);

   PStackPushInt(pos, 0);
   PStackPushInt(pos, 0);
   PStackPushInt(pos, -1);

   PStackPushInt(pos, -2);

   res = IndexFPFlexCreate(t, pos, 3);

   PStackFree(pos);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: IndexDTCreate()
//
//   Create a fingerprint that samples t at all its positions (in
//   depths-first LR order) and no others. Building an FP-Tree with
//   these samples will not build an FP-Index, but a (non-perfect)
//   discrimination tree. This means that retrieval will require
//   special code, it cannot use simple FP-Index retrieval.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

IndexFP_p IndexDTCreate(Term_p t)
{
   PStack_p stack = PStackAlloc();
   IndexFP_p res;
   PStackPointer sp;
   int i, len;

   push_fcodes(stack, t);

   len = PStackGetSP(stack);
   res = SizeMalloc(sizeof(FunCode)*(len+1));
   res[0] = len+1;
   for(sp = 0, i=1; sp<len; sp++, i++)
   {
      res[i] = PStackElementInt(stack, sp);
   }
   PStackFree(stack);
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: IndexFPFree()
//
//   Free an IndexFP data-structure (i.e. a self-describing FunCode
//   array).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void IndexFPFree(IndexFP_p junk)
{
   SizeFree(junk, sizeof(FunCode)*junk[0]);
}


/*-----------------------------------------------------------------------
//
// Function: GetFPIndexFunction()
//
//   Given a name, return the corresponding index function, or NULL.
//
// Global Variables: fp_index_names, fp_index_funs
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

FPIndexFunction GetFPIndexFunction(char* name)
{
   int i;

   for(i=0; FPIndexNames[i]; i++)
   {
      if(strcmp(FPIndexNames[i], name)==0)
      {
         return fp_index_funs[i];
      }
   }
   return NULL;
}



/*-----------------------------------------------------------------------
//
// Function: IndexFPPrint()
//
//   Print a term fingerprint.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void IndexFPPrint(FILE* out, IndexFP_p fp)
{
   int i, limit=fp[0];

   if(limit>=2)
   {
      fprintf(out, "<%ld", fp[1]);
      for(i=2; i<limit; i++)
      {
         fprintf(out, ",%ld", fp[i]);
      }
      fprintf(out, ">");
   }
   else
   {
      fprintf(out, "<>");
   }
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
