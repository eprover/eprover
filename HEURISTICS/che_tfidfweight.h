/*-----------------------------------------------------------------------

  File  : che_tfidfweight.h

  Author: Stephan Schulz, yan

  Contents
 
  Iplementation of conjecture frequency weight (TfIdf) from [CICM'16/Sec.3].
  
  Copyright 1998-2018 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Wed Nov  7 21:37:27 CET 2018

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

