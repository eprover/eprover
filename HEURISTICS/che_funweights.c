/*-----------------------------------------------------------------------

File  : che_funweights.c

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Weight functions dealing with individual weights for individual
  function- and predicate symbols.

  Copyright 2005 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sat May  7 21:22:32 CEST 2005
    New

-----------------------------------------------------------------------*/

#include "che_funweights.h"



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
// Function: init_conj_vector()
//
//   Initialize the function weight vector based on the data in data
//   ;-). Factored out so it can be called from the weight
//   function(s).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void init_conj_vector(FunWeightParam_p data)
{
   if(!data->fweights)
   {
      FunCode i;
      Clause_p handle;

      data->flimit   = data->ocb->sig->f_count+1;
      data->fweights = SizeMalloc(data->flimit*sizeof(long));

      for(i=0;i<data->flimit; i++)
      {
         data->fweights[i] = 0;
      }
      for(handle=data->axioms->anchor->succ;
          handle!=data->axioms->anchor;
          handle = handle->succ)
      {
         if(ClauseQueryTPTPType(handle)==CPTypeNegConjecture)
         {
            ClauseAddSymbolDistribution(handle, data->fweights);
         }
      }
      for(i=1;i<data->flimit; i++)
      {
         if(data->fweights[i] == 0)
         {
            data->fweights[i] = SigIsPredicate(data->ocb->sig, i)?data->pweight:
               (SigFindArity(data->ocb->sig,i)?data->fweight:data->cweight);
         }
         else
         {
            data->fweights[i] = SigIsPredicate(data->ocb->sig, i)?data->conj_pweight:
               (SigFindArity(data->ocb->sig,i)?data->conj_fweight:data->conj_cweight);
         }
      }
   }
}

