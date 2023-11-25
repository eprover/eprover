/*-----------------------------------------------------------------------

  File  : che_to_params.c

  Author: Stephan Schulz (schulz@eprover.org)

  Contents

  Functions for dealing with term ordering parameters.

  Copyright 2020 by the authors.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

  Created: Thu Dec 31 17:42:46 MET 1998

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
#ifdef ENABLE_LFHO
   "typefreq",         /* PByTypeFreq */
   "invtypefreq",      /* PByInvTypeFreq */
   "combfreq",         /* PByCombFreq */
   "invcombfreq",      /* PByInvCombFreq */
#endif
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
#ifdef ENABLE_LFHO
   "typefreqrank",           /* WTypeFrequencyRank */
   "typefreqcount",          /* WTypeFrequencyCount */
   "invtypefreqrank",        /* WInvTypeFrequencyRank */
   "invtypefreqcount",       /* WInvTypeFrequencyCount */
   "combfreqrank",           /* WCombFrequencyRank */
   "combfreqcount",          /* WCombFrequencyCount */
   "invcombfreqrank",        /* WInvCombFrequencyRank */
   "invcombfreqcount",       /* WInvCombFrequencyCount */
#endif
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
   handle->force_kbo_var_weight          = false;
   handle->to_pre_weights                = NULL;
   handle->to_const_weight               = WConstNoWeight;
   handle->to_defs_min                   = false;
   handle->lit_cmp                       = LCNormal;
   handle->ho_order_kind                 = LFHO_ORDER;
   handle->lam_w                         = DEFAULT_LAMBDA_WEIGHT;
   handle->db_w                          = DEFAULT_DB_WEIGHT;
}


/*-----------------------------------------------------------------------
//
// Function: OrderParmsPrint()
//
//    Print the ordering parameters in Human/Machine-readable form.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void OrderParmsPrint(FILE* out, OrderParms_p handle)
{
   fprintf(out, "   {\n");
   fprintf(out, "      ordertype:               %s\n",
           TONames[handle->ordertype]);
   fprintf(out, "      to_weight_gen:           %s\n",
           TOWeightGenNames[handle->to_weight_gen]);
   fprintf(out, "      to_prec_gen:             %s\n",
           TOPrecGenNames[handle->to_prec_gen]);
   fprintf(out, "      rewrite_strong_rhs_inst: %s\n",
           BOOL2STR(handle->rewrite_strong_rhs_inst));

   fprintf(out, "      to_pre_prec:             \"%s\"\n",
           handle->to_pre_prec?handle->to_pre_prec:"");
   fprintf(out, "      conj_only_mod:           %d\n", handle->conj_only_mod);
   fprintf(out, "      conj_axiom_mod:          %d\n", handle->conj_axiom_mod);
   fprintf(out, "      axiom_only_mod:          %d\n", handle->axiom_only_mod);
   fprintf(out, "      skolem_mod:              %d\n", handle->skolem_mod);
   fprintf(out, "      defpred_mod:             %d\n", handle->defpred_mod);
   fprintf(out, "      force_kbo_var_weight:    %s\n",
           BOOL2STR(handle->force_kbo_var_weight));
   fprintf(out, "      to_pre_weights:          \"%s\"\n",
           handle->to_pre_weights?handle->to_pre_weights:"");

   fprintf(out, "      to_const_weight:         %ld\n", handle->to_const_weight);
   fprintf(out, "      to_defs_min:             %s\n",
           BOOL2STR(handle->to_defs_min));
   fprintf(out, "      lit_cmp:                 %d\n", handle->lit_cmp);
   fprintf(out, "      lam_w:                   %d\n", handle->lam_w);
   fprintf(out, "      db_w:                    %d\n", handle->db_w);
   fprintf(out, "      ho_order_kind:           %s\n", HOK2STR(handle->ho_order_kind));

   fprintf(out, "   }\n");
}

/*-----------------------------------------------------------------------
//
// Function: OrderParmsParseInto()
//
//   Parse the OrderParram-Cell into/over the existing
//   cell. Parameters are expected in-order, but may be
//   missing. Returns true if all parameters have been found, false
//   otherwise.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

HoOrderKind str2hok(char* v)
{
   HoOrderKind hok = STR2HOK(v);
   if(hok==-1)
   {
      Error("Unknown HOOrderKind", USAGE_ERROR);
   }
   return hok;
}

bool OrderParmsParseInto(Scanner_p in,
                         OrderParms_p handle,
                         bool warn_missing)
{
   bool res = true;

   AcceptInpTok(in, OpenCurly);

   PARSE_IDENT_NO(ordertype, TONames);
   PARSE_IDENT_NO(to_weight_gen, TOWeightGenNames);
   PARSE_IDENT_NO(to_prec_gen, TOPrecGenNames);
   PARSE_BOOL(rewrite_strong_rhs_inst);
   PARSE_STRING(to_pre_prec);
   if(handle->to_pre_prec && strcmp(handle->to_pre_prec, "")==0)
   {
      handle->to_pre_prec = NULL;
   }
   PARSE_INT(conj_only_mod);
   PARSE_INT(conj_axiom_mod);
   PARSE_INT(axiom_only_mod);
   PARSE_INT(skolem_mod);
   PARSE_INT(defpred_mod);
   PARSE_BOOL(force_kbo_var_weight);
   PARSE_STRING(to_pre_weights);
   if(handle->to_pre_weights && strcmp(handle->to_pre_weights, "")==0)
   {
      handle->to_pre_weights = NULL;
   }
   PARSE_INT(to_const_weight);
   PARSE_BOOL(to_defs_min);
   PARSE_INT(lit_cmp);
   PARSE_INT(lam_w);
   PARSE_INT(db_w);
   PARSE_STRING_AND_CONVERT(ho_order_kind, str2hok);

   AcceptInpTok(in, CloseCurly);

   return res;
}




/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
