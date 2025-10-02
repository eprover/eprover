/*-----------------------------------------------------------------------

  File  : cto_ocb.c

  Author: Stephan Schulz

  Contents

  Functions for describing orderings, precedences and so on.

  Copyright 1998, 1999, 2019 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Thu Apr 30 03:11:31 MET DST 1998

  -----------------------------------------------------------------------*/

#include "cto_ocb.h"
#include <che_to_params.h>



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

char* TONames[]=
{
   "NoOrdering",
   "Optimize",
   "KBO",
   "KBO6",
   "LPO",
   "LPOCopy",
   "LPO4",
   "LPO4Copy",
   "RPO",
   "Empty",
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
// Function: free_val()
//
//   Frees the value stored in the
//
// Global Variables: -
//
// Side Effects    : Changes ocb->precedences
//
/----------------------------------------------------------------------*/

void free_val(void* key, void* val)
{
   SizeFree(val, sizeof(long));
}

/*-----------------------------------------------------------------------
//
// Function: ocb_trans_compute()
//
//   Given the relations between f1 and f2, and f2 and f3, compute the
//   relation between f1 and f3. Return true, if it can be set, false
//   otherwise.
//
// Global Variables: -
//
// Side Effects    : Changes ocb->precedences
//
/----------------------------------------------------------------------*/

bool ocb_trans_compute(OCB_p ocb, FunCode f1, FunCode f2, FunCode f3)
{
   CompareResult rel12, rel23;
   bool          res = true;

   rel12 = OCBFunCompare(ocb, f1, f2);
   rel23 = OCBFunCompare(ocb, f2, f3);

   switch(rel12)
   {
   case to_uncomparable:
         break;
   case to_equal:
         if(rel23 != to_uncomparable)
         {
            res = OCBPrecedenceAddTuple(ocb, f1, f3, rel23);
         }
         break;
   case to_greater:
         if(rel23 == to_equal || rel23 == to_greater)
         {
            res = OCBPrecedenceAddTuple(ocb, f1, f3, to_greater);
         }
         break;
   case to_lesser:
         if(rel23 == to_equal || rel23 == to_lesser)
         {
            res = OCBPrecedenceAddTuple(ocb, f1, f3, to_lesser);
         }
         break;
   default:
         assert(false);
         break;
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: alloc_precedence()
//
//   Initialize handle->precedence or handle->prec_weights according
//   to the value of prec_by_weight.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void alloc_precedence(OCB_p handle, bool prec_by_weight)
{
   if(prec_by_weight)
   {
      handle->precedence = NULL;
      handle->prec_weights =
         SizeMalloc(sizeof(long)*(handle->sig_size+1));
   }
   else
   {
      handle->precedence = SizeMalloc(sizeof(CompareResult)
                                      *handle->sig_size
                                      *handle->sig_size);
      handle->prec_weights = NULL;
   }
}



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: OCBAlloc()
//
//   Allocate an initialized order control block.
//
// Global Variables: -
//
// Side Effects    : Memory management
//
/----------------------------------------------------------------------*/

OCB_p OCBAlloc(TermOrdering type, bool prec_by_weight, Sig_p sig, HoOrderKind ho_order_kind)
{
   OCB_p handle;
   int   i,j;

   handle = OCBCellAlloc();

   handle->type  = type;
   handle->sig   = sig;
   handle->min_constants  = PDIntArrayAlloc(16,0);
   handle->ho_order_kind = ho_order_kind;
   handle->weights    = NULL;
   handle->sig_size = sig->f_count;
   handle->statestack = PStackAlloc();
   handle->var_weight = 1;
   handle->lit_cmp    = LCNormal;
   handle->rewrite_strong_rhs_inst = false;
   handle->wb      = 0;
   handle->pos_bal = 0;
   handle->neg_bal = 0;
   handle->max_var = 0;
   handle->vb_size = ho_order_kind == LAMBDA_ORDER ? 1 : 64;
   handle->vb      = handle->vb_size? SizeMalloc(handle->vb_size*sizeof(int)):NULL;
   handle->db_weight = DEFAULT_DB_WEIGHT;
   handle->lam_weight = DEFAULT_LAMBDA_WEIGHT;
   handle->ho_vb   = NULL;
   for(size_t i=0; i<handle->vb_size; i++)
   {
      handle->vb[i] = 0;
   }

   switch(type)
   {
   case KBO:
   case KBO6:
         handle->weights = SizeMalloc(sizeof(long)*(handle->sig_size+1));
         alloc_precedence(handle, prec_by_weight);
         break;
   case LPO:
   case LPOCopy:
   case LPO4:
   case LPO4Copy:
         alloc_precedence(handle, prec_by_weight);
         break;
   case RPO:
         alloc_precedence(handle, prec_by_weight);
         break;
   case EMPTY:
         break;
   default:
         assert(false);
         break;
   }

   if(handle->weights)
   {
      for(size_t i=0; i<=handle->sig_size; i++)
      {
         *OCBFunWeightPos(handle,i) = 1;
      }
   }

   if(handle->precedence)
   {
      for(i=1; i<=handle->sig_size; i++)
      {
         for(j=1; j<=handle->sig_size; j++)
         {
            *OCBFunComparePos(handle, i,j) =
               ((i==j) ? to_equal : to_uncomparable);
         }
      }
   }

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: OCBFree()
//
//   Free the memory taken by an order control block. Note: The
//   signature is not considered part of the ocb and is not free'd.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void OCBFree(OCB_p junk)
{
   assert(junk);

   if(junk->weights)
   {
      assert(junk->type == KBO || junk->type==KBO6);
      SizeFree(junk->weights, sizeof(long)*(junk->sig_size+1));
      junk->weights = NULL;
   }
   if(junk->precedence)
   {
      assert(!junk->prec_weights);
      assert(junk->type == KBO ||
             junk->type == KBO6 ||
             junk->type == LPO ||
             junk->type == LPOCopy ||
             junk->type == LPO4 ||
             junk->type == LPO4Copy ||
             junk->type == RPO);
      SizeFree(junk->precedence, sizeof(CompareResult)
               * junk->sig_size * junk->sig_size);
      junk->precedence = NULL;
   }
   if(junk->prec_weights)
   {
      assert(!junk->precedence);
      assert(junk->type == KBO ||
             junk->type == KBO6 ||
             junk->type == LPO ||
             junk->type == LPOCopy ||
             junk->type == LPO4 ||
             junk->type == LPO4Copy ||
             junk->type == RPO);
      SizeFree(junk->prec_weights, sizeof(long)*(junk->sig_size+1));
      junk->prec_weights = NULL;
   }
   PDArrayFree(junk->min_constants);
   assert(junk);
   if(junk->vb)
   {
      SizeFree(junk->vb, junk->vb_size*sizeof(int));
   }
   PStackFree(junk->statestack);
   PObjMapFreeWDeleter(junk->ho_vb, free_val);
   OCBCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: OCBDebugPrint()
//
//   Print an OCB in debug-friendly form (not suitable for
//   re-parsing, revealing a lot of internal information).
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void OCBDebugPrint(FILE* out, OCB_p ocb)
{
   long i,j;

   fprintf(out, COMCHAR" ==============OCB-Debug-Information============\n");
   fprintf(out, COMCHAR" ===============================================\n");
   if(ocb->sig)
   {
      SigPrint(out, ocb->sig);
   }
   else
   {
      fprintf(out, COMCHAR" No sig!\n");
   }
   fprintf(out, COMCHAR" -----------------------------------------------\n");
   if(ocb->weights)
   {
      fprintf(out, COMCHAR" Weights:");
      for(i=1; i<=ocb->sig_size;i++)
      {
         if(!((i-1)%8))
         {
            fprintf(out, "\n"COMCHAR" ");
         }
         if(ocb->sig)
         {
            fprintf(out, " (%s = %ld) ", SigFindName(ocb->sig,i), OCBFunWeight(ocb, i));
         }
         else
         {
            fprintf(out, " (%ld = %ld) ", i, OCBFunWeight(ocb, i));
         }
      }
      fprintf(out, "\n\n");
   }
   else
   {
      fprintf(out, COMCHAR" No weights!\n");
   }
   fprintf(out, COMCHAR" -----------------------------------------------\n");
   if(ocb->precedence)
   {
      fprintf(out, COMCHAR" Precedence Matrix:\n"COMCHAR"       ");
      for(j=1; j<=ocb->sig_size; j++)
      {
         fprintf(out, " %2ld ", j);
      }
      fprintf(out, "\n");
      for(i=1; i<=ocb->sig_size; i++)
      {
         fprintf(out, COMCHAR" %2ld  | ", i);
         fflush(stdout);
         for(j=1; j<=ocb->sig_size; j++)
         {
            char* symb;

            symb = POCompareSymbol[OCBFunCompare(ocb, i, j)];
            fprintf(out, " %s", symb);
            fflush(stdout);
         }
         fprintf(out, "\n");
      }
   }
   else
   {
      fprintf(out, COMCHAR" No precedence!\n");
   }
   fprintf(out, COMCHAR" ===============================================\n");
}


/*-----------------------------------------------------------------------
//
// Function: OCBPrecedenceAddTuple()
//
//   Add a new binary relation to the precedence stored in the ocb and
//   compute the new transitive closure of the to_greater, to_smaller
//   and to_equal. Store updated cell in ocb->statestackcell. Return
//   the new stackpointer if everything went fine, undo all changes
//   and return 0 otherwise.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

PStackPointer OCBPrecedenceAddTuple(OCB_p ocb, FunCode f1, FunCode f2,
                                    CompareResult relation)
{
   FunCode       i;
   PStackPointer res = 0, old;

   assert(ocb);
   assert(ocb->precedence);
   assert(f1<=ocb->sig_size);
   assert(f2<=ocb->sig_size);
   assert(relation!=to_uncomparable);

   old = PStackGetSP(ocb->statestack);

   if(OCBFunCompare(ocb, f1, f2)==relation)
   {
      res = old;
   }
   else if(OCBFunCompare(ocb, f1, f2)!=to_uncomparable)
   {
      res = 0;
   }
   else
   {
      PStackPushInt(ocb->statestack, f1);
      PStackPushInt(ocb->statestack, f2);
      *OCBFunComparePos(ocb, f1, f2) = relation;
      *OCBFunComparePos(ocb, f2, f1) = POInverseRelation(relation);

      for(i=1; i<=ocb->sig_size; i++)
      {
         res = ocb_trans_compute(ocb, f1, f2, i);
         if(!res)
         {
            break;
         }
         res = ocb_trans_compute(ocb, i, f1, f2);
         if(!res)
         {
            break;
         }
      }
      if(!res)
      { /* Error case, undo changes */
         f2 = PStackPopInt(ocb->statestack);
         f1 = PStackPopInt(ocb->statestack);
         *OCBFunComparePos(ocb, f1, f2) = to_uncomparable;
         *OCBFunComparePos(ocb, f2, f1) = to_uncomparable;
      }
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: OCBPrecedenceBacktrack()
//
//   Backtrack the precedence matrix to a given state. Return true if
//   the stack is non-empty afterwards, false otherwise.
//
// Global Variables: -
//
// Side Effects    : Changes the matrix
//
/----------------------------------------------------------------------*/

bool OCBPrecedenceBacktrack(OCB_p ocb, PStackPointer state)
{
   FunCode f1,f2;

   while(state!=PStackGetSP(ocb->statestack))
   {
      assert(!PStackEmpty(ocb->statestack));
      f2 = PStackPopInt(ocb->statestack);
      assert(!PStackEmpty(ocb->statestack));
      f1 = PStackPopInt(ocb->statestack);

      assert(OCBFunCompare(ocb, f1, f2) != to_uncomparable);
      *OCBFunComparePos(ocb, f1, f2) = to_uncomparable;
      assert(OCBFunCompare(ocb, f2, f1) != to_uncomparable);
      *OCBFunComparePos(ocb, f2, f1) = to_uncomparable;
   }
   return !PStackEmpty(ocb->statestack);
}



/*-----------------------------------------------------------------------
//
// Function: OCBMinConst()
//
//   Return mininmal constant for type (if already fixed). Return 0
//   otherwise.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

FunCode OCBMinConst(OCB_p ocb, Type_p type)
{
   long sort = type->type_uid;
   FunCode cand;

   cand = PDArrayElementInt(ocb->min_constants, sort);

   return cand;
}

/*-----------------------------------------------------------------------
//
// Function: OCBCondSetMinConst()
//
//   Set mininmal constant for type (if not already fixed).
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void OCBCondSetMinConst(OCB_p ocb, Type_p type, FunCode cand)
{
   long sort = type->type_uid;

   if(!OCBMinConst(ocb, type))
   {
      PDArrayAssignInt(ocb->min_constants, sort, cand);
   }
}



/*-----------------------------------------------------------------------
//
// Function: OCBFindMinConst()
//
//   Find a minimal (by precedence) function symbol constant in
//   ocb->sig. Store it in ocb->min_constant. If no constant
//   exists, create one.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

FunCode OCBFindMinConst(OCB_p ocb, Type_p type)
{
   FunCode i, cand=0;
   assert(type);

   assert(ocb && ocb->sig);

   cand = OCBMinConst(ocb, type);
   if(!cand)
   {
      for(i=ocb->sig->internal_symbols+1; i<=ocb->sig->f_count; i++)
      {
         if(SigIsFunConst(ocb->sig, i) &&
            !SigIsSpecial(ocb->sig, i) &&
            (SigGetType(ocb->sig,i) == type) &&
            (!cand || (OCBFunCompare(ocb, i, cand)==to_greater)))
         {
            cand = i;
         }
      }
      if(!cand)
      {
         cand = SigGetNewSkolemCode(ocb->sig, 0);
         SigDeclareFinalType(ocb->sig, cand, type);
      }
      OCBCondSetMinConst(ocb, type, cand);
   }
   return cand;
}


/*-----------------------------------------------------------------------
//
// Function: OCBTermMaxFunCode()
//
//   Return the (or rather a) maximal function symbol (according to
//   ocb->precedence) from term. Follows bindings exactly once
//   (i.e. assumes that substitutions are matches).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

FunCode OCBTermMaxFunCode(OCB_p ocb, Term_p term)
{
   int       i;
   FunCode   res = 0, tmp ;
   DerefType deref = DEREF_ONCE;

   assert(ocb->precedence||ocb->prec_weights);

   // it follows all bindings once, so there is no need
   // to change anything -- normal deref behaves the same ways
   term = TermDeref(term, &deref);

   if(TermIsAnyVar(term))
   {
      return res;
   }
   res = term->f_code;

   for(i=1; i<term->arity; i++)
   {
      tmp = OCBTermMaxFunCode(ocb, term->args[i]);
      if(OCBFunCompare(ocb, tmp, res) == to_greater)
      {
         res = tmp;
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: OCBFunCompareMatrix()
//
//   Return comparison result of two symbols in precedence via the
//   full precedence matrix. Symbols
//   not covered by the ocb are smaller than all others. Equal symbols
//   are not allowed (captured at OCBFunCompare).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

CompareResult OCBFunCompareMatrix(OCB_p ocb, FunCode f1, FunCode f2)
{
   assert(ocb->precedence);
   assert(f1!=f2);

   if(f1<=ocb->sig_size)
   {
      if(f2<=ocb->sig_size)
      {
         return *(OCBFunComparePos(ocb, f1, f2));
      }
      return to_greater;
   }
   if(f2<=ocb->sig_size)
   {
      return to_lesser;
   }
   assert((f1>ocb->sig_size) && (f1>ocb->sig_size));
   return Q_TO_PART(f2-f1);
}

/*-----------------------------------------------------------------------
//
// Function: OCBResetHOVarMap()
//
//   Resets mapping of (applied) variables to number of occurrences.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void OCBResetHOVarMap(OCB_p ocb)
{
   PObjMapFreeWDeleter(ocb->ho_vb, free_val);
   ocb->ho_vb = NULL;
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
