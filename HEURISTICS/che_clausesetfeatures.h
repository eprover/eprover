/*-----------------------------------------------------------------------

File  : che_clausesetfeatures.h

Author: Stephan Schulz

Contents
 
  Functions for determining various features of clause sets.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
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
   SpecMaxArity0,
   SpecMaxArity1,
   SpecMaxArity2,
   SpecMaxArity3Plus
}SpecFeatures;

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
   long         clauses;
   long         goals;
   long         axioms;
   long         literals;
   long         term_cells;
   long         unit;
   long         unitgoals;
   long         unitaxioms;
   long         horn;
   long         horngoals;
   long		hornaxioms;
   long		eq_clauses;
   long		peq_clauses;
   long		groundunitaxioms;
   long         positiveaxioms;
   long         groundpositiveaxioms;
   long		groundgoals;
   double       ng_unit_axioms_part;
   double       ground_positive_axioms_part;
   int          max_fun_arity; 
   int          avg_fun_arity;
   int          sum_fun_arity;
   int          max_pred_arity; 
   int          avg_pred_arity; 
   int          sum_pred_arity; 
   int          fun_const_count;
}SpecFeatureCell, *SpecFeature_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define FEW_DEFAULT  0.25
#define MANY_DEFAULT 0.75
#define FEW_ABSDEFAULT  2
#define MANY_ABSDEFAULT 5

#define AX_SOME_DEFAULT      30
#define AX_MANY_DEFAULT     150
#define LIT_SOME_DEFAULT     15
#define LIT_MANY_DEFAULT    100
#define TERM_MED_DEFAULT     60
#define TERM_LARGE_DEFAULT 1000


#define DEFAULT_OUTPUT_DESCRIPTOR "eigEIG"

#define SpecFeatureCellAlloc() \
        (SpecFeatureCell*)SizeMalloc(sizeof(SpecFeatureCell)) 
#define SpecFeatureCellFree(junk) \
        SizeFree(junk, sizeof(SpecFeatureCell))

#define Spec(spec) (true) /* For auto-generated code */

#define SpecAxiomsAreUnit(spec) ((spec)->axioms == (spec)->unitaxioms)
#define SpecAxiomsAreHorn(spec) ((spec)->axioms == (spec)->hornaxioms)
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
        ((spec)->max_fun_ar_class == SpecMaxArity0)
#define SpecMaxFArity1(spec) \
        ((spec)->max_fun_ar_class == SpecMaxArity1)
#define SpecMaxFArity2(spec) \
        ((spec)->max_fun_ar_class == SpecMaxArity2)
#define SpecMaxFArity3Plus(spec) \
        ((spec)->max_fun_ar_class ==SpecMaxArity3Plus)

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
long    ClauseSetCountTPTPRangeRestricted(ClauseSet_p set);
double  ClauseSetNonGoundAxiomPart(ClauseSet_p set);

long    ClauseSetCollectArityInformation(ClauseSet_p set,
					 Sig_p sig,
					 int *max_fun_arity,
					 int *avg_fun_arity,
					 int *sum_fun_arity,
					 int *max_pred_arity,
					 int *avg_pred_arity,
					 int *sum_pred_arity);

long    ClauseSetCountMaximalTerms(ClauseSet_p set);
long    ClauseSetCountMaximalLiterals(ClauseSet_p set);
long    ClauseSetCountUnorientableLiterals(ClauseSet_p set);
long    ClauseSetCountEqnLiterals(ClauseSet_p set);

long    ClauseSetTermCells(ClauseSet_p set);
long    ClauseSetMaxLiteralNumber(ClauseSet_p set);
long    ClauseSetCountVariables(ClauseSet_p set);
long    ClauseSetCountSingletons(ClauseSet_p set);
long    ClauseSetTPTPDepthInfoAdd(ClauseSet_p set, long* depthmax,
				  long* depthsum, long* count);
void    SpecFeaturesCompute(SpecFeature_p features, ClauseSet_p set,
			    Sig_p sig);
void    SpecFeaturesAddEval(SpecFeature_p features, double few_limit,
			    double many_limit, bool absolute, long
			    ax_some_limit, long ax_many_limit, long
			    lit_some_limit, long lit_many_limit, long
			    term_medium_limit, long term_large_limit);

void    SpecFeaturesPrint(FILE* out, SpecFeature_p features);
void    SpecTypePrint(FILE* out, SpecFeature_p features, char* mask);

void    ClauseSetPrintPosUnits(FILE* out, ClauseSet_p set, bool
			       printinfo);
void    ClauseSetPrintNegUnits(FILE* out, ClauseSet_p set, bool
			       printinfo);
void    ClauseSetPrintNonUnits(FILE* out, ClauseSet_p set, bool
			       printinfo);
void    ProofStatePrintSelective(FILE* out, ProofState_p state, char*
				 descriptor, bool printinfo);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





