/*-----------------------------------------------------------------------

File  : che_clausesetfeatures.h

Author: Stephan Schulz

Contents

  Functions for determining various features of clause sets.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Mon Sep 28 19:17:50 MET DST 1998
    New

-----------------------------------------------------------------------*/

#ifndef CHE_CLAUSESETFEATURES

#define CHE_CLAUSESETFEATURES

#include <che_clausefeatures.h>
#include <ccl_proofstate.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef enum
{
   SpecUnit,
   SpecHorn,
   SpecGeneral,
   SpecNoEq,
   SpecSomeEq,
   SpecPureEq,
   SpecFewPosNonGroundUnits,
   SpecSomePosNonGroundUnits,
   SpecManyPosNonGroundUnits,
   SpecFewPosGround,
   SpecSomePosGround,
   SpecManyPosGround,
   SpecFewAxioms,
   SpecSomeAxioms,
   SpecManyAxioms,
   SpecFewLiterals,
   SpecSomeLiterals,
   SpecManyLiterals,
   SpecSmallTerms,
   SpecMediumTerms,
   SpecLargeTerms,
   SpecArity0,
   SpecArity1,
   SpecArity2,
   SpecArity3Plus,
   SpecAritySumSmall,
   SpecAritySumMedium,
   SpecAritySumLarge,
   SpecDepthShallow,
   SpecDepthMedium,
   SpecDepthDeep,
   SpecFO,
   SpecSO,
   SpecHO,
   SpecFewDefs,
   SpecMediumDefs,
   SpecManyDefs,
   SpecFewFormDefs,
   SpecMediumFormDefs,
   SpecManyFormDefs,
   SpecFewApplits,
   SpecMediumApplits,
   SpecManyApplits,
}SpecFeatures;



/* Limits for designating feature-based classes. They will be set in
   SpecFeatureCells based on these values. */

typedef struct spec_limits_cell
{
   bool   ngu_absolute;
   double ngu_few_limit;
   double ngu_many_limit;
   bool   gpc_absolute;
   double gpc_few_limit;
   double gpc_many_limit;
   long   ax_some_limit;
   long   ax_many_limit;
   long   lit_some_limit;
   long   lit_many_limit;
   long   term_medium_limit;
   long   term_large_limit;
   long   far_sum_medium_limit;
   long   far_sum_large_limit;
   long   depth_medium_limit;
   long   depth_deep_limit;
   int    symbols_medium_limit;
   int    symbols_large_limit;
   int    predc_medium_limit;
   int    predc_large_limit;
   int    pred_medium_limit;
   int    pred_large_limit;
   int    func_medium_limit;
   int    func_large_limit;
   int    fun_medium_limit;
   int    fun_large_limit;
   int    order_medium_limit;
   int    order_large_limit;
   int    num_of_lams_medium_limit;
   int    num_of_lams_large_limit;
   int    num_of_defs_medium_limit;
   int    num_of_defs_large_limit;
   double perc_form_defs_medium_limit;
   double perc_form_defs_large_limit;
   double perc_app_lits_medium_limit;
   double perc_app_lits_large_limit;
}SpecLimitsCell, *SpecLimits_p;


/* Stores all the precomputed feature values (including, after a call
   to SpecFeaturesAddEval, the classifications */
