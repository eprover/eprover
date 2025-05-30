/*-----------------------------------------------------------------------

  File  : ccl_eqn.h

  Author: Stephan Schulz

  Contents

  The termpair datatype: Rules, Equations, positive and negative
  literals.

  Copyright 1998-2017 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Fri Mar 13 17:09:13 MET 1998

  -----------------------------------------------------------------------*/

#ifndef CCL_EQN

#define CCL_EQN

#include <cte_acterms.h>
#include <cte_match_mgu_1-1.h>
#include <cte_replace.h>
#include <cto_orderings.h>
#include <cte_termweightext.h>



/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef enum
{
   EPNoProps           =     0, /* No properties set or selected */
   EPIsPositive        =     1, /* s=t (as opposed to s!=t) */
   EPIsMaximal         =     2, /* Eqn is maximal in a clause */
   EPIsStrictlyMaximal =     4, /* Eqn is strictly maximal */
   EPIsEquLiteral      =     8, /* s=t, not s=$true */
   EPIsOriented        =    16, /* s=>t  or s=t ? */
   EPMaxIsUpToDate     =    32, /* Orientation status is up to date */
   EPHasEquiv          =    64, /* Literal has been used in
                                   multiset-comparison (and found an
                                   equivalent partner) */
   EPIsDominated       =   128, /* Literal is dominated by another one */
   EPDominates         =   EPIsDominated, /* Double use of this property
                                             in potentially maximal or
                                             minimal clauses */
   EPIsUsed            =   256, /* For non-injective subsumption and
                                   pattern-generation */
   EPGONatural         =   512, /* Set if left side is bigger in the
                                   special (total) ground ordering
                                   treating variables as small
                                   constants */
   EPIsSelected        =  1024, /* For selective superpostion */
   EPIsPMIntoLit       =  2048, /* For inheriting selection */
   EPFromClauseLit     =  4096, /* This comes from the from clause in
                                   a paramod step */
   EPPseudoLit         =  8192, /* This is a pseudo-literal that does
                                   not contribute to the semantic
                                   evaluation of the clause. */
   EPLPatMinimal       = 16384, /* Eqn l=r is Pattern-Minimal */
   EPRPatMinimal       = 32768, /* Eqn r=l is Pattern-Minimal */
   EPIsSplitLit        = 65636  /* This literal has been introduced by
                                   splitting */
}EqnProperties;


/* Basic data structure for rules, equations, literals. Terms are
   always assumed to be shared and need to be manipulated while taking
   care about references! */

typedef struct eqncell
{
   EqnProperties  properties;/* Positive, maximal, equational */
   int            pos;
   Term_p         lterm;
   Term_p         rterm;
   TB_p           bank;      /* Terms are from this bank */
   struct eqncell *next;     /* For lists of equations */
}EqnCell, *Eqn_p, **EqnRef;


typedef enum
{
   NoSide = 0,
   LeftSide = 1,
   MaxSide = 1,
   RightSide = 2,
   MinSide = 2,
   BothSides = 3
}EqnSide;


/* Which way to read an equation */

typedef enum
{
   PENormal  = 0,
   PEReverse =1
}PatEqnDirection;


/* What will be parsed as the equal-predicate: */

#define EQUAL_PREDICATE "equal"

#ifdef CONSTANT_MEM_ESTIMATE
#define EQN_CELL_MEM 24
#else
#define EQN_CELL_MEM   (MEMSIZE(EqnCell)+8) /* Just a hack because
                                               SPARCs seem to work
                                               like that... */
#endif


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


extern bool EqnUseInfix; /* s = t or EQ(s = t)  ? */
extern bool EqnFullEquationalRep; /* P(x) = $true ? */
extern IOFormat OutputFormat;

#define EqnCellAlloc()    (EqnCell*)SizeMalloc(sizeof(EqnCell))
#define EqnCellFree(junk) SizeFree(junk, sizeof(EqnCell))