/*-----------------------------------------------------------------------
//
// Function: init_conj_t_vector()
//
//   Initialize the function weight vector based on the data in data
//   ;-). Factored out so it can be called from the weight
//   function(s). NB: Does not consider occurences of symbols themselves
//   but the occurence of symbol's type. data->type_freqs stays NULL!
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void init_conj_t_vector(FunWeightParam_p data)
{
   if(!data->fweights)
   {
      FunCode i;
      Clause_p handle;
      Sig_p   sig = data->ocb->sig;

      data->flimit   = data->ocb->sig->f_count+1;
      data->fweights = SizeMalloc(data->flimit*sizeof(long));
      data->type_freqs = SizeMalloc((sig->type_bank->types_count+1)
                                         *sizeof(long));

      for(i=0;i<data->flimit; i++)
      {
         data->fweights[i] = 0;
      }
      for(i=0;i<data->ocb->sig->type_bank->types_count+1;i++)
      {
         data->type_freqs[i] = 0;
      }

      for(handle=data->axioms->anchor->succ;
          handle!=data->axioms->anchor;
          handle = handle->succ)
      {
         if(ClauseQueryTPTPType(handle)==CPTypeNegConjecture)
         {
            ClauseAddTypeDistribution(handle, data->type_freqs);
         }
      }

      for(i=1;i<data->flimit; i++)
      {
         TypeUniqueID type_uid = SigGetType(sig, i) ? (SigGetType(sig, i))->type_uid : 0;
         if(!data->type_freqs[type_uid])
         {
            data->fweights[i] = SigIsPredicate(data->ocb->sig, i)?data->pweight:
               (SigFindArity(data->ocb->sig,i)?data->fweight:data->cweight);
         }
         else
         {
            data->fweights[i] = SigIsPredicate(data->ocb->sig, i)?data->conj_pweight:
               (SigFindArity(data->ocb->sig,i)?data->conj_fweight:data->conj_cweight);
         }
      }

      for(i=0;i<data->ocb->sig->type_bank->types_count+1;i++)
      {
         data->type_freqs[i] = data->type_freqs[i]>0 ? data->vweight : 2*data->vweight;
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: init_conj_typeweight_vector()
//
//   Initialize the function weight vector based on the data in data
//   ;-). Factored out so it can be called from the weight
//   function(s). Initializes function symbol weights to be equal
//   to the inverse of occurence of symbol's type  + 
//   2*occurence of symbol in the conjecture(s). 
//   Leaves type data in the data->type_freqs.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void init_conj_typeweight_vector(FunWeightParam_p data)
{
   if(!data->fweights)
   {
      FunCode i;
      Clause_p handle;
      Sig_p   sig = data->ocb->sig;

      data->flimit   = data->ocb->sig->f_count+1;
      data->fweights = SizeMalloc(data->flimit*sizeof(long));
      data->type_freqs  = SizeMalloc((sig->type_bank->types_count+1)
                                     *sizeof(long));

      for(i=0;i<data->flimit; i++)
      {
         data->fweights[i] = 0;
      }
      for(i=0;i<data->ocb->sig->type_bank->types_count+1;i++)
      {
         data->type_freqs[i] = 0;
      }

      for(handle=data->axioms->anchor->succ;
          handle!=data->axioms->anchor;
          handle = handle->succ)
      {
         if(ClauseQueryTPTPType(handle)==CPTypeNegConjecture)
         {
            ClauseAddTypeDistribution(handle, data->type_freqs);
            ClauseAddSymbolDistribution(handle, data->fweights);
         }
      }

      long max_occurrence = 0;
      for(i=1;i<data->flimit; i++)
      {
         TypeUniqueID type_uid = SigGetType(sig, i) ? (SigGetType(sig, i))->type_uid : 0;
         max_occurrence = MAX(max_occurrence, data->type_freqs[type_uid] + 2*data->fweights[i]); 
      }
      max_occurrence++;

      for(i=1;i<data->flimit; i++)
      {
         TypeUniqueID type_uid = SigGetType(sig, i) ? (SigGetType(sig, i))->type_uid : 0;
         if(!data->type_freqs[type_uid])
         {
            data->fweights[i] = 5*max_occurrence;
         }
         else
         {
            data->fweights[i] = max_occurrence - (data->type_freqs[type_uid] + 2*data->fweights[i]);
         }
      }
      for(i=0;i<data->ocb->sig->type_bank->types_count+1;i++)
      {
         // app vars are going to use this array 
         data->type_freqs[i] = max_occurrence - data->type_freqs[i];
      }
   }
}

/*-----------------------------------------------------------------------
//
// Function: init_relevance_vector2()
//
//   Initialize the function weight vector based on the data in data
//   ;-). Uses relevance levels.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void init_relevance_vector2(FunWeightParam_p data)
{
   if(!data->fweights)
   {
      Relevance_p reldata = RelevanceDataCompute(data->proofstate);
      FunCode i;
      long eff_rel, base;

      data->flimit   = data->ocb->sig->f_count+1;
      data->fweights = SizeMalloc(data->flimit*sizeof(long));

      for(i=1;i<data->flimit; i++)
      {
         if(SigIsSpecial(data->ocb->sig, i))
         {
            eff_rel = 1;
         }
         else
         {
            eff_rel = PDArrayElementInt(reldata->fcode_relevance, i);
         }

         if(!eff_rel)
         {
            eff_rel = data->default_level_penalty+reldata->max_level;
         }
         base = SigIsPredicate(data->ocb->sig, i)?data->pweight:
            (SigFindArity(data->ocb->sig,i)?data->fweight:data->cweight);

         data->fweights[i] = base*
            (data->level_poly_const
             +data->level_poly_lin*eff_rel
             +data->level_poly_square*eff_rel*eff_rel);
      }
      RelevanceFree(reldata);
   }
}


/*-----------------------------------------------------------------------
//
// Function: init_relevance_vector()
//
//   Initialize the function weight vector based on the data in data
//   ;-). Uses relevance levels.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void init_relevance_vector(FunWeightParam_p data)
{
   if(!data->fweights)
   {
      Relevance_p reldata = RelevanceDataCompute(data->proofstate);
      FunCode i;
      long eff_rel, base;

      data->flimit   = data->ocb->sig->f_count+1;
      data->fweights = SizeMalloc(data->flimit*sizeof(long));

      for(i=1;i<data->flimit; i++)
      {
         eff_rel = PDArrayElementInt(reldata->fcode_relevance, i);
         if(!eff_rel)
         {
            eff_rel = data->default_level_penalty+reldata->max_level;
         }
         base = SigIsPredicate(data->ocb->sig, i)?data->pweight:
            (SigFindArity(data->ocb->sig,i)?data->fweight:data->cweight);

         data->fweights[i] = base*
            (data->level_poly_const
             +data->level_poly_lin*eff_rel
             +data->level_poly_square*eff_rel*eff_rel);
      }
      RelevanceFree(reldata);
   }
}



/*-----------------------------------------------------------------------
//
// Function: init_fun_weights.
//
//   Initialize the function weight vector based on the data in
//   data. Symbols named in data->weight_stack will get the assigned
//   weight, the rest will get data->fweight.
//
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void init_fun_weights(FunWeightParam_p data)
{
   if(!data->fweights)
   {
      FunCode i;
      PStackPointer sp;
      char* fun;

      data->flimit   = data->ocb->sig->f_count+1;
      data->fweights = SizeMalloc(data->flimit*sizeof(long));

      for(i=1;i<data->flimit; i++)
      {
         data->fweights[i] = data->fweight;
      }
      for(sp=0; sp<PStackGetSP(data->weight_stack); sp+=2)
      {
         fun = PStackElementP(data->weight_stack, sp);
         i = SigFindFCode(data->ocb->sig, fun);
         if(i)
         {
            assert(i<data->flimit);
            data->fweights[i] = PStackElementInt(data->weight_stack, sp+1);
         }
         else
         {
            DStr_p msg = DStrAlloc();

            DStrAppendStr(msg, "Cannot assign weight to unknown symbol ");
            DStrAppendStr(msg, fun);
            Warning(DStrView(msg));
            DStrFree(msg);
         }
      }
   }
}

/*-----------------------------------------------------------------------
//
// Function: parse_op_weight()
//
//   Parse a tuple fun:weight and push it onto the result stack.
//
// Global Variables: -
//
// Side Effects    : Allocates a string copy, which is placed on the
//                   stack and becomes the responsibility of the
//                   caller.
//
/----------------------------------------------------------------------*/

