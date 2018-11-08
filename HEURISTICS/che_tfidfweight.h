/*-----------------------------------------------------------------------

File  : che_tfidfweight.h

Author: could be anyone

Contents
 
  Auto generated. Your comment goes here ;-).

  Copyright 2016 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Tue Mar  8 22:40:31 CET 2016
    New

-----------------------------------------------------------------------*/

#ifndef CHE_TFIDFWEIGHT

#define CHE_TFIDFWEIGHT

#include <che_termweight.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct tfidfweightparamcell
{
   OCB_p        ocb;
   ProofState_p proofstate;

   VarNormStyle var_norm;
   RelatedTermSet rel_terms;

   TB_p eval_bank;
   NumTree_p eval_freqs;
   PDTree_p documents;
   int update_docs;
   double tf_fact;

   TermWeightExtension_p twe;
   void   (*init_fun)(struct tfidfweightparamcell*);
}TfIdfWeightParamCell, *TfIdfWeightParam_p;

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define TfIdfWeightParamCellAlloc() (TfIdfWeightParamCell*) \
        SizeMalloc(sizeof(TfIdfWeightParamCell))
#define TfIdfWeightParamCellFree(junk) \
        SizeFree(junk, sizeof(TfIdfWeightParamCell))

TfIdfWeightParam_p TfIdfWeightParamAlloc(void);
void              TfIdfWeightParamFree(TfIdfWeightParam_p junk);


WFCB_p ConjectureTermTfIdfWeightParse(
   Scanner_p in, 
   OCB_p ocb, 
   ProofState_p state);

WFCB_p ConjectureTermTfIdfWeightInit(
   ClausePrioFun prio_fun, 
   OCB_p ocb,
   ProofState_p proofstate,
   VarNormStyle var_norm,
   RelatedTermSet rel_terms,
   int update_docs,
   double tf_fact,
   TermWeightExtenstionStyle ext_style,
   double max_term_multiplier,
   double max_literal_multiplier,
   double pos_multiplier);

double ConjectureTermTfIdfWeightCompute(void* data, Clause_p clause);

void ConjectureTermTfIdfWeightExit(void* data);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/