Eqn_p   EqnAlloc(Term_p lterm, Term_p rterm, TB_p bank, bool positive);
Eqn_p   EqnAllocFlatten(Term_p lterm, TB_p bank, bool sign);
void    EqnFree(Eqn_p junk);

#define EqnCreateTrueLit(bank) (EqnAlloc((bank)->true_term, (bank)->true_term, bank, true))

#define EqnGCMarkTerms(eqn) TBGCMarkTerm((eqn)->bank,(eqn)->lterm);     \
   TBGCMarkTerm((eqn)->bank,(eqn)->rterm)

#define EqnSetProp(eqn, prop)  SetProp((eqn), (prop))
#define EqnDelProp(eqn, prop)  DelProp((eqn), (prop))
#define EqnFlipProp(eqn, prop) FlipProp((eqn), (prop))

/* Are _all_ properties in prop set in eqn? */
#define EqnQueryProp(eqn, prop) QueryProp((eqn), (prop))

/* Are any properties in prop set in eqn? */
#define EqnIsAnyPropSet(eqn, prop) IsAnyPropSet((eqn), (prop))

#define EqnIsOriented(eq) EqnQueryProp((eq), EPIsOriented)
#define EqnIsPositive(eq) EqnQueryProp((eq), EPIsPositive)
#define EqnIsNegative(eq) (!(EqnQueryProp((eq), EPIsPositive)))
#define EqnIsEquLit(eq)   (assert(EqnQueryProp((eq), EPIsEquLiteral) || (eq)->rterm == (eq)->bank->true_term),\
                           assert(!EqnQueryProp((eq), EPIsEquLiteral) || (eq)->rterm != (eq)->bank->true_term),\
                          EqnQueryProp((eq), EPIsEquLiteral))
#define EqnIsMaximal(eq)  EqnQueryProp((eq), EPIsMaximal)
#define EqnIsStrictlyMaximal(eq)                                        \
   EqnQueryProp((eq), EPIsStrictlyMaximal)

#define EqnGetPredCodeFO(eq) (EqnIsEquLit(eq)?0:(eq)->lterm->f_code)
#define EqnGetPredCodeHO(eq) (EqnIsEquLit(eq)?0:((TermIsAnyVar((eq)->lterm) || TermIsPhonyApp((eq)->lterm)) ? \
                                                  0 : (eq)->lterm->f_code))

#ifdef ENABLE_LFHO
#define EqnGetPredCode(eq) (problemType == PROBLEM_HO ? EqnGetPredCodeHO(eq) : EqnGetPredCodeFO(eq))
#else
#define EqnGetPredCode(eq) (EqnGetPredCodeFO(eq))
#endif


#define EqnIsSplitLit(eq)                       \
   (EqnIsEquLit(eq)?false:                                              \
    SigQueryFuncProp((eq)->bank->sig, EqnGetPredCode(eq), FPClSplitDef))

#define EqnHasEquiv(eq)  EqnQueryProp((eq), EPHasEquiv)
#define EqnIsDominated(eq)  EqnQueryProp((eq), EPIsDominated)
#define EqnDominates(eq)  EqnQueryProp((eq), EPDominates)
#define EqnIsSelected(eq) EqnQueryProp((eq), EPIsSelected)

#define EqnIsPropTrue(eq)  (((eq)->lterm == (eq)->rterm) && EqnIsPositive(eq))
#define EqnIsPropFalse(eq) (((eq)->lterm == (eq)->rterm) && EqnIsNegative(eq))

#define EqnIsBoolVar(eq) (TermIsFreeVar((eq)->lterm) && ((eq)->rterm == (eq)->bank->true_term))

#define EqnIsGround(eq)                                         \
   (TBTermIsGround((eq)->lterm) && TBTermIsGround((eq)->rterm))
#define EqnIsPureVar(eq)                                \
   (TermIsFreeVar((eq)->lterm) && TermIsFreeVar((eq)->rterm))