typedef struct spec_feature_cell
{
   SpecFeatures axiomtypes;
   SpecFeatures goaltypes;
   SpecFeatures eq_content;
   SpecFeatures ng_unit_content;
   SpecFeatures ground_positive_content;
   bool         goals_are_ground;
   SpecFeatures set_clause_size;
   SpecFeatures set_literal_size;
   SpecFeatures set_termcell_size;
   SpecFeatures max_fun_ar_class; /* Arity of _real_ function symbols,
               0,1,2,>2 */
   SpecFeatures avg_fun_ar_class;
   SpecFeatures sum_fun_ar_class;
   SpecFeatures max_depth_class;

   bool         has_ho_features; // has any HO feature
   bool         quantifies_booleans; // is there any variable in whose type $o
                                     // appears
   bool         has_defined_choice; // there is a clause that defines choice
   SpecFeatures order_class;
   SpecFeatures goal_order_class;
   SpecFeatures defs_class;
   SpecFeatures form_defs_class;
   SpecFeatures appvar_lits_class;

   long         clauses;
   long         goals;
   long         axioms;
   long         literals;
   long         term_cells;
   long         clause_max_depth;
   long         clause_avg_depth;
   long         unit;
   long         unitgoals;
   long         unitaxioms;
   long         horn;
   long         horngoals;
   long         hornaxioms;
   long         eq_clauses;
   long         peq_clauses;
   long         groundunitaxioms;
   long         positiveaxioms;
   long         groundpositiveaxioms;
   long         groundgoals;
   double       ng_unit_axioms_part;
   double       ground_positive_axioms_part;
   int          max_fun_arity;
   int          avg_fun_arity;
   int          sum_fun_arity;
   int          max_pred_arity;
   int          avg_pred_arity;
   int          sum_pred_arity;
   int          fun_const_count;
   int          fun_nonconst_count;
   int          pred_nonconst_count;

   /* HO features */
   int          order; // maximal order of the problem
   int          goal_order; // maximal order of hypothesis/conjecture
   int          num_of_definitions; // number of formulas tagged with definition
   double       perc_of_form_defs; // percentage of which defines formulas
   double       perc_of_appvar_lits; // percentage of which defines formulas
}SpecFeatureCell, *SpecFeature_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define NGU_ABSOLUTE           true
#define NGU_FEW_DEFAULT        0.25
#define NGU_MANY_DEFAULT       0.75
#define NGU_FEW_ABSDEFAULT        1
#define NGU_MANY_ABSDEFAULT       3
#define GPC_ABSOLUTE           true
#define GPC_FEW_DEFAULT        0.25
#define GPC_MANY_DEFAULT       0.75
#define GPC_FEW_ABSDEFAULT        2
#define GPC_MANY_ABSDEFAULT       5
#define AX_1_DEFAULT             10
#define AX_4_DEFAULT             15
#define AX_SOME_DEFAULT        1000
#define AX_MANY_DEFAULT       10000
#define LIT_SOME_DEFAULT        400
#define LIT_MANY_DEFAULT       4000
#define TERM_MED_DEFAULT        200
#define TERM_LARGE_DEFAULT     1500
#define FAR_SUM_MED_DEFAULT      4
#define FAR_SUM_LARGE_DEFAULT   29
#define DEPTH_MEDIUM_DEFAULT     0 /* Partitioning two ways turns out
                                      to be nearly as good as 3 way on
                                      the test set */
#define DEPTH_DEEP_DEFAULT       6
#define SYMBOLS_MEDIUM_DEFAULT   100
#define SYMBOLS_LARGE_DEFAULT    1000

#define PREDC_MEDIUM_DEFAULT     0
#define PREDC_LARGE_DEFAULT      2
#define PRED_MEDIUM_DEFAULT      1225
#define PRED_LARGE_DEFAULT       4000
#define FUNC_MEDIUM_DEFAULT      8
#define FUNC_LARGE_DEFAULT       110
#define FUN_MEDIUM_DEFAULT       360
#define FUN_LARGE_DEFAULT        400

#define NUM_LAMS_MEDIUM_DEFAULT  2
#define NUM_LAMS_LARGE_DEFAULT   8
#define ORDER_MEDIUM_DEFAULT     2 // second order is already medium
#define ORDER_LARGE_DEFAULT      3 // third order is already high
#define DEFS_MEDIUM_DEFAULT      8
#define DEFS_LARGE_DEFAULT       64
#define DEFS_PERC_MEDIUM_DEFAULT 0.15
#define DEFS_PERC_LARGE_DEFAULT  0.5
#define PERC_APPLIT_MEDIUM_DEFAULT 0.1
#define PERC_APPLIT_LARGE_DEFAULT  0.5

#define SPEC_STRING_MEM          22

#define DEFAULT_OUTPUT_DESCRIPTOR "eigEIG"
#define DEFAULT_CLASS_MASK "aaaaaaaaaaaaa"


#define SpecLimitsCellAlloc() \
        (SpecLimitsCell*)SizeMalloc(sizeof(SpecLimitsCell))
#define SpecLimitsCellFree(junk) \
        SizeFree(junk, sizeof(SpecLimitsCell))
SpecLimits_p SpecLimitsAlloc(void);

#define SpecFeatureCellAlloc() \
        (SpecFeatureCell*)SizeMalloc(sizeof(SpecFeatureCell))
#define SpecFeatureCellFree(junk) \
        SizeFree(junk, sizeof(SpecFeatureCell))

#define Spec(spec) (true) /* For auto-generated code */

#define SpecIsFO(spec) (true)


#define SpecAxiomsAreUnit(spec) ((spec)->axioms == (spec)->unitaxioms)
#define SpecAxiomsAreHorn(spec) ((spec)->axioms == (spec)->hornaxioms)
#define SpecAxiomsAreNonUnitHorn(spec) (SpecAxiomsAreHorn(spec)&&\
                                        !(SpecAxiomsAreUnit(spec)))
#define SpecAxiomsAreGeneral(spec) ((spec)->axioms >  (spec)->hornaxioms)

#define SpecGoalsAreUnit(spec) ((spec)->goals == (spec)->unitgoals)
#define SpecGoalsAreHorn(spec) (!SpecGoalsAreUnit(spec))
#define SpecGoalsAreGround(spec) ((spec)->goals_are_ground)
#define SpecGoalsHaveVars(spec) (!SpecGoalsAreGround(spec))

