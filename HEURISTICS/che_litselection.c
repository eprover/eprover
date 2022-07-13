/*-----------------------------------------------------------------------

File  : che_litselection.c

Author: Stephan Schulz

Contents

  Functions for selection certain literals (and hence superposition
  strategies).


  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Fri May 21 22:16:27 GMT 1999
    New

-----------------------------------------------------------------------*/

#include "che_litselection.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


static LitSelNameFunAssocCell name_fun_assoc[] =
{
   {"NoSelection",                           SelectNoLiterals},
   {"NoGeneration",                          SelectNoGeneration},
   {"SelectNegativeLiterals",                SelectNegativeLiterals},
   {"PSelectNegativeLiterals",               PSelectNegativeLiterals},
   {"SelectPureVarNegLiterals",              SelectFirstVariableLiteral},
   {"PSelectPureVarNegLiterals",             PSelectFirstVariableLiteral},
   {"SelectLargestNegLit",                   SelectLargestNegativeLiteral},
   {"PSelectLargestNegLit",                  PSelectLargestNegativeLiteral},
   {"SelectSmallestNegLit",                  SelectSmallestNegativeLiteral},
   {"PSelectSmallestNegLit",                 PSelectSmallestNegativeLiteral},
   {"SelectLargestOrientable",               SelectLargestOrientableLiteral},
   {"PSelectLargestOrientable",              PSelectLargestOrientableLiteral},
   {"MSelectLargestOrientable",              MSelectLargestOrientableLiteral},
   {"SelectSmallestOrientable",              SelectSmallestOrientableLiteral},
   {"PSelectSmallestOrientable",             PSelectSmallestOrientableLiteral},
   {"MSelectSmallestOrientable",             MSelectSmallestOrientableLiteral},
   {"SelectDiffNegLit",                      SelectDiffNegativeLiteral},
   {"PSelectDiffNegLit",                     PSelectDiffNegativeLiteral},
   {"SelectGroundNegLit",                    SelectGroundNegativeLiteral},
   {"PSelectGroundNegLit",                   PSelectGroundNegativeLiteral},
   {"SelectOptimalLit",                      SelectOptimalLiteral},
   {"PSelectOptimalLit",                     PSelectOptimalLiteral},
   {"SelectMinOptimalLit",                   SelectMinOptimalLiteral},
   {"PSelectMinOptimalLit",                  PSelectMinOptimalLiteral},
   {"SelectMinOptimalNoTypePred",            SelectMinOptimalNoTypePred},
   {"PSelectMinOptimalNoTypePred",           PSelectMinOptimalNoTypePred},
   {"SelectMinOptimalNoXTypePred",           SelectMinOptimalNoXTypePred},
   {"PSelectMinOptimalNoXTypePred",          PSelectMinOptimalNoXTypePred},
   {"SelectMinOptimalNoRXTypePred",          SelectMinOptimalNoRXTypePred},
   {"PSelectMinOptimalNoRXTypePred",         PSelectMinOptimalNoRXTypePred},
   {"SelectCondOptimalLit",                  SelectCondOptimalLiteral},
   {"PSelectCondOptimalLit",                 PSelectCondOptimalLiteral},
   {"SelectAllCondOptimalLit",               SelectAllCondOptimalLiteral},
   {"PSelectAllCondOptimalLit",              PSelectAllCondOptimalLiteral},
   {"SelectOptimalRestrDepth2",              SelectDepth2OptimalLiteral},
   {"PSelectOptimalRestrDepth2",             PSelectDepth2OptimalLiteral},
   {"SelectOptimalRestrPDepth2",             SelectPDepth2OptimalLiteral},
   {"PSelectOptimalRestrPDepth2",            PSelectPDepth2OptimalLiteral},
   {"SelectOptimalRestrNDepth2",             SelectNDepth2OptimalLiteral},
   {"PSelectOptimalRestrNDepth2",            PSelectNDepth2OptimalLiteral},
   {"SelectNonRROptimalLit",                 SelectNonRROptimalLiteral},
   {"PSelectNonRROptimalLit",                PSelectNonRROptimalLiteral},
   {"SelectNonStrongRROptimalLit",           SelectNonStrongRROptimalLiteral},
   {"PSelectNonStrongRROptimalLit",          PSelectNonStrongRROptimalLiteral},
   {"SelectAntiRROptimalLit",                SelectAntiRROptimalLiteral},
   {"PSelectAntiRROptimalLit",               PSelectAntiRROptimalLiteral},
   {"SelectNonAntiRROptimalLit",             SelectNonAntiRROptimalLiteral},
   {"PSelectNonAntiRROptimalLit",            PSelectNonAntiRROptimalLiteral},
   {"SelectStrongRRNonRROptimalLit",         SelectStrongRRNonRROptimalLiteral},
   {"PSelectStrongRRNonRROptimalLit",        PSelectStrongRRNonRROptimalLiteral},
   {"SelectUnlessUniqMax",                   SelectUnlessUniqMaxOptimalLiteral},
   {"PSelectUnlessUniqMax",                  PSelectUnlessUniqMaxOptimalLiteral},
   {"SelectUnlessPosMax",                    SelectUnlessPosMaxOptimalLiteral},
   {"PSelectUnlessPosMax",                   PSelectUnlessPosMaxOptimalLiteral},
   {"SelectUnlessUniqPosMax",                SelectUnlessUniqPosMaxOptimalLiteral},
   {"PSelectUnlessUniqPosMax",               PSelectUnlessUniqPosMaxOptimalLiteral},
   {"SelectUnlessUniqMaxPos",                SelectUnlessUniqMaxPosOptimalLiteral},
   {"PSelectUnlessUniqMaxPos",               PSelectUnlessUniqMaxPosOptimalLiteral},
   {"SelectComplex",                         SelectComplex},
   {"PSelectComplex",                        PSelectComplex},
   {"SelectComplexExceptRRHorn",             SelectComplexExceptRRHorn},
   {"PSelectComplexExceptRRHorn",            PSelectComplexExceptRRHorn},
   {"SelectLComplex",                        SelectLComplex},
   {"PSelectLComplex",                       PSelectLComplex},
   {"SelectMaxLComplex",                     SelectMaxLComplex},
   {"PSelectMaxLComplex",                    PSelectMaxLComplex},
   {"SelectMaxLComplexNoTypePred",           SelectMaxLComplexNoTypePred},
   {"PSelectMaxLComplexNoTypePred",          PSelectMaxLComplexNoTypePred},
   {"SelectMaxLComplexNoXTypePred",          SelectMaxLComplexNoXTypePred},
   {"PSelectMaxLComplexNoXTypePred",         PSelectMaxLComplexNoXTypePred},
   {"SelectComplexPreferNEQ",                SelectComplexPreferNEQ},
   {"PSelectComplexPreferNEQ",               PSelectComplexPreferNEQ},
   {"SelectComplexPreferEQ",                 SelectComplexPreferEQ},
   {"PSelectComplexPreferEQ",                PSelectComplexPreferEQ},
   {"SelectComplexExceptUniqMaxHorn",        SelectComplexExceptUniqMaxHorn},
   {"PSelectComplexExceptUniqMaxHorn",       PSelectComplexExceptUniqMaxHorn},
   {"MSelectComplexExceptUniqMaxHorn",       MSelectComplexExceptUniqMaxHorn},
   {"SelectNewComplex",                      SelectNewComplex},
   {"PSelectNewComplex",                     PSelectNewComplex},
   {"SelectNewComplexExceptUniqMaxHorn",     SelectNewComplexExceptUniqMaxHorn},
   {"PSelectNewComplexExceptUniqMaxHorn",    PSelectNewComplexExceptUniqMaxHorn},
   {"SelectMinInfpos",                       SelectMinInfpos},
   {"PSelectMinInfpos",                      PSelectMinInfpos},
   {"HSelectMinInfpos",                      HSelectMinInfpos},
   {"GSelectMinInfpos",                      GSelectMinInfpos},
   {"SelectMinInfposNoTypePred",             SelectMinInfposNoTypePred},
   {"PSelectMinInfposNoTypePred",            PSelectMinInfposNoTypePred},
   {"SelectMin2Infpos",                      SelectMin2Infpos},
   {"PSelectMin2Infpos",                     PSelectMin2Infpos},
   {"SelectComplexExceptUniqMaxPosHorn",     SelectComplexExceptUniqMaxPosHorn},
   {"PSelectComplexExceptUniqMaxPosHorn",    PSelectComplexExceptUniqMaxPosHorn},
   {"SelectUnlessUniqMaxSmallestOrientable", SelectUnlessUniqMaxSmallestOrientable},
   {"PSelectUnlessUniqMaxSmallestOrientable",PSelectUnlessUniqMaxSmallestOrientable},
   {"SelectDivLits",                         SelectDiversificationLiterals},
   {"SelectDivPreferIntoLits",               SelectDiversificationPreferIntoLiterals},
   {"SelectMaxLComplexG",                    SelectMaxLComplexG},
   {"SelectMaxLComplexAvoidPosPred",         SelectMaxLComplexAvoidPosPred},
   {"SelectMaxLComplexAPPNTNp",              SelectMaxLComplexAPPNTNp},
   {"SelectMaxLComplexAPPNoType",            SelectMaxLComplexAPPNoType},
   {"SelectMaxLComplexAvoidPosUPred",        SelectMaxLComplexAvoidPosUPred},
   {"SelectComplexG",                        SelectComplexG},
   {"SelectComplexAHP",                      SelectComplexAHP},
   {"PSelectComplexAHP",                     PSelectComplexAHP},
   {"SelectNewComplexAHP",                   SelectNewComplexAHP},
   {"PSelectNewComplexAHP",                  PSelectNewComplexAHP},
   {"SelectComplexAHPExceptRRHorn",          SelectComplexAHPExceptRRHorn},
   {"PSelectComplexAHPExceptRRHorn",         PSelectComplexAHPExceptRRHorn},

   {"SelectNewComplexAHPExceptRRHorn",       SelectNewComplexAHPExceptRRHorn},
   {"PSelectNewComplexAHPExceptRRHorn",      PSelectNewComplexAHPExceptRRHorn},

   {"SelectNewComplexAHPExceptUniqMaxHorn",  SelectNewComplexAHPExceptUniqMaxHorn},
   {"PSelectNewComplexAHPExceptUniqMaxHorn", PSelectNewComplexAHPExceptUniqMaxHorn},
   {"SelectNewComplexAHPNS",                 SelectNewComplexAHPNS},
   {"SelectVGNonCR",                         SelectVGNonCR},

   {"SelectCQArEqLast",                      SelectCQArEqLast},
   {"SelectCQArEqFirst",                     SelectCQArEqFirst},
   {"SelectCQIArEqLast",                     SelectCQIArEqLast},
   {"SelectCQIArEqFirst",                    SelectCQIArEqFirst},
   {"SelectCQAr",                            SelectCQAr},
   {"SelectCQIAr",                           SelectCQIAr},
   {"SelectCQArNpEqFirst",                   SelectCQArNpEqFirst},
   {"SelectCQIArNpEqFirst",                  SelectCQIArNpEqFirst},

   {"SelectGrCQArEqFirst",                   SelectGrCQArEqFirst},
   {"SelectCQGrArEqFirst",                   SelectCQGrArEqFirst},
   {"SelectCQArNTEqFirst",                   SelectCQArNTEqFirst},
   {"SelectCQIArNTEqFirst",                  SelectCQIArNTEqFirst},
   {"SelectCQArNTNpEqFirst",                 SelectCQArNTNpEqFirst},
   {"SelectCQIArNTNpEqFirst",                SelectCQIArNTNpEqFirst},
   {"SelectCQArNXTEqFirst" ,                 SelectCQArNXTEqFirst},
   {"SelectCQIArNXTEqFirst" ,                SelectCQIArNXTEqFirst},

   {"SelectCQArNTNp",                        SelectCQArNTNp},
   {"SelectCQIArNTNp",                       SelectCQIArNTNp},
   {"SelectCQArNT",                          SelectCQArNT},
   {"SelectCQIArNT",                         SelectCQIArNT},
   {"SelectCQArNp",                          SelectCQArNp},
   {"SelectCQIArNp",                         SelectCQIArNp},

   {"SelectCQArNpEqFirstUnlessPDom",         SelectCQArNpEqFirstUnlessPDom},
   {"SelectCQArNTEqFirstUnlessPDom",         SelectCQArNTEqFirstUnlessPDom},

   {"SelectCQPrecW",                         SelectCQPrecW},
   {"SelectCQIPrecW",                        SelectCQIPrecW},
   {"SelectCQPrecWNTNp",                     SelectCQPrecWNTNp},
   {"SelectCQIPrecWNTNp",                    SelectCQIPrecWNTNp},

   {"SelectMaxLComplexAvoidAppVar",          SelectMaxLComplexAvoidAppVar},
   {"SelectMaxLComplexStronglyAvoidAppVar",  SelectMaxLComplexStronglyAvoidAppVar},
   {"SelectMaxLComplexPreferAppVar",         SelectMaxLComplexPreferAppVar},

   {NULL, (LiteralSelectionFun)NULL}
};


static long literal_weight_counter=0;

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

#define lit_sel_diff_weight(handle) \
        ((100*EqnStandardDiff(handle))+EqnStandardWeight(handle))

/*-----------------------------------------------------------------------
//
// Function: find_maxlcomplex_literal()
//
//   Find a maximal negative literal to select (see
//   SelectMaxLComplex() below.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static Eqn_p find_maxlcomplex_literal(Clause_p clause)
{
   Eqn_p handle = clause->literals, selected = NULL;
   long select_weight, weight;

   while(handle)
   {
      if(EqnIsNegative(handle)&&
    EqnIsMaximal(handle) &&
    TermIsFreeVar(handle->lterm) &&
    TermIsFreeVar(handle->rterm))
      {
    selected = handle;
    break;
      }
      handle = handle->next;
   }
   if(selected)
   {
      return selected;
   }
   select_weight = -1;
   handle = clause->literals;

   while(handle)
   {
      if(EqnIsNegative(handle) && EqnIsMaximal(handle)&&
    EqnIsGround(handle))
      {
    weight = lit_sel_diff_weight(handle);
    if(weight > select_weight)
    {
       select_weight = weight;
       selected = handle;
         }
      }
      handle = handle->next;
   }
   if(selected)
   {
      return selected;
   }
   select_weight = -1;
   handle = clause->literals;

   while(handle)
   {
      if(EqnIsNegative(handle) && EqnIsMaximal(handle))
      {
    weight = lit_sel_diff_weight(handle);
    if(weight > select_weight)
    {
       select_weight = weight;
       selected = handle;
    }
      }
      handle = handle->next;
   }
   return selected;
}


/*-----------------------------------------------------------------------
//
// Function: find_lcomplex_literal()
//
//   Find a non-maximal negative literal to select (see
//   SelectComplex() below.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static Eqn_p find_lcomplex_literal(Clause_p clause)
{
   Eqn_p handle = clause->literals, selected = NULL;
   long select_weight, weight;

   while(handle)
   {
      if(EqnIsNegative(handle)&&
    !EqnIsMaximal(handle) &&
    TermIsFreeVar(handle->lterm) &&
    TermIsFreeVar(handle->rterm))
      {
    selected = handle;
    break;
      }
      handle = handle->next;
   }
   if(selected)
   {
      return selected;
   }

   select_weight = -1;
   handle = clause->literals;

   while(handle)
   {
      if(EqnIsNegative(handle)&&!EqnIsMaximal(handle)&&EqnIsGround(handle))
      {
    weight = lit_sel_diff_weight(handle);
    if(weight > select_weight)
    {
       select_weight = weight;
       selected = handle;
    }
      }
      handle = handle->next;
   }
   if(selected)
   {
      return selected;
   }

   select_weight = -1;
   handle = clause->literals;

   while(handle)
   {
      if(EqnIsNegative(handle)&&!EqnIsMaximal(handle))
      {
    weight = lit_sel_diff_weight(handle);
    if(weight > select_weight)
    {
       select_weight = weight;
       selected = handle;
    }
      }
      handle = handle->next;
   }
   return selected;
}



/*-----------------------------------------------------------------------
//
// Function: find_smallest_neg_ground_lit()
//
//   Return smallest negative ground literal, or NULL if no negative
//   ground literal exists.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static Eqn_p find_smallest_neg_ground_lit(Clause_p clause)
{
   Eqn_p handle = clause->literals, selected = NULL;
   long select_weight = LONG_MAX;

   while(handle)
   {
      if(EqnIsNegative(handle)&&
    EqnIsGround(handle))
      {
    assert(EqnIsOriented(handle));

    if(EqnStandardWeight(handle) < select_weight)
    {
       selected = handle;
       select_weight = EqnStandardWeight(handle);
    }
      }
      handle = handle->next;
   }
   return selected;
}


/*-----------------------------------------------------------------------
//
// Function: find_smallest_max_neg_ground_lit()
//
//   Return the ground literal with the smallest maximal side. Assumes
//   that all literals have been oriented (if possible).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static Eqn_p find_smallest_max_neg_ground_lit(Clause_p clause)
{
   Eqn_p handle = clause->literals, selected = NULL;
   long select_weight = LONG_MAX;

   while(handle)
   {
      if(EqnIsNegative(handle)&&
    EqnIsGround(handle))
      {
    /* assert(EqnIsOriented(handle)); Only true if we don't run
          * into LPORecursionDepthLimit */

    if(handle->lterm->weight < select_weight)
    {
       selected = handle;
       select_weight = handle->lterm->weight;
    }
      }
      handle = handle->next;
   }
   return selected;
}


/*-----------------------------------------------------------------------
//
// Function: find_ng_min11_infpos_no_xtype_lit()
//
//   Return the non-ground, non-xpos literal with the smallest number
//   of inference positions.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static Eqn_p find_ng_min11_infpos_no_xtype_lit(Clause_p clause)
{
   Eqn_p handle = clause->literals, selected = NULL;
   long select_weight = LONG_MAX, weight;

   while(handle)
   {
      if(EqnIsNegative(handle)&&
    !EqnIsGround(handle)&&
    !EqnIsXTypePred(handle))
      {
    weight = TermWeight(handle->lterm,1,1);
    if(!EqnIsOriented(handle))
    {
       weight += TermWeight(handle->rterm,1,1);
    }
    if(weight < select_weight)
    {
       select_weight = weight;
       selected = handle;
    }
      }
      handle = handle->next;
   }
   return selected;
}



/*-----------------------------------------------------------------------
//
// Function: find_max_xtype_no_type_lit()
//
//   Return the biggest xtype literal (but never a type literal).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static Eqn_p find_max_xtype_no_type_lit(Clause_p clause)
{
   Eqn_p handle = clause->literals, selected = NULL;
   long select_weight = -1, weight;

   while(handle)
   {
      if(EqnIsNegative(handle)&&
    EqnIsXTypePred(handle) &&
    !EqnIsTypePred(handle))
      {
    assert(EqnIsOriented(handle));

    weight = handle->lterm->weight;
    if(weight > select_weight)
    {
       select_weight = weight;
       selected = handle;
    }
      }
      handle = handle->next;
   }
   return selected;
}




/*-----------------------------------------------------------------------
//
// Function: clause_select_pos()
//
//   Select all positive literals in clause.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void clause_select_pos(Clause_p clause)
{
   Eqn_p handle=clause->literals;

   assert(clause->neg_lit_no);

   while(handle)
   {
      if(EqnIsPositive(handle))
      {
    EqnSetProp(handle, EPIsSelected);
      }
      handle = handle->next;
   }
}


/*-----------------------------------------------------------------------
//
// Function: lit_eval_compare()
//
//   Return integer smaller than 0, 0, or int > than zero if le1 is
//   smaller, equal to, or larger than le2 (by weight). Highest
//   priority is implicit sign!
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int lit_eval_compare(const void* le1, const void* le2)
{
   const LitEval_p eval1 = (const LitEval_p) le1;
   const LitEval_p eval2 = (const LitEval_p) le2;
   int res;

   res = EqnIsPositive(eval1->literal)-EqnIsPositive(eval2->literal);
   if(res)
   {
      return res;
   }
   res = eval1->w1 - eval2->w1;
   if(res)
   {
      return res;
   }
   res = eval1->w2 - eval2->w2;
   if(res)
   {
      return res;
   }
   res = eval1->w3 - eval2->w3;
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: generic_uniq_selection()
//
//   Function implementing generic weight-based selection for cases
//   where at most one negative literal is selected (the one which is
//   assigned minimal weight by weight_fun).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void generic_uniq_selection(OCB_p ocb, Clause_p clause, bool positive,
                            bool needs_ordering,
                            LitWeightFun weight_fun, void* data)
{
   int       len  = ClauseLiteralNumber(clause);
   LitEval_p lits, tmp;
   int i, cand;
   Eqn_p handle;
   bool selected = false;

   assert(ocb);
   assert(clause);
   assert(clause->neg_lit_no);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   lits = SizeMalloc(len*sizeof(LitEvalCell));
   if(needs_ordering)
   {
      ClauseCondMarkMaximalTerms(ocb, clause);
   }
   for(handle=clause->literals, i=0; handle; handle=handle->next,i++)
   {
      lits[i].literal = handle;
      tmp = &(lits[i]);
      LitEvalInit(tmp);
      assert(clause);
      weight_fun(tmp, clause, data);
   }
   cand = 0;
   for(handle=clause->literals->next, i=1; handle; handle=handle->next,i++)
   {
      if(lit_eval_compare(&(lits[i]),&(lits[cand]))<0)
      {
         cand = i;
      }
   }
   /*printf("cand: %d :",cand);
     ClausePrint(stdout, clause, true);
     printf("\n");*/

   assert(EqnIsNegative(lits[cand].literal));
   if(!lits[cand].forbidden)
   {
      EqnSetProp(lits[cand].literal, EPIsSelected);
      selected = true;
      ClauseDelProp(clause, CPIsOriented);
   }
   SizeFree(lits,len*sizeof(LitEvalCell));
   if(positive && selected)
   {
      clause_select_pos(clause);
   }
}

