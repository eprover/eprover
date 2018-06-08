/*-----------------------------------------------------------------------

  File  : pcl_expressions.h

  Author: Stephan Schulz

  Contents

  PCL2 expressions and uexpressions.

  Copyright 1998-2018 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Mon Mar 27 15:10:31 MET DST 2000

  -----------------------------------------------------------------------*/

#ifndef PCL_EXPRESSIONS

#define PCL_EXPRESSIONS

//#include <can_infstate.h>
#include <ccl_inferencedoc.h>
#include <ccl_clausesets.h>

#include <pcl_idents.h>
#include <pcl_positions.h>
#include <ccl_clauseinfo.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* If you extend this, also extend InferenceWeightParamAlloc() in
 * pcl_lemmas.c */

typedef enum
{
   PCLOpNoOp,
   PCLOpInitial,
   PCLOpIntroDef,
   PCLOpQuote,
   PCLOpEvalGC,
   PCLOpParamod,
   PCLOpSimParamod,
   PCLOpEResolution,
   PCLOpSatCheck,
   PCLOpCondense,
   PCLOpEFactoring,
   PCLOpSimplifyReflect,
   PCLOpContextSimplifyReflect,
   PCLOpACResolution,
   PCLOpRewrite,
   PCLOpURewrite,
   PCLOpClauseNormalize,
   PCLOpSplitClause,
   PCLOpSplitEquiv,
   PCLOpApplyDef,
   PCLOpFOFSplitConjunct,
   PCLOpFOFSimplify,
   PCLOpFOFDeMorgan,
   PCLOpFOFDistributeQuantors,
   PCLOpFOFDistributeDisjunction,
   PCLOpAnnotateQuestion,
   PCLOpEvalAnswers,
   PCLOpFOFVarRename,
   PCLOpFOFSkolemize,
   PCLOpFOFAssumeNegation,
   PCLOpMaxOp
}PCLOpcodes;


#define PCL_OP_NOOP_WEIGHT            0
#define PCL_OP_INITIAL_WEIGHT         1
#define PCL_OP_QUOTE_WEIGHT           0
#define PCL_OP_EVALGC_WEIGHT          0
#define PCL_OP_PARAMOD_WEIGHT         1
#define PCL_OP_SIM_PARAMOD_WEIGHT     1
#define PCL_OP_ERESOLUTION_WEIGHT     1
#define PCL_OP_CONDENSE_WEIGHT        1
#define PCL_OP_EFACTORING_WEIGHT      1
#define PCL_OP_SIMPLIFYREFLECT_WEIGHT 1
#define PCL_OP_CONTEXTSIMPLIFYREFLECT_WEIGHT 1
#define PCL_OP_ACRESOLUTION_WEIGHT    2
#define PCL_OP_REWRITE_WEIGHT         1
#define PCL_OP_UREWRITE_WEIGHT        1
#define PCL_OP_CLAUSENORMALIZE_WEIGHT 1
#define PCL_OP_SPLITCLAUSE_WEIGHT     1


typedef struct pclexprcell
{
   PCLOpcodes op;
   long       arg_no;
   PDArray_p  args; /* 2 Words per arg: argument, position */
}PCLExprCell, *PCLExpr_p;


#define PCL_VAR_ARG -1

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define PCLExprCellAlloc() (PCLExprCell*)SizeMalloc(sizeof(PCLExprCell))
#define PCLExprCellFree(junk)        SizeFree(junk, sizeof(PCLExprCell))

#define PCLExprArg(expr,i)     PDArrayElementP((expr)->args,2*(i))
#define PCLExprArgInt(expr,i)  PDArrayElementInt((expr)->args,2*(i))
#define PCLExprArgPos(expr,i)  PDArrayElementP((expr)->args,2*(i)+1)

PCLExpr_p PCLExprAlloc(void);
void      PCLExprFree(PCLExpr_p junk);

/* MiniExprs are the same basic data type. However, MiniPCL-Ids are
   just plain longs, not full PCL identifiers */
void      PCLMiniExprFree(PCLExpr_p junk);

PCLExpr_p PCLExprParse(Scanner_p in, bool mini);
#define   PCLFullExprParse(in) PCLExprParse((in),false)
#define   PCLMiniExprParse(in) PCLExprParse((in),true)

void      PCLExprPrint(FILE* out, PCLExpr_p expr, bool mini);
#define   PCLFullExprPrint(out, expr) PCLExprPrint((out),(expr),false)
#define   PCLMiniExprPrint(out, expr) PCLExprPrint((out),(expr),true)

void PCLExprPrintTSTP(FILE* out, PCLExpr_p expr, bool mini);
#define   PCLFullExprPrintTSTP(out, expr) PCLExprPrintTSTP((out),(expr),false)
#define   PCLMiniExprPrintTSTP(out, expr) PCLExprPrintTSTP((out),(expr),true)

bool      PCLStepExtract(char* extra);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