#define EqnIsPartVar(eq)                                \
   (TermIsFreeVar((eq)->lterm) || TermIsFreeVar((eq)->rterm))
#define EqnIsPropositional(eq)                          \
   ((!EqnIsEquLit(eq)) && TermIsConst((eq)->lterm))
#define EqnIsTypePred(eq)                               \
   ((!EqnIsEquLit(eq))&&TBTermIsTypeTerm((eq)->lterm))
#define EqnIsXTypePred(eq)                              \
   ((!EqnIsEquLit(eq))&&TBTermIsXTypeTerm((eq)->lterm))
#define EqnIsRealXTypePred(eq)                          \
   ((!EqnIsEquLit(eq))&&TermIsDefTerm((eq)->lterm,1))
#define EqnIsSimpleAnswer(eq)                                   \
   ((!TermIsDBVar((eq)->lterm)) && SigIsSimpleAnswerPred((eq)->bank->sig, (eq)->lterm->f_code))

#define EqnTermSetProp(eq,prop) TermSetProp((eq)->lterm, DEREF_NEVER, (prop));\
   TermSetProp((eq)->rterm, DEREF_NEVER, (prop))

#define EqnTBTermDelPropCount(eq,prop)                  \
   (TBTermDelPropCount((eq)->lterm, (prop))+            \
          TBTermDelPropCount((eq)->rterm, (prop)))
#define EqnTermDelProp(eqn, prop)                       \
   TermDelProp((eqn)->lterm, DEREF_NEVER, (prop));      \
   TermDelProp((eqn)->rterm, DEREF_NEVER, (prop))

#define EqnIsClausifiable(eq) \
   TypeIsBool((eq)->lterm->type) && \
   ((eq)->rterm != (eq)->bank->true_term || \
      (!TermIsAnyVar((eq)->lterm) && \
      SigIsLogicalSymbol((eq)->bank->sig, (eq)->lterm->f_code)))

bool    EqnParseInfix(Scanner_p in, TB_p bank, Term_p *lref, Term_p *rref);
Eqn_p   EqnParse(Scanner_p in, TB_p bank);
Eqn_p   EqnFOFParse(Scanner_p in, TB_p bank);
Eqn_p   EqnHOFParse(Scanner_p in, TB_p terms, bool *continue_parsing);
Term_p  EqnTermsTBTermEncode(TB_p bank, Term_p lterm, Term_p rterm,
                             bool positive, PatEqnDirection dir);
#define EqnTBTermEncode(eqn, dir) \
   EqnTermsTBTermEncode((eqn)->bank, (eqn)->lterm,      \
                        (eqn)->rterm, EqnIsPositive(eqn), (dir))
Eqn_p   EqnTBTermDecode(TB_p terms, Term_p eqn);
Term_p  EqnTBTermParse(Scanner_p in, TB_p bank);
void    EqnPrint(FILE* out, Eqn_p eq, bool negated, bool fullterms);
void    EqnPrintDBG(FILE* out, Eqn_p eq);
#define EqnPrintOriginal(out, eq)               \
        EqnPrint((out), (eq), normal, true)
void    EqnPrintDeref(FILE* out, Eqn_p eq, DerefType deref);

void    EqnFOFPrint(FILE* out, Eqn_p eq, bool negated, bool fullterms, bool pcl);
void    EqnTSTPPrint(FILE* out, Eqn_p eq, bool fullterms);

void    EqnSwapSidesSimple(Eqn_p eq);
void    EqnSwapSides(Eqn_p eq);
void    EqnRecordTermDates(Eqn_p eq);

