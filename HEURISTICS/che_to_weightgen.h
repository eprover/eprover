/*-----------------------------------------------------------------------

File  : che_to_weightgen.h

Author: Stephan Schulz

Contents

  Routines for generating weights for term orderings

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Fri Sep 25 02:49:11 MET DST 1998
    New

-----------------------------------------------------------------------*/

#ifndef CHE_TO_WEIGHTGEN

#define CHE_TO_WEIGHTGEN


#include <che_fcode_featurearrays.h>



/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef enum
{
   WInvalidEntry = -1,
   WNoMethod = 0,         /* Nothing */
   WSelectMaximal,        /* First maximal symbol in precedence gets
                             weight 0 */
   WArityWeight,          /* Weight(f) = Arity(f)+1 */
   WArityMax0,            /* Weight(f) = Arity(f)+1, 0 for first max*/
   WModArityWeight,       /* Weight(f) = Arity(f)+W_TO_BASEWEIGHT */
   WModArityMax0,         /* Weight(f) = Arity(f)+W_TO_BASEWEIGHT, 0
                             for first max*/
   WAritySqWeight,        /* Weight(f) = Arity(f)^2+1) */
   WAritySqMax0,          /* Weight(f) = Arity(f)^2+1), 0 for first
                             max */
   WInvArityWeight,       /* Weight(f) = Maxarity+1-Arity(f) */
   WInvArityMax0,         /* Weight(f) = Maxarity+1-Arity(f), 0 for
                             first max */
   WInvAritySqWeight,     /* Weight(f) = Maxarity^2+1-Arity(f)^2 */
   WInvAritySqMax0,       /* Weight(f) = Maxarity^2+1-Arity(f)^2, 0
                             for first max */
   WPrecedence,           /* Weight(f) = |{g|g<f}| */
   WPrecedenceInv,        /* Weight(f) = |{g|g>f}| */
   WPrecRank5,            /* */
   WPrecRank10,            /* */
   WPrecRank20,            /* */
   WFrequency,            /* Weight(f) = |Axioms|_f */
   WInvFrequency,         /* Weight(f) = Maxfreq+1-|Axioms|_f */
   WFrequencyRank,        /* Weight(f) = Rank in frequency-induced
                             quasi-ordering */
   WInvFrequencyRank,     /* Weight(f) = Inverse rank in
                             frequency-induced * quasi-ordering */
   WInvConjFrequencyRank, /* Weight(f) = Inverse rank in
                             conjecture-frequency-induced
                             quasi-ordering */
   WFrequencyRankSq,      /* As above, but squared */
   WInvFrequencyRankSq,   /* Ditto */
   WInvModFreqRank,       /* As WInvFrequencyRank, but difference
                             between ranks is cardinality of set of
                             symbols in rank */
   WInvModFreqRankMax0,   /* As above, but first maximal unary is 0 */
   WConstantWeight,       /* All weights 1 */
   WMinMethod = WSelectMaximal,
   WMaxMethod = WConstantWeight /* Update as required! */
}TOWeightGenMethod;

/* Think about goal-directedness, prefer symbols occuring in the goal */

#define WConstNoSpecialWeight -1
#define WConstNoWeight         0


/* Used as base weight for ModArity */
#define W_TO_BASEWEIGHT 4


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


extern char* TOWeightGenNames[];

#define TOGetWeightGenName(method) \
        (TOWeightGenNames[(method)])

TOWeightGenMethod TOTranslateWeightGenMethod(char* name);

#define TOGenerateDefaultWeights(ocb) \
        TOGenerateWeights((ocb), NULL, WSelectMaximal, \
           W_DEFAULT_WEIGHT)

void TOGenerateWeights(OCB_p ocb, ClauseSet_p axioms, char *pre_weights,
             TOWeightGenMethod method, long const_weight);



#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/