/*-----------------------------------------------------------------------
//
// Function: pos_pred_dist_array_compute()
//
//   Compute a distribution array of predicate symbols (or
//   uninterpreted predicate symbols in positive literals of clause.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/


static PDArray_p pos_pred_dist_array_compute(Clause_p clause)
{
   PDArray_p pred_dist;
   Eqn_p     handle;

   pred_dist = PDIntArrayAlloc(10,30);

   for(handle = clause->literals;
       handle && EqnIsPositive(handle);
       handle = handle->next)
   {
      PDArrayElementIncInt(pred_dist, EqnGetPredCode(handle), 1);
   }
   return pred_dist;
}

#define pred_dist_array_free(array) PDArrayFree(array)

/*-----------------------------------------------------------------------
//
// Function: generic_app_var_sel()
//
//   Factors out computation needed for Avoid/PrefferAppVar family of
//   functions.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/
void generic_app_var_sel(OCB_p ocb, Clause_p clause, LitWeightFun* fun)
{
   long  lit_no;
   PDArray_p pred_dist;

   assert(ocb);
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no==0)
   {
      return;
   }
   ClauseCondMarkMaximalTerms(ocb, clause);

   lit_no = EqnListQueryPropNumber(clause->literals, EPIsMaximal);

   if(lit_no <=1)
   {
      return;
   }
   pred_dist = pos_pred_dist_array_compute(clause);
   generic_uniq_selection(ocb,clause,false, true,
                           fun, pred_dist);
   pred_dist_array_free(pred_dist);
}



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: GetLitSelFun()
//
//   Given an external name, return a literal selection function or
//   NULL if the name does not match any known function.
//
// Global Variables: LiteralSelectionFunNames, litsel_fun_array
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

LiteralSelectionFun GetLitSelFun(char* name)
{
   int i;

   assert(name);

   for(i=0; name_fun_assoc[i].name; i++)
   {
      if(strcmp(name, name_fun_assoc[i].name)==0)
      {
         return name_fun_assoc[i].fun;
      }
   }
   return (LiteralSelectionFun)0;
}


/*-----------------------------------------------------------------------
//
// Function: GetLitSelName()
//
//   Given a LiteralSelectionFun, return the corresponding
//   name. Fails/Undefined, if function is not found.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

char* GetLitSelName(LiteralSelectionFun fun)
{
   int  i;
   char *res = NULL;

   assert(fun);
   for(i=0; name_fun_assoc[i].name; i++)
   {
      if(name_fun_assoc[i].fun == fun)
      {
    res = name_fun_assoc[i].name;
    assert(res);
    break;
      }
   }
   assert(res);
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: LitSelAppendNames()
//
//   Append all valid literal selection function names
//   (comma-separated) to str.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void LitSelAppendNames(DStr_p str)
{
   int i;
   char* sel = "";

   for(i=0; name_fun_assoc[i].name; i++)
   {
      DStrAppendStr(str, sel);
      DStrAppendStr(str, name_fun_assoc[i].name);
      sel = ", ";
   }
}



/*-----------------------------------------------------------------------
//
// Function: SelectNoLiterals()
//
//   Unselect all literals (now a dummy, this is done further up).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void SelectNoLiterals(OCB_p ocb, Clause_p clause)
{
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);
}

/*-----------------------------------------------------------------------
//
// Function: SelectNoGeneration()
//
//   Do nothing with a clause.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void SelectNoGeneration(OCB_p ocb, Clause_p clause)
{
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);
}


/*-----------------------------------------------------------------------
//
// Function: SelectNegativeLiterals()
//
//   If the clause has negative literals, mark them as selected.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void SelectNegativeLiterals(OCB_p ocb, Clause_p clause)
{
   Eqn_p handle = clause->literals;

   while(handle)
   {
      if(!EqnIsPositive(handle))
      {
         EqnSetProp(handle, EPIsSelected);
      }
      handle = handle->next;
   }
}


/*-----------------------------------------------------------------------
//
// Function: PSelectNegativeLiterals()
//
//   If the clause has negative literals, mark all literals as
//   selected.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void PSelectNegativeLiterals(OCB_p ocb, Clause_p clause)
{
   Eqn_p handle = clause->literals;

   while(handle)
   {
      EqnSetProp(handle, EPIsSelected);
      handle = handle->next;
   }
}


/*-----------------------------------------------------------------------
//
// Function: SelectFirstVariableLiteral()
//
//   Select first literal of the form X!=Y.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void SelectFirstVariableLiteral(OCB_p ocb, Clause_p clause)
{
   Eqn_p handle = ClauseFindNegPureVarLit(clause);

   if(handle)
   {
      EqnSetProp(handle, EPIsSelected);
   }
}


/*-----------------------------------------------------------------------
//
// Function: PSelectFirstVariableLiteral()
//
//   If a literal of the form X!=Y exist, select it and all positive
//   literals. Otherwise unselect all literals.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void PSelectFirstVariableLiteral(OCB_p ocb, Clause_p clause)
{
   Eqn_p handle = ClauseFindNegPureVarLit(clause);

   if(handle)
   {
      clause_select_pos(clause);
   }
}


/*-----------------------------------------------------------------------
//
// Function: SelectLargestNegativeLiteral()
//
//   Select the largest of the clauses negative literals.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void SelectLargestNegativeLiteral(OCB_p ocb, Clause_p clause)
{
   Eqn_p handle = clause->literals, selected = NULL;
   long  select_weight = 0;

   while(handle)
   {
      if(EqnIsNegative(handle))
      {
         if(EqnStandardWeight(handle) > select_weight)
         {
            select_weight = EqnStandardWeight(handle);
            selected = handle;
         }
      }
      handle = handle->next;
   }
   assert(selected);
   EqnSetProp(selected, EPIsSelected);
}


/*-----------------------------------------------------------------------
//
// Function: PSelectLargestNegativeLiteral()
//
//   If clause has negative literals, select the largest of the
//   clauses negative literals and positive literals.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void PSelectLargestNegativeLiteral(OCB_p ocb, Clause_p clause)
{
   Eqn_p handle = clause->literals, selected = NULL;
   long  select_weight = 0;

   while(handle)
   {
      if(EqnIsPositive(handle))
      {
         EqnSetProp(handle, EPIsSelected);
      }
      else
      {
         if(EqnStandardWeight(handle) > select_weight)
         {
            select_weight = EqnStandardWeight(handle);
            selected = handle;
         }
      }
      handle = handle->next;
   }
   assert(selected);
   EqnSetProp(selected, EPIsSelected);
}



/*-----------------------------------------------------------------------
//
// Function: SelectSmallestNegativeLiteral()
//
//   Select the smallest of the clauses negative literals.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void SelectSmallestNegativeLiteral(OCB_p ocb, Clause_p clause)
{
   Eqn_p handle = clause->literals, selected = NULL;
   long  select_weight = LONG_MAX;

   while(handle)
   {
      if(EqnIsNegative(handle))
      {
         if(EqnStandardWeight(handle) < select_weight)
         {
            select_weight = EqnStandardWeight(handle);
            selected = handle;
         }
      }
      handle = handle->next;
   }
   assert(selected);
   EqnSetProp(selected, EPIsSelected);
}

/*-----------------------------------------------------------------------
//
// Function: PSelectSmallestNegativeLiteral()
//
//   If clause has negative literals, select the smallest of the
//   clauses negative literals and positive literals.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void PSelectSmallestNegativeLiteral(OCB_p ocb, Clause_p clause)
{
   Eqn_p handle = clause->literals, selected = NULL;
   long  select_weight = LONG_MAX;

   while(handle)
   {
      if(EqnIsPositive(handle))
      {
         EqnSetProp(handle, EPIsSelected);
      }
      else
      {
         if(EqnStandardWeight(handle) < select_weight)
         {
            select_weight = EqnStandardWeight(handle);
            selected = handle;
         }
      }
      handle = handle->next;
   }
   assert(selected);
   EqnSetProp(selected, EPIsSelected);
}


/*-----------------------------------------------------------------------
//
// Function: SelectLargestOrientableLiteral()
//
//   If there is at least one negative orientable literal, select the
//   largest one, otherwise select the largest one.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void SelectLargestOrientableLiteral(OCB_p ocb, Clause_p clause)
{
   Eqn_p handle = clause->literals, selected = NULL;
   long  select_weight = 0;
   bool  oriented = false;

   ClauseCondMarkMaximalTerms(ocb, clause);

   while(handle)
   {
    if(EqnIsNegative(handle))
    {
       if((!oriented && EqnIsOriented(handle))
          ||
          (
        ((oriented&&EqnIsOriented(handle))||(!oriented&&!EqnIsOriented(handle)))
        &&
        (EqnStandardWeight(handle) > select_weight)
        ))
       {
          select_weight = EqnStandardWeight(handle);
          oriented = EqnIsOriented(handle);
          selected = handle;
       }
    }
    handle = handle->next;
   }
   assert(selected);
   EqnSetProp(selected, EPIsSelected);
   ClauseDelProp(clause, CPIsOriented);
}


/*-----------------------------------------------------------------------
//
// Function: PSelectLargestOrientableLiteral()
//
//   If there is at least one negative orientable literal, select the
//   largest one, otherwise select the largest one. Also select
//   positive literals.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void PSelectLargestOrientableLiteral(OCB_p ocb, Clause_p clause)
{
   Eqn_p handle = clause->literals, selected = NULL;
   long  select_weight = 0;
   bool  oriented = false;

   ClauseCondMarkMaximalTerms(ocb, clause);

   while(handle)
   {
      if(EqnIsPositive(handle))
      {
         EqnSetProp(handle, EPIsSelected);
      }
      else
      {
         if((!oriented && EqnIsOriented(handle))
            ||
            (
               ((oriented&&EqnIsOriented(handle))||(!oriented&&!EqnIsOriented(handle)))
        &&
               (EqnStandardWeight(handle) > select_weight)
               ))
         {
            select_weight = EqnStandardWeight(handle);
            oriented = EqnIsOriented(handle);
            selected = handle;
         }
      }
      handle = handle->next;
   }
   assert(selected);
   EqnSetProp(selected, EPIsSelected);
   ClauseDelProp(clause, CPIsOriented);
}


/*-----------------------------------------------------------------------
//
// Function: MSelectLargestOrientableLiteral()
//
//   For horn clauses, call PSelectLargestOrientableLiteral,
//   otherwise call SelectLargestOrientableLiteral.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void MSelectLargestOrientableLiteral(OCB_p ocb, Clause_p clause)
{
   if(ClauseIsHorn(clause))
   {
      PSelectLargestOrientableLiteral(ocb,clause);
   }
   else
   {
      SelectLargestOrientableLiteral(ocb,clause);
   }
}


/*-----------------------------------------------------------------------
//
// Function: SelectSmallestOrientableLiteral()
//
//   If there is at least one negative orientable literal, select the
//   smallest one, otherwise select the largest one.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void SelectSmallestOrientableLiteral(OCB_p ocb, Clause_p clause)
{
   Eqn_p handle = clause->literals, selected = NULL;
   long  select_weight = LONG_MAX;
   bool  oriented = false;

   ClauseCondMarkMaximalTerms(ocb, clause);

   while(handle)
   {
      if(EqnIsNegative(handle))
      {
         if((!oriented && EqnIsOriented(handle))
            ||
            (
               ((oriented&&EqnIsOriented(handle))||(!oriented&&!EqnIsOriented(handle)))
        &&
               (EqnStandardWeight(handle) < select_weight)
               ))
         {
            select_weight = EqnStandardWeight(handle);
            oriented = EqnIsOriented(handle);
            selected = handle;
         }
      }
      handle = handle->next;
   }
   assert(selected);
   EqnSetProp(selected, EPIsSelected);
   ClauseDelProp(clause, CPIsOriented);
}


/*-----------------------------------------------------------------------
//
// Function: PSelectSmallestOrientableLiteral()
//
//   If there is at least one negative orientable literal, select the
//   smallest one, otherwise select the largest one. Also select
//   positive literals.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void PSelectSmallestOrientableLiteral(OCB_p ocb, Clause_p clause)
{
   Eqn_p handle = clause->literals, selected = NULL;
   long  select_weight = LONG_MAX;
   bool  oriented = false;

   ClauseCondMarkMaximalTerms(ocb, clause);

   while(handle)
   {
      if(EqnIsPositive(handle))
      {
         EqnSetProp(handle, EPIsSelected);
      }
      else
      {
         if((!oriented && EqnIsOriented(handle))
            ||
            (
               ((oriented&&EqnIsOriented(handle))||(!oriented&&!EqnIsOriented(handle)))
               &&
        (EqnStandardWeight(handle) < select_weight)
               ))
         {
            select_weight = EqnStandardWeight(handle);
            oriented = EqnIsOriented(handle);
            selected = handle;
         }
      }
      handle = handle->next;
   }
   assert(selected);
   EqnSetProp(selected, EPIsSelected);
   ClauseDelProp(clause, CPIsOriented);
}


/*-----------------------------------------------------------------------
//
// Function: MSelectSmallestOrientableLiteral()
//
//   For horn clauses, call PSelectSmallestOrientableLiteral,
//   otherwise call SelectSmallestOrientableLiteral.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void MSelectSmallestOrientableLiteral(OCB_p ocb, Clause_p clause)
{
   if(ClauseIsHorn(clause))
   {
      PSelectSmallestOrientableLiteral(ocb,clause);
   }
   else
   {
      SelectSmallestOrientableLiteral(ocb,clause);
   }
}


/*-----------------------------------------------------------------------
//
// Function: SelectDiffNegativeLiteral()
//
//   Select the most unbalanced of the clauses negative literals.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void SelectDiffNegativeLiteral(OCB_p ocb, Clause_p clause)
{
   Eqn_p handle = clause->literals, selected = NULL;
   long  select_weight = -1;

   while(handle)
   {
      if(EqnIsNegative(handle))
      {
         if(lit_sel_diff_weight(handle) > select_weight)
         {
            select_weight = lit_sel_diff_weight(handle);
            selected = handle;
         }
      }
      handle = handle->next;
   }

   if(selected)
   {
      EqnSetProp(selected, EPIsSelected);
   }
}


/*-----------------------------------------------------------------------
//
// Function: PSelectDiffNegativeLiteral()
//
//   If clause has negative literals, select the most unbalanced one
//   of the clauses negative literals and all positive literals.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void PSelectDiffNegativeLiteral(OCB_p ocb, Clause_p clause)
{
   Eqn_p handle = clause->literals, selected = NULL;
   long  select_weight = -1;

   while(handle)
   {
      if(EqnIsPositive(handle))
      {
         EqnSetProp(handle, EPIsSelected);
      }
      else
      {
         if(lit_sel_diff_weight(handle) > select_weight)
         {
            select_weight = lit_sel_diff_weight(handle);
            selected = handle;
         }
      }
      handle = handle->next;
   }
   assert(selected);
   EqnSetProp(selected, EPIsSelected);
}


/*-----------------------------------------------------------------------
//
// Function: SelectGroundNegativeLiteral()
//
//   If there are negative ground literals, select the one with maximal
//   lit_sel_diff_weight.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void SelectGroundNegativeLiteral(OCB_p ocb, Clause_p clause)
{
   Eqn_p handle = clause->literals, selected = NULL;
   long  select_weight = -1;

   while(handle)
   {
      if(EqnIsNegative(handle) && EqnIsGround(handle))
      {
         if(lit_sel_diff_weight(handle) > select_weight)
         {
            select_weight = lit_sel_diff_weight(handle);
            selected = handle;
         }
      }
      handle = handle->next;
   }
   if(selected)
   {
      EqnSetProp(selected, EPIsSelected);
   }
}


/*-----------------------------------------------------------------------
//
// Function: PSelectGroundNegativeLiteral()
//
//   If there are negative ground literals, select the one with maximal
//   lit_sel_diff_weight and select all positive literals.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void PSelectGroundNegativeLiteral(OCB_p ocb, Clause_p clause)
{
   Eqn_p handle = clause->literals, selected = NULL;
   long  select_weight = -1;

   while(handle)
   {
      if(EqnIsNegative(handle))
      {
         if(EqnIsGround(handle))
         {
            if(lit_sel_diff_weight(handle) > select_weight)
            {
               select_weight = lit_sel_diff_weight(handle);
               selected = handle;
            }
         }
      }
      else
      {
         assert(EqnIsPositive(handle));
         EqnSetProp(handle, EPIsSelected);
      }
      handle = handle->next;
   }
   if(selected)
   {
      EqnSetProp(selected, EPIsSelected);
   }
   else
   {
      EqnListDelProp(clause->literals, EPIsSelected);
   }
}


/*-----------------------------------------------------------------------
//
// Function: SelectOptimalLiteral()
//
//   (Hah! Believe it at your peril ;-). If there is a ground negative
//   literal, select it, otherwise select the negative literal with the
//   largest size difference.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void SelectOptimalLiteral(OCB_p ocb, Clause_p clause)
{
   Eqn_p handle = clause->literals, selected = NULL;
   long  select_weight = -1;

   while(handle)
   {
      if(EqnIsNegative(handle) && EqnIsGround(handle))
      {
         if(lit_sel_diff_weight(handle) > select_weight)
         {
            select_weight = lit_sel_diff_weight(handle);
            selected = handle;
         }
      }
      handle = handle->next;
   }
   if(selected)
   {
      EqnSetProp(selected, EPIsSelected);
   }
   else
   {
      SelectDiffNegativeLiteral(ocb,clause);
   }
}



/*-----------------------------------------------------------------------
//
// Function: PSelectOptimalLiteral()
//
//   (Hah! Believe it at your peril ;-). If there is a ground negative
//   literal, select it, otherwise select the negative literal with the
//   largest size difference and all positive literals.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void PSelectOptimalLiteral(OCB_p ocb, Clause_p clause)
{
   Eqn_p handle = clause->literals, selected = NULL;
   long  select_weight = -1;

   while(handle)
   {
      if(EqnIsNegative(handle) && EqnIsGround(handle))
      {
         if(lit_sel_diff_weight(handle) > select_weight)
         {
            select_weight = lit_sel_diff_weight(handle);
            selected = handle;
         }
      }
      handle = handle->next;
   }
   if(selected)
   {
      EqnSetProp(selected, EPIsSelected);
   }
   else
   {
      PSelectDiffNegativeLiteral(ocb,clause);
   }
}


/*-----------------------------------------------------------------------
//
// Function: SelectMinOptimalLiteral()
//
//   If there is a ground negative
//   literal, select it, otherwise select the smallest negative
//   literal.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void SelectMinOptimalLiteral(OCB_p ocb, Clause_p clause)
{
   Eqn_p handle = clause->literals, selected = NULL;
   long  select_weight = LONG_MAX, weight;

   while(handle)
   {
      if(EqnIsNegative(handle) && EqnIsGround(handle))
      {
         weight = EqnStandardWeight(handle);
         if(weight < select_weight)
         {
            select_weight = weight;
            selected = handle;
         }
      }
      handle = handle->next;
   }
   if(selected)
   {
      EqnSetProp(selected, EPIsSelected);
   }
   else
   {
      SelectSmallestNegativeLiteral(ocb,clause);
   }
}


/*-----------------------------------------------------------------------
//
// Function: PSelectMinOptimalLiteral()
//
//   If there is a ground negative
//   literal, select it, otherwise select the smallest negative
//   literal and positive literals.
//
// Global Variables: -
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void PSelectMinOptimalLiteral(OCB_p ocb, Clause_p clause)
{
   Eqn_p handle = clause->literals, selected = NULL;
   long  select_weight = LONG_MAX, weight;

   while(handle)
   {
      if(EqnIsNegative(handle) && EqnIsGround(handle))
      {
         weight = EqnStandardWeight(handle);
         if(weight < select_weight)
         {
            select_weight = weight;
            selected = handle;
         }
      }
      handle = handle->next;
   }
   if(selected)
   {
      EqnSetProp(selected, EPIsSelected);
   }
   else
   {
      PSelectSmallestNegativeLiteral(ocb,clause);
   }
}


/*-----------------------------------------------------------------------
//
// Function: SelectMinOptimalNoTypePred()
//
//   If there is a ground negative
//   literal, select it, otherwise select the smallest negative
//   literal, but never select type literals.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void SelectMinOptimalNoTypePred(OCB_p ocb, Clause_p clause)
{
   Eqn_p selected = NULL;

   selected = find_smallest_neg_ground_lit(clause);

   if(!selected)
   {
      Eqn_p handle = clause->literals;
      long select_weight = LONG_MAX, weight;
      while(handle)
      {
         if(EqnIsNegative(handle) && !EqnIsTypePred(handle))
         {
            weight = EqnStandardWeight(handle);
            if(weight < select_weight)
            {
               select_weight = weight;
               selected = handle;
            }
         }
         handle = handle->next;
      }
   }
   if(selected)
   {
      EqnSetProp(selected, EPIsSelected);
      ClauseDelProp(clause, CPIsOriented);
   }
}


/*-----------------------------------------------------------------------
//
// Function: PSelectMinOptimalNoTypePred()
//
//   If there is a ground negative
//   literal, select it, otherwise select the smallest negative
//   literal, but never select type literals. If a negative literal is
//   selected, also select positive ones.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void PSelectMinOptimalNoTypePred(OCB_p ocb, Clause_p clause)
{
   Eqn_p selected = NULL;

   selected = find_smallest_neg_ground_lit(clause);

   if(!selected)
   {
      Eqn_p handle = clause->literals;
      long select_weight = LONG_MAX, weight;
      while(handle)
      {
         if(EqnIsNegative(handle) && !EqnIsTypePred(handle))
         {
            weight = EqnStandardWeight(handle);
            if(weight < select_weight)
            {
               select_weight = weight;
               selected = handle;
            }
         }
         handle = handle->next;
      }
   }
   if(selected)
   {
      EqnSetProp(selected, EPIsSelected);
      clause_select_pos(clause);
      ClauseDelProp(clause, CPIsOriented);
   }
}


/*-----------------------------------------------------------------------
//
// Function: SelectMinOptimalNoXTypePred()
//
//   If there is a ground negative
//   literal, select it, otherwise select the smallest negative
//   literal, but never select extendet type literals.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void SelectMinOptimalNoXTypePred(OCB_p ocb, Clause_p clause)
{
   Eqn_p selected = NULL;

   selected = find_smallest_neg_ground_lit(clause);

   if(!selected)
   {
      Eqn_p handle = clause->literals;
      long select_weight = LONG_MAX, weight;
      while(handle)
      {
         if(EqnIsNegative(handle) && !EqnIsXTypePred(handle))
         {
            weight = EqnStandardWeight(handle);
            if(weight < select_weight)
            {
               select_weight = weight;
               selected = handle;
            }
         }
         handle = handle->next;
      }
   }
   if(selected)
   {
      EqnSetProp(selected, EPIsSelected);
      ClauseDelProp(clause, CPIsOriented);
   }
}


/*-----------------------------------------------------------------------
//
// Function: PSelectMinOptimalNoXTypePred()
//
//   If there is a ground negative
//   literal, select it, otherwise select the smallest negative
//   literal, but never select extendet type literals. If a negative
//   literal is selected, also select positive ones.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void PSelectMinOptimalNoXTypePred(OCB_p ocb, Clause_p clause)
{
   Eqn_p selected = NULL;

   selected = find_smallest_neg_ground_lit(clause);

   if(!selected)
   {
      Eqn_p handle = clause->literals;
      long select_weight = LONG_MAX, weight;
      while(handle)
      {
         if(EqnIsNegative(handle) && !EqnIsXTypePred(handle))
         {
            weight = EqnStandardWeight(handle);
            if(weight < select_weight)
            {
               select_weight = weight;
               selected = handle;
            }
         }
         handle = handle->next;
      }
   }
   if(selected)
   {
      EqnSetProp(selected, EPIsSelected);
      clause_select_pos(clause);
      ClauseDelProp(clause, CPIsOriented);
   }
}


/*-----------------------------------------------------------------------
//
// Function: SelectMinOptimalNoRXTypePred()
//
//   If there is a ground negative
//   literal, select it, otherwise select the smallest negative
//   literal, but never select real extended type literals.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void SelectMinOptimalNoRXTypePred(OCB_p ocb, Clause_p clause)
{
   Eqn_p selected = NULL;

   selected = find_smallest_neg_ground_lit(clause);

   if(!selected)
   {
      Eqn_p handle = clause->literals;
      long select_weight = LONG_MAX, weight;
      while(handle)
      {
         if(EqnIsNegative(handle) && !EqnIsRealXTypePred(handle))
         {
            weight = EqnStandardWeight(handle);
            if(weight < select_weight)
            {
               select_weight = weight;
               selected = handle;
            }
         }
         handle = handle->next;
      }
   }
   if(selected)
   {
      EqnSetProp(selected, EPIsSelected);
      ClauseDelProp(clause, CPIsOriented);
   }
}


/*-----------------------------------------------------------------------
//
// Function: PSelectMinOptimalNoRXTypePred()
//
//   If there is a ground negative
//   literal, select it, otherwise select the smallest negative
//   literal, but never select real extendet type literals. If a
//   negative literal is selected, also select positive ones.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void PSelectMinOptimalNoRXTypePred(OCB_p ocb, Clause_p clause)
{
   Eqn_p selected = NULL;

   selected = find_smallest_neg_ground_lit(clause);

   if(!selected)
   {
      Eqn_p handle = clause->literals;
      long select_weight = LONG_MAX, weight;
      while(handle)
      {
         if(EqnIsNegative(handle) && !EqnIsRealXTypePred(handle))
         {
            weight = EqnStandardWeight(handle);
            if(weight < select_weight)
            {
               select_weight = weight;
               selected = handle;
            }
         }
         handle = handle->next;
      }
   }
   if(selected)
   {
      EqnSetProp(selected, EPIsSelected);
      clause_select_pos(clause);
      ClauseDelProp(clause, CPIsOriented);
   }
}


/*-----------------------------------------------------------------------
//
// Function: SelectCondOptimalLiteral()
//
//   As above, but if the clause has a positive literal that is very
//   uninstantiated, select no literal at all.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

#define VAR_FACTOR 3

void SelectCondOptimalLiteral(OCB_p ocb, Clause_p clause)
{
   Eqn_p handle = clause->literals;
   bool found = false;
   long weight, sweight;

   while(handle)
   {
      if(EqnIsPositive(handle))
      {
         weight = TermWeight(handle->lterm, 0,VAR_FACTOR);
         sweight = TermStandardWeight(handle->lterm);

         if(EqnIsEquLit(handle))
         {
            weight += TermWeight(handle->rterm, 0,VAR_FACTOR);
            sweight += TermStandardWeight(handle->rterm);
         }
         if(sweight <= weight)
         {
            found = true;
            break;
         }
      }
      handle = handle->next;
   }
   if(!found)
   {
      SelectOptimalLiteral(ocb,clause);
   }
   else
   {
      EqnListDelProp(clause->literals, EPIsSelected);
   }
}


/*-----------------------------------------------------------------------
//
// Function: PSelectCondOptimalLiteral()
//
//   As above, but select positive literals as well.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void PSelectCondOptimalLiteral(OCB_p ocb, Clause_p clause)
{
   Eqn_p handle = clause->literals;
   bool found = false;
   long weight, sweight;

   while(handle)
   {
      if(EqnIsPositive(handle))
      {
         weight = TermWeight(handle->lterm, 0,VAR_FACTOR);
         sweight = TermStandardWeight(handle->lterm);

         if(EqnIsEquLit(handle))
         {
            weight += TermWeight(handle->rterm, 0,VAR_FACTOR);
            sweight += TermStandardWeight(handle->rterm);
         }
         if(sweight <= weight)
         {
            found = true;
            break;
         }
      }
      handle = handle->next;
   }
   if(!found)
   {
      PSelectOptimalLiteral(ocb,clause);
   }
   else
   {
      EqnListDelProp(clause->literals, EPIsSelected);
   }
}


/*-----------------------------------------------------------------------
//
// Function: SelectAllCondOptimalLiteral()
//
//   As above, but if the clause has only positive literals that are very
//   uninstantiated, select no literal at all.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void SelectAllCondOptimalLiteral(OCB_p ocb, Clause_p clause)
{
   Eqn_p handle = clause->literals;
   bool found = true;
   long weight, sweight;

   while(handle)
   {
      if(EqnIsPositive(handle))
      {
         weight = TermWeight(handle->lterm, 0,VAR_FACTOR);
         sweight = TermStandardWeight(handle->lterm);

         if(EqnIsEquLit(handle))
         {
            weight += TermWeight(handle->rterm, 0,VAR_FACTOR);
            sweight += TermStandardWeight(handle->rterm);
         }
         if(sweight > weight)
         {
            found = false;
            break;
         }
      }
      handle = handle->next;
   }
   if(!found)
   {
      SelectOptimalLiteral(ocb,clause);
   }
   else
   {
      EqnListDelProp(clause->literals, EPIsSelected);
   }
}


/*-----------------------------------------------------------------------
//
// Function: PSelectAllCondOptimalLiteral()
//
//   As above, but select positive literals as well.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void PSelectAllCondOptimalLiteral(OCB_p ocb, Clause_p clause)
{
   Eqn_p handle = clause->literals;
   bool found = true;
   long weight, sweight;

   while(handle)
   {
      if(EqnIsPositive(handle))
      {
         weight = TermWeight(handle->lterm, 0,VAR_FACTOR);
         sweight = TermStandardWeight(handle->lterm);

         if(EqnIsEquLit(handle))
         {
            weight += TermWeight(handle->rterm, 0,VAR_FACTOR);
            sweight += TermStandardWeight(handle->rterm);
         }
         if(sweight > weight)
         {
            found = false;
            break;
         }
      }
      handle = handle->next;
   }
   if(!found)
   {
      PSelectOptimalLiteral(ocb,clause);
   }
   else
   {
      EqnListDelProp(clause->literals, EPIsSelected);
   }
}


/*-----------------------------------------------------------------------
//
// Function: SelectDepth2OptimalLiteral()
//
//   Select optimal literal unless there is a literal with depth <= 2,
//   then select no literal.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void SelectDepth2OptimalLiteral(OCB_p ocb, Clause_p clause)
{
   Eqn_p handle = clause->literals;
   bool found = false;

   while(handle)
   {
      if(EqnDepth(handle)<=2)
      {
         found = true;
         break;
      }
      handle = handle->next;
   }
   if(!found)
   {
      SelectOptimalLiteral(ocb,clause);
   }
   else
   {
      EqnListDelProp(clause->literals, EPIsSelected);
   }
}


/*-----------------------------------------------------------------------
//
// Function: PSelectDepth2OptimalLiteral()
//
//   As above, but select positive literals as well.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void PSelectDepth2OptimalLiteral(OCB_p ocb, Clause_p clause)
{
   Eqn_p handle = clause->literals;
   bool found = false;

   while(handle)
   {
      if(EqnDepth(handle)<=2)
      {
         found = true;
         break;
      }
      handle = handle->next;
   }
   if(!found)
   {
      PSelectOptimalLiteral(ocb,clause);
   }
   else
   {
      EqnListDelProp(clause->literals, EPIsSelected);
    }
}


/*-----------------------------------------------------------------------
//
// Function: SelectPDepth2OptimalLiteral()
//
//   Select optimal literal unless there is a positive literal with
//   depth <= 2, then select no literal.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void SelectPDepth2OptimalLiteral(OCB_p ocb, Clause_p clause)
{
   Eqn_p handle = clause->literals;
   bool found = false;

   while(handle)
   {
      if(EqnIsPositive(handle) && (EqnDepth(handle)<=2))
      {
         found = true;
         break;
      }
      handle = handle->next;
   }
   if(!found)
   {
      SelectOptimalLiteral(ocb,clause);
   }
   else
   {
      EqnListDelProp(clause->literals, EPIsSelected);
   }
}


/*-----------------------------------------------------------------------
//
// Function: PSelectPDepth2OptimalLiteral()
//
//   As above, with positive literals.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void PSelectPDepth2OptimalLiteral(OCB_p ocb, Clause_p clause)
{
   Eqn_p handle = clause->literals;
   bool found = false;

   while(handle)
   {
      if(EqnIsPositive(handle) && (EqnDepth(handle)<=2))
      {
         found = true;
         break;
      }
      handle = handle->next;
   }
   if(!found)
   {
      PSelectOptimalLiteral(ocb,clause);
   }
   else
   {
      EqnListDelProp(clause->literals, EPIsSelected);
   }
}


/*-----------------------------------------------------------------------
//
// Function: SelectNDepth2OptimalLiteral()
//
//   Select optimal literal unless there is a negative literal with
//   depth <= 2, then select no literal.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void SelectNDepth2OptimalLiteral(OCB_p ocb, Clause_p clause)
{
   Eqn_p handle = clause->literals;
   bool found = false;

   while(handle)
   {
      if(EqnIsNegative(handle) && (EqnDepth(handle)<=2))
      {
         found = true;
         break;
      }
      handle = handle->next;
   }
   if(!found)
   {
      SelectOptimalLiteral(ocb,clause);
   }
   else
   {
      EqnListDelProp(clause->literals, EPIsSelected);
   }
}


/*-----------------------------------------------------------------------
//
// Function: PSelectNDepth2OptimalLiteral()
//
//   As above, with positive literals.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void PSelectNDepth2OptimalLiteral(OCB_p ocb, Clause_p clause)
{
   Eqn_p handle = clause->literals;
   bool found = false;

   while(handle)
   {
      if(EqnIsNegative(handle) && (EqnDepth(handle)<=2))
      {
         found = true;
         break;
      }
      handle = handle->next;
   }
   if(!found)
   {
      PSelectOptimalLiteral(ocb,clause);
   }
   else
   {
      EqnListDelProp(clause->literals, EPIsSelected);
   }
}


/*-----------------------------------------------------------------------
//
// Function: SelectNonRROptimalLiteral()
//
//   If a clause is not range-restricted, select the optimal literal,
//   otherwise select no literal.
//
// Global Variables: -
//
// Side Effects    : Minimal ;-) (only by called functions)
//
/----------------------------------------------------------------------*/