Eqn_p   EqnCopy(Eqn_p eq, TB_p bank);
Eqn_p   EqnFlatCopy(Eqn_p eq);
Eqn_p   EqnCopyRepl(Eqn_p eq, TB_p bank, Term_p old, Term_p repl);
Eqn_p   EqnCopyReplPlain(Eqn_p eq, TB_p bank, Term_p old, Term_p repl);
#define EqnSkolemSubst(handle, subst, sig)                      \
   SubstSkolemizeTerm((handle)->lterm, (subst), (sig));         \
   SubstSkolemizeTerm((handle)->rterm, (subst), (sig))
Eqn_p   EqnCopyOpt(Eqn_p eq);
Eqn_p   EqnCopyDisjoint(Eqn_p eq);

#define EqnIsTrivial(eq) ((eq)->lterm == (eq)->rterm)

bool    EqnIsACTrivial(Eqn_p eq);

bool    EqnTermsAreDistinct(Eqn_p eq);
bool    EqnIsTrue(Eqn_p eq);
bool    EqnIsFalse(Eqn_p eq);

bool    EqnHasUnboundVars(Eqn_p eq, EqnSide dom_side);

EqnSide EqnIsDefinition(Eqn_p eq, int min_arity);

int     EqnSubsumeQOrderCompare(const void* lit1, const void* lit2);
int     EqnSubsumeInverseCompareRef(const void* lit1ref, const void* lit2ref);
int     EqnSubsumeInverseRefinedCompareRef(const void* lit1ref, const void* lit2ref);
int     EqnSubsumeCompare(Eqn_p l1, Eqn_p l2);

Eqn_p   EqnCanonize(Eqn_p eq);
long    EqnStructWeightCompare(Eqn_p l1, Eqn_p l2);
int     EqnCanonCompareRef(const void* lit1ref, const void* l2ref);
long    EqnStructWeightLexCompare(Eqn_p l1, Eqn_p lit2);
#define EqnEqualDirected(eq1, eq2) \
   (((eq1)->lterm == (eq2)->lterm) && ((eq1)->rterm == (eq2)->rterm))
#define EqnEqualDirectedDeref(eq1, eq2, d1, d2) \
   ((d1 == DEREF_NEVER && d2 == DEREF_NEVER) ?\
      EqnEqualDirected(eq1, eq2) :\
      (TermStructEqualDeref((eq1)->lterm, (eq2)->lterm, d1, d2) && \
       TermStructEqualDeref((eq1)->rterm, (eq2)->rterm, d1, d2)))
bool    EqnEqualDeref(Eqn_p eq1,  Eqn_p eq2, DerefType d1, DerefType d2);
#define EqnEqual(eq1, eq2) (EqnEqualDeref(eq1, eq2, DEREF_NEVER, DEREF_NEVER))
#define LiteralEqual(eq1, eq2) \
   (PropsAreEquiv((eq1),(eq2),EPIsPositive) && EqnEqual((eq1),(eq2)))

bool    EqnSubsumeDirected(Eqn_p subsumer, Eqn_p subsumed, Subst_p subst);
bool    EqnSubsume(Eqn_p subsumer, Eqn_p subsumed, Subst_p subst);
bool    EqnSubsumeP(Eqn_p subsumer, Eqn_p subsumed);

bool    LiteralSubsumeP(Eqn_p subsumer, Eqn_p subsumed);

#define EqnEquiv(eq1, eq2) (EqnSubsumeP((eq1),(eq2))&&(EqnSubsumeP((eq2),(eq1)))

