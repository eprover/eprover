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

<1> Wed Apr 29 02:51:28 MET DST 1998
    New

-----------------------------------------------------------------------*/

#ifndef CTO_OCB

#define CTO_OCB

#include <cte_termbanks.h>
#include <cto_kbodata.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/



typedef enum 
{
   NoOrdering,
   AUTO,
   AUTOCASC,
   AUTODEV,
   AUTOSCHED0,
   AUTOSCHED1,
   AUTOSCHED2,
   AUTOSCHED3,
   AUTOSCHED4,
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


typedef struct ocb_cell
{
   TermOrdering  type;
   Sig_p         sig;          /* Slightly hacked...this is only an
				   unsupervised reference (but will
				   stay) ! */
   FunCode       min_constant;
   int           sig_size;
   long          *weights;     /* Array of weights */
   long          var_weight;   /* Variable Weight */
   long          *prec_weights;/* Precedence defined by weight - only
				  for total precedenced */
   CompareResult *precedence;  /* The most general case, interpreted
				  as two-dimensional array, indexed by
				  two symbols */
   bool           no_lit_cmp;  /* If true, all literals are
				  uncomparable (useful for SOS
				  strategy) */
   PStack_p       statestack;  /* Contains backtrack information */
   KBOLin_p       kbobalance; 
}OCBCell, *OCB_p;

#define OCB_FUN_DEFAULT_WEIGHT 1

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

OCB_p         OCBAlloc(TermOrdering type, bool prec_by_weight, Sig_p sig);
void          OCBFree(OCB_p junk);

void          OCBDebugPrint(FILE* out, OCB_p ocb);

PStackPointer OCBPrecedenceAddTuple(OCB_p ocb, FunCode f1, FunCode f2,
				    CompareResult relation);

bool          OCBPrecedenceBacktrack(OCB_p ocb, PStackPointer state);
#define OCBPrecedenceGetState(ocb) \
              PStackGetSP((ocb)->statestack)

/* Getting the addresses of OCB entries for modification */


/*
#define OCBFunWeightPos(ocb, f) \
              (assert((f)>0), assert((f)<=(ocb)->sig_size), &((ocb)->weights[(f)-1]))
#define OCBFunComparePos(ocb, f1, f2) \
              (assert((f1)>0), assert((f2)>0), assert((f1)<=(ocb)->sig_size),\
	       assert((f2)<=(ocb)->sig_size),\
              (&((ocb)->precedence[((f2)-1)*(ocb)->sig_size+((f1)-1)])))
*/

#define OCBFunWeightPos(ocb, f) &((ocb)->weights[(f)-1])
#define OCBFunComparePos(ocb, f1, f2) (&((ocb)->precedence[((f2)-1)*(ocb)->sig_size+((f1)-1)]))


FunCode OCBFindMinConst(OCB_p ocb);

#define OCBDesignatedMinConst(ocb) ((ocb)->min_constant?\
                                   (ocb)->min_constant:OCBFindMinConst(ocb))

#define OCBDesignatedMinTerm(ocb, terms) ((terms)->min_term?\
                                   (terms)->min_term:\
                                   TBCreateMinTerm((terms),OCBDesignatedMinConst(ocb)))

/* Functions for Querying the OCB */

static __inline__ long          OCBFunWeight(OCB_p ocb, FunCode f);
static __inline__ CompareResult OCBFunCompare(OCB_p ocb, FunCode f1, FunCode f2);

CompareResult OCBFunCompareMatrix(OCB_p ocb, FunCode f1, FunCode f2);
FunCode       OCBTermMaxFunCode(OCB_p ocb, Term_p term);


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

static __inline__ long OCBFunWeight(OCB_p ocb, FunCode f)
{
   if(f<=ocb->sig_size)
   {
      return *(OCBFunWeightPos(ocb, (f)));
   }
   return OCB_FUN_DEFAULT_WEIGHT;
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

static __inline__ CompareResult OCBFunCompare(OCB_p ocb, FunCode f1,
					      FunCode f2)
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
      long w1,w2;
      
      if(f1<=ocb->sig_size)
      {
	 w1 = ocb->prec_weights[f1];
      }
      else 
      {
	 w1 = -f1;
      }
      if(f2<=ocb->sig_size)
      {
	 w2 = ocb->prec_weights[f2];
      }
      else 
      {
	 w2 = -f2;
      }
      return Q_TO_PART(w1-w2);
   }
   return OCBFunCompareMatrix(ocb, f1, f2);
}



#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





