/*-----------------------------------------------------------------------

  File  : cto_ocb.h

  Author: Stephan Schulz

  Contents

  Global definitions for orderings: Comparison results, precedences,
  order control blocks.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: Wed Apr 29 02:51:28 MET DST 1998

  -----------------------------------------------------------------------*/

#ifndef CTO_OCB

#define CTO_OCB

#include <cte_termbanks.h>
#include <clb_objmaps.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/



typedef enum
{
   NoOrdering,
   OPTIMIZE_AX,
   KBO,
   KBO6,
   LPO,
   LPOCopy,
   LPO4,
   LPO4Copy,
   RPO,
   EMPTY
}TermOrdering;

typedef enum
{
   LCNoCmp,
   LCNormal,
   LCTFOEqMax,
   LCTFOEqMin
}LiteralCmp;


typedef struct ocb_cell
{
   TermOrdering  type;
   long          sig_size;
   Sig_p         sig;          /* Slightly hacked...this is only an
                                  unsupervised reference (but will
                                  stay)! Always free the OCB before
                                  the signature and the signature
                                  independently after the OCB. */
   PDArray_p     min_constants; /* Indexed by sort */
   long          *weights;     /* Array of weights */
   long          var_weight;   /* Variable Weight */
   long          lam_weight;   /* Variable Weight */
   long          db_weight;   /* Variable Weight */
   long          *prec_weights;/* Precedence defined by weight - only
                                  for total precedences */
   CompareResult *precedence;  /* The most general case, interpreted
                                  as two-dimensional array, indexed by
                                  two symbols */
   LiteralCmp    lit_cmp;      /* Incomparable, as terms, or with
                                  fake transfinite KBO on predicate
                                  symbols. */
   bool          rewrite_strong_rhs_inst;
   PStack_p      statestack;   /* Contains backtrack information */
   long          wb;
   long          pos_bal;
   long          neg_bal;
   long          max_var;
   long          vb_size;
   int           *vb;
   PObjMap_p     ho_vb; // mapping (applied) vars to num of occurrences
   HoOrderKind   ho_order_kind;
}OCBCell, *OCB_p;

#define OCB_FUN_DEFAULT_WEIGHT 1
#define MK_HO_VB_KEY(key, x) (key) = ((long*)SizeMalloc(sizeof(long))); *(key) = x
#define OCBLamWeight(ocb) ((const long)((ocb)->lam_weight))
#define OCBDBWeight(ocb) ((const long)((ocb)->db_weight))

/* Default weight for symbols not treated in a special way, also used
   as multiplier for other generated weights. */

#define W_DEFAULT_WEIGHT 1

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define OCBCellAlloc() (OCBCell*)SizeMalloc(sizeof(OCBCell))
#define OCBCellFree(junk)        SizeFree(junk, sizeof(OCBCell))

/* Symbolic representation of ordering relations */

extern char*  TONames[];

OCB_p         OCBAlloc(TermOrdering type, bool prec_by_weight, Sig_p sig, HoOrderKind ho_order_kind);
void          OCBFree(OCB_p junk);

void          OCBDebugPrint(FILE* out, OCB_p ocb);

PStackPointer OCBPrecedenceAddTuple(OCB_p ocb, FunCode f1, FunCode f2,
                                    CompareResult relation);

bool          OCBPrecedenceBacktrack(OCB_p ocb, PStackPointer state);
#define OCBPrecedenceGetState(ocb)              \
   PStackGetSP((ocb)->statestack)

/* Getting the addresses of OCB entries for modification */

/*
  #define OCBFunWeightPos(ocb, f)                                       \
  (assert((f)>0), assert((f)<=(ocb)->sig_size), &((ocb)->weights[(f)]))
  #define OCBFunComparePos(ocb, f1, f2)                                 \
  (assert((f1)>0), assert((f2)>0), assert((f1)<=(ocb)->sig_size),       \
  assert((f2)<=(ocb)->sig_size),                                        \
  (&((ocb)->precedence[((f2)-1)*(ocb)->sig_size+((f1)-1)])))
*/