#define LiteralEquiv(eq1, eq2) \
   (((eq1)->positive == (eq2)->positive) && EqnEquiv((eq1),(eq2))

bool    EqnUnifyDirected(Eqn_p eq1, Eqn_p eq2, Subst_p subst);
bool    EqnUnify(Eqn_p eq1, Eqn_p eq2, Subst_p subst);
bool    EqnUnifyP(Eqn_p eq1, Eqn_p eq2);

bool    LiteralUnifyOneWay(Eqn_p eq1, Eqn_p eq2, Subst_p subst, bool swapped);

int     EqnSyntaxCompare(const void* l1, const void* l2);
int     LiteralSyntaxCompare(const void* l1, const void* l2);

bool    EqnOrient(OCB_p ocb, Eqn_p eq);

CompareResult EqnCompare(OCB_p ocb, Eqn_p eq1, Eqn_p eq2);
bool          EqnGreater(OCB_p ocb, Eqn_p eq1, Eqn_p eq2);
CompareResult LiteralCompare(OCB_p ocb, Eqn_p eq1, Eqn_p eq2);
bool          LiteralGreater(OCB_p ocb, Eqn_p eq1, Eqn_p eq2);

PStackPointer SubstNormEqn(Eqn_p eq, Subst_p subst, VarBank_p vars);

double  EqnWeight(Eqn_p eq, double max_multiplier, long vweight, long
                  fweight, double app_var_mult);
double  EqnDAGWeight(Eqn_p eq, double uniqmax_multiplier,
                     double max_multiplier, long vweight, long fweight,
                     long dup_weight, bool new_eqn, bool new_terms);
double  EqnDAGWeight2(Eqn_p eq, double maxw_multiplier,
                      long vweight, long fweight, long dup_weight);

#define EqnStandardWeight(eqn)             \
   (TermStandardWeight((eqn)->lterm)+      \
    TermStandardWeight((eqn)->rterm))

#define EqnSplitModStandardWeight(eqn)                  \
   EqnQueryProp(eqn,EPIsSplitLit|EPIsPositive)?                         \
   SigGetSpecialWeight(eqn->bank->sig, eqn->lterm->f_code):             \
   EqnStandardWeight(eqn)


double EqnFunWeight(Eqn_p eq, double max_multiplier, long vweight,
                    long flimit, long *fweights, long default_fweight,
                    double app_var_mult, long* typefreqs);

double  EqnNonLinearWeight(Eqn_p eq, double max_multiplier, long
                           vlweight, long vweight, long fweight,
                           double app_var_mult);
double  EqnSymTypeWeight(Eqn_p eq, double max_multiplier, long
                         vweight, long fweight, long cweight, long
                         pweight, double app_var_mult);

double  EqnMaxWeight(Eqn_p eq, long vweight, long fweight,
                     double app_var_mult);

#define EqnStandardDiff(eqn)                    \
   (MAX(TermStandardWeight((eqn)->lterm),       \
        TermStandardWeight((eqn)->rterm)) -     \
    MIN(TermStandardWeight((eqn)->lterm),       \
        TermStandardWeight((eqn)->rterm)))

long EqnMaxTermPositions(Eqn_p eqn);
long EqnInferencePositions(Eqn_p eqn);

double  LiteralWeight(Eqn_p eq, double max_term_multiplier, double
                      max_literal_multiplier, double
                      pos_multiplier, long vweight, long fweight,
                      double app_var_mult, bool
                      count_eq_encoding);

double  LiteralFunWeight(Eqn_p eq,
                         double max_term_multiplier,
                         double max_literal_multiplier,
                         double pos_multiplier,
                         long vweight,
                         long flimit,
                         long *fweights,
                         long default_fweight,
                         double app_var_mult,
                         long* typefreqs);

double  LiteralTermExtWeight(Eqn_p eq, TermWeightExtension_p twe);

double LiteralNonLinearWeight(Eqn_p eq, double max_term_multiplier,
                              double max_literal_multiplier, double
                              pos_multiplier, long vlweight, long
                              vweight, long fweight,double app_var_mult,bool
                              count_eq_encoding);
double LiteralSymTypeWeight(Eqn_p eq, double max_term_multiplier,
                            double max_literal_multiplier, double
                            pos_multiplier, long vweight, long
                            fweight, long cweight, long pweight,
                            double app_var_mult);

#define EqnCountMaximalLiterals(eqn) (EqnIsOriented(eqn)?1:2)

static inline long EqnDepth(Eqn_p eqn);

int     LiteralCompareFun(Eqn_p lit1, Eqn_p lit2);

#define EqnAddSymbolDistribution(eqn, dist_array)               \
   TermAddSymbolDistribution((eqn)->lterm, (dist_array));       \
   TermAddSymbolDistribution((eqn)->rterm, (dist_array))
#define EqnAddSymbolDistExist(eqn, dist_array, exist)                   \
   TermAddSymbolDistExist((eqn)->lterm, (dist_array), (exist));         \
   TermAddSymbolDistExist((eqn)->rterm, (dist_array), (exist))

#define EqnAddTypeDistribution(eqn, type_array) \
   TermAddTypeDistribution((eqn)->lterm, (eqn)->bank->sig, type_array);\
   TermAddTypeDistribution((eqn)->rterm, (eqn)->bank->sig, type_array)

#define EqnAddSymbolDistributionLimited(eqn, dist_array, limit)         \
   TermAddSymbolDistributionLimited((eqn)->lterm, (dist_array), (limit)); \
   TermAddSymbolDistributionLimited((eqn)->rterm, (dist_array), (limit))
#define EqnAddSymbolFeaturesLimited(eqn, freq_array, depth_array, limit) \
   TermAddSymbolFeaturesLimited((eqn)->lterm, 0, (freq_array),          \
                                (depth_array), (limit));                \
   TermAddSymbolFeaturesLimited((eqn)->rterm, 0, (freq_array),          \
                                (depth_array), (limit))

void    EqnAddSymbolFeatures(Eqn_p eq, PStack_p mod_stack, long *feature_array);


#define EqnComputeFunctionRanks(eqn, rank_array, count)                 \
   TermComputeFunctionRanks((eqn)->lterm, (rank_array), (count));       \
   TermComputeFunctionRanks((eqn)->rterm, (rank_array), (count))

#define EqnCollectVariables(eqn, tree)          \
   (TermCollectVariables((eqn)->lterm,(tree))+  \
    TermCollectVariables((eqn)->rterm,(tree)))
#define EqnCollectFCodes(eqn, tree)          \
   (TermCollectFCodes((eqn)->lterm,(tree))+  \
    TermCollectFCodes((eqn)->rterm,(tree)))

#define EqnCollectPropVariables(eqn, tree, prop)                \
   (TermCollectPropVariables((eqn)->lterm,(tree), (prop))+      \
    TermCollectPropVariables((eqn)->rterm,(tree), (prop)))

#define EqnAddFunOccs(eqn, f_occur, res_stack)                \
   (TermAddFunOcc((eqn)->lterm,(f_occur), (res_stack))+       \
    TermAddFunOcc((eqn)->rterm, (f_occur), (res_stack)))

long    EqnCollectSubterms(Eqn_p eqn, PStack_p collector);
#define EqnCollectGroundTerms(eqn, res, all_subterms) \
   (TermCollectGroundTerms((eqn)->lterm, (res), (all_subterms))+     \
    TermCollectGroundTerms((eqn)->rterm, (res), (all_subterms)))

void EqnAppEncode(FILE* out, Eqn_p eq, bool negated);
bool EqnHasAppVar(Eqn_p eq);
void EqnMap(Eqn_p eq, TermMapper_p f, void* arg);

/*---------------------------------------------------------------------*/
/*                        Inline Functions                             */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: EqnDepth()
//
//   Return the depth of an equation
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static inline long EqnDepth(Eqn_p eqn)
{
   long ldepth, rdepth;

   ldepth = TermDepth(eqn->lterm);
   rdepth = TermDepth(eqn->rterm);

   return MAX(ldepth, rdepth);
}


/*-----------------------------------------------------------------------
//
// Function: EqnIsUntyped
//
//   Return true iff the equation is untyped, ie belongs to untyped logic
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static inline bool EqnIsUntyped(Eqn_p eqn)
{
   return TermIsUntyped(eqn->lterm) && TermIsUntyped(eqn->rterm);
}


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