void SelectNonRROptimalLiteral(OCB_p ocb, Clause_p clause)
{
   if(!ClauseIsRangeRestricted(clause))
   {
      SelectOptimalLiteral(ocb,clause);
   }
}


/*-----------------------------------------------------------------------
//
// Function: PSelectNonRROptimalLiteral()
//
//   If a clause is not range-restricted, select the optimal literal
//   and positive literals, otherwise select no literal.
//
// Global Variables: -
//
// Side Effects    : Minimal ;-) (only by called functions)
//
/----------------------------------------------------------------------*/

void PSelectNonRROptimalLiteral(OCB_p ocb, Clause_p clause)
{
   if(!ClauseIsRangeRestricted(clause))
   {
      PSelectOptimalLiteral(ocb,clause);
   }
}


/*-----------------------------------------------------------------------
//
// Function: SelectNonStrongRROptimalLiteral()
//
//   If a clause is not strongly range-restricted, select the optimal
//   literal, otherwise select no literal.
//
// Global Variables: -
//
// Side Effects    : Minimal ;-) (only by called functions)
//
/----------------------------------------------------------------------*/

void SelectNonStrongRROptimalLiteral(OCB_p ocb, Clause_p clause)
{
   if(!ClauseIsStronglyRangeRestricted(clause))
   {
      SelectOptimalLiteral(ocb,clause);
   }
}


/*-----------------------------------------------------------------------
//
// Function: PSelectNonStrongRROptimalLiteral()
//
//   If a clause is not Strong range-restricted, select the optimal
//   literal and positive literals, otherwise select no literal.
//
// Global Variables: -
//
// Side Effects    : Minimal ;-) (only by called functions)
//
/----------------------------------------------------------------------*/

void PSelectNonStrongRROptimalLiteral(OCB_p ocb, Clause_p clause)
{
   if(!ClauseIsStronglyRangeRestricted(clause))
   {
      PSelectOptimalLiteral(ocb,clause);
   }
}



/*-----------------------------------------------------------------------
//
// Function: SelectAntiRROptimalLiteral()
//
//   If a clause is anti-range-restricted, select the optimal literal,
//   otherwise select no literal.
//
// Global Variables: -
//
// Side Effects    : Minimal ;-) (only by called functions)
//
/----------------------------------------------------------------------*/

void SelectAntiRROptimalLiteral(OCB_p ocb, Clause_p clause)
{
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no==0)
   {
      return;
   }
   if(ClauseIsAntiRangeRestricted(clause))
   {
      SelectOptimalLiteral(ocb,clause);
   }
}


/*-----------------------------------------------------------------------
//
// Function: PSelectAntiRROptimalLiteral()
//
//   If a clause is anti-range-restricted, select the optimal literal
//   and positive literals, otherwise select no literal.
//
// Global Variables: -
//
// Side Effects    : Minimal ;-) (only by called functions)
//
/----------------------------------------------------------------------*/

void PSelectAntiRROptimalLiteral(OCB_p ocb, Clause_p clause)
{
   if(ClauseIsAntiRangeRestricted(clause))
   {
      PSelectOptimalLiteral(ocb,clause);
   }
}


/*-----------------------------------------------------------------------
//
// Function: SelectNonAntiRROptimalLiteral()
//
//   If a clause is not anti-range-restricted, select the optimal
//   literal, otherwise select no literal.
//
// Global Variables: -
//
// Side Effects    : Minimal ;-) (only by called functions)
//
/----------------------------------------------------------------------*/

void SelectNonAntiRROptimalLiteral(OCB_p ocb, Clause_p clause)
{
   if(!ClauseIsAntiRangeRestricted(clause))
   {
      SelectOptimalLiteral(ocb,clause);
   }
}


/*-----------------------------------------------------------------------
//
// Function: PSelectNonAntiRROptimalLiteral()
//
//   If a clause is not anti-range-restricted, select the optimal literal
//   and positive literals, otherwise select no literal.
//
// Global Variables: -
//
// Side Effects    : Minimal ;-) (only by called functions)
//
/----------------------------------------------------------------------*/

void PSelectNonAntiRROptimalLiteral(OCB_p ocb, Clause_p clause)
{
   if(!ClauseIsAntiRangeRestricted(clause))
   {
      PSelectOptimalLiteral(ocb,clause);
   }
}


/*-----------------------------------------------------------------------
//
// Function: SelectStrongRRNonRROptimalLiteral()
//
//   If a clause is not range-restricted or strongly range-restricted
//   select the optimal literal, otherwise select no literal.
//
// Global Variables: -
//
// Side Effects    : Minimal ;-) (only by called functions)
//
/----------------------------------------------------------------------*/

void SelectStrongRRNonRROptimalLiteral(OCB_p ocb, Clause_p clause)
{
   if(!ClauseIsRangeRestricted(clause)||
      ClauseIsStronglyRangeRestricted(clause))
   {
      SelectOptimalLiteral(ocb,clause);
   }
}

/*-----------------------------------------------------------------------
//
// Function: PSelectStrongRRNonRROptimalLiteral()
//
//   If a clause is not range-restricted or strongly range-restricted
//   select the optimal literal and positive literals, otherwise
//   select no literal.
//
// Global Variables: -
//
// Side Effects    : Minimal ;-) (only by called functions)
//
/----------------------------------------------------------------------*/