#define OCBFunWeightPos(ocb, f) &((ocb)->weights[(f)])
#define OCBFunComparePos(ocb, f1, f2) (&((ocb)->precedence[((f2)-1)*(ocb)->sig_size+((f1)-1)]))


void    OCBCondSetMinConst(OCB_p ocb, Type_p type, FunCode cand);
FunCode OCBMinConst(OCB_p ocb, Type_p type);
FunCode OCBFindMinConst(OCB_p ocb, Type_p type);
void OCBSetMinConst(OCB_p ocb, Type_p type, FunCode cand);

#define OCBDesignatedMinTerm(ocb, terms, type) \
   TBCreateMinTerm((terms),OCBFindMinConst((ocb),(type)))

/* Functions for Querying the OCB */

static inline long OCBFunWeight(OCB_p ocb, FunCode f);
static inline long OCBFunPrecWeight(OCB_p ocb, FunCode f);
static inline CompareResult OCBFunCompare(OCB_p ocb, FunCode f1, FunCode f2);

CompareResult OCBFunCompareMatrix(OCB_p ocb, FunCode f1, FunCode f2);
FunCode       OCBTermMaxFunCode(OCB_p ocb, Term_p term);
void OCBResetHOVarMap(OCB_p ocb);


/*---------------------------------------------------------------------*/
/*                        Inline Functions                             */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: OCBFunWeight()
//
//   Return the weight of f in ocb. For symbols entered in the OCB
//   after creation return OCB_FUN_DEFAULT_WEIGHT.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static inline long OCBFunWeight(OCB_p ocb, FunCode f)
{
   if(f<=ocb->sig_size)
   {
      return *(OCBFunWeightPos(ocb, (f)));
   }
   return OCB_FUN_DEFAULT_WEIGHT;
}


/*-----------------------------------------------------------------------
//
// Function: OCBFunPrecWeight()
//
//   If f has a weight in  ocb->prec_weights, return it. Otherwise
//   return a unique negative ficticious weight smaller than all
//   normal weights.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static inline long OCBFunPrecWeight(OCB_p ocb, FunCode f)
{
   if(ocb->prec_weights && f<=ocb->sig_size)
   {
      return ocb->prec_weights[f];
   }
   return -f;
}


/*-----------------------------------------------------------------------
//
// Function: OCBFunCompare()
//
//   Return comparison result of two symbols in precedence. Symbols
//   not covered by the ocb are smaller than all others (except for
//   $true), and older symbols are smaller than new ones.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static inline CompareResult OCBFunCompare(OCB_p ocb, FunCode f1, FunCode f2)
{
   long tmp;

   assert((f1>0)&&(f2>0));

   if(f1==f2)
   {
      return to_equal;
   }
   if(f1==SIG_TRUE_CODE)
   {
      return to_lesser;
   }
   if(f2==SIG_TRUE_CODE)
   {
      return to_greater;
   }
   tmp = (long)SigIsAnyFuncPropSet(ocb->sig, f2, ocb->sig->distinct_props)-
      (long)SigIsAnyFuncPropSet(ocb->sig, f1, ocb->sig->distinct_props);
   if(tmp)
   {
      /* printf("f1 = %ld, f2 = %ld, res = %ld\n", f1, f2, tmp); */
      return Q_TO_PART(tmp);
   }

   if(ocb->prec_weights)
   {
      long w1 = (f1<=ocb->sig_size) ? ocb->prec_weights[f1] : -f1;
      long w2 = (f2<=ocb->sig_size) ? ocb->prec_weights[f2] : -f2;
      return Q_TO_PART(w1-w2);
   }
   return OCBFunCompareMatrix(ocb, f1, f2);
}


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
