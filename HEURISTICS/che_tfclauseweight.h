/*-----------------------------------------------------------------------

File  : che_tfclauseweight.h


-----------------------------------------------------------------------*/

#ifndef CHE_TFCLAUSEWEIGHT

#define CHE_TFCLAUSEWEIGHT

#include <che_wfcb.h>

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

void TensorFlowInit(PQueue_p clauses);

WFCB_p TensorFlowWeightInit(ClausePrioFun prio_fun);

WFCB_p TensorFlowWeightParse(Scanner_p in, OCB_p ocb, ProofState_p state);

void TensorFlowWeightBatchCompute(PQueue_p batch);

double TensorFlowWeightCompute(void* data, Clause_p clause);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