static void parse_op_weight(Scanner_p in, PStack_p res_stack)
{
   DStr_p op = DStrAlloc();
   long   weight;

   TermParseOperator(in, op);
   AcceptInpTok(in, Colon);
   weight = AktToken(in)->numval;
   AcceptInpTok(in, PosInt);
   PStackPushP(res_stack, DStrCopy(op));
   PStackPushInt(res_stack, weight);

   DStrFree(op);
}


/*-----------------------------------------------------------------------
//
// Function: parse_op_signweight()
//
//   Parse a tuple fun:weight and push it onto the result stack.
//
// Global Variables: -
//
// Side Effects    : Allocates a string copy, which is placed on the
//                   stack and becomes the responsibility of the
//                   caller.
//
/----------------------------------------------------------------------*/

void parse_op_signweight(Scanner_p in, PStack_p res_stack)
{
   DStr_p op = DStrAlloc();
   long   weight;

   TermParseOperator(in, op);
   AcceptInpTok(in, Colon);
   weight = ParseInt(in);
   PStackPushP(res_stack, DStrCopy(op));
   PStackPushInt(res_stack, weight);

   DStrFree(op);
}




/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: FunWeightParamAlloc()
//
//   Return an FunWeightParamCell where the pointer-related members
//   are properly initialized.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

FunWeightParam_p FunWeightParamAlloc(void)
{
   FunWeightParam_p res = FunWeightParamCellAlloc();

   res->weight_stack = NULL;
   res->fweights     = NULL;
   res->flimit       = 0;
   res->f_occur      = NULL;
   res->app_var_mult = 0;
   res->type_freqs   = NULL;

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: FunWeightParamFree()
//
//   Free a initialized FunWeightParamCell, including the data stored
//   on the weight_stack (if any).
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void FunWeightParamFree(FunWeightParam_p junk)
{
   char* cjunk;

   if(junk->fweights)
   {
      assert(junk->flimit > 0);
      SizeFree(junk->fweights, sizeof(long)*junk->flimit);
   }
   if(junk->weight_stack)
   {
      while(!PStackEmpty(junk->weight_stack))
      {
         (void)PStackPopInt(junk->weight_stack);
         assert(!PStackEmpty(junk->weight_stack));
         cjunk = PStackPopP(junk->weight_stack);
         FREE(cjunk);
      }
      PStackFree(junk->weight_stack);
   }
   if(junk->f_occur)
   {
      PDArrayFree(junk->f_occur);
   }
   if(junk->type_freqs)
   {
      SizeFree(junk->type_freqs, 
               (junk->ocb->sig->type_bank->types_count+1)*sizeof(long));
   }
   FunWeightParamCellFree(junk);
}



/*-----------------------------------------------------------------------
//
// Function: ConjectureSymbolWeightInit()
//
//   Return an initialized WFCB for FunWeight evaluation. This
//   gives different weights to conjecture predicates/function
//   symbols, and non-conjecture predicate/function symbols.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

WFCB_p ConjectureSymbolWeightInit(ClausePrioFun prio_fun,
                                  OCB_p ocb,
                                  ClauseSet_p axioms,
                                  double max_term_multiplier,
                                  double max_literal_multiplier,
                                  double pos_multiplier,
                                  long   vweight,
                                  long   fweight,
                                  long   cweight,
                                  long   pweight,
                                  long   conj_fweight,
                                  long   conj_cweight,
                                  long   conj_pweight,
                                  double app_var_mult,
                                  void   (*init_fun)(struct funweightparamcell*))
{
   FunWeightParam_p data = FunWeightParamAlloc();

   data->init_fun               = init_fun;
   data->ocb                    = ocb;
   data->axioms                 = axioms;
   data->pos_multiplier         = pos_multiplier;
   data->max_term_multiplier    = max_term_multiplier;
   data->max_literal_multiplier = max_literal_multiplier;

   data->vweight                = vweight;

   data->fweight                = fweight;
   data->cweight                = cweight;
   data->pweight                = pweight;

   data->conj_fweight           = conj_fweight;
   data->conj_cweight           = conj_cweight;
   data->conj_pweight           = conj_pweight;

   data->app_var_mult        = app_var_mult;

   /* Weight vector is computed on first call of weight function to
      avoid overhead is many funweigh-based functions are predefined
      */
   return WFCBAlloc(GenericFunWeightCompute, prio_fun,
                    GenericFunWeightExit, data);
}




