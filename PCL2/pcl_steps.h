/*-----------------------------------------------------------------------

File  : pcl_steps.h

Author: Stephan Schulz

Contents

  PCL steps.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Thu Mar 30 17:52:53 MET DST 2000
    New

-----------------------------------------------------------------------*/

#ifndef PCL_STEPS

#define PCL_STEPS

#include <pcl_expressions.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

#define PCL_PROOF_DIST_INFINITY LONG_MAX /* It's magic */
#define PCL_PROOF_DIST_DEFAULT  10 /* Default for non-proofs */
#define PCL_PROOF_DIST_UNKNOWN  -1 /* Not yet computed */


typedef enum
{
   PCLNoProp         =  0,
   PCLIsLemma        =  1,
   PCLIsInitial      =  2,
   PCLIsFinal        =  4,
   PCLIsMarked       =  8,
   PCLIsProofStep    = 16,
   PCLIsExample      = 32, /* Selected for learning */
   PCLIsFOFStep      = 64, /* Otherwise its a clause */
   PCLIsShellStep    = 128,
   PCLType1          = CPType1, /* 256 */
   PCLType2          = CPType2,
   PCLType3          = CPType3,
   PCLTypeMask       = CPTypeMask,
   PCLTypeUnknown    = 0,                /* Also used as wildcard */
   PCLTypeAxiom      = CPTypeAxiom,      /* Formula is Axiom */
   PCLTypeHypothesis = CPTypeHypothesis, /* Formula is Hypothesis */
   PCLTypeConjecture = CPTypeConjecture, /* Formula is Conjecture */
   /* No Lemma type, in PLC that is independent of step type! */
   PCLTypeNegConjecture = CPTypeNegConjecture, /* Formula is NegConjecture */
   PCLTypeQuestion      = CPTypeQuestion, /* It's a question */

}PCLStepProperties;


typedef struct pclstepcell
{
   TB_p              bank;
   PCLId_p           id;
   union
   {
      Clause_p          clause;
      TFormula_p        formula;
   }logic;
   PCLExpr_p         just;
   char*             extra;
   PCLStepProperties properties;
   /* The following data is collected for lemma evaluation */
   long              proof_dag_size;
   long              proof_tree_size;
   long              active_pm_refs;
   long              other_generating_refs;
   long              active_simpl_refs;
   long              passive_simpl_refs;
   long              pure_quote_refs;
   float             lemma_quality;
   /* The following data is collected for pattern-based learning */
   long              contrib_simpl_refs; /* Simplification of proof
                                          * clauses -- counts active
                                          * use only! */
   long              contrib_gen_refs;   /* Generation of proof clauses */
   long              useless_simpl_refs; /* Simplification of
                                          * superfluous c.-- counts
                                          * active use only! */
   long              useless_gen_refs;   /* Generation of superfluous c. */
   long              proof_distance;
}PCLStepCell, *PCLStep_p;

#define PCLNoWeight -1


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

extern bool SupportShellPCL;

#define PCLStepCellAlloc() (PCLStepCell*)SizeMalloc(sizeof(PCLStepCell))
#define PCLStepCellFree(junk)         SizeFree(junk, sizeof(PCLStepCell))

#define PCLStepSetProp(clause, prop) SetProp((clause), (prop))
#define PCLStepDelProp(clause, prop) DelProp((clause), (prop))
#define PCLStepGiveProps(clause, prop) GiveProps((clause), (prop))
#define PCLStepQueryProp(clause, prop) QueryProp((clause), (prop))
#define PCLStepIsAnyPropSet(clause, prop) IsAnyPropSet((clause), (prop))

#define PCLStepIsFOF(step) PCLStepQueryProp((step), PCLIsFOFStep)
#define PCLStepIsShell(step) PCLStepQueryProp((step), PCLIsShellStep)
#define PCLStepIsClausal(step) (!PCLStepIsFOF(step))

void      PCLStepFree(PCLStep_p junk);

PCLStepProperties PCLParseExternalType(Scanner_p in);
PCLStep_p PCLStepParse(Scanner_p in, TB_p bank);
void      PCLPrintExternalType(FILE* out, PCLStepProperties props);
void      PCLStepPrintExtra(FILE* out, PCLStep_p step, bool data);
#define   PCLStepPrint(out, step) PCLStepPrintExtra((out),(step),false)
char *    PCLPropToTSTPType(PCLStepProperties props);
void      PCLStepPrintTSTP(FILE* out, PCLStep_p step);
void      PCLStepPrintTPTP(FILE* out, PCLStep_p step);
void      PCLStepPrintLOP(FILE* out, PCLStep_p step);
void      PCLStepPrintFormat(FILE* out, PCLStep_p step, bool data,
              OutputFormatType format);
void      PCLStepPrintExample(FILE* out, PCLStep_p step, long id,
                              long proof_steps, long total_steps);
int       PCLStepIdCompare(const void* s1, const void* s2);
void      PCLStepResetTreeData(PCLStep_p step, bool just_weights);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