void PSelectStrongRRNonRROptimalLiteral(OCB_p ocb, Clause_p clause)
{
   if(!ClauseIsRangeRestricted(clause)||
      ClauseIsStronglyRangeRestricted(clause))
   {
      PSelectOptimalLiteral(ocb,clause);
   }
   else
   {
      EqnListDelProp(clause->literals, EPIsSelected);
   }
}


/*-----------------------------------------------------------------------
//
// Function: SelectUnlessUniqMaxOptimalLiteral()
//
//   If a clause has a single maximal literal, do not select,
//   otherwise select the optimal literal.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void SelectUnlessUniqMaxOptimalLiteral(OCB_p ocb, Clause_p clause)
{
   ClauseCondMarkMaximalTerms(ocb, clause);

   if(EqnListQueryPropNumber(clause->literals, EPIsMaximal)>1)
   {
      SelectOptimalLiteral(ocb,clause);
      ClauseDelProp(clause, CPIsOriented);
   }
}

/*-----------------------------------------------------------------------
//
// Function: PSelectUnlessUniqMaxOptimalLiteral()
//
//   If a clause has a single maximal literal, do not select,
//   otherwise select the optimal literal.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void PSelectUnlessUniqMaxOptimalLiteral(OCB_p ocb, Clause_p clause)
{
   ClauseCondMarkMaximalTerms(ocb, clause);

   if(EqnListQueryPropNumber(clause->literals, EPIsMaximal)>1)
   {
      PSelectOptimalLiteral(ocb,clause);
      ClauseDelProp(clause, CPIsOriented);
   }
}


/*-----------------------------------------------------------------------
//
// Function: SelectUnlessUniqMaxSmallestOrientable()
//
//   If a clause has a single maximal literal, do not select,
//   otherwise select the smallest orientable literal.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void SelectUnlessUniqMaxSmallestOrientable(OCB_p ocb, Clause_p clause)
{
   ClauseCondMarkMaximalTerms(ocb, clause);

   if(EqnListQueryPropNumber(clause->literals, EPIsMaximal)>1)
   {
      SelectSmallestOrientableLiteral(ocb,clause);
      ClauseDelProp(clause, CPIsOriented);
   }
}


/*-----------------------------------------------------------------------
//
// Function: PSelectUnlessUniqMaxSmallestOrientable()
//
//   If a clause has a single maximal literal, do not select,
//   otherwise select the smallest orientable literal and all positive
//   ones.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void PSelectUnlessUniqMaxSmallestOrientable(OCB_p ocb, Clause_p clause)
{
   ClauseCondMarkMaximalTerms(ocb, clause);

   if(EqnListQueryPropNumber(clause->literals, EPIsMaximal)>1)
   {
      PSelectSmallestOrientableLiteral(ocb,clause);
      ClauseDelProp(clause, CPIsOriented);
   }
}



/*-----------------------------------------------------------------------
//
// Function: SelectUnlessPosMaxOptimalLiteral()
//
//   If a clause has a positive maximal literal (i.e. is potentially
//   reductive), do not select, otherwise select optimal literal.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void SelectUnlessPosMaxOptimalLiteral(OCB_p ocb, Clause_p clause)
{
   ClauseCondMarkMaximalTerms(ocb, clause);

   if(EqnListQueryPropNumber(clause->literals,
              EPIsMaximal|EPIsPositive)==0)
   {
      SelectOptimalLiteral(ocb,clause);
      ClauseDelProp(clause, CPIsOriented);
   }
}

/*-----------------------------------------------------------------------
//
// Function: PSelectUnlessPosMaxOptimalLiteral()
//
//   If a clause has a positive maximal literal (i.e. is potentially
//   reductive), do not select, otherwise select optimal literal and
//   positive literals.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void PSelectUnlessPosMaxOptimalLiteral(OCB_p ocb, Clause_p clause)
{
   ClauseCondMarkMaximalTerms(ocb, clause);

   if(EqnListQueryPropNumber(clause->literals,
              EPIsMaximal|EPIsPositive)==0)
   {
      PSelectOptimalLiteral(ocb,clause);
      ClauseDelProp(clause, CPIsOriented);
   }
}


/*-----------------------------------------------------------------------
//
// Function: SelectUnlessUniqPosMaxOptimalLiteral()
//
//   If a clause has a uniqe positive maximal literal do not select,
//   otherwise select optimal literal.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void SelectUnlessUniqPosMaxOptimalLiteral(OCB_p ocb, Clause_p clause)
{
   ClauseCondMarkMaximalTerms(ocb, clause);

   if(EqnListQueryPropNumber(clause->literals,
              EPIsMaximal|EPIsPositive)!=1)
   {
      SelectOptimalLiteral(ocb,clause);
      ClauseDelProp(clause, CPIsOriented);
   }
}


/*-----------------------------------------------------------------------
//
// Function: PSelectUnlessUniqPosMaxOptimalLiteral()
//
//   If a clause has a uniqe positive maximal literal do not select,
//   otherwise select optimal literal and positive literals.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void PSelectUnlessUniqPosMaxOptimalLiteral(OCB_p ocb, Clause_p clause)
{
   ClauseCondMarkMaximalTerms(ocb, clause);

   if(EqnListQueryPropNumber(clause->literals,
              EPIsMaximal|EPIsPositive)!=1)
   {
      PSelectOptimalLiteral(ocb,clause);
      ClauseDelProp(clause, CPIsOriented);
   }
}


/*-----------------------------------------------------------------------
//
// Function: SelectUnlessUniqMaxPosOptimalLiteral()
//
//   If a clause has a maximal literal that is positive, do not
//   select, otherwise select optimal literal.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void SelectUnlessUniqMaxPosOptimalLiteral(OCB_p ocb, Clause_p clause)
{
   Eqn_p handle;
   bool found = false;

   ClauseCondMarkMaximalTerms(ocb, clause);

   for(handle=clause->literals; handle; handle=handle->next)
   {
      if(EqnIsMaximal(handle))
      {
    if(!EqnIsPositive(handle))
    {
       break;
    }
    if(found)
    {
       break;
    }
    found = true;
      }
   }
   if(handle)
   {
      SelectOptimalLiteral(ocb,clause);
      ClauseDelProp(clause, CPIsOriented);
   }
}


/*-----------------------------------------------------------------------
//
// Function: PSelectUnlessUniqMaxPosOptimalLiteral()
//
//   If a clause has a maximal literal that is positive, do not
//   select, otherwise select optimal literal.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void PSelectUnlessUniqMaxPosOptimalLiteral(OCB_p ocb, Clause_p clause)
{
   Eqn_p handle;
   bool found = false;

   ClauseCondMarkMaximalTerms(ocb, clause);

   for(handle=clause->literals; handle; handle=handle->next)
   {
      if(EqnIsMaximal(handle))
      {
    if(!EqnIsPositive(handle))
    {
       break;
    }
    if(found)
    {
       break;
    }
    found = true;
      }
   }
   if(handle)
   {
      PSelectOptimalLiteral(ocb,clause);
      ClauseDelProp(clause, CPIsOriented);
   }
}


/*-----------------------------------------------------------------------
//
// Function: SelectComplex()
//
//   If there is a pure variable literal, select it. Otherwise, if
//   there is at least one ground literal, select the smallest
//   one. Otherwise, select the literal with the largest size
//   difference.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void SelectComplex(OCB_p ocb, Clause_p clause)
{
   Eqn_p handle, selected;
   long select_weight, weight;

   selected = ClauseFindNegPureVarLit(clause);

   if(!selected)
   {
      select_weight = LONG_MAX;
      handle = clause->literals;

      while(handle)
      {
         if(EqnIsNegative(handle) && EqnIsGround(handle))
         {
            weight = EqnStandardWeight(handle);
            if(weight < select_weight)
            {
        select_weight = weight;
        selected = handle;
            }
         }
         handle = handle->next;
      }
   }
   if(selected)
   {
      EqnSetProp(selected, EPIsSelected);
   }
   else
   {
      SelectDiffNegativeLiteral(ocb, clause);
   }
}


/*-----------------------------------------------------------------------
//
// Function: PSelectComplex()
//
//   If there is a pure variable literal, select it. Otherwise, if
//   there is at least one ground literal, select the smallest
//   one. Otherwise, select the literal with the largest size
//   difference and the positive literals.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void PSelectComplex(OCB_p ocb, Clause_p clause)
{
   Eqn_p handle, selected;
   long select_weight, weight;

   selected = ClauseFindNegPureVarLit(clause);

   if(!selected)
   {
      select_weight = LONG_MAX;
      handle = clause->literals;

      while(handle)
      {
         if(EqnIsNegative(handle) && EqnIsGround(handle))
         {
            weight = EqnStandardWeight(handle);
            if(weight < select_weight)
            {
               select_weight = weight;
               selected = handle;
            }
         }
         handle = handle->next;
      }
   }
   if(selected)
   {
      EqnSetProp(selected, EPIsSelected);
   }
   else
   {
      PSelectDiffNegativeLiteral(ocb, clause);
   }
}

/*-----------------------------------------------------------------------
//
// Function: SelectComplexExceptRRHorn()
//
//   If a clause is Horn and range-restricted, do no select. Otherwise
//   use SelectComplex() (above).
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void SelectComplexExceptRRHorn(OCB_p ocb, Clause_p clause)
{
   if(!(ClauseIsHorn(clause) && ClauseIsRangeRestricted(clause)))
   {
      SelectComplex(ocb, clause);
   }
}


/*-----------------------------------------------------------------------
//
// Function: PSelectComplexExceptRRHorn()
//
//   If a clause is Horn and range-restricted, do no select. Otherwise
//   use PSelectComplex() (above).
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void PSelectComplexExceptRRHorn(OCB_p ocb, Clause_p clause)
{
   if(!(ClauseIsHorn(clause) && ClauseIsRangeRestricted(clause)))
   {
      PSelectComplex(ocb, clause);
   }
}


/*-----------------------------------------------------------------------
//
// Function: SelectLComplex()
//
//   Similar to SelectComplex, but always select largest diff
//   literals first.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void SelectLComplex(OCB_p ocb, Clause_p clause)
{
   Eqn_p handle, selected;
   long select_weight, weight;

   selected = ClauseFindNegPureVarLit(clause);

   if(!selected)
   {
      select_weight = -1;
      handle = clause->literals;

      while(handle)
      {
         if(EqnIsNegative(handle) && EqnIsGround(handle))
         {
            weight = lit_sel_diff_weight(handle);
            if(weight > select_weight)
            {
        select_weight = weight;
        selected = handle;
            }
         }
         handle = handle->next;
      }
   }
   if(selected)
   {
      EqnSetProp(selected, EPIsSelected);
   }
   else
   {
      SelectDiffNegativeLiteral(ocb, clause);
   }
}

/*-----------------------------------------------------------------------
//
// Function: PSelectLComplex()
//
//   Similar to PSelectComplex, but always select largest diff
//   literals first.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void PSelectLComplex(OCB_p ocb, Clause_p clause)
{
   Eqn_p handle, selected;
   long select_weight, weight;

   selected = ClauseFindNegPureVarLit(clause);

   if(!selected)
   {
      select_weight = -1;
      handle = clause->literals;

      while(handle)
      {
         if(EqnIsNegative(handle) && EqnIsGround(handle))
         {
            weight = lit_sel_diff_weight(handle);
            if(weight > select_weight)
            {
        select_weight = weight;
        selected = handle;
            }
         }
         handle = handle->next;
      }
   }
   if(selected)
   {
      EqnSetProp(selected, EPIsSelected);
   }
   else
   {
      PSelectDiffNegativeLiteral(ocb, clause);
   }
}


/*-----------------------------------------------------------------------
//
// Function: SelectMaxLComplex()
//
//   If there is more than one maximal literal, select a negative
//   literal, with the following priority:
//
//   Maximal, pure variable
//   Maximal, largest difference ground
//   Maximal, largest difference non-ground
//   pure variable
//   largest difference ground
//   largest difference non-ground
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void SelectMaxLComplex(OCB_p ocb, Clause_p clause)
{
   long  lit_no;
   Eqn_p selected;

   ClauseCondMarkMaximalTerms(ocb, clause);

   lit_no = EqnListQueryPropNumber(clause->literals, EPIsMaximal);
   if(lit_no>1)
   {
      ClauseDelProp(clause, CPIsOriented);
      if(EqnListQueryPropNumber(clause->literals,
            EPIsMaximal|EPIsPositive)!=lit_no)
      { /* There is at least one maximal negative literal */

    selected = find_maxlcomplex_literal(clause);
    assert(selected);
    EqnSetProp(selected, EPIsSelected);
      }
      else
      { /* Normal selection */
    SelectLComplex(ocb, clause);
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: PSelectMaxLComplex()
//
//   As above, but in the default case select positive literals as
//   well.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void PSelectMaxLComplex(OCB_p ocb, Clause_p clause)
{
   long lit_no;
   Eqn_p selected;

   ClauseCondMarkMaximalTerms(ocb, clause);

   lit_no = EqnListQueryPropNumber(clause->literals, EPIsMaximal);
   if(lit_no>1)
   {
      ClauseDelProp(clause, CPIsOriented);
      if(EqnListQueryPropNumber(clause->literals,
            EPIsMaximal|EPIsPositive)!=lit_no)
      { /* There is at least one maximal negative literal */

    selected = find_maxlcomplex_literal(clause);
    assert(selected);
    EqnSetProp(selected, EPIsSelected);
      }
      else
      { /* Normal selection */
    PSelectLComplex(ocb, clause);
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: SelectMaxLComplexNoTypePred()
//
//   If there is more than one maximal literal, select a negative
//   literal, with the following priority:
//
//   Maximal, pure variable
//   Maximal, largest difference ground
//   Maximal, largest difference non-ground
//   pure variable
//   largest difference ground
//   largest difference non-ground
//
//   Never select a type literal. If all negative literals are type
//   literals, select nothing.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void SelectMaxLComplexNoTypePred(OCB_p ocb, Clause_p clause)
{
   long  lit_no;
   Eqn_p selected = NULL;

   ClauseCondMarkMaximalTerms(ocb, clause);

   lit_no = EqnListQueryPropNumber(clause->literals, EPIsMaximal);
   if(lit_no>1)
   {
      ClauseDelProp(clause, CPIsOriented);
      if(EqnListQueryPropNumber(clause->literals,
            EPIsMaximal|EPIsPositive)!=lit_no)
      { /* There is at least one maximal negative literal */

    selected = find_maxlcomplex_literal(clause);
    assert(selected);
    if(!EqnIsTypePred(selected))
    {
       EqnSetProp(selected, EPIsSelected);
    }
    else
    {
       selected = NULL;
    }
      }
      if(!selected)
      { /* Normal selection */
    selected = find_lcomplex_literal(clause);
    if(selected&&!EqnIsTypePred(selected))
    {
       EqnSetProp(selected, EPIsSelected);
    }
      }
   }
}

/*-----------------------------------------------------------------------
//
// Function: PSelectMaxLComplexNoTypePred()
//
//   If there is more than one maximal literal, select a negative
//   literal, with the following priority:
//
//   Maximal, pure variable
//   Maximal, largest difference ground
//   Maximal, largest difference non-ground
//   pure variable
//   largest difference ground
//   largest difference non-ground
//
//   Never select a type literal. If all negative literals are type
//   literals, select nothing. If selected, also select positive
//   literals.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void PSelectMaxLComplexNoTypePred(OCB_p ocb, Clause_p clause)
{
   long  lit_no;
   Eqn_p selected = NULL;

   ClauseCondMarkMaximalTerms(ocb, clause);

   lit_no = EqnListQueryPropNumber(clause->literals, EPIsMaximal);
   if(lit_no>1)
   {
      ClauseDelProp(clause, CPIsOriented);
      if(EqnListQueryPropNumber(clause->literals,
            EPIsMaximal|EPIsPositive)!=lit_no)
      { /* There is at least one maximal negative literal */

    selected = find_maxlcomplex_literal(clause);
    assert(selected);
    if(!EqnIsTypePred(selected))
    {
       EqnSetProp(selected, EPIsSelected);
       clause_select_pos(clause);
    }
    else
    {
       selected = NULL;
    }
      }
      if(!selected)
      { /* Normal selection */
    selected = find_lcomplex_literal(clause);
    if(selected&&!EqnIsTypePred(selected))
    {
       EqnSetProp(selected, EPIsSelected);
       clause_select_pos(clause);
    }
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: SelectMaxLComplexNoXTypePred()
//
//   If there is more than one maximal literal, select a negative
//   literal, with the following priority:
//
//   Maximal, pure variable
//   Maximal, largest difference ground
//   Maximal, largest difference non-ground
//   pure variable
//   largest difference ground
//   largest difference non-ground
//
//   Never select an extended type literal P(X1,...,Xn). If all
//   negative literals are extended type literals, select nothing.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void SelectMaxLComplexNoXTypePred(OCB_p ocb, Clause_p clause)
{
   long  lit_no;
   Eqn_p selected = NULL;

   ClauseCondMarkMaximalTerms(ocb, clause);

   lit_no = EqnListQueryPropNumber(clause->literals, EPIsMaximal);
   if(lit_no>1)
   {
      ClauseDelProp(clause, CPIsOriented);
      if(EqnListQueryPropNumber(clause->literals,
            EPIsMaximal|EPIsPositive)!=lit_no)
      { /* There is at least one maximal negative literal */

    selected = find_maxlcomplex_literal(clause);
    assert(selected);
    if(!EqnIsXTypePred(selected))
    {
       EqnSetProp(selected, EPIsSelected);
    }
    else
    {
       selected = NULL;
    }
      }
      if(!selected)
      { /* Normal selection */
    selected = find_lcomplex_literal(clause);
    if(selected && !EqnIsXTypePred(selected))
    {
       EqnSetProp(selected, EPIsSelected);
    }
      }
   }
}

/*-----------------------------------------------------------------------
//
// Function: PSelectMaxLComplexNoXTypePred()
//
//   If there is more than one maximal literal, select a negative
//   literal, with the following priority:
//
//   Maximal, pure variable
//   Maximal, largest difference ground
//   Maximal, largest difference non-ground
//   pure variable
//   largest difference ground
//   largest difference non-ground
//
//   Never select an extended type literal. If all negative literals
//   are extended type literals, select nothing. If selected, also
//   select positive literals.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void PSelectMaxLComplexNoXTypePred(OCB_p ocb, Clause_p clause)
{
   long  lit_no;
   Eqn_p selected = NULL;

   ClauseCondMarkMaximalTerms(ocb, clause);

   lit_no = EqnListQueryPropNumber(clause->literals, EPIsMaximal);
   if(lit_no>1)
   {
      ClauseDelProp(clause, CPIsOriented);
      if(EqnListQueryPropNumber(clause->literals,
            EPIsMaximal|EPIsPositive)!=lit_no)
      { /* There is at least one maximal negative literal */

    selected = find_maxlcomplex_literal(clause);
    assert(selected);
    if(!EqnIsXTypePred(selected))
    {
       EqnSetProp(selected, EPIsSelected);
       clause_select_pos(clause);
    }
    else
    {
       selected = NULL;
    }
      }
      if(!selected)
      { /* Normal selection */
    selected = find_lcomplex_literal(clause);
    if(selected&&!EqnIsXTypePred(selected))
    {
       EqnSetProp(selected, EPIsSelected);
       clause_select_pos(clause);
    }
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: SelectComplexPreferNEQ()
//
//   Select a negative literal as in SelectComplex, but prefer
//   non-equational literals.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void SelectComplexPreferNEQ(OCB_p ocb, Clause_p clause)
{
   bool sel_neq    = false, cur_neq;
   bool sel_var    = false, cur_var;
   bool sel_ground = false, cur_ground;
   long sel_weight = -1,    cur_weight;
   Eqn_p handle, selected = NULL;

   for(handle=clause->literals; handle; handle=handle->next)
   {
      if(EqnIsNegative(handle))
      {
         cur_neq = !EqnIsEquLit(handle);
         cur_var = EqnIsPureVar(handle);
         cur_ground = EqnIsGround(handle);
         cur_weight = -1;

         if(sel_neq && !cur_neq)
         {
            break;
         }
         else if(EQUIV(cur_neq, sel_neq))
         {
            if(sel_var && !cur_var)
            {
        break;
            }
            else if(EQUIV(cur_var, sel_var))
            {
        if(sel_ground && !cur_ground)
        {
           break;
        }
        else if(EQUIV(cur_ground, sel_ground))
        {
           cur_weight = lit_sel_diff_weight(handle);
           if(cur_weight <=sel_weight)
           {
         break;
           }
        }
            }
         }
         if(cur_weight == -1)
         {
            cur_weight = lit_sel_diff_weight(handle);
         }
         selected   = handle;
         sel_weight = cur_weight;
         sel_ground = cur_ground;
         sel_var    = cur_var;
         sel_neq    = cur_neq;
      }
   }
   if(selected)
   {
      EqnSetProp(selected, EPIsSelected);
   }
}


/*-----------------------------------------------------------------------
//
// Function: PSelectComplexPreferNEQ()
//
//   Select a negative literal as in PSelectComplex, but prefer
//   non-equational literals.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void PSelectComplexPreferNEQ(OCB_p ocb, Clause_p clause)
{
   bool sel_neq    = false, cur_neq;
   bool sel_var    = false, cur_var;
   bool sel_ground = false, cur_ground;
   long sel_weight = -1,    cur_weight;
   Eqn_p handle, selected = NULL;

   for(handle=clause->literals; handle; handle=handle->next)
   {
      if(EqnIsNegative(handle))
      {
         cur_neq = !EqnIsEquLit(handle);
         cur_var = EqnIsPureVar(handle);
         cur_ground = EqnIsGround(handle);
         cur_weight = -1;

         if(sel_neq && !cur_neq)
         {
            break;
         }
         else if(EQUIV(cur_neq, sel_neq))
         {
            if(sel_var && !cur_var)
            {
        break;
            }
            else if(EQUIV(cur_var, sel_var))
            {
        if(sel_ground && !cur_ground)
        {
           break;
        }
        else if(EQUIV(cur_ground, sel_ground))
        {
           cur_weight = lit_sel_diff_weight(handle);
           if(cur_weight <=sel_weight)
           {
         break;
           }
        }
            }
         }
         if(cur_weight == -1)
         {
            cur_weight = lit_sel_diff_weight(handle);
         }
         selected   = handle;
         sel_weight = cur_weight;
         sel_ground = cur_ground;
         sel_var    = cur_var;
         sel_neq    = cur_neq;
      }
   }
   if(selected)
   {
      clause_select_pos(clause);
      EqnSetProp(selected, EPIsSelected);
   }
}


/*-----------------------------------------------------------------------
//
// Function: SelectComplexPreferEQ()
//
//   Select a negative literal as in SelectComplex, but prefer
//   equational literals.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void SelectComplexPreferEQ(OCB_p ocb, Clause_p clause)
{
   bool sel_eq     = false, cur_eq;
   bool sel_var    = false, cur_var;
   bool sel_ground = false, cur_ground;
   long sel_weight = -1,    cur_weight;
   Eqn_p handle, selected = NULL;

   for(handle=clause->literals; handle; handle=handle->next)
   {
      if(EqnIsNegative(handle))
      {
         cur_eq = EqnIsEquLit(handle);
         cur_var = EqnIsPureVar(handle);
         cur_ground = EqnIsGround(handle);
         cur_weight = -1;

         if(sel_eq && !cur_eq)
         {
            break;
         }
         else if(EQUIV(cur_eq, sel_eq))
         {
            if(sel_var && !cur_var)
            {
        break;
            }
            else if(EQUIV(cur_var, sel_var))
            {
        if(sel_ground && !cur_ground)
        {
           break;
        }
        else if(EQUIV(cur_ground, sel_ground))
        {
           cur_weight = lit_sel_diff_weight(handle);
           if(cur_weight <=sel_weight)
           {
         break;
           }
        }
            }
         }
         if(cur_weight == -1)
         {
            cur_weight = lit_sel_diff_weight(handle);
         }
         selected   = handle;
         sel_weight = cur_weight;
         sel_ground = cur_ground;
         sel_var    = cur_var;
         sel_eq    = cur_eq;
      }
   }
   if(selected)
   {
      EqnSetProp(selected, EPIsSelected);
   }
}


/*-----------------------------------------------------------------------
//
// Function: PSelectComplexPreferEQ()
//
//   Select a negative literal as in PSelectComplex, but prefer
//   equational literals.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void PSelectComplexPreferEQ(OCB_p ocb, Clause_p clause)
{
   bool sel_eq     = false, cur_eq;
   bool sel_var    = false, cur_var;
   bool sel_ground = false, cur_ground;
   long sel_weight = -1,    cur_weight;
   Eqn_p handle, selected = NULL;

   for(handle=clause->literals; handle; handle=handle->next)
   {
      if(EqnIsNegative(handle))
      {
         cur_eq = EqnIsEquLit(handle);
         cur_var = EqnIsPureVar(handle);
         cur_ground = EqnIsGround(handle);
         cur_weight = -1;

         if(sel_eq && !cur_eq)
         {
            break;
         }
         else if(EQUIV(cur_eq, sel_eq))
         {
            if(sel_var && !cur_var)
            {
        break;
            }
            else if(EQUIV(cur_var, sel_var))
            {
        if(sel_ground && !cur_ground)
        {
           break;
        }
        else if(EQUIV(cur_ground, sel_ground))
        {
           cur_weight = lit_sel_diff_weight(handle);
           if(cur_weight <=sel_weight)
           {
         break;
           }
        }
            }
         }
         if(cur_weight == -1)
         {
            cur_weight = lit_sel_diff_weight(handle);
         }
         selected   = handle;
         sel_weight = cur_weight;
         sel_ground = cur_ground;
         sel_var    = cur_var;
         sel_eq    = cur_eq;
      }
   }
   if(selected)
   {
      clause_select_pos(clause);
      EqnSetProp(selected, EPIsSelected);
   }
}


/*-----------------------------------------------------------------------
//
// Function: SelectComplexExceptUniqMaxHorn()
//
//   Select literal as in SelectComplex unless the clause is a Horn
//   clause with a unique maximal literal.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void SelectComplexExceptUniqMaxHorn(OCB_p ocb, Clause_p clause)
{
   if(ClauseIsHorn(clause))
   {
      ClauseCondMarkMaximalTerms(ocb, clause);
      if(EqnListQueryPropNumber(clause->literals, EPIsMaximal)==1)
      {
    return;
      }
   }
   SelectComplex(ocb,clause);
   ClauseDelProp(clause, CPIsOriented);
}


/*-----------------------------------------------------------------------
//
// Function: PSelectComplexExceptUniqMaxHorn()
//
//   Select literal as in PSelectComplex unless the clause is a Horn
//   clause with a unique maximal literal.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void PSelectComplexExceptUniqMaxHorn(OCB_p ocb, Clause_p clause)
{
   if(ClauseIsHorn(clause))
   {
      ClauseCondMarkMaximalTerms(ocb, clause);
      if(EqnListQueryPropNumber(clause->literals, EPIsMaximal)==1)
      {
         return;
      }
   }
   PSelectComplex(ocb,clause);
   ClauseDelProp(clause, CPIsOriented);
}


/*-----------------------------------------------------------------------
//
// Function: MSelectComplexExceptUniqMaxHorn()
//
//   For horn clauses, call PSelectComplexExceptUniqMaxHorn,
//   otherwise call SelectComplexExceptUniqMaxHorn.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void MSelectComplexExceptUniqMaxHorn(OCB_p ocb, Clause_p clause)
{
   if(ClauseIsHorn(clause))
   {
      PSelectComplexExceptUniqMaxHorn(ocb,clause);
   }
   else
   {
      SelectComplexExceptUniqMaxHorn(ocb,clause);
   }
}


/*-----------------------------------------------------------------------
//
// Function: SelectNewComplex()
//
//   If there is a negative ground literal, select the one with the
//   smallest maximal side.
//   Else: Select the minimal inference position non-XType orientable
//   negative literal.
//   Else: Select the lagest XType literal.
//   Never select a Type literal - if all negative literals are type
//   literals, do not select at all.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void SelectNewComplex(OCB_p ocb, Clause_p clause)
{
   Eqn_p selected;

   ClauseCondMarkMaximalTerms(ocb, clause);

   selected = find_smallest_max_neg_ground_lit(clause);
   if(!selected)
   {
      selected = find_ng_min11_infpos_no_xtype_lit(clause);
   }
   if(!selected)
   {
      selected = find_max_xtype_no_type_lit(clause);
   }
   if(selected)
   {
      EqnSetProp(selected, EPIsSelected);
      ClauseDelProp(clause, CPIsOriented);
   }
}


/*-----------------------------------------------------------------------
//
// Function: PSelectNewComplex()
//
//   If there is a negative ground literal, select the one with the
//   smallest maximal side.
//   Else: Select the minimal inference position non-XType orientable
//   negative literal.
//   Else: Select the lagest XType literal.
//   Never select a Type literal - if all negative literals are type
//   literals, do not select at all.
//   If anything is selected, select positive literals as well.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void PSelectNewComplex(OCB_p ocb, Clause_p clause)
{
   Eqn_p selected;

   ClauseCondMarkMaximalTerms(ocb, clause);

   selected = find_smallest_max_neg_ground_lit(clause);
   if(!selected)
   {
      selected = find_ng_min11_infpos_no_xtype_lit(clause);
   }
   if(!selected)
   {
      selected = find_max_xtype_no_type_lit(clause);
   }
   if(selected)
   {
      EqnSetProp(selected, EPIsSelected);
      ClauseDelProp(clause, CPIsOriented);
      clause_select_pos(clause);
   }
}


/*-----------------------------------------------------------------------
//
// Function: SelectNewComplexExceptUniqMaxHorn()
//
//   Select literal as in SelectNewsComplex unless the clause is a
//   Horn clause with a unique maximal literal.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void SelectNewComplexExceptUniqMaxHorn(OCB_p ocb, Clause_p clause)
{
   if(ClauseIsHorn(clause))
   {
      ClauseCondMarkMaximalTerms(ocb, clause);
      if(EqnListQueryPropNumber(clause->literals, EPIsMaximal)==1)
      {
    return;
      }
   }
   SelectNewComplex(ocb,clause);
}


/*-----------------------------------------------------------------------
//
// Function: PSelectNewComplexExceptUniqMaxHorn()
//
//   Select literal as in PSelectNewsComplex unless the clause is a
//   Horn clause with a unique maximal literal.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void PSelectNewComplexExceptUniqMaxHorn(OCB_p ocb, Clause_p clause)
{
   if(ClauseIsHorn(clause))
   {
      ClauseCondMarkMaximalTerms(ocb, clause);
      if(EqnListQueryPropNumber(clause->literals, EPIsMaximal)==1)
      {
    return;
      }
   }
   PSelectNewComplex(ocb,clause);
}


/*-----------------------------------------------------------------------
//
// Function: SelectMinInfpos()
//
//   Select the literal with the smallest number of potential
//   inference positions, i.e. smallest sum of maximal size weights.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void SelectMinInfpos(OCB_p ocb, Clause_p clause)
{
   Eqn_p handle = clause->literals, selected = NULL;
   long  select_weight = LONG_MAX, currw;

   ClauseCondMarkMaximalTerms(ocb, clause);
   while(handle)
   {
      if(EqnIsNegative(handle))
      {
         currw = TermStandardWeight(handle->lterm);
         if(!EqnIsOriented(handle))
         {
            currw += TermStandardWeight(handle->rterm);
         }
         if(currw < select_weight)
         {
            select_weight = currw;
            selected = handle;
         }
      }
      handle = handle->next;
   }
   assert(selected);
   EqnSetProp(selected, EPIsSelected);
   ClauseDelProp(clause, CPIsOriented);
}


/*-----------------------------------------------------------------------
//
// Function: PSelectMinInfpos()
//
//   Select the literal with the smallest number of potential
//   inference positions, i.e. smallest sum of maximal size weights,
//   and select positive literals as well.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void PSelectMinInfpos(OCB_p ocb, Clause_p clause)
{
   Eqn_p handle = clause->literals, selected = NULL;
   long  select_weight = LONG_MAX, currw;

   ClauseCondMarkMaximalTerms(ocb, clause);
   while(handle)
   {
      if(EqnIsPositive(handle))
      {
         EqnSetProp(handle, EPIsSelected);
      }
      else
      {
         currw = TermStandardWeight(handle->lterm);
         if(!EqnIsOriented(handle))
         {
            currw += TermStandardWeight(handle->rterm);
         }
         if(currw < select_weight)
         {
            select_weight = currw;
            selected = handle;
         }
      }
      handle = handle->next;
   }
   assert(selected);
   EqnSetProp(selected, EPIsSelected);
   ClauseDelProp(clause, CPIsOriented);
}


/*-----------------------------------------------------------------------
//
// Function: HSelectMinInfpos()
//
//   Select the literal with the smallest number of potential
//   inference positions, i.e. smallest sum of maximal size weights.
//   If this is not ground, select positive ones as well.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void HSelectMinInfpos(OCB_p ocb, Clause_p clause)
{
   Eqn_p handle = clause->literals, selected = NULL;
   long  select_weight = LONG_MAX, currw;

   ClauseCondMarkMaximalTerms(ocb, clause);
   while(handle)
   {
      if(EqnIsNegative(handle))
      {
         currw = TermStandardWeight(handle->lterm);
         if(!EqnIsOriented(handle))
         {
            currw += TermStandardWeight(handle->rterm);
         }
         if(currw < select_weight)
         {
            select_weight = currw;
            selected = handle;
         }
      }
      handle = handle->next;
   }
   assert(selected);
   EqnSetProp(selected, EPIsSelected);
   ClauseDelProp(clause, CPIsOriented);
   if(!EqnIsGround(selected))
   {
      for(handle = clause->literals; handle; handle = handle->next)
      {
         if(EqnIsPositive(handle))
         {
            EqnSetProp(handle, EPIsSelected);
         }
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: GSelectMinInfpos()
//
//   Select the literal with the smallest number of potential
//   inference positions, i.e. smallest sum of maximal size weights.
//   If this is ground, select positive ones as well.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void GSelectMinInfpos(OCB_p ocb, Clause_p clause)
{
   Eqn_p handle = clause->literals, selected = NULL;
   long  select_weight = LONG_MAX, currw;

   ClauseCondMarkMaximalTerms(ocb, clause);
   while(handle)
   {
      if(EqnIsNegative(handle))
      {
         currw = TermStandardWeight(handle->lterm);
         if(!EqnIsOriented(handle))
         {
            currw += TermStandardWeight(handle->rterm);
         }
         if(currw < select_weight)
         {
            select_weight = currw;
            selected = handle;
         }
      }
      handle = handle->next;
   }
   assert(selected);
   EqnSetProp(selected, EPIsSelected);
   ClauseDelProp(clause, CPIsOriented);
   if(EqnIsGround(selected))
   {
      for(handle = clause->literals; handle; handle = handle->next)
      {
         if(EqnIsPositive(handle))
         {
            EqnSetProp(handle, EPIsSelected);
         }
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: SelectMinInfposNoTypePred()
//
//   Select the literal with the smallest number of potential
//   inference positions, i.e. smallest sum of maximal size weights,
//   but never select type predicates.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void SelectMinInfposNoTypePred(OCB_p ocb, Clause_p clause)
{
   Eqn_p handle = clause->literals, selected = NULL;
   long  select_weight = LONG_MAX, currw;

   ClauseCondMarkMaximalTerms(ocb, clause);
   while(handle)
   {
      if(EqnIsNegative(handle)&&!EqnIsTypePred(handle))
      {
         currw = TermStandardWeight(handle->lterm);
         if(!EqnIsOriented(handle))
         {
            currw += TermStandardWeight(handle->rterm);
         }
         if(currw < select_weight)
         {
            select_weight = currw;
            selected = handle;
         }
      }
      handle = handle->next;
   }
   if(selected)
   {
      EqnSetProp(selected, EPIsSelected);
      ClauseDelProp(clause, CPIsOriented);
   }
}

/*-----------------------------------------------------------------------
//
// Function: PSelectMinInfposNoTypePred()
//
//   Select the literal with the smallest number of potential
//   inference positions, i.e. smallest sum of maximal size weights,
//   but never select type predicates. If literal is selected, also
//   select positive ones.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void PSelectMinInfposNoTypePred(OCB_p ocb, Clause_p clause)
{
   Eqn_p handle = clause->literals, selected = NULL;
   long  select_weight = LONG_MAX, currw;

   ClauseCondMarkMaximalTerms(ocb, clause);
   while(handle)
   {
      if(EqnIsNegative(handle)&&!EqnIsTypePred(handle))
      {
         currw = TermStandardWeight(handle->lterm);
         if(!EqnIsOriented(handle))
         {
            currw += TermStandardWeight(handle->rterm);
         }
         if(currw < select_weight)
         {
            select_weight = currw;
            selected = handle;
         }
      }
      handle = handle->next;
   }
   if(selected)
   {
      EqnSetProp(selected, EPIsSelected);
      clause_select_pos(clause);
      ClauseDelProp(clause, CPIsOriented);
   }
}


/*-----------------------------------------------------------------------
//
// Function: SelectMin2Infpos()
//
//   Select the literal with the smallest number of potential
//   inference positions, i.e. smallest sum of maximal size weights
//   for f_weight = 1, v_weight = 2
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void SelectMin2Infpos(OCB_p ocb, Clause_p clause)
{
   Eqn_p handle = clause->literals, selected = NULL;
   long  select_weight = LONG_MAX, currw;

   ClauseCondMarkMaximalTerms(ocb, clause);
   while(handle)
   {
      if(EqnIsNegative(handle))
      {
         currw = TermWeight(handle->lterm,2,1);
         if(!EqnIsOriented(handle))
         {
            currw += TermWeight(handle->rterm,2,1);
         }
         if(currw < select_weight)
         {
            select_weight = currw;
            selected = handle;
         }
      }
      handle = handle->next;
   }
   assert(selected);
   EqnSetProp(selected, EPIsSelected);
   ClauseDelProp(clause, CPIsOriented);
}


/*-----------------------------------------------------------------------
//
// Function: PSelectMin2Infpos()
//
//   Select the literal with the smallest number of potential
//   inference positions, i.e. smallest sum of maximal size weights
//   (as above), and select positive literals as well.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void PSelectMin2Infpos(OCB_p ocb, Clause_p clause)
{
   Eqn_p handle = clause->literals, selected = NULL;
   long  select_weight = LONG_MAX, currw;

   ClauseCondMarkMaximalTerms(ocb, clause);
   while(handle)
   {
      if(EqnIsPositive(handle))
      {
         EqnSetProp(handle, EPIsSelected);
      }
      else
      {
         currw = TermWeight(handle->lterm,2,1);
         if(!EqnIsOriented(handle))
         {
            currw += TermWeight(handle->rterm,2,1);
         }
         if(currw < select_weight)
         {
            select_weight = currw;
            selected = handle;
         }
      }
      handle = handle->next;
   }
   assert(selected);
   EqnSetProp(selected, EPIsSelected);
   ClauseDelProp(clause, CPIsOriented);
}


/*-----------------------------------------------------------------------
//
// Function: SelectComplexExceptUniqMaxPosHorn()
//
//   Select literal as in SelectComplex unless the clause is a Horn
//   clause with a unique maximal positive literal.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void SelectComplexExceptUniqMaxPosHorn(OCB_p ocb, Clause_p clause)
{
   if(ClauseIsHorn(clause))
   {
      ClauseCondMarkMaximalTerms(ocb, clause);
      if((EqnListQueryPropNumber(clause->literals, EPIsMaximal)==1)&&
    (EqnListQueryPropNumber(clause->literals, EPIsMaximal|EPIsPositive)==1))
      {
    return;
      }
   }
   SelectComplex(ocb,clause);
   ClauseDelProp(clause, CPIsOriented);
}


/*-----------------------------------------------------------------------
//
// Function: PSelectComplexExceptUniqMaxPosHorn()
//
//   Select literal as in PSelectComplex unless the clause is a Horn
//   clause with a unique maximal positive literal.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void PSelectComplexExceptUniqMaxPosHorn(OCB_p ocb, Clause_p clause)
{
   if(ClauseIsHorn(clause))
   {
      ClauseCondMarkMaximalTerms(ocb, clause);
      if((EqnListQueryPropNumber(clause->literals, EPIsMaximal)==1)&&
    (EqnListQueryPropNumber(clause->literals, EPIsMaximal|EPIsPositive)==1))
      {
    return;
      }
   }
   PSelectComplex(ocb,clause);
   ClauseDelProp(clause, CPIsOriented);
}


/*-----------------------------------------------------------------------
//
// Function: diversification_weight()
//
//   Assign pseudo-random weight to negative literals, 0 to positive
//   ones.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

static void diversification_weight(LitEval_p lit, Clause_p clause, void* dummy)
{
   if(EqnIsNegative(lit->literal))
   {
      lit->w1 = literal_weight_counter % clause->neg_lit_no;
   }
   literal_weight_counter++;
}


/*-----------------------------------------------------------------------
//
// Function: SelectDiversificationLiterals()
//
//   Systematically select a pseudo-random literal in clause (where
//   pseudo is large and random in small).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void SelectDiversificationLiterals(OCB_p ocb, Clause_p clause)
{
   generic_uniq_selection(ocb,clause,false, false, diversification_weight, NULL);
}


/*-----------------------------------------------------------------------
//
// Function: diversification_prefer_into_weight()
//
//   Assing pseudo-random weight to negative literals, 0 to positive
//   ones. However, always prefer literals comming from the into
//   clause of a paramodulation to those of a from clause.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void diversification_prefer_into_weight(LitEval_p lit,
                                               Clause_p clause,
                                               void* dummy)
{
   lit->w1 = -ClauseQueryProp(lit->literal, EPIsPMIntoLit);
   if(EqnIsNegative(lit->literal))
   {
      lit->w2 = literal_weight_counter % clause->neg_lit_no;
   }
   literal_weight_counter++;
}


/*-----------------------------------------------------------------------
//
// Function: SelectDiversificationPreferIntoLiterals()
//
//   Systematically select a pseudo-random literal in clause (where
//   pseudo is large and random in small), but prefer into-literals.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void SelectDiversificationPreferIntoLiterals(OCB_p ocb, Clause_p clause)
{
   generic_uniq_selection(ocb,clause,false, false,
                          diversification_prefer_into_weight,
                          NULL);
}

/*-----------------------------------------------------------------------
//
// Function: maxlcomplex_weight()
//
//   Initialize weights to mimic SelectMaxLComplexWeight()
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void maxlcomplex_weight(LitEval_p lit, Clause_p clause, void *dummy)
{
   if(EqnIsNegative(lit->literal))
   {
      if(EqnIsMaximal(lit->literal))
      {
         lit->w1=0;
      }
      else
      {
         lit->w1=100;
      }
      if(!EqnIsPureVar(lit->literal))
      {
         lit->w1+=10;
      }
      if(!EqnIsGround(lit->literal))
      {
         lit->w1+=1;
      }
      lit->w2 = -lit_sel_diff_weight(lit->literal);
      lit->w3 = literal_weight_counter % clause->neg_lit_no;
   }
   literal_weight_counter++;
}

/*-----------------------------------------------------------------------
//
// Function: SelectMaxLComplexG()
//
//   Reimplementation of SelectMaxLComplex() using the generic literal
//   selection framework.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void SelectMaxLComplexG(OCB_p ocb, Clause_p clause)
{
   long  lit_no;

   assert(ocb);
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no==0)
   {
      return;
   }
   ClauseCondMarkMaximalTerms(ocb, clause);

   lit_no = EqnListQueryPropNumber(clause->literals, EPIsMaximal);

   if(lit_no <=1)
   {
      return;
   }
   generic_uniq_selection(ocb,clause,false, true,
                          maxlcomplex_weight, NULL);
}

/*-----------------------------------------------------------------------
//
// Function: maxlcomplexavoidpred_weight()
//
//   Initialize weights to mimic SelectMaxLComplexWeight(), but defer
//   literals with which occur often in pred_dist.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void maxlcomplexavoidpred_weight(LitEval_p lit, Clause_p clause,
                                        void *pred_dist)
{
   PDArray_p pd = pred_dist;

   if(EqnIsNegative(lit->literal))
   {
      if(EqnIsMaximal(lit->literal))
      {
         lit->w1=0;
      }
      else
      {
         lit->w1=100;
      }
      if(!EqnIsPureVar(lit->literal))
      {
         lit->w1+=10;
      }
      if(!EqnIsGround(lit->literal))
      {
         lit->w1+=1;
      }
      lit->w2 = -lit_sel_diff_weight(lit->literal);
      if(EqnIsEquLit(lit->literal) || TermIsAnyVar(lit->literal->lterm) || TermIsPhonyApp(lit->literal->lterm))
      {
         lit->w3 = PDArrayElementInt(pd, 0);
      }
      else
      {
         lit->w3 = PDArrayElementInt(pd, lit->literal->lterm->f_code);
      }
   }
}

/*-----------------------------------------------------------------------
//
// Function: maxlcomplexavoidappvar_weight()
//
//   Initialize weights to mimic SelectMaxLComplexWeight(), but defer
//   literals with applied variables, and put them right after pure vars
//   and defer literals which occur often in pred_dist.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void maxlcomplexavoidappvar_weight(LitEval_p lit, Clause_p clause,
                                        void *pred_dist)
{
   PDArray_p pd = pred_dist;

   if(EqnIsNegative(lit->literal))
   {
      if(EqnIsMaximal(lit->literal))
      {
         lit->w1=0;
      }
      else
      {
         lit->w1=100;
      }
      if(!EqnIsPureVar(lit->literal))
      {
         lit->w1+=10;
      }
      if(!EqnIsGround(lit->literal))
      {
         lit->w1+=1;
      }
      if(EqnHasAppVar(lit->literal))
      {
         lit->w1+=20;
      }
      lit->w2 = -lit_sel_diff_weight(lit->literal);
      if(EqnIsEquLit(lit->literal))
      {
         lit->w3 = PDArrayElementInt(pd, 0);
      }
      else
      {
         lit->w3 = PDArrayElementInt(pd, lit->literal->lterm->f_code);
      }
   }
}

/*-----------------------------------------------------------------------
//
// Function: maxlcomplexstronglyavoidappvar_weight()
//
//   Initialize weights to mimic SelectMaxLComplexWeight(), but defer
//   literals with applied variables, and put them right after maximal lits
//   and defer literals which occur often in pred_dist.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void maxlcomplexstronglyavoidappvar_weight(LitEval_p lit, Clause_p clause,
                                        void *pred_dist)
{
   PDArray_p pd = pred_dist;

   if(EqnIsNegative(lit->literal))
   {
      if(EqnIsMaximal(lit->literal))
      {
         lit->w1=0;
      }
      else
      {
         lit->w1=100;
      }
      if(!EqnIsPureVar(lit->literal))
      {
         lit->w1+=10;
      }
      if(!EqnIsGround(lit->literal))
      {
         lit->w1+=1;
      }
      if(EqnHasAppVar(lit->literal))
      {
         lit->w1+=200;
      }
      lit->w2 = -lit_sel_diff_weight(lit->literal);
      if(EqnIsEquLit(lit->literal))
      {
         lit->w3 = PDArrayElementInt(pd, 0);
      }
      else
      {
         lit->w3 = PDArrayElementInt(pd, lit->literal->lterm->f_code);
      }
   }
}

/*-----------------------------------------------------------------------
//
// Function: maxlcomplexstronglypreferappvar_weight()
//
//   Initialize weights to mimic SelectMaxLComplexWeight(), but prefer
//   literals with applied variables and
//    defer literals which occur often in pred_dist.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void maxlcomplexstronglypreferappvar_weight(LitEval_p lit, Clause_p clause,
                                                   void *pred_dist)
{
   PDArray_p pd = pred_dist;

   if(EqnIsNegative(lit->literal))
   {
      if(EqnIsMaximal(lit->literal))
      {
         lit->w1=0;
      }
      else
      {
         lit->w1=100;
      }
      if(!EqnIsPureVar(lit->literal))
      {
         lit->w1+=10;
      }
      if(!EqnIsGround(lit->literal))
      {
         lit->w1+=1;
      }
      if(!EqnHasAppVar(lit->literal))
      {
         lit->w1+=200;
      }
      lit->w2 = -lit_sel_diff_weight(lit->literal);
      if(EqnIsEquLit(lit->literal))
      {
         lit->w3 = PDArrayElementInt(pd, 0);
      }
      else
      {
         lit->w3 = PDArrayElementInt(pd, lit->literal->lterm->f_code);
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: SelectMaxLComplexAvoidPosPred()
//
//   As SelectMaxLComplex, but preferably select literals that do not
//   share the predicate symbol with a positive literal.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/


void SelectMaxLComplexAvoidPosPred(OCB_p ocb, Clause_p clause)
{
   long  lit_no;
   PDArray_p pred_dist;

   assert(ocb);
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no==0)
   {
      return;
   }
   ClauseCondMarkMaximalTerms(ocb, clause);

   lit_no = EqnListQueryPropNumber(clause->literals, EPIsMaximal);

   if(lit_no <=1)
   {
      return;
   }
   pred_dist = pos_pred_dist_array_compute(clause);
   generic_uniq_selection(ocb,clause,false, true,
                          maxlcomplexavoidpred_weight, pred_dist);
   pred_dist_array_free(pred_dist);
}


/*-----------------------------------------------------------------------
//
// Function: SelectMaxLComplexAvoidAppVar()
//
//   As SelectMaxLComplex, but preferably select literals that do not
//   have applied variables and the ones that do not
//   share the predicate symbol with a positive literal.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void SelectMaxLComplexAvoidAppVar(OCB_p ocb, Clause_p clause)
{
   generic_app_var_sel(ocb, clause, maxlcomplexavoidappvar_weight);
}

/*-----------------------------------------------------------------------
//
// Function: SelectMaxLComplexStronglyAvoidAppVar()
//
//   As above, but avoids app vars stronger (considers them even before
//   maximality of literals).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void SelectMaxLComplexStronglyAvoidAppVar(OCB_p ocb, Clause_p clause)
{
   generic_app_var_sel(ocb, clause, maxlcomplexstronglyavoidappvar_weight);
}

/*-----------------------------------------------------------------------
//
// Function: SelectMaxLComplexPreferAppVar()
//
//   As above, but prefers app vars (stronger than other conditions).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void SelectMaxLComplexPreferAppVar(OCB_p ocb, Clause_p clause)
{
   generic_app_var_sel(ocb, clause, maxlcomplexstronglypreferappvar_weight);
}


/*-----------------------------------------------------------------------
//
// Function: maxlcomplexappNTNpweight()
//
//   Initialize weights to mimic SelectMaxLComplexWeight(), but defer
//   literals with which occur often in pred_dist. Never select type-
//   and propositional literals.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void maxlcomplexappNTNp_weight(LitEval_p lit, Clause_p clause,
                                      void *pred_dist)
{
   PDArray_p pd = pred_dist;

   if(EqnIsNegative(lit->literal))
   {
      if(EqnIsTypePred(lit->literal)||EqnIsPropositional(lit->literal))
      {
         lit->w1 = 100000;
         lit->forbidden = true;
      }
      else
      {
         if(EqnIsMaximal(lit->literal))
         {
            lit->w1=0;
         }
         else
         {
            lit->w1=100;
         }
      }
      if(!EqnIsPureVar(lit->literal))
      {
         lit->w1+=10;
      }
      if(!EqnIsGround(lit->literal))
      {
         lit->w1+=1;
      }
      lit->w2 = -lit_sel_diff_weight(lit->literal);
      if(EqnIsEquLit(lit->literal))
      {
         lit->w3 = PDArrayElementInt(pd, 0);
      }
      else
      {
         lit->w3 = PDArrayElementInt(pd, lit->literal->lterm->f_code);
      }
   }
}

/*-----------------------------------------------------------------------
//
// Function: SelectMaxLComplexAPPNTNp()
//
//   As SelectMaxLComplexAvoidPosPred, but also avoid propositional
//   and type literals.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/


void SelectMaxLComplexAPPNTNp(OCB_p ocb, Clause_p clause)
{
   long  lit_no;
   PDArray_p pred_dist;

   assert(ocb);
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no==0)
   {
      return;
   }
   ClauseCondMarkMaximalTerms(ocb, clause);

   lit_no = EqnListQueryPropNumber(clause->literals, EPIsMaximal);

   if(lit_no <=1)
   {
      return;
   }
   pred_dist = pos_pred_dist_array_compute(clause);
   generic_uniq_selection(ocb,clause,false, true,
                          maxlcomplexappNTNp_weight, pred_dist);
   pred_dist_array_free(pred_dist);
}




/*-----------------------------------------------------------------------
//
// Function: maxlcomplexavoidprednotype_weight()
//
//   As  maxlcomplexavoidpred_weigth, but never select type literals.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void maxlcomplexavoidprednotype_weight(LitEval_p lit, Clause_p clause,
                                        void *pred_dist)
{
   maxlcomplexavoidpred_weight(lit, clause, pred_dist);
   if(EqnIsTypePred(lit->literal))
   {
      lit->forbidden = true;
   }
}


/*-----------------------------------------------------------------------
//
// Function: SelectMaxLComplexAPPNoType()
//
//   As SelectMaxLComplexAvoidPosPred, but never select type
//   literals.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/


void SelectMaxLComplexAPPNoType(OCB_p ocb, Clause_p clause)
{
   long  lit_no;
   PDArray_p pred_dist;

   assert(ocb);
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no==0)
   {
      return;
   }
   ClauseCondMarkMaximalTerms(ocb, clause);

   lit_no = EqnListQueryPropNumber(clause->literals, EPIsMaximal);

   if(lit_no <=1)
   {
      return;
   }
   pred_dist = pos_pred_dist_array_compute(clause);
   generic_uniq_selection(ocb,clause,false, true,
                          maxlcomplexavoidprednotype_weight, pred_dist);
   pred_dist_array_free(pred_dist);
}




/*-----------------------------------------------------------------------
//
// Function: SelectMaxLComplexAvoidPosUPred()
//
//   As SelectMaxLComplex, but preferably select literals that do not
//   share the predicate symbol with a positive literal.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/


void SelectMaxLComplexAvoidPosUPred(OCB_p ocb, Clause_p clause)
{
   long  lit_no;
   PDArray_p pred_dist;

   assert(ocb);
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   ClauseCondMarkMaximalTerms(ocb, clause);

   lit_no = EqnListQueryPropNumber(clause->literals, EPIsMaximal);

   if(lit_no <=1)
   {
      return;
   }
   pred_dist = pos_pred_dist_array_compute(clause);
   PDArrayAssignInt(pred_dist, 0, 0);

   generic_uniq_selection(ocb,clause,false, true,
                          maxlcomplexavoidpred_weight, pred_dist);
   pred_dist_array_free(pred_dist);
}

/*-----------------------------------------------------------------------
//
// Function: complex_weight
//
//   Implement a weight function mimicking the old SelectComplex.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/


static void complex_weight(LitEval_p lit, Clause_p clause,
                           void *dummy)
{
   if(EqnIsNegative(lit->literal))
   {
      if(EqnIsPureVar(lit->literal))
      {
         lit->w1 = 0;
      }
      else if(EqnIsGround(lit->literal))
      {
         lit->w1 = 10;
         lit->w2 = EqnStandardWeight(lit->literal);
      }
      else
      {
         lit->w1 = 20;
         lit->w2 = -lit_sel_diff_weight(lit->literal);
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: SelectComplexG()
//
//   Simulate old SelectComplex() with new schema
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void SelectComplexG(OCB_p ocb, Clause_p clause)
{
   generic_uniq_selection(ocb,clause,false, false,
                          complex_weight,
                          NULL);
}


/*-----------------------------------------------------------------------
//
// Function: complex_weight_ahp()
//
//   Implement a weight function mimicking the old SelectComplex, but,
//   other things being equal, avoid head predicate symbols.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/


static void complex_weight_ahp(LitEval_p lit, Clause_p clause,
                           void *pred_dist)
{
   PDArray_p pd = pred_dist;

   assert(clause);

   if(EqnIsNegative(lit->literal))
   {
      if(EqnIsPureVar(lit->literal))
      {
         lit->w1 = 0;
      }
      else if(EqnIsGround(lit->literal))
      {
         lit->w1 = 10;
         lit->w2 = EqnStandardWeight(lit->literal);
      }
      else
      {
         lit->w1 = 20;
         lit->w2 = -lit_sel_diff_weight(lit->literal);
      }
   }
   lit->w3 = 0;
   if(lit->literal->lterm->f_code > 0)
   {
      lit->w3 = PDArrayElementInt(pd, lit->literal->lterm->f_code);
   }
}


/*-----------------------------------------------------------------------
//
// Function: SelectComplexAHP()
//
//   As SelectComplexG, but preferably select literals that do not
//   share the predicate symbol with a positive literal.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/


void SelectComplexAHP(OCB_p ocb, Clause_p clause)
{
   PDArray_p pred_dist;

   assert(ocb);
   assert(clause);
   assert(clause->neg_lit_no);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   pred_dist = pos_pred_dist_array_compute(clause);

   generic_uniq_selection(ocb,clause,false, true,
                          complex_weight_ahp, pred_dist);
   pred_dist_array_free(pred_dist);
}


/*-----------------------------------------------------------------------
//
// Function: PSelectComplexAHP()
//
//   As SelectComplexAHP, but also select positive literals.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/


void PSelectComplexAHP(OCB_p ocb, Clause_p clause)
{
   PDArray_p pred_dist;

   assert(ocb);
   assert(clause);
   assert(clause->neg_lit_no);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   pred_dist = pos_pred_dist_array_compute(clause);

   generic_uniq_selection(ocb,clause,true, true,
                          complex_weight_ahp, pred_dist);
   pred_dist_array_free(pred_dist);
}


/*-----------------------------------------------------------------------
//
// Function: new_complex_notp_ahp()
//
//   Implement a weight function mimicking the old SelectNewComplex,
//   but, other things being equal, avoid head predicate symbols.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/


static void new_complex_notp_ahp(LitEval_p lit, Clause_p clause,
                                 void *pred_dist)
{
   PDArray_p pd = pred_dist;

   assert(clause);

   if(EqnIsNegative(lit->literal))
   {
      if(EqnIsGround(lit->literal))
      {
         lit->w1 = 0;
         lit->w2 = TermStandardWeight(lit->literal->lterm);
      }
      else if(!EqnIsXTypePred(lit->literal))
      {
         lit->w1 = 10;
         lit->w2 = EqnMaxTermPositions(lit->literal);
      }
      else if(!EqnIsTypePred(lit->literal))
      {
         lit->w1 = 20;
         lit->w2 = -TermStandardWeight(lit->literal->lterm);
      }
      else
      {
         assert(EqnIsTypePred(lit->literal));
         lit->w1 = 100000;
         lit->forbidden = 1;
      }
   }
   lit->w3 = 0;
   if(!TermIsFreeVar(lit->literal->lterm))
   {
      lit->w3 = PDArrayElementInt(pd, lit->literal->lterm->f_code);
   }
}




/*-----------------------------------------------------------------------
//
// Function: SelectNewComplexAHP()
//
//   Mimic SelectNewComplex(),  but with the AHP property.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void SelectNewComplexAHP(OCB_p ocb, Clause_p clause)
{
   PDArray_p pred_dist;

   assert(ocb);
   assert(clause);
   assert(clause->neg_lit_no);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   pred_dist = pos_pred_dist_array_compute(clause);

   generic_uniq_selection(ocb,clause,false, true,
                          new_complex_notp_ahp, pred_dist);
   pred_dist_array_free(pred_dist);
}


/*-----------------------------------------------------------------------
//
// Function: PSelectNewComplexAHP
//
//   Mimic PSelectNewComplex(),  but with the AHP property.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void PSelectNewComplexAHP(OCB_p ocb, Clause_p clause)
{
   PDArray_p pred_dist;

   assert(ocb);
   assert(clause);
   assert(clause->neg_lit_no);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   pred_dist = pos_pred_dist_array_compute(clause);

   generic_uniq_selection(ocb,clause, true, true,
                          new_complex_notp_ahp, pred_dist);
   pred_dist_array_free(pred_dist);
}




/*-----------------------------------------------------------------------
//
// Function: SelectComplexAHPExceptRRHorn()
//
//   If a clause is Horn and range-restricted, do no select. Otherwise
//   use SelectComplexAHP() (above).
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void SelectComplexAHPExceptRRHorn(OCB_p ocb, Clause_p clause)
{
   if(!(ClauseIsHorn(clause) && ClauseIsRangeRestricted(clause)))
   {
      SelectComplexAHP(ocb, clause);
   }
}


/*-----------------------------------------------------------------------
//
// Function: PSelectComplexAHPExceptRRHorn()
//
//   If a clause is Horn and range-restricted, do no select. Otherwise
//   use PSelectComplexAHP() (above).
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void PSelectComplexAHPExceptRRHorn(OCB_p ocb, Clause_p clause)
{
   if(!(ClauseIsHorn(clause) && ClauseIsRangeRestricted(clause)))
   {
      PSelectComplexAHP(ocb, clause);
   }
}


/*-----------------------------------------------------------------------
//
// Function: SelectNewComplexAHPExceptRRHorn()
//
//   If a clause is Horn and range-restricted, do no select. Otherwise
//   use SelectNewComplexAHP() (above).
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void SelectNewComplexAHPExceptRRHorn(OCB_p ocb, Clause_p clause)
{
   if(!(ClauseIsHorn(clause) && ClauseIsRangeRestricted(clause)))
   {
      SelectNewComplexAHP(ocb, clause);
   }
}


/*-----------------------------------------------------------------------
//
// Function: PSelectNewComplexAHPExceptRRHorn()
//
//   If a clause is Horn and range-restricted, do no select. Otherwise
//   use PSelectNewComplexAHP() (above).
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void PSelectNewComplexAHPExceptRRHorn(OCB_p ocb, Clause_p clause)
{
   if(!(ClauseIsHorn(clause) && ClauseIsRangeRestricted(clause)))
   {
      PSelectNewComplexAHP(ocb, clause);
   }
}


/*-----------------------------------------------------------------------
//
// Function: SelectNewComplexAHPExceptUniqMaxHorn()
//
//   Select literal as in SelectNewComplexAHP unless the clause is a
//   Horn clause with a unique maximal literal.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void SelectNewComplexAHPExceptUniqMaxHorn(OCB_p ocb, Clause_p clause)
{
   if(ClauseIsHorn(clause))
   {
      ClauseCondMarkMaximalTerms(ocb, clause);
      if(EqnListQueryPropNumber(clause->literals, EPIsMaximal)==1)
      {
    return;
      }
   }
   SelectNewComplexAHP(ocb,clause);
}



/*-----------------------------------------------------------------------
//
// Function: PSelectNewComplexAHPExceptUniqMaxHorn()
//
//   Select literal as in PSelectNewComplexAHP unless the clause is a
//   Horn clause with a unique maximal literal.
//
// Global Variables: -
//
// Side Effects    : Changes property in literals
//
/----------------------------------------------------------------------*/

void PSelectNewComplexAHPExceptUniqMaxHorn(OCB_p ocb, Clause_p clause)
{
   if(ClauseIsHorn(clause))
   {
      ClauseCondMarkMaximalTerms(ocb, clause);
      if(EqnListQueryPropNumber(clause->literals, EPIsMaximal)==1)
      {
    return;
      }
   }
   PSelectNewComplexAHP(ocb,clause);
}





/*-----------------------------------------------------------------------
//
// Function: new_complex_notp_ahp_ns
//
//   Implement a weight function mimicking the old SelectNewComplex,
//   but, other things being equal, avoid head predicate
//   symbols. Always avoid split symbols.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/


static void new_complex_notp_ahp_ns(LitEval_p lit, Clause_p clause,
                                    void *pred_dist)
{
   PDArray_p pd = pred_dist;

   assert(clause);

   if(EqnIsNegative(lit->literal))
   {
      if(EqnIsSplitLit(lit->literal))
      {
         lit->w1 = 100000;
         lit->forbidden = 1;
      }
      else if(EqnIsGround(lit->literal))
      {
         lit->w1 = 0;
         lit->w2 = TermStandardWeight(lit->literal->lterm);
      }
      else if(!EqnIsXTypePred(lit->literal))
      {
         lit->w1 = 10;
         lit->w2 = EqnMaxTermPositions(lit->literal);
      }
      else if(!EqnIsTypePred(lit->literal))
      {
         lit->w1 = 20;
         lit->w2 = -TermStandardWeight(lit->literal->lterm);
      }
      else
      {
         assert(EqnIsTypePred(lit->literal));
         lit->w1 = 100000;
         lit->forbidden = 1;
      }
   }
   lit->w3 = 0;
   if(!TermIsFreeVar(lit->literal->lterm))
   {
      lit->w3 = PDArrayElementInt(pd, lit->literal->lterm->f_code);
   }
}




/*-----------------------------------------------------------------------
//
// Function: SelectNewComplexAHPNS
//
//   Mimic SelectNewComplex(),  but with the AHP property and never
//   select split literals.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void SelectNewComplexAHPNS(OCB_p ocb, Clause_p clause)
{
   PDArray_p pred_dist;

   assert(ocb);
   assert(clause);
   assert(clause->neg_lit_no);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   pred_dist = pos_pred_dist_array_compute(clause);

   generic_uniq_selection(ocb,clause,false, true,
                          new_complex_notp_ahp_ns, pred_dist);
   pred_dist_array_free(pred_dist);
}


/*-----------------------------------------------------------------------
//
// Function: SelectVGNonCR()
//
//   If there is a negative pure variable literal, select it.
//   If there is a negative ground literal, select the smallest one.
//   Otherwise, if there is a unique positive maximal literal, don't
//   select.
//   Otherwise select as SelectNewComplexAHPNS.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void SelectVGNonCR(OCB_p ocb, Clause_p clause)
{
   Eqn_p     handle;
   int       maxlits, maxposlits;
   PDArray_p pred_dist;

   assert(ocb);
   assert(clause);
   assert(clause->neg_lit_no);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   handle = ClauseFindNegPureVarLit(clause);

   if(handle)
   {
      EqnSetProp(handle, EPIsSelected);
      return;
   }

   ClauseCondMarkMaximalTerms(ocb, clause);
   handle = find_smallest_neg_ground_lit(clause);
   if(handle)
   {
      EqnSetProp(handle, EPIsSelected);
      return;
   }
   maxlits = EqnListQueryPropNumber(clause->literals,
                                    EPIsMaximal);
   if(maxlits == 1)
   {
      maxposlits = EqnListQueryPropNumber(clause->literals,
                                          EPIsMaximal|EPIsPositive);
      if(maxposlits == 1)
      {
         return;
      }
   }
   pred_dist = pos_pred_dist_array_compute(clause);

   generic_uniq_selection(ocb,clause,false, true,
                          new_complex_notp_ahp_ns, pred_dist);
   pred_dist_array_free(pred_dist);
}


/*-----------------------------------------------------------------------
//
// Function: select_cq_ar_eql_weight()
// Function: SelectCQArEqLast()
//
//   Select based on a total ordering on predicate symbols. Preferably
//   select symbols with high arity. Equality is always selected last.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void select_cq_ar_eql_weight(LitEval_p lit, Clause_p clause,
                                    void* dummy)
{
   Eqn_p l = lit->literal;

   if(EqnIsEquLit(l) || TermIsFreeVar(l->lterm))
   {
      lit->w1 = 1000000;
      lit->w2 = 0;
   }
   else
   {
      lit->w1 = -SigFindArity(l->bank->sig, l->lterm->f_code);
      lit->w2 = SigGetAlphaRank(l->bank->sig, l->lterm->f_code);
   }
   lit->w3 =lit_sel_diff_weight(l);
}

void SelectCQArEqLast(OCB_p ocb, Clause_p clause)
{
   assert(ocb);
   assert(clause);
   assert(clause->neg_lit_no);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   generic_uniq_selection(ocb,clause,false, true,
                          select_cq_ar_eql_weight, NULL);
}


/*-----------------------------------------------------------------------
//
// Function: select_cq_ar_eqf_weight()
// Function: SelectCQArEqFirst()
//
//   Select based on a total ordering on predicate symbols. Preferably
//   select symbols with high arity. Equality is always selected
//   first.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void select_cq_ar_eqf_weight(LitEval_p lit, Clause_p clause,
                                    void* dummy)
{
   Eqn_p l = lit->literal;

   if(EqnIsEquLit(l) || TermIsFreeVar(l->lterm))
   {
      lit->w1 = -100000;
      lit->w2 = 0;
   }
   else
   {
      lit->w1 = -SigFindArity(l->bank->sig, l->lterm->f_code);
      lit->w2 = SigGetAlphaRank(l->bank->sig, l->lterm->f_code);
   }
   lit->w3 =lit_sel_diff_weight(l);
}

void SelectCQArEqFirst(OCB_p ocb, Clause_p clause)
{
   assert(ocb);
   assert(clause);
   assert(clause->neg_lit_no);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   generic_uniq_selection(ocb,clause,false, true,
                          select_cq_ar_eqf_weight, NULL);
}

/*-----------------------------------------------------------------------
//
// Function: select_cq_iar_eql_weight()
// Function: SelectCQIArEqLast()
//
//   Select based on a total ordering on predicate symbols. Preferably
//   select symbols with low arity. Equality is always selected
//   last.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void select_cq_iar_eql_weight(LitEval_p lit, Clause_p clause,
                                    void* dummy)
{
   Eqn_p l = lit->literal;

   if(EqnIsEquLit(l) || TermIsFreeVar(l->lterm))
   {
      lit->w1 = 100000;
      lit->w2 = 0;
   }
   else
   {
      lit->w1 = SigFindArity(l->bank->sig, l->lterm->f_code);
      lit->w2 = SigGetAlphaRank(l->bank->sig, l->lterm->f_code);
   }
   lit->w3 =lit_sel_diff_weight(l);
}

void SelectCQIArEqLast(OCB_p ocb, Clause_p clause)
{
   assert(ocb);
   assert(clause);
   assert(clause->neg_lit_no);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   generic_uniq_selection(ocb,clause,false, true,
                          select_cq_iar_eql_weight, NULL);
}


/*-----------------------------------------------------------------------
//
// Function: select_cq_iar_eqf_weight()
// Function: SelectCQIArEqFirst()
//
//   Select based on a total ordering on predicate symbols. Preferable
//   select symbols with low arity. Equality is always selected
//   first.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void select_cq_iar_eqf_weight(LitEval_p lit, Clause_p clause,
                                    void* dummy)
{
   Eqn_p l = lit->literal;

   if(EqnIsEquLit(l) || TermIsFreeVar(l->lterm))
   {
      lit->w1 = -100000;
      lit->w2 = 0;
   }
   else
   {
      lit->w1 = SigFindArity(l->bank->sig, l->lterm->f_code);
      lit->w2 = SigGetAlphaRank(l->bank->sig, l->lterm->f_code);
   }
   lit->w3 =lit_sel_diff_weight(l);
}

void SelectCQIArEqFirst(OCB_p ocb, Clause_p clause)
{
   assert(ocb);
   assert(clause);
   assert(clause->neg_lit_no);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   generic_uniq_selection(ocb,clause,false, true,
                          select_cq_iar_eqf_weight, NULL);
}



/*-----------------------------------------------------------------------
//
// Function: select_cq_ar_weight()
// Function: SelectCQAr()
//
//   Select based on a total ordering on predicate symbols. Preferably
//   select symbols with high arity. Equality is treated as a normal
//   predicate symbol (nearly).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/


static void select_cq_ar_weight(LitEval_p lit, Clause_p clause,
                         void* dummy)
{
   Eqn_p l = lit->literal;

   if(EqnIsEquLit(l) || TermIsFreeVar(l->lterm))
   {
      lit->w1 = -2;
      lit->w2 = l->lterm->f_code > 0
         ? SigGetAlphaRank(l->bank->sig, l->lterm->f_code)
         : 0;
   }
   else
   {
      lit->w1 = -SigFindArity(l->bank->sig, l->lterm->f_code);
      lit->w2 = SigGetAlphaRank(l->bank->sig, l->lterm->f_code);
   }
   lit->w3 =lit_sel_diff_weight(l);
}

void SelectCQAr(OCB_p ocb, Clause_p clause)
{
   assert(ocb);
   assert(clause);
   assert(clause->neg_lit_no);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   generic_uniq_selection(ocb,clause,false, true,
                          select_cq_ar_weight, NULL);
}



/*-----------------------------------------------------------------------
//
// Function: select_cq_iar_weight()
// Function: SelectCQIAr()
//
//   Select based on a total ordering on predicate symbols. Preferably
//   select symbols with low arity. Equality is treated as a normal
//   predicate symbol (nearly).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void select_cq_iar_weight(LitEval_p lit, Clause_p clause,
                                    void* dummy)
{
   Eqn_p l = lit->literal;

   if(EqnIsEquLit(l) || TermIsFreeVar(l->lterm))
   {
     lit->w1 = 2;
      lit->w2 = l->lterm->f_code > 0
         ? SigGetAlphaRank(l->bank->sig, l->lterm->f_code)
         : 0;
   }
   else
   {
      lit->w1 = SigFindArity(l->bank->sig, l->lterm->f_code);
      lit->w2 = SigGetAlphaRank(l->bank->sig, l->lterm->f_code);
   }
   lit->w3 =lit_sel_diff_weight(l);
}


void SelectCQIAr(OCB_p ocb, Clause_p clause)
{
   assert(ocb);
   assert(clause);
   assert(clause->neg_lit_no);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   generic_uniq_selection(ocb,clause,false, true,
                          select_cq_iar_weight, NULL);
}


/*-----------------------------------------------------------------------
//
// Function: select_cq_arnp_eqf_weight()
// Function: SelectCQArNpEqFirst()
//
//   Select based on a total ordering on predicate symbols. Preferably
//   select symbols with high arity. Equaliy comes
//   first. Propositional symbols are never selected.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void select_cq_arnp_eqf_weight(LitEval_p lit, Clause_p clause,
                                    void* dummy)
{
   Eqn_p l = lit->literal;

   if(EqnIsEquLit(l) || TermIsFreeVar(l->lterm))
   {
      lit->w1 = -100000;
      lit->w2 = 0;
   }
   else
   {
      lit->w1 = -SigFindArity(l->bank->sig, l->lterm->f_code);
      lit->w2 = SigGetAlphaRank(l->bank->sig, l->lterm->f_code);
      if(lit->w1 == 0)
      {
         lit->w1 = 100000;
         lit->forbidden = true;
      }
   }
   lit->w3 =lit_sel_diff_weight(l);
}


void SelectCQArNpEqFirst(OCB_p ocb, Clause_p clause)
{
   assert(ocb);
   assert(clause);
   assert(clause->neg_lit_no);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   generic_uniq_selection(ocb,clause,false, true,
                          select_cq_arnp_eqf_weight, NULL);
}



/*-----------------------------------------------------------------------
//
// Function: select_cq_iarnp_eqf_weight()
// Function: SelectCQIArNpEqFirst()
//
//   Select based on a total ordering on predicate symbols. Preferably
//   select symbols with low arity. Equaliy comes
//   first. Propositional symbols are never selected.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void select_cq_iarnp_eqf_weight(LitEval_p lit, Clause_p clause,
                                    void* dummy)
{
   Eqn_p l = lit->literal;

   if(EqnIsEquLit(l) || TermIsFreeVar(l->lterm))
   {
      lit->w1 = -1000000;
      lit->w2 = 0;
   }
   else
   {
      lit->w1 = SigFindArity(l->bank->sig, l->lterm->f_code);
      lit->w2 = SigGetAlphaRank(l->bank->sig, l->lterm->f_code);
      if(lit->w1 == 0)
      {
         lit->w1 = 1000000;
         lit->forbidden = true;
      }
   }
   lit->w3 =lit_sel_diff_weight(l);
}

void SelectCQIArNpEqFirst(OCB_p ocb, Clause_p clause)
{
   assert(ocb);
   assert(clause);
   assert(clause->neg_lit_no);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   generic_uniq_selection(ocb,clause,false, true,
                          select_cq_iarnp_eqf_weight, NULL);
}


/*-----------------------------------------------------------------------
//
// Function: select_grcq_ar_eqf_weight()
// Function: SelectGrCQArEqFirst()
//
//   Select ground literals first, then others. Among
//   ground/nonground, select symbols with high arity. Equality is
//   always selected first.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

static void select_grcq_ar_eqf_weight(LitEval_p lit, Clause_p clause,
                                      void* dummy)
{
   Eqn_p l = lit->literal;

   if(EqnIsEquLit(l) || TermIsFreeVar(l->lterm))
   {
      lit->w1 = -1000000;
      lit->w2 = 0;
   }
   else
   {
      lit->w1 = -SigFindArity(l->bank->sig, l->lterm->f_code);
      lit->w2 = SigGetAlphaRank(l->bank->sig, l->lterm->f_code);
   }
   lit->w3 =lit_sel_diff_weight(l);
   if(EqnIsGround(l))
   {
      lit->w1 -= 2000000;
   }
}


void SelectGrCQArEqFirst(OCB_p ocb, Clause_p clause)
{
   assert(ocb);
   assert(clause);
   assert(clause->neg_lit_no);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   generic_uniq_selection(ocb,clause,false, true,
                          select_grcq_ar_eqf_weight, NULL);
}


/*-----------------------------------------------------------------------
//
// Function: select_cqgr_ar_eqf_weight()
// Function: SelectCQGrArEqFirst()
//
//   Select based on a total ordering on predicate symbols. Preferably
//   select symbols with low arity. Equality is always selected
//   first. Among literals with the same symbol, prefer ground.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void select_cqgr_ar_eqf_weight(LitEval_p lit, Clause_p clause,
                                      void* dummy)
{
   Eqn_p l = lit->literal;

   if(EqnIsEquLit(l) || TermIsFreeVar(l->lterm))
   {
      lit->w1 = -1000000;
      lit->w2 = 0;
   }
   else
   {
      lit->w1 = -SigFindArity(l->bank->sig, l->lterm->f_code);
      lit->w2 = SigGetAlphaRank(l->bank->sig, l->lterm->f_code);
   }
   lit->w3 =lit_sel_diff_weight(l);
   if(EqnIsGround(l))
   {
      lit->w2 -= 2000000;
   }
}

void SelectCQGrArEqFirst(OCB_p ocb, Clause_p clause)
{
   assert(ocb);
   assert(clause);
   assert(clause->neg_lit_no);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   generic_uniq_selection(ocb,clause,false, true,
                          select_cqgr_ar_eqf_weight, NULL);
}


/*-----------------------------------------------------------------------
//
// Function: select_cq_arnt_eqf_weight()
// Function: SelectCQArNTEqFirst()
//
//   Select based on a total ordering on predicate symbols. Preferably
//   select symbols with low arity. Equality comes
//   first. Type literals p(X) are never selected.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void select_cq_arnt_eqf_weight(LitEval_p lit, Clause_p clause,
                                    void* dummy)
{
   Eqn_p l = lit->literal;

   if(EqnIsEquLit(l) || TermIsFreeVar(l->lterm))
   {
      lit->w1 = -100000;
      lit->w2 = 0;
   }
   else
   {
      lit->w1 = -SigFindArity(l->bank->sig, l->lterm->f_code);
      lit->w2 = SigGetAlphaRank(l->bank->sig, l->lterm->f_code);
      if(EqnIsTypePred(l))
      {
         lit->w1 = 100000;
         lit->forbidden = true;
      }
   }
   lit->w3 =lit_sel_diff_weight(l);
}

void SelectCQArNTEqFirst(OCB_p ocb, Clause_p clause)
{
   assert(ocb);
   assert(clause);
   assert(clause->neg_lit_no);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   generic_uniq_selection(ocb,clause,false, true,
                          select_cq_arnt_eqf_weight, NULL);
}


/*-----------------------------------------------------------------------
//
// Function: select_cq_iarnt_eqf_weight()
// Function: SelectCQIArNTEqFirst()
//
//   Select based on a total ordering on predicate symbols. Preferably
//   select symbols with low arity. Equality comes
//   first. Type literals p(X) are never selected.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void select_cq_iarnt_eqf_weight(LitEval_p lit, Clause_p clause,
                                    void* dummy)
{
   Eqn_p l = lit->literal;

   if(EqnIsEquLit(l) || TermIsFreeVar(l->lterm))
   {
      lit->w1 = -100000;
      lit->w2 = 0;
   }
   else
   {
      lit->w1 = SigFindArity(l->bank->sig, l->lterm->f_code);
      lit->w2 = SigGetAlphaRank(l->bank->sig, l->lterm->f_code);
      if(EqnIsTypePred(l))
      {
         lit->w1 = 100000;
         lit->forbidden = true;
      }
   }
   lit->w3 =lit_sel_diff_weight(l);
}

void SelectCQIArNTEqFirst(OCB_p ocb, Clause_p clause)
{
   assert(ocb);
   assert(clause);
   assert(clause->neg_lit_no);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   generic_uniq_selection(ocb,clause,false, true,
                          select_cq_iarnt_eqf_weight, NULL);
}


/*-----------------------------------------------------------------------
//
// Function: select_cq_arntnp_eqf_weight()
// Function: SelectCQArNTNpEqFirst()
//
//   Select based on a total ordering on predicate symbols. Preferably
//   select symbols with low arity. Equality comes
//   first. Type literals p(X) and propositional lierals are never
//   selected.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void select_cq_arntnp_eqf_weight(LitEval_p lit, Clause_p clause,
                                        void* dummy)
{
   Eqn_p l = lit->literal;

   if(EqnIsEquLit(l) || TermIsFreeVar(l->lterm))
   {
      lit->w1 = -100000;
      lit->w2 = 0;
   }
   else
   {
      lit->w1 = -SigFindArity(l->bank->sig, l->lterm->f_code);
      lit->w2 = SigGetAlphaRank(l->bank->sig, l->lterm->f_code);
      if(EqnIsTypePred(l)||EqnIsPropositional(l))
      {
         lit->w1 = 100000;
         lit->forbidden = true;
      }
   }
   lit->w3 =lit_sel_diff_weight(l);
}

void SelectCQArNTNpEqFirst(OCB_p ocb, Clause_p clause)
{
   assert(ocb);
   assert(clause);
   assert(clause->neg_lit_no);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   generic_uniq_selection(ocb,clause,false, true,
                          select_cq_arntnp_eqf_weight, NULL);
}


/*-----------------------------------------------------------------------
//
// Function: select_cq_iarntnp_eqf_weight()
// Function: SelectCQIArNTNpEqFirst()
//
//   Select based on a total ordering on predicate symbols. Preferably
//   select symbols with low arity. Equality comes
//   first. Type literals p(X) and propositional lierals are never
//   selected.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void select_cq_iarntnp_eqf_weight(LitEval_p lit, Clause_p clause,
                                        void* dummy)
{
   Eqn_p l = lit->literal;

   if(EqnIsEquLit(l) || TermIsFreeVar(l->lterm))
   {
      lit->w1 = -100000;
      lit->w2 = 0;
   }
   else
   {
      lit->w1 = SigFindArity(l->bank->sig, l->lterm->f_code);
      lit->w2 = SigGetAlphaRank(l->bank->sig, l->lterm->f_code);
      if(EqnIsTypePred(l)||EqnIsPropositional(l))
      {
         lit->w1 = 100000;
         lit->forbidden = true;
      }
   }
   lit->w3 =lit_sel_diff_weight(l);
}

void SelectCQIArNTNpEqFirst(OCB_p ocb, Clause_p clause)
{
   assert(ocb);
   assert(clause);
   assert(clause->neg_lit_no);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   generic_uniq_selection(ocb,clause,false, true,
                          select_cq_iarntnp_eqf_weight, NULL);
}


/*-----------------------------------------------------------------------
//
// Function: select_cq_arnxt_eqf_weight()
// Function: SelectCQArNXTEqFirst()
//
//   Select based on a total ordering on predicate symbols. Preferably
//   select symbols with low arity. Equality comes
//   first. XType literals p(X,Y,...) are never selected.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void select_cq_arnxt_eqf_weight(LitEval_p lit, Clause_p clause,
                                    void* dummy)
{
   Eqn_p l = lit->literal;

   if(EqnIsEquLit(l) || TermIsFreeVar(l->lterm))
   {
      lit->w1 = -100000;
      lit->w2 = 0;
   }
   else
   {
      lit->w1 = -SigFindArity(l->bank->sig, l->lterm->f_code);
      lit->w2 = SigGetAlphaRank(l->bank->sig, l->lterm->f_code);
      if(EqnIsXTypePred(l))
      {
         lit->w1 = 100000;
         lit->forbidden = true;
      }
   }
   lit->w3 =lit_sel_diff_weight(l);
}

void SelectCQArNXTEqFirst(OCB_p ocb, Clause_p clause)
{
   assert(ocb);
   assert(clause);
   assert(clause->neg_lit_no);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   generic_uniq_selection(ocb,clause,false, true,
                          select_cq_arnxt_eqf_weight, NULL);
}


/*-----------------------------------------------------------------------
//
// Function: select_cq_iarnxt_eqf_weight()
// Function: SelectCQIArNXTEqFirst()
//
//   Select based on a total ordering on predicate symbols. Preferably
//   select symbols with low arity. Equality comes
//   first. XType literals p(X, Y,...) are never selected.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/


static void select_cq_iarnxt_eqf_weight(LitEval_p lit, Clause_p clause,
                                    void* dummy)
{
   Eqn_p l = lit->literal;

   if(EqnIsEquLit(l) || TermIsFreeVar(l->lterm))
   {
      lit->w1 = -100000;
      lit->w2 = 0;
   }
   else
   {
      lit->w1 = SigFindArity(l->bank->sig, l->lterm->f_code);
      lit->w2 = SigGetAlphaRank(l->bank->sig, l->lterm->f_code);
      if(EqnIsXTypePred(l))
      {
         lit->w1 = 100000;
         lit->forbidden = true;
      }
   }
   lit->w3 =lit_sel_diff_weight(l);
}

void SelectCQIArNXTEqFirst(OCB_p ocb, Clause_p clause)
{
   assert(ocb);
   assert(clause);
   assert(clause->neg_lit_no);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   generic_uniq_selection(ocb,clause,false, true,
                          select_cq_iarnxt_eqf_weight, NULL);
}



/*-----------------------------------------------------------------------
//
// Function: SelectCQArNTNp()
//
//   Select based on a total ordering on predicate symbols. Preferably
//   select symbols with high arity. Never select propositional and
//   type predicates.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void select_cq_arntnp_weight(LitEval_p lit, Clause_p clause,
                         void* dummy)
{
   Eqn_p l = lit->literal;

   if(EqnIsEquLit(l) || TermIsFreeVar(l->lterm))
   {
      lit->w1 = -2;
      lit->w2 = l->lterm->f_code > 0
         ? SigGetAlphaRank(l->bank->sig, l->lterm->f_code)
         : 0;
   }
   else
   {
      lit->w1 = -SigFindArity(l->bank->sig, l->lterm->f_code);
      lit->w2 = SigGetAlphaRank(l->bank->sig, l->lterm->f_code);
      if(EqnIsTypePred(l)||EqnIsPropositional(l))
      {
         lit->w1 = 100000;
         lit->forbidden = true;
      }
   }
   lit->w3 =lit_sel_diff_weight(l);
}

void SelectCQArNTNp(OCB_p ocb, Clause_p clause)
{
   assert(ocb);
   assert(clause);
   assert(clause->neg_lit_no);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   generic_uniq_selection(ocb,clause,false, true,
                          select_cq_arntnp_weight, NULL);
}


/*-----------------------------------------------------------------------
//
// Function: SelectCQIArNTNp()
//
//   Select based on a total ordering on predicate symbols. Preferably
//   select symbols with low arity. Never select propositional and
//   type predicates.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void select_cq_iarntnp_weight(LitEval_p lit, Clause_p clause,
                         void* dummy)
{
   Eqn_p l = lit->literal;

   if(EqnIsEquLit(l) || TermIsFreeVar(l->lterm))
   {
      lit->w1 = 2;
      lit->w2 = l->lterm->f_code > 0
         ? SigGetAlphaRank(l->bank->sig, l->lterm->f_code)
         : 0;
   }
   else
   {
      lit->w1 = SigFindArity(l->bank->sig, l->lterm->f_code);
      lit->w2 = SigGetAlphaRank(l->bank->sig, l->lterm->f_code);
      if(EqnIsTypePred(l)||EqnIsPropositional(l))
      {
         lit->w1 = 100000;
         lit->forbidden = true;
      }
   }
   lit->w3 =lit_sel_diff_weight(l);
}

void SelectCQIArNTNp(OCB_p ocb, Clause_p clause)
{
   assert(ocb);
   assert(clause);
   assert(clause->neg_lit_no);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   generic_uniq_selection(ocb,clause,false, true,
                          select_cq_iarntnp_weight, NULL);
}


/*-----------------------------------------------------------------------
//
// Function: SelectCQArNT()
//
//   Select based on a total ordering on predicate symbols. Preferably
//   select symbols with high arity. Never select type predicates.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void select_cq_arnt_weight(LitEval_p lit, Clause_p clause,
                         void* dummy)
{
   Eqn_p l = lit->literal;

   if(EqnIsEquLit(l) || TermIsFreeVar(l->lterm))
   {
      lit->w1 = -2;
      lit->w2 = l->lterm->f_code > 0
         ? SigGetAlphaRank(l->bank->sig, l->lterm->f_code)
         : 0;
   }
   else
   {
      lit->w1 = -SigFindArity(l->bank->sig, l->lterm->f_code);
      lit->w2 = SigGetAlphaRank(l->bank->sig, l->lterm->f_code);
      if(EqnIsTypePred(l))
      {
         lit->w1 = 100000;
         lit->forbidden = true;
      }
   }
   lit->w3 =lit_sel_diff_weight(l);
}

void SelectCQArNT(OCB_p ocb, Clause_p clause)
{
   assert(ocb);
   assert(clause);
   assert(clause->neg_lit_no);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   generic_uniq_selection(ocb,clause,false, true,
                          select_cq_arnt_weight, NULL);
}


/*-----------------------------------------------------------------------
//
// Function: SelectCQIArNT()
//
//   Select based on a total ordering on predicate symbols. Preferably
//   select symbols with low arity. Never select type predicates.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void select_cq_iarnt_weight(LitEval_p lit, Clause_p clause,
                         void* dummy)
{
   Eqn_p l = lit->literal;

   if(EqnIsEquLit(l) || TermIsFreeVar(l->lterm))
   {
      lit->w1 = 2;
      lit->w2 = l->lterm->f_code > 0
         ? SigGetAlphaRank(l->bank->sig, l->lterm->f_code)
         : 0;
   }
   else
   {
      lit->w1 = SigFindArity(l->bank->sig, l->lterm->f_code);
      lit->w2 = SigGetAlphaRank(l->bank->sig, l->lterm->f_code);
      if(EqnIsTypePred(l))
      {
         lit->w1 = 100000;
         lit->forbidden = true;
      }
   }
   lit->w3 =lit_sel_diff_weight(l);
}

void SelectCQIArNT(OCB_p ocb, Clause_p clause)
{
   assert(ocb);
   assert(clause);
   assert(clause->neg_lit_no);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   generic_uniq_selection(ocb,clause,false, true,
                          select_cq_iarnt_weight, NULL);
}


/*-----------------------------------------------------------------------
//
// Function: SelectCQArNp()
//
//   Select based on a total ordering on predicate symbols. Preferably
//   select symbols with high arity. Never select propositional predicates.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void select_cq_arnp_weight(LitEval_p lit, Clause_p clause,
                                  void* dummy)
{
   Eqn_p l = lit->literal;

   if(EqnIsEquLit(l) || TermIsFreeVar(l->lterm))
   {
      lit->w1 = -2;
      lit->w2 = l->lterm->f_code > 0
         ? SigGetAlphaRank(l->bank->sig, l->lterm->f_code)
         : 0;
   }
   else
   {
      lit->w1 = -SigFindArity(l->bank->sig, l->lterm->f_code);
      lit->w2 = SigGetAlphaRank(l->bank->sig, l->lterm->f_code);
      if(EqnIsPropositional(l))
      {
         lit->w1 = 100000;
         lit->forbidden = true;
      }
   }
   lit->w3 =lit_sel_diff_weight(l);
}

void SelectCQArNp(OCB_p ocb, Clause_p clause)
{
   assert(ocb);
   assert(clause);
   assert(clause->neg_lit_no);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   generic_uniq_selection(ocb,clause,false, true,
                          select_cq_arnp_weight, NULL);
}


/*-----------------------------------------------------------------------
//
// Function: SelectCQIArNp()
//
//   Select based on a total ordering on predicate symbols. Preferably
//   select symbols with low arity. Never select propositional
//   predicates.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void select_cq_iarnp_weight(LitEval_p lit, Clause_p clause,
                         void* dummy)
{
   Eqn_p l = lit->literal;

   if(EqnIsEquLit(l) || TermIsFreeVar(l->lterm))
   {
      lit->w1 = 2;
      lit->w2 = l->lterm->f_code > 0
         ? SigGetAlphaRank(l->bank->sig, l->lterm->f_code)
         : 0;
   }
   else
   {
      lit->w1 = SigFindArity(l->bank->sig, l->lterm->f_code);
      lit->w2 = SigGetAlphaRank(l->bank->sig, l->lterm->f_code);
      if(EqnIsPropositional(l))
      {
         lit->w1 = 100000;
         lit->forbidden = true;
      }
   }
   lit->w3 =lit_sel_diff_weight(l);
}

void SelectCQIArNp(OCB_p ocb, Clause_p clause)
{
   assert(ocb);
   assert(clause);
   assert(clause->neg_lit_no);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   generic_uniq_selection(ocb,clause,false, true,
                          select_cq_iarnp_weight, NULL);
}




/*-----------------------------------------------------------------------
//
// Function: select_unless_pdom()
//
//   If there is a maximal positive literal with the same predicate
//   symbol as a negative literal, don't select. Otherwise use
//   the provided function.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void select_unless_pdom(OCB_p ocb, Clause_p clause, LiteralSelectionFun selfun)
{
   PStack_p neg_lits = PStackAlloc();
   bool     *dom_array;
   Eqn_p lit;
   bool dom = false;
   int i;
   long sig_size;

   assert(ocb);
   assert(clause);
   assert(clause->neg_lit_no);
   assert(clause->literals);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   sig_size = clause->literals->bank->sig->size;
   dom_array = SizeMalloc(sizeof(bool)*sig_size);
   for(i=0; i<sig_size; i++)
   {
      dom_array[i] = false;
   }

   ClauseCondMarkMaximalTerms(ocb, clause);
   //printf("Selection: ");
   //ClausePrint(stdout, clause, true);
   //printf("\n");

   for(lit = clause->literals; lit; lit = lit->next)
   {
      if(EqnIsPositive(lit)&&EqnIsMaximal(lit))
      {
         //    printf("Posmax: %ld\n", EqnGetPredCode(lit));
         dom_array[EqnGetPredCode(lit)] = true;
      }
      else if(EqnIsNegative(lit))
      {
         PStackPushP(neg_lits, lit);
      }
   }
   while(!PStackEmpty(neg_lits))
   {
      lit = PStackPopP(neg_lits);
      //printf("%p - %ld\n", lit, EqnGetPredCode(lit));
      if(dom_array[EqnGetPredCode(lit)])
      {
         dom = true;
         break;
      }
   }

   PStackFree(neg_lits);
   SizeFree(dom_array, sizeof(bool)*sig_size);
   if(dom)
   {
      //printf("nixda\n");
   }
   else
   {
      //printf("Select\n");
      selfun(ocb, clause);
   }
}


/*-----------------------------------------------------------------------
//
// Function: SelectCQArNpEqFirstUnlessPDom()
//
//   If there is a maximal positive literal with the same predicate
//   symbol as a negative literal, don't select. Otherwise use
//   SelectCQArNpEqFirst.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void SelectCQArNpEqFirstUnlessPDom(OCB_p ocb, Clause_p clause)
{
   select_unless_pdom(ocb, clause, SelectCQArNpEqFirst);
}


/*-----------------------------------------------------------------------
//
// Function: SelectCQArNTEqFirst()
//
//   If there is a maximal positive literal with the same predicate
//   symbol as a negative literal, don't select. Otherwise use
//   SelectCQArNpEqFirst.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void SelectCQArNTEqFirstUnlessPDom(OCB_p ocb, Clause_p clause)
{
   select_unless_pdom(ocb, clause, SelectCQArNTEqFirst);
}


/*-----------------------------------------------------------------------
//
// Function: SelectCQPrecW()
//
//   Select literals based on the predicate, with the order of the
//   predicates defined by the prec_weights (i.e. the preoder on the
//   function symbols). Prefer larger (?) symbols.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void select_cq_precw_weight(LitEval_p lit, Clause_p clause,
                                   void* vocb)
{
   OCB_p ocb = (OCB_p)vocb;
   Eqn_p l   = lit->literal;

   if(TermIsFreeVar(l->lterm))
   {
      lit->w1 = 0;
      lit->w2 = 0;
   }
   else
   {
      lit->w1 = OCBFunPrecWeight(ocb, l->lterm->f_code);
      lit->w2 = SigGetAlphaRank(l->bank->sig, l->lterm->f_code);
   }
   lit->w3 =lit_sel_diff_weight(l);
}

void SelectCQPrecW(OCB_p ocb, Clause_p clause)
{
   assert(ocb);
   assert(clause);
   assert(clause->neg_lit_no);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   generic_uniq_selection(ocb,clause,false, true,
                          select_cq_precw_weight, ocb);
}


/*-----------------------------------------------------------------------
//
// Function: SelectCQIPrecW()
//
//   Select literals based on the predicate, with the order of the
//   predicates defined by the prec_weights (i.e. the preoder on the
//   function symbols). Prefere smaller (?) symbols.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/


static void select_cq_iprecw_weight(LitEval_p lit, Clause_p clause,
                                    void* vocb)
{
   OCB_p ocb = (OCB_p)vocb;
   Eqn_p l   = lit->literal;

   if(TermIsFreeVar(l->lterm))
   {
      lit->w1 = 0;
      lit->w2 = 0;
   }
   else
   {
      lit->w1 = -OCBFunPrecWeight(ocb, l->lterm->f_code);
      lit->w2 = SigGetAlphaRank(l->bank->sig, l->lterm->f_code);
   }
   lit->w3 =lit_sel_diff_weight(l);
}

void SelectCQIPrecW(OCB_p ocb, Clause_p clause)
{
   assert(ocb);
   assert(clause);
   assert(clause->neg_lit_no);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   generic_uniq_selection(ocb,clause,false, true,
                          select_cq_iprecw_weight, ocb);
}


/*-----------------------------------------------------------------------
//
// Function: SelectCQPrecWNTNp()
//
//   Select literals based on the predicate, with the order of the
//   predicates defined by the prec_weights (i.e. the preoder on the
//   function symbols). Prefere larger symbols. Never select
//   propostional or type predicates.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/


static void select_cq_precwntnp_weight(LitEval_p lit, Clause_p clause,
                                   void* vocb)
{
   OCB_p ocb = (OCB_p)vocb;
   Eqn_p l   = lit->literal;

   if(TermIsFreeVar(l->lterm))
   {
      lit->w1 = 0;
      lit->w2 = 0;
   }
   else
   {
      if(EqnIsTypePred(l)||EqnIsPropositional(l))
      {
         lit->w1 = 100000;
         lit->forbidden = true;
      }
      else
      {
         lit->w1 = OCBFunPrecWeight(ocb, l->lterm->f_code);
         lit->w2 = SigGetAlphaRank(l->bank->sig, l->lterm->f_code);
      }
   }
   lit->w3 =lit_sel_diff_weight(l);
}

void SelectCQPrecWNTNp(OCB_p ocb, Clause_p clause)
{
   assert(ocb);
   assert(clause);
   assert(clause->neg_lit_no);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   generic_uniq_selection(ocb,clause,false, true,
                          select_cq_precwntnp_weight, ocb);
}


/*-----------------------------------------------------------------------
//
// Function: SelectCQPrecWNTNp()
//
//   Select literals based on the predicate, with the order of the
//   predicates defined by the prec_weights (i.e. the preoder on the
//   function symbols). Prefere larger symbols. Never select
//   propostional or type predicates.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/


static void select_cq_iprecwntnp_weight(LitEval_p lit, Clause_p clause,
                                   void* vocb)
{
   OCB_p ocb = (OCB_p)vocb;
   Eqn_p l   = lit->literal;

   if(TermIsFreeVar(l->lterm))
   {
      lit->w1 = 0;
      lit->w2 = 0;
   }
   else
   {
      if(EqnIsTypePred(l)||EqnIsPropositional(l))
      {
         lit->w1 = 100000;
         lit->forbidden = true;
      }
      else
      {
         lit->w1 = -OCBFunPrecWeight(ocb, l->lterm->f_code);
         lit->w2 = SigGetAlphaRank(l->bank->sig, l->lterm->f_code);
      }
   }
   lit->w3 =lit_sel_diff_weight(l);
}

void SelectCQIPrecWNTNp(OCB_p ocb, Clause_p clause)
{
   assert(ocb);
   assert(clause);
   assert(clause->neg_lit_no);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   generic_uniq_selection(ocb,clause,false, true,
                          select_cq_iprecwntnp_weight, ocb);
}






/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