/*-----------------------------------------------------------------------
//
// Function: RelevanceLevelWeightInit()
//
//   Return an initialized WFCB for FunWeight evaluation. This
//   gives different weights based on the relevancy level.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

WFCB_p RelevanceLevelWeightInit(ClausePrioFun prio_fun,
                                OCB_p ocb,
                                ProofState_p state,
                                double max_term_multiplier,
                                double max_literal_multiplier,
                                double pos_multiplier,
                                long   vweight,
                                long   fweight,
                                long   cweight,
                                long   pweight,
                                long   level_poly_const,
                                double level_poly_lin,
                                double level_poly_square,
                                long   default_level_penalty,
                                double app_var_mult)

{
   FunWeightParam_p data = FunWeightParamAlloc();

   data->init_fun               = init_relevance_vector;
   data->ocb                    = ocb;
   data->proofstate             = state;
   data->pos_multiplier         = pos_multiplier;
   data->max_term_multiplier    = max_term_multiplier;
   data->max_literal_multiplier = max_literal_multiplier;

   data->vweight                = vweight;

   data->fweight                = fweight;
   data->cweight                = cweight;
   data->pweight                = pweight;

   data->level_poly_const       = level_poly_const;
   data->level_poly_lin         = level_poly_lin;
   data->level_poly_square      = level_poly_square;
   data->default_level_penalty  = default_level_penalty;

   data->app_var_mult        = app_var_mult;

   /* Weight vector is computed on first call of weight function to
      avoid overhead is many funweigh-based functions are predefined
      */
   return WFCBAlloc(GenericFunWeightCompute, prio_fun,
                    GenericFunWeightExit, data);
}

/*-----------------------------------------------------------------------
//
// Function: RelevanceLevelWeightInit2()
//
//   Return an initialized WFCB for FunWeight evaluation. This
//   gives different weights based on the relevancy level.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

WFCB_p RelevanceLevelWeightInit2(ClausePrioFun prio_fun,
                                 OCB_p ocb,
                                 ProofState_p state,
                                 double max_term_multiplier,
                                double max_literal_multiplier,
                                 double pos_multiplier,
                                 long   vweight,
                                 long   fweight,
                                 long   cweight,
                                 long   pweight,
                                 long   level_poly_const,
                                 double level_poly_lin,
                                 double level_poly_square,
                                 long   default_level_penalty,
                                 double app_var_mult)
{
   FunWeightParam_p data = FunWeightParamAlloc();

   data->init_fun               = init_relevance_vector2;
   data->ocb                    = ocb;
   data->proofstate             = state;
   data->pos_multiplier         = pos_multiplier;
   data->max_term_multiplier    = max_term_multiplier;
   data->max_literal_multiplier = max_literal_multiplier;

   data->vweight                = vweight;

   data->fweight                = fweight;
   data->cweight                = cweight;
   data->pweight                = pweight;

   data->level_poly_const       = level_poly_const;
   data->level_poly_lin         = level_poly_lin;
   data->level_poly_square      = level_poly_square;
   data->default_level_penalty  = default_level_penalty;

   data->app_var_mult        = app_var_mult;
   /* Weight vector is computed on first call of weight function to
      avoid overhead if many funweigh-based functions are predefined
      */
   return WFCBAlloc(GenericFunWeightCompute, prio_fun,
                    GenericFunWeightExit, data);
}






/*-----------------------------------------------------------------------
//
// Function: ConjectureSymbolWeightParse()
//
//   Parse a funweight-weight function giving different weights to
//   conjecture symbols and other symbols.
//
// Global Variables: -
//
// Side Effects    : Via ConjectureFunWeightInit, I/O.
//
/----------------------------------------------------------------------*/

