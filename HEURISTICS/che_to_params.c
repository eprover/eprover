/*-----------------------------------------------------------------------

  File  : che_te_params.c

  Author: Stephan Schulz (schulz@eprover.org)

  Contents

  Functions for dealing with term ordering parameters.

  Copyright 2020 by the authors.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

  Created:

  -----------------------------------------------------------------------*/

#include "che_to_params.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


char* TOPrecGenNames[]=
{
   "none",             /* PNoMethod */
   "unary_first",      /* PUnaryFirst */
   "unary_freq",       /* PUnaryFristFreq */
   "arity",            /* PArity */
   "invarity",         /* PInvArity */
   "const_max",        /* PConstMax  */
   "const_min",        /* PInvArConstMin */
   "freq",             /* PByFrequency */
   "invfreq",          /* PByInvFrequency */
   "invconjfreq",      /* PByInvConjFrequency */
   "invfreqconjmax",   /* PByInvFreqConjMax */
   "invfreqconjmin",   /* PByInvFreqConjMin */
   "invfreqconstmin",  /* PByInvFreqConstMin */
   "invfreqhack",      /* PByInvFreqHack */
   "typefreq",         /* PByTypeFreq */
   "invtypefreq",      /* PByInvTypeFreq */
   "combfreq",         /* PByCombFreq */
   "invcombfreq",      /* PByInvCombFreq */
   "arrayopt",         /* PArrayOpt */
   "orient_axioms",    /* POrientAxioms */
   NULL
};



char* TOWeightGenNames[]=
{
   "none",                   /* WNoMethod */
   "firstmaximal0",          /* WSelectMaximal */
   "arity",                  /* WArityWeight */
   "aritymax0",              /* WArityMax0 */
   "modarity",               /* WModArityWeight */
   "modaritymax0",           /* WModArityMax0 */
   "aritysquared",           /* WAritySqWeight */
   "aritysquaredmax0",       /* WAritySqMax0 */
   "invarity",               /* WInvArityWeight */
   "invaritymax0",           /* WInvArityMax0 */
   "invaritysquared",        /* WInvSqArityWeight */
   "invaritysquaredmax0",    /* WInvAritySqMax0 */
   "precedence",             /* WPrecedence */
   "invprecedence",          /* WPrecedenceInv */
   "precrank5",
   "precrank10",
   "precrank20",
   "freqcount",
   "invfreqcount",
   "freqrank",
   "invfreqrank",
   "invconjfreqrank",        /* WInvConjFrequencyRank */
   "freqranksquare",
   "invfreqranksquare",
   "invmodfreqrank",         /* WModFreqRank */
   "invmodfreqrankmax0",     /* WModFreqRankMax0 */
   "typefreqrank",           /* WTypeFrequencyRank */
   "typefreqcount",          /* WTypeFrequencyCount */
   "invtypefreqrank",        /* WInvTypeFrequencyRank */
   "invtypefreqcount",       /* WInvTypeFrequencyCount */
   "combfreqrank",           /* WCombFrequencyRank */
   "combfreqcount",          /* WCombFrequencyCount */
   "invcombfreqrank",        /* WInvCombFrequencyRank */
   "invcombfreqcount",       /* WInvCombFrequencyCount */
   "constant",               /* WConstantWeight */
   NULL
};


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: OrderParmsInitialize()
//
//   Initialize an ordering parameter cell with rational default
//   values.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void OrderParmsInitialize(OrderParms_p handle)
{
   handle->ordertype                     = KBO6;
   handle->to_weight_gen                 = WNoMethod;
   handle->to_prec_gen                   = PNoMethod;
   handle->rewrite_strong_rhs_inst       = false;
   handle->to_pre_prec                   = NULL;
   handle->conj_only_mod                 = 0;
   handle->conj_axiom_mod                = 0;
   handle->axiom_only_mod                = 0;
   handle->skolem_mod                    = 0;
   handle->defpred_mod                   = 0;
   handle->to_pre_weights                = NULL;
   handle->to_const_weight               = WConstNoWeight;
   handle->to_defs_min                   = false;
   handle->lit_cmp                       = LCNormal;
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