#define SpecPureEq(spec) ((spec)->clauses==(spec)->peq_clauses)
#define SpecSomeEq(spec) ((spec)->eq_clauses && !SpecPureEq(spec))
#define SpecNoEq(spec)   (!(spec)->eq_clauses)

#define SpecFewNGPosUnits(spec) \
        ((spec)->ng_unit_content == SpecFewPosNonGroundUnits)
#define SpecSomeNGPosUnits(spec) \
        ((spec)->ng_unit_content == SpecSomePosNonGroundUnits)
#define SpecManyNGPosUnits(spec) \
        ((spec)->ng_unit_content == SpecManyPosNonGroundUnits)

#define SpecFewGroundPos(spec) \
        ((spec)->ground_positive_content == SpecFewPosGround)
#define SpecSomeGroundPos(spec) \
        ((spec)->ground_positive_content == SpecSomePosGround)
#define SpecManyGroundPos(spec) \
        ((spec)->ground_positive_content == SpecManyPosGround)

#define SpecFewAxioms(spec) \
        ((spec)->set_clause_size == SpecFewAxioms)
#define SpecSomeAxioms(spec) \
        ((spec)->set_clause_size == SpecSomeAxioms)
#define SpecManyAxioms(spec) \
        ((spec)->set_clause_size == SpecManyAxioms)

#define SpecFewLiterals(spec) \
        ((spec)->set_literal_size == SpecFewLiterals)
#define SpecSomeLiterals(spec) \
        ((spec)->set_literal_size == SpecSomeLiterals)
#define SpecManyLiterals(spec) \
        ((spec)->set_literal_size == SpecManyLiterals)

#define SpecSmallTerms(spec) \
        ((spec)->set_termcell_size == SpecSmallTerms)
#define SpecMediumTerms(spec) \
        ((spec)->set_termcell_size == SpecMediumTerms)
#define SpecLargeTerms(spec) \
        ((spec)->set_termcell_size == SpecLargeTerms)

#define SpecMaxFArity0(spec) \
        ((spec)->max_fun_ar_class == SpecArity0)
#define SpecMaxFArity1(spec) \
        ((spec)->max_fun_ar_class == SpecArity1)
#define SpecMaxFArity2(spec) \
        ((spec)->max_fun_ar_class == SpecArity2)
#define SpecMaxFArity3Plus(spec) \
        ((spec)->max_fun_ar_class ==SpecArity3Plus)

#define SpecAvgFArity0(spec) \
        ((spec)->avg_fun_ar_class == SpecArity0)
#define SpecAvgFArity1(spec) \
        ((spec)->avg_fun_ar_class == SpecArity1)
#define SpecAvgFArity2(spec) \
        ((spec)->avg_fun_ar_class == SpecArity2)
#define SpecAvgFArity3Plus(spec) \
        ((spec)->avg_fun_ar_class ==SpecArity3Plus)

#define SpecSmallFArSum(spec) \
        ((spec)->sum_fun_ar_class == SpecAritySumSmall)
#define SpecMediumFArSum(spec) \
        ((spec)->sum_fun_ar_class == SpecAritySumMedium)
#define SpecLargeFArSum(spec) \
        ((spec)->sum_fun_ar_class == SpecAritySumLarge)

#define SpecShallowMaxDepth(spec) \
        ((spec)->max_depth_class == SpecDepthShallow)
#define SpecMediumMaxDepth(spec) \
        ((spec)->max_depth_class == SpecDepthMedium)
#define SpecDeepMaxDepth(spec) \
        ((spec)->max_depth_class == SpecDepthDeep)

long    ClauseSetCountGoals(ClauseSet_p set);
#define ClauseSetCountAxioms(set)\
        ((set)->members-ClauseSetCountGoals(set))

long    ClauseSetCountUnit(ClauseSet_p set);
long    ClauseSetCountUnitGoals(ClauseSet_p set);
#define ClauseSetCountUnitAxioms(set)\
        (ClauseSetCountUnit(set)-ClauseSetCountUnitGoals(set))
#define ClauseSetIsUnitSet(set) \
        ((set)->members == ClauseSetCountUnit(set))
#define ClauseSetAxiomsAreUnit(set) \
        (ClauseSetCountUnitAxioms(set) == ClauseSetCountAxioms(set))
#define ClauseSetGoalsAreUnit(set) \
        (ClauseSetCountUnitGoals(set) == ClauseSetCountGoals(set))

long    ClauseSetCountHorn(ClauseSet_p set);
long    ClauseSetCountHornGoals(ClauseSet_p set);
#define ClauseSetCountHornAxioms(set) \
        (ClauseSetCountHorn(set)-ClauseSetCountHornGoals(set))