WFCB_p ConjectureSymbolWeightParse(Scanner_p in, OCB_p ocb, ProofState_p
                                state)
{
   ClausePrioFun prio_fun;
   int vweight, fweight, pweight, cweight, conj_fweight, conj_pweight, conj_cweight;
   double pos_multiplier, max_term_multiplier, max_literal_multiplier,
          app_var_mult = APP_VAR_MULT_DEFAULT;

   AcceptInpTok(in, OpenBracket);
   prio_fun = ParsePrioFun(in);
   AcceptInpTok(in, Comma);

   fweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   cweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   pweight = ParseInt(in);
   AcceptInpTok(in, Comma);

   conj_fweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   conj_cweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   conj_pweight = ParseInt(in);
   AcceptInpTok(in, Comma);

   vweight = ParseInt(in);
   AcceptInpTok(in, Comma);

   max_term_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   max_literal_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   pos_multiplier = ParseFloat(in);

   PARSE_OPTIONAL_AV_PENALTY(in, app_var_mult);

   AcceptInpTok(in, CloseBracket);

   return ConjectureSymbolWeightInit(prio_fun,
                                     ocb,
                                     state->axioms,
                                     max_term_multiplier,
                                     max_literal_multiplier,
                                     pos_multiplier,
                                     vweight,
                                     fweight,
                                     cweight,
                                     pweight,
                                     conj_fweight,
                                     conj_cweight,
                                     conj_pweight,
                                     app_var_mult,
                                     init_conj_vector);
}

/*-----------------------------------------------------------------------
//
// Function: ConjectureSimplifiedSymbolWeightParse()
//
//   Parse a funweight-weight function giving different weights to
//   conjecture symbols and other symbols. Does not special-case
//   constants.
//
// Global Variables: -
//
// Side Effects    : Via ConjectureFunWeightInit, I/O.
//
/----------------------------------------------------------------------*/

WFCB_p ConjectureSimplifiedSymbolWeightParse(Scanner_p in, OCB_p ocb,
                                             ProofState_p state)
{
   ClausePrioFun prio_fun;
   int vweight, fweight, pweight, conj_fweight, conj_pweight;
   double pos_multiplier, max_term_multiplier, max_literal_multiplier,
          app_var_mult = APP_VAR_MULT_DEFAULT;

   AcceptInpTok(in, OpenBracket);
   prio_fun = ParsePrioFun(in);
   AcceptInpTok(in, Comma);

   fweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   pweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   conj_fweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   conj_pweight = ParseInt(in);
   AcceptInpTok(in, Comma);

   vweight = ParseInt(in);
   AcceptInpTok(in, Comma);

   max_term_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   max_literal_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   pos_multiplier = ParseFloat(in);

   PARSE_OPTIONAL_AV_PENALTY(in, app_var_mult);

   AcceptInpTok(in, CloseBracket);

   return ConjectureSymbolWeightInit(prio_fun,
                                     ocb,
                                     state->axioms,
                                     max_term_multiplier,
                                     max_literal_multiplier,
                                     pos_multiplier,
                                     vweight,
                                     fweight,
                                     fweight,
                                     pweight,
                                     conj_fweight,
                                     conj_fweight,
                                     conj_pweight,
                                     app_var_mult,
                                     init_conj_vector);
}


