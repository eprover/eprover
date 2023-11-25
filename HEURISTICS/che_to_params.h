/*-----------------------------------------------------------------------

  File  : che_to_params.h

  Author: Stephan Schulz

  Contents

  Data types and auxilliary functions for describing orderig
  parameters.

  Copyright 2020-2023 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Thu Dec 31 17:39:46 MET 1998

-----------------------------------------------------------------------*/

#ifndef CHE_TO_PARAMS

#define CHE_TO_PARAMS

#include <clb_permastrings.h>
#include <cto_ocb.h>

#define DEFAULT_LAMBDA_WEIGHT 20
#define DEFAULT_DB_WEIGHT 10

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/



typedef enum
{
   PInvalidEntry = -1,
   PNoMethod = 0,       /* Nothing */
   PUnaryFirst,         /* My hack ;-) */
   PUnaryFirstFreq,     /* My new hack ;-) */
   PArity,              /* Sort by arity */
   PInvArity,           /* Sort by inverse arity */
   PConstMax,           /* Sort by arity, but constants first (Default
                           for SPASS) */
   PInvArConstMin,      /* Sort by inverse arity, but make constants
                           minimal */
   PByFrequency,        /* Make often occuring symbols big */
   PByInvFrequency,     /* Make often occuring symbols small */
   PByInvConjFrequency, /* Make often occuring symbols small,
                         * conjecture symbols large */
   PByInvFreqConjMax,   /* Make conjecture symbols maximal, otherwise
                           use invfreq */
   PByInvFreqConjMin,   /* Make conjecture symbols minimal, otherwise
                           use invfreq */
   PByInvFreqConstMin,  /* Make rarely occuring symbols small, except for
                           constants */
   PByInvFreqHack,      /* Make constants minimal, frequent unary
                           symbols maximal, otherwise as
                           PByInvFrequency */
#ifdef ENABLE_LFHO
   PByTypeFreq,        /*  By frequency of type function symbol corresponds
                           to */
   PByInvTypeFreq,     /*  Same as prev, only inverse*/
   PByCombFreq,        /*  Based on sybmol type frequency + symbol frequency */
   PByInvCombFreq,     /*  Inverse of the previous */
#endif
   PArrayOpt,           /* Special hack for theory of array with
                           conceptually typed symbols recognized by
                           name. */
   POrientAxioms,       /* My (planned) hack */
   PMinMethod = PUnaryFirst,
   PMaxMethod = POrientAxioms
}TOPrecGenMethod;


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
#ifdef ENABLE_LFHO
   WTypeFrequencyRank,    /* Similar to above, however, they work not by
                             value of symbol, but by its type */
   WTypeFrequencyCount,
   WInvTypeFrequencyRank,
   WInvTypeFrequencyCount,
   WCombFrequencyRank,    /* Similar to type frequency schemes, but
                             combines type frequencies with symbol frequencies */
   WCombFrequencyCount,
   WInvCombFrequencyRank,
   WInvCombFrequencyCount,
#endif
   WConstantWeight,       /* All weights 1 */
   WMinMethod = WSelectMaximal,
   WMaxMethod = WConstantWeight /* Update as required! */
}TOWeightGenMethod;

typedef struct order_parms_cell
{
   TermOrdering      ordertype;
   TOWeightGenMethod to_weight_gen;
   TOPrecGenMethod   to_prec_gen;
   /* When generating an ordering, the most significant key is one of
    * this modifiers */
   int               conj_only_mod;
   int               conj_axiom_mod;
   int               axiom_only_mod;
   int               skolem_mod;
   int               defpred_mod;
   /* Set KBO varweight to minimal constant weight (if not already so) */
   bool              force_kbo_var_weight;
   /* Ground unbound RHS variables when rewriting */
   bool              rewrite_strong_rhs_inst;
   /* User-provided ordering parameters. Only pointers, not copies */
   char*             to_pre_prec;
   char*             to_pre_weights;
   /* Separate weight for constants */
   long              to_const_weight;
   /* Separare weight for introduced definition symbols */
   bool              to_defs_min;
   /* How to compare literals */
   LiteralCmp        lit_cmp;
   HoOrderKind       ho_order_kind;
   int               lam_w;
   int               db_w;
}OrderParmsCell, *OrderParms_p;

#define HOK2STR(x) (((x) == LFHO_ORDER) ? ("lfho") : (((x) == LAMBDA_ORDER) ? "lambda" : "unknown"))

#define STR2HOK(val) (!strcmp((val), "lfho") ? LFHO_ORDER\
                      :!strcmp((val), "lambda") ? LAMBDA_ORDER\
                      :(-1))


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: PARSE_BOOL/PARSE_INT()
//
//   Macros for parsing individual parameters.
//
// Global Variables: -
//
// Side Effects    : Reading input, manipulating "res" in the local
//                   environment, assigning the parameter.
//
/----------------------------------------------------------------------*/