#define ClauseSetIsHornSet(set) \
        ((set)->members == ClauseSetCountHorn(set))
#define ClauseSetAxiomsAreHorn(set) \
        (ClauseSetCountHornAxioms(set) == ClauseSetCountAxioms(set))
#define ClauseSetGoalsAreHorn(set) \
        (ClauseSetCountHornGoals(set) == ClauseSetCountGoals(set))

long    ClauseSetCountEquational(ClauseSet_p set);
bool    ClauseSetHasHOFeatures(ClauseSet_p set);
int     ClauseSetComputeMaxOrder(ClauseSet_p set, Sig_p sig);

/* Are all clauses equational? */
#define ClauseSetIsEquationalSet(set) \
        ((set)->members == ClauseSetCountEquational(set))
/* Is there equality in the clause set? */
#define ClauseSetIsEquational(set) \
        (ClauseSetCountEquational(set)>=1)

long    ClauseSetCountPureEquational(ClauseSet_p set);
#define ClauseSetIsPureEquationalSet(set) \
        ((set)->members == ClauseSetCountPureEquational(set))

long    ClauseSetCountGroundGoals(ClauseSet_p set);
#define ClauseSetGoalsAreGround(set) \
        (ClauseSetCountGoals(set)==ClauseSetCountGroundGoals(set))

long    ClauseSetCountGround(ClauseSet_p set);
#define ClauseSetIsGround(set)\
        (ClauseSetCountGround(set)==(set)->members)

long    ClauseSetCountGroundPositiveAxioms(ClauseSet_p set);
long    ClauseSetCountPositiveAxioms(ClauseSet_p set);

long    ClauseSetCountGroundUnitAxioms(ClauseSet_p set);
#define ClauseSetCountNonGroundUnitAxioms(set) \
        (ClauseSetCountUnitAxioms(set)-ClauseSetCountGroundUnitAxioms(set))
long    ClauseSetCountRangeRestricted(ClauseSet_p set);
double  ClauseSetNonGoundAxiomPart(ClauseSet_p set);

long    ClauseSetCollectArityInformation(ClauseSet_p set,
                Sig_p sig,
                int *max_fun_arity,
                int *avg_fun_arity,
                int *sum_fun_arity,
                int *max_pred_arity,
                int *avg_pred_arity,
                int *sum_pred_arity,
                int *non_const_funs,
                int *non_const_preds);

long    ClauseSetCountMaximalTerms(ClauseSet_p set);
long    ClauseSetCountMaximalLiterals(ClauseSet_p set);
long    ClauseSetCountUnorientableLiterals(ClauseSet_p set);
long    ClauseSetCountEqnLiterals(ClauseSet_p set);
long    ClauseSetMaxStandardWeight(ClauseSet_p set);

long    ClauseSetTermCells(ClauseSet_p set);
long    ClauseSetMaxLiteralNumber(ClauseSet_p set);
long    ClauseSetCountVariables(ClauseSet_p set);
long    ClauseSetCountSingletons(ClauseSet_p set);
long    ClauseSetTPTPDepthInfoAdd(ClauseSet_p set, long* depthmax,
              long* depthsum, long* count);
void    ClauseSetComputeHOFeatures(ClauseSet_p set, Sig_p sig,
                                   bool* has_ho_features,
                                   int* order,
                                   bool* quantifies_bools,
                                   bool* has_defined_choice,
                                   double* perc_appvar_lit);
void    SpecFeaturesCompute(SpecFeature_p features, ClauseSet_p cset,
                            FormulaSet_p fset, FormulaSet_p arch, TB_p bank);
void    SpecFeaturesAddEval(SpecFeature_p features, SpecLimits_p limits);

void    SpecFeaturesPrint(FILE* out, SpecFeature_p features);
void    SpecLimitsPrint(FILE* out, SpecLimits_p limits);


void    SpecFeaturesParse(Scanner_p in, SpecFeature_p features);

char*    SpecTypeString(SpecFeature_p features, const char* mask);
void    SpecTypePrint(FILE* out, SpecFeature_p features, char* mask);

void    ClauseSetPrintPosUnits(FILE* out, ClauseSet_p set, bool
                printinfo);
void    ClauseSetPrintNegUnits(FILE* out, ClauseSet_p set, bool
                printinfo);
void    ClauseSetPrintNonUnits(FILE* out, ClauseSet_p set, bool
                printinfo);
void    ProofStatePrintSelective(FILE* out, ProofState_p state, char*
             descriptor, bool printinfo);

SpecLimits_p CreateDefaultSpecLimits(void);

void ClausifyAndClassifyWTimeout(ProofState_p state, int timeout,
                                 char* mask,
                                 char class[SPEC_STRING_MEM]);



#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