/*-----------------------------------------------------------------------
//
// Function: ConjectureRelativeSymbolWeightParse()
//
//   As above, but give the weight of conjecture symbols as a
//   multiple of non-conjecture symbols weight. Note that all weights
//   are rounded down to the next integer!
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

WFCB_p ConjectureRelativeSymbolWeightParse(Scanner_p in, OCB_p ocb,
                                           ProofState_p state)
{
   ClausePrioFun prio_fun;
   int fweight, pweight, cweight, vweight;
   double conj_multiplier, pos_multiplier, max_term_multiplier, max_literal_multiplier,
          app_var_mult = APP_VAR_MULT_DEFAULT;

   AcceptInpTok(in, OpenBracket);
   prio_fun = ParsePrioFun(in);
   AcceptInpTok(in, Comma);

   conj_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);

   fweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   cweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   pweight = ParseInt(in);
   AcceptInpTok(in, Comma);

   vweight = ParseInt(in);
   AcceptInpTok(in, Comma);

   max_term_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   max_literal_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   pos_multiplier = ParseFloat(in);

   PARSE_OPTIONAL_AV_PENALTY(in, app_var_mult);

   AcceptInpTok(in, CloseBracket);

   return ConjectureSymbolWeightInit(prio_fun,
                                     ocb,
                                     state->axioms,
                                     max_term_multiplier,
                                     max_literal_multiplier,
                                     pos_multiplier,
                                     vweight,
                                     fweight,
                                     cweight,
                                     pweight,
                                     conj_multiplier*fweight,
                                     conj_multiplier*cweight,
                                     conj_multiplier*pweight,
                                     app_var_mult,
                                     init_conj_vector);
}


/*-----------------------------------------------------------------------
//
// Function: ConjectureTypeBasedWeightParse()
//
//   Assign each function symbol the weight equal to occurence of the
//   symbol's type in conjecture + 2*symbols occurence in conjecture
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

WFCB_p ConjectureTypeBasedWeightParse(Scanner_p in, OCB_p ocb, ProofState_p
                                      state)
{
   ClausePrioFun prio_fun;
   int vweight;
   double pos_multiplier, max_term_multiplier, max_literal_multiplier,
          app_var_mult = APP_VAR_MULT_DEFAULT;

   AcceptInpTok(in, OpenBracket);
   prio_fun = ParsePrioFun(in);
   AcceptInpTok(in, Comma);

   vweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   
   max_term_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   max_literal_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   pos_multiplier = ParseFloat(in);

   PARSE_OPTIONAL_AV_PENALTY(in, app_var_mult);

   AcceptInpTok(in, CloseBracket);

   return ConjectureSymbolWeightInit(prio_fun, ocb, state->axioms,
                                     max_term_multiplier, max_literal_multiplier, pos_multiplier,
                                     vweight, 0, 0, 0, 0, 0, 0, app_var_mult,
                                     init_conj_typeweight_vector);
}

/*-----------------------------------------------------------------------
//
// Function: ConjectureRelativeSymbolTypeWeightParse()
//
//   As above, but give the weight of conjecture symbols as a
//   multiple of non-conjecture symbols weight. Note that all weights
//   are rounded down to the next integer! NOTE: Symbol is considered
//   a conjecture symbol if a symbol of the same type appears in the 
//   conjecture -- difference from above functions. 
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

WFCB_p ConjectureRelativeSymbolTypeWeightParse(Scanner_p in, OCB_p ocb,
                                           ProofState_p state)
{
   ClausePrioFun prio_fun;
   int fweight, pweight, cweight, vweight;
   double conj_multiplier, pos_multiplier, max_term_multiplier, max_literal_multiplier,
          app_var_mult = APP_VAR_MULT_DEFAULT;

   AcceptInpTok(in, OpenBracket);
   prio_fun = ParsePrioFun(in);
   AcceptInpTok(in, Comma);

   conj_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);

   fweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   cweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   pweight = ParseInt(in);
   AcceptInpTok(in, Comma);

   vweight = ParseInt(in);
   AcceptInpTok(in, Comma);

   max_term_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   max_literal_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   pos_multiplier = ParseFloat(in);

   PARSE_OPTIONAL_AV_PENALTY(in, app_var_mult);

   AcceptInpTok(in, CloseBracket);

   return ConjectureSymbolWeightInit(prio_fun,
                                     ocb,
                                     state->axioms,
                                     max_term_multiplier,
                                     max_literal_multiplier,
                                     pos_multiplier,
                                     vweight,
                                     fweight,
                                     cweight,
                                     pweight,
                                     conj_multiplier*fweight,
                                     conj_multiplier*cweight,
                                     conj_multiplier*pweight,
                                     app_var_mult,
                                     init_conj_t_vector);
}



/*-----------------------------------------------------------------------
//
// Function: RelevanceLevelWeightParse()
//
//   Parse the specification of a RelevanceLevelWeight function.
//
//   The parameters are:
//
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/


WFCB_p RelevanceLevelWeightParse(Scanner_p in, OCB_p ocb,
                                 ProofState_p state)
{
   ClausePrioFun prio_fun;
   int
      fweight,
      pweight,
      cweight,
      vweight,
      default_level_penalty;

   double
      max_term_multiplier,
      max_literal_multiplier,
      pos_multiplier,
      level_poly_const,
      level_poly_lin,
      level_poly_square,
      app_var_mult = APP_VAR_MULT_DEFAULT;

   AcceptInpTok(in, OpenBracket);
   prio_fun = ParsePrioFun(in);
   AcceptInpTok(in, Comma);

   level_poly_const = ParseFloat(in);
   AcceptInpTok(in, Comma);
   level_poly_lin = ParseFloat(in);
   AcceptInpTok(in, Comma);
   level_poly_square = ParseFloat(in);
   AcceptInpTok(in, Comma);

   default_level_penalty = ParseInt(in);
   AcceptInpTok(in, Comma);

   fweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   cweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   pweight = ParseInt(in);
   AcceptInpTok(in, Comma);

   vweight = ParseInt(in);
   AcceptInpTok(in, Comma);

   max_term_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   max_literal_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   pos_multiplier = ParseFloat(in);

   PARSE_OPTIONAL_AV_PENALTY(in, app_var_mult);

   AcceptInpTok(in, CloseBracket);

   return RelevanceLevelWeightInit(prio_fun,
                                   ocb,
                                   state,
                                   max_term_multiplier,
                                   max_literal_multiplier,
                                   pos_multiplier,
                                   vweight,
                                   fweight,
                                   cweight,
                                   pweight,
                                   level_poly_const,
                                   level_poly_lin,
                                   level_poly_square,
                                   default_level_penalty,
                                   app_var_mult);
}


/*-----------------------------------------------------------------------
//
// Function: RelevanceLevelWeightParse2()
//
//   Parse the specification of a RelevanceLevelWeight function.
//
//   The parameters are:
//
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/


WFCB_p RelevanceLevelWeightParse2(Scanner_p in, OCB_p ocb,
                                 ProofState_p state)
{
   ClausePrioFun prio_fun;
   int
      fweight,
      pweight,
      cweight,
      vweight,
      default_level_penalty;

   double
      max_term_multiplier,
      max_literal_multiplier,
      pos_multiplier,
      level_poly_const,
      level_poly_lin,
      level_poly_square,
      app_var_mult = APP_VAR_MULT_DEFAULT;

   AcceptInpTok(in, OpenBracket);
   prio_fun = ParsePrioFun(in);
   AcceptInpTok(in, Comma);

   level_poly_const = ParseFloat(in);
   AcceptInpTok(in, Comma);
   level_poly_lin = ParseFloat(in);
   AcceptInpTok(in, Comma);
   level_poly_square = ParseFloat(in);
   AcceptInpTok(in, Comma);

   default_level_penalty = ParseInt(in);
   AcceptInpTok(in, Comma);

   fweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   cweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   pweight = ParseInt(in);
   AcceptInpTok(in, Comma);

   vweight = ParseInt(in);
   AcceptInpTok(in, Comma);

   max_term_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   max_literal_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   pos_multiplier = ParseFloat(in);

   PARSE_OPTIONAL_AV_PENALTY(in, app_var_mult);

   AcceptInpTok(in, CloseBracket);

   return RelevanceLevelWeightInit2(prio_fun,
                                   ocb,
                                   state,
                                   max_term_multiplier,
                                   max_literal_multiplier,
                                   pos_multiplier,
                                   vweight,
                                   fweight,
                                   cweight,
                                   pweight,
                                   level_poly_const,
                                   level_poly_lin,
                                   level_poly_square,
                                   default_level_penalty,
                                   app_var_mult);
}


/*-----------------------------------------------------------------------
//
// Function: FunWeightInit()
//
//   Initialize a weight function with explicit weights for (some)
//   function symbols.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

WFCB_p FunWeightInit(ClausePrioFun prio_fun,
                     OCB_p ocb,
                     double max_term_multiplier,
                     double max_literal_multiplier,
                     double pos_multiplier,
                     long vweight,
                     long fweight,
                     PStack_p fweights,
                     double app_var_mult)
{
   FunWeightParam_p data = FunWeightParamAlloc();

   data->init_fun               = init_fun_weights;
   data->ocb                    = ocb;
   data->pos_multiplier         = pos_multiplier;
   data->max_term_multiplier    = max_term_multiplier;
   data->max_literal_multiplier = max_literal_multiplier;

   data->vweight                = vweight;

   data->fweight                = fweight;
   data->weight_stack           = fweights;

   data->app_var_mult        = app_var_mult;

   return WFCBAlloc(GenericFunWeightCompute, prio_fun,
                    GenericFunWeightExit, data);

}

/*-----------------------------------------------------------------------
//
// Function: FunWeightParse()
//
//   Parse a FunWeight evaluation function.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

WFCB_p FunWeightParse(Scanner_p in, OCB_p ocb,
                     ProofState_p state)
{
   ClausePrioFun
      prio_fun;
   int
      vweight,
      fweight;
   double
      max_term_multiplier,
      max_literal_multiplier,
      pos_multiplier,
      app_var_mult = APP_VAR_MULT_DEFAULT;
   PStack_p fweights;

   AcceptInpTok(in, OpenBracket);
   prio_fun = ParsePrioFun(in);
   AcceptInpTok(in, Comma);

   fweight = ParseInt(in);
   AcceptInpTok(in, Comma);

   vweight = ParseInt(in);
   AcceptInpTok(in, Comma);

   max_term_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   max_literal_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   pos_multiplier = ParseFloat(in);

   fweights = PStackAlloc();

   while(TestInpTok(in, Comma))
   {
      AcceptInpTok(in, Comma);
      parse_op_weight(in, fweights);
   }

   PARSE_OPTIONAL_AV_PENALTY(in, app_var_mult);

   AcceptInpTok(in, CloseBracket);

   return FunWeightInit(prio_fun,
                        ocb,
                        max_term_multiplier,
                        max_literal_multiplier,
                        pos_multiplier,
                        vweight,
                        fweight,
                        fweights,
                        app_var_mult);
}



/*-----------------------------------------------------------------------
//
// Function: SymOffsetWeightInit()
//
//   Initialize a weight function with explicit offsets for (some)
//   function symbols.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

WFCB_p SymOffsetWeightInit(ClausePrioFun prio_fun,
                           OCB_p ocb,
                           double max_term_multiplier,
                           double max_literal_multiplier,
                           double pos_multiplier,
                           long vweight,
                           long fweight,
                           PStack_p fweights,
                           double app_var_mult)
{
   FunWeightParam_p data = FunWeightParamAlloc();

   data->init_fun               = init_fun_weights;
   data->ocb                    = ocb;
   data->pos_multiplier         = pos_multiplier;
   data->max_term_multiplier    = max_term_multiplier;
   data->max_literal_multiplier = max_literal_multiplier;

   data->vweight                = vweight;

   data->fweight                = fweight;
   data->weight_stack           = fweights;
   data->f_occur                = PDIntArrayAlloc(8, 0);

   data->app_var_mult        = app_var_mult;

   return WFCBAlloc(SymOffsetWeightCompute, prio_fun,
                    GenericFunWeightExit, data);

}



/*-----------------------------------------------------------------------
//
// Function: SymOffsetWeightParse()
//
//   Parse a FunWeight evaluation function.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

WFCB_p SymOffsetWeightParse(Scanner_p in, OCB_p ocb,
                            ProofState_p state)
{
   ClausePrioFun
      prio_fun;
   int
      vweight,
      fweight;
   double
      max_term_multiplier,
      max_literal_multiplier,
      pos_multiplier,
      app_var_mult = APP_VAR_MULT_DEFAULT;
   PStack_p fweights;

   AcceptInpTok(in, OpenBracket);
   prio_fun = ParsePrioFun(in);
   AcceptInpTok(in, Comma);

   fweight = ParseInt(in);
   AcceptInpTok(in, Comma);

   vweight = ParseInt(in);
   AcceptInpTok(in, Comma);

   max_term_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   max_literal_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   pos_multiplier = ParseFloat(in);

   fweights = PStackAlloc();

   while(TestInpTok(in, Comma))
   {
      AcceptInpTok(in, Comma);
      parse_op_signweight(in, fweights);
   }

   PARSE_OPTIONAL_AV_PENALTY(in, app_var_mult);

   AcceptInpTok(in, CloseBracket);

   return SymOffsetWeightInit(prio_fun,
                              ocb,
                              max_term_multiplier,
                              max_literal_multiplier,
                              pos_multiplier,
                              vweight,
                              fweight,
                              fweights,
                              app_var_mult);
}



/*-----------------------------------------------------------------------
//
// Function: GenericFunWeightCompute()
//
//   Compute a clause weight as Refinedweight(), but use the function
//   symbol weights in data->fweights for individual values.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double GenericFunWeightCompute(void* data, Clause_p clause)
{
   FunWeightParam_p local = data;

   local->init_fun(data);
   ClauseCondMarkMaximalTerms(local->ocb, clause);
   return ClauseFunWeight(clause,
                          local->max_term_multiplier,
                          local->max_literal_multiplier,
                          local->pos_multiplier,
                          local->vweight,
                          local->flimit,
                          local->fweights,
                          local->fweight,
                          local->app_var_mult,
                          local->type_freqs);
}


/*-----------------------------------------------------------------------
//
// Function: SymOffsetWeightCompute()
//
//   Compute a clause weight as Refinedweight(), but use the function
//   symbol weights in data->fweights to compute an extra per-symbol
//   (not per symbol occurrence!) offset.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double SymOffsetWeightCompute(void* data, Clause_p clause)
{
   FunWeightParam_p local = data;
   double   res;
   long     index;
   long     woffset;
   PStack_p res_stack;

   local->init_fun(data);
   ClauseCondMarkMaximalTerms(local->ocb, clause);
   res = ClauseWeight(clause,
                      local->max_term_multiplier,
                      local->max_literal_multiplier,
                      local->pos_multiplier,
                      local->vweight,
                      local->fweight,
                      local->app_var_mult,
                      false);
   res_stack = PStackAlloc();
   ClauseAddFunOccs(clause, local->f_occur, res_stack);
   while(!PStackEmpty(res_stack))
   {
      index   = PStackPopInt(res_stack);
      woffset = index < local->flimit?
         local->fweights[index]:local->fweight;
      res += woffset;
      PDArrayAssignInt(local->f_occur, index, 0);
   }
   PStackFree(res_stack);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: GenericFunWeightExit()
//
//   Free an FunWeightParamCell, including the optional weight array.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void GenericFunWeightExit(void* data)
{
   FunWeightParam_p junk = data;

   FunWeightParamFree(junk);
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