#define PARSE_BOOL(name)\
   if(TestInpId(in, #name))\
   {\
      NextToken(in);\
      AcceptInpTok(in, Colon);\
      handle->name = ParseBool(in);\
   }\
   else\
   {\
      res = false;\
      if(warn_missing)\
      {\
         Warning("Config misses %s\n", #name);\
      }\
   }

#define PARSE_INTMAX(name)\
   if(TestInpId(in, #name))\
   {\
      NextToken(in);\
      AcceptInpTok(in, Colon);\
      handle->name = ParseIntMax(in);\
   }\
   else\
   {\
      res = false;\
      if(warn_missing)\
      {\
         Warning("Config misses %s\n", #name);\
      }\
   }


#define PARSE_INT(name)\
   if(TestInpId(in, #name))\
   {\
      NextToken(in);\
      AcceptInpTok(in, Colon);\
      handle->name = ParseInt(in);\
   }\
   else\
   {\
      res = false;\
      if(warn_missing)\
      {\
         Warning("Config misses %s\n", #name);\
      }\
   }

#define PARSE_INT_LIMITED(name, low, high) \
   if(TestInpId(in, #name))\
   {\
      NextToken(in);\
      AcceptInpTok(in, Colon);\
      handle->name = ParseIntLimited(in, low, high); \
   }\
   else\
   {\
      res = false;\
      if(warn_missing)\
      {\
         Warning("Config misses %s\n", #name);\
      }\
   }


#define PARSE_IDENTIFIER(name)\
   if(TestInpId(in, #name))\
   {\
      NextToken(in);\
      AcceptInpTok(in, Colon);\
      CheckInpTok(in, Identifier);\
      handle->name = PermaString(DStrView(AktToken(in)->literal)); \
      NextToken(in);\
   }\
   else\
   {\
      res = false;\
      if(warn_missing)\
      {\
         Warning("Config misses %s\n", #name);\
      }\
   }

#define PARSE_IDENT_INTO(name, maxlen)                 \
   if(TestInpId(in, #name))\
   {\
      NextToken(in);\
      AcceptInpTok(in, Colon);\
      CheckInpTok(in, Identifier);\
      strncpy(handle->name, DStrView(AktToken(in)->literal), maxlen-1); \
      handle->name[maxlen-1] = '\0'; \
      NextToken(in);\
   }\
   else\
   {\
      res = false;\
      if(warn_missing)\
      {\
         Warning("Config misses %s\n", #name);\
      }\
   }


#define PARSE_IDENT_NO(name, ids)                 \
   if(TestInpId(in, #name))\
   {\
      NextToken(in);\
      AcceptInpTok(in, Colon);\
      CheckInpTok(in, Identifier);\
      handle->name = StringIndex(DStrView(AktToken(in)->literal), ids);\
      if(handle->name==-1)\
      {\
         DStr_p errstr = DStrAlloc();           \
         DStrAppendStrArray(errstr, ids, "|");  \
         CheckInpId(in, DStrView(errstr));      \
         DStrFree(errstr);                      \
      }                                         \
      NextToken(in);                            \
   }\
   else\
   {\
      res = false;\
      if(warn_missing)\
      {\
         Warning("Config misses %s\n", #name);\
      }\
   }


#define PARSE_STRING(name)\
   if(TestInpId(in, #name))\
   {\
      NextToken(in);                            \
      AcceptInpTok(in, Colon);\
      CheckInpTok(in, String);                            \
      handle->name = PermaStringStore(DStrCopyCore(AktToken(in)->literal)); \
      NextToken(in);\
   }\
   else\
   {\
      res = false;\
      if(warn_missing)\
      {\
         Warning("Config misses %s\n", #name);\
      }\
   }

// Does not assign to handle
#define PARSE_STRING_AND_CONVERT(name, converter)\
   if(TestInpId(in, #name))\
   {\
      NextToken(in);                            \
      AcceptInpTok(in, Colon);\
      CheckInpTok(in, String|Identifier);                            \
      handle->name = converter(DStrView(AktToken(in)->literal));     \
      NextToken(in);\
   }\
   else\
   {\
      res = false;\
      if(warn_missing)\
      {\
         Warning("Config misses %s\n", #name);\
      }\
   }



extern char* TOPrecGenNames[];

#define TOGetPrecGenName(method) (TOPrecGenNames[(method)])

TOPrecGenMethod TOTranslatePrecGenMethod(char* name);



/* Think about goal-directedness, prefer symbols occuring in the goal */

#define WConstNoSpecialWeight -1
#define WConstNoWeight         0

extern char* TOWeightGenNames[];

#define TOGetWeightGenName(method) \
        (TOWeightGenNames[(method)])

TOWeightGenMethod TOTranslateWeightGenMethod(char* name);



#define OrderParmsCellAlloc() \
   (OrderParmsCell*)SizeMalloc(sizeof(OrderParmsCell))
#define OrderParmsCellFree(junk) \
   SizeFree(junk, sizeof(OrderParmsCell))

void OrderParmsInitialize(OrderParms_p handle);
void OrderParmsPrint(FILE* out, OrderParms_p handle);
bool OrderParmsParseInto(Scanner_p in,
                         OrderParms_p handle,
                         bool warn_missing);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
