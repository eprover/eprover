/*-----------------------------------------------------------------------

File  : che_litselection.c

Author: Stephan Schulz

Contents

  Functions for selection certain literals (and hence superposition
  strategies).


  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Fri May 21 22:16:27 GMT 1999
    New

-----------------------------------------------------------------------*/

#include "che_litselection.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

char* LiteralSelectionFunNames[]=
{ 
   "NoSelection",
   "NoGeneration",
   "SelectNegativeLiterals",
   "PSelectNegativeLiterals",
   "SelectPureVarNegLiterals",
   "PSelectPureVarNegLiterals",
   "SelectLargestNegLit",
   "PSelectLargestNegLit",
   "SelectSmallestNegLit",
   "PSelectSmallestNegLit",
   "SelectLargestOrientable",
   "PSelectLargestOrientable",
   "MSelectLargestOrientable",
   "SelectSmallestOrientable",
   "PSelectSmallestOrientable",
   "MSelectSmallestOrientable",
   "SelectDiffNegLit",
   "PSelectDiffNegLit",
   "SelectGroundNegLit",
   "PSelectGroundNegLit",
   "SelectOptimalLit",
   "PSelectOptimalLit",
   "SelectMinOptimalLit",
   "PSelectMinOptimalLit",
   "SelectCondOptimalLit",
   "PSelectCondOptimalLit",
   "SelectAllCondOptimalLit",
   "PSelectAllCondOptimalLit",
   "SelectOptimalRestrDepth2",
   "PSelectOptimalRestrDepth2",
   "SelectOptimalRestrPDepth2",
   "PSelectOptimalRestrPDepth2",
   "SelectOptimalRestrNDepth2",
   "PSelectOptimalRestrNDepth2",
   "SelectNonRROptimalLit",
   "PSelectNonRROptimalLit",
   "SelectNonStrongRROptimalLit",
   "PSelectNonStrongRROptimalLit",
   "SelectAntiRROptimalLit",
   "PSelectAntiRROptimalLit",
   "SelectNonAntiRROptimalLit",
   "PSelectNonAntiRROptimalLit",
   "SelectStrongRRNonRROptimalLit",
   "PSelectStrongRRNonRROptimalLit",
   "SelectUnlessUniqMax",
   "PSelectUnlessUniqMax",
   "SelectUnlessPosMax",
   "PSelectUnlessPosMax",
   "SelectUnlessUniqPosMax",
   "PSelectUnlessUniqPosMax",
   "SelectUnlessUniqMaxPos",
   "PSelectUnlessUniqMaxPos",
   "SelectComplex",
   "PSelectComplex",
   "SelectComplexExceptRRHorn",
   "PSelectComplexExceptRRHorn",
   "SelectLComplex",
   "PSelectLComplex",
   "SelectMaxLComplex",
   "PSelectMaxLComplex",
   "SelectMaxLComplexNoTypePred",
   "PSelectMaxLComplexNoTypePred",
   "SelectMaxLComplexNoXTypePred",
   "PSelectMaxLComplexNoXTypePred",
   "SelectComplexPreferNEQ",
   "PSelectComplexPreferNEQ",
   "SelectComplexPreferEQ",
   "PSelectComplexPreferEQ",
   "SelectComplexExceptUniqMaxHorn",
   "PSelectComplexExceptUniqMaxHorn",
   "MSelectComplexExceptUniqMaxHorn",
   "SelectNewComplex",
   "PSelectNewComplex",
   "SelectNewComplexExceptUniqMaxHorn",
   "PSelectNewComplexExceptUniqMaxHorn",
   "SelectMinInfpos",
   "PSelectMinInfpos",
   "HSelectMinInfpos",
   "GSelectMinInfpos",
   "SelectMin2Infpos",
   "PSelectMin2Infpos",
   "SelectComplexExceptUniqMaxPosHorn",
   "PSelectComplexExceptUniqMaxPosHorn",
   "SelectUnlessUniqMaxSmallestOrientable",
   "PSelectUnlessUniqMaxSmallestOrientable",
   NULL
};



static LiteralSelectionFun litsel_fun_array[]=
{
   SelectNoLiterals,
   SelectNoGeneration,
   SelectNegativeLiterals,
   PSelectNegativeLiterals,
   SelectFirstVariableLiteral,
   PSelectFirstVariableLiteral,
   SelectLargestNegativeLiteral,
   PSelectLargestNegativeLiteral,
   SelectSmallestNegativeLiteral,
   PSelectSmallestNegativeLiteral,
   SelectLargestOrientableLiteral,
   PSelectLargestOrientableLiteral,
   MSelectLargestOrientableLiteral,
   SelectSmallestOrientableLiteral,
   PSelectSmallestOrientableLiteral,
   MSelectSmallestOrientableLiteral,
   SelectDiffNegativeLiteral,
   PSelectDiffNegativeLiteral,
   SelectGroundNegativeLiteral,
   PSelectGroundNegativeLiteral,
   SelectOptimalLiteral,
   PSelectOptimalLiteral,          
   SelectMinOptimalLiteral,
   PSelectMinOptimalLiteral,          
   SelectCondOptimalLiteral,
   PSelectCondOptimalLiteral,
   SelectAllCondOptimalLiteral,
   PSelectAllCondOptimalLiteral,
   SelectDepth2OptimalLiteral,
   PSelectDepth2OptimalLiteral,
   SelectPDepth2OptimalLiteral,
   PSelectPDepth2OptimalLiteral,
   SelectNDepth2OptimalLiteral,
   PSelectNDepth2OptimalLiteral,
   SelectNonRROptimalLiteral,
   PSelectNonRROptimalLiteral,
   SelectNonStrongRROptimalLiteral,
   PSelectNonStrongRROptimalLiteral,
   SelectAntiRROptimalLiteral,
   PSelectAntiRROptimalLiteral,
   SelectNonAntiRROptimalLiteral,
   PSelectNonAntiRROptimalLiteral,
   SelectStrongRRNonRROptimalLiteral,
   PSelectStrongRRNonRROptimalLiteral,
   SelectUnlessUniqMaxOptimalLiteral,
   PSelectUnlessUniqMaxOptimalLiteral,
   SelectUnlessPosMaxOptimalLiteral,
   PSelectUnlessPosMaxOptimalLiteral,
   SelectUnlessUniqPosMaxOptimalLiteral,
   PSelectUnlessUniqPosMaxOptimalLiteral,
   SelectUnlessUniqMaxPosOptimalLiteral,
   PSelectUnlessUniqMaxPosOptimalLiteral,
   SelectComplex,
   PSelectComplex,
   SelectComplexExceptRRHorn,
   PSelectComplexExceptRRHorn,
   SelectLComplex,
   PSelectLComplex,
   SelectMaxLComplex,
   PSelectMaxLComplex,
   SelectMaxLComplexNoTypePred,
   PSelectMaxLComplexNoTypePred,
   SelectMaxLComplexNoXTypePred,
   PSelectMaxLComplexNoXTypePred,
   SelectComplexPreferNEQ,
   PSelectComplexPreferNEQ,
   SelectComplexPreferEQ,
   PSelectComplexPreferEQ,
   SelectComplexExceptUniqMaxHorn,
   PSelectComplexExceptUniqMaxHorn,
   MSelectComplexExceptUniqMaxHorn,
   SelectNewComplex,
   PSelectNewComplex,
   SelectNewComplexExceptUniqMaxHorn,
   PSelectNewComplexExceptUniqMaxHorn,
   SelectMinInfpos,
   PSelectMinInfpos,
   HSelectMinInfpos,
   GSelectMinInfpos,
   SelectMin2Infpos,
   PSelectMin2Infpos,
   SelectComplexExceptUniqMaxPosHorn,
   PSelectComplexExceptUniqMaxPosHorn,
   SelectUnlessUniqMaxSmallestOrientable,
   PSelectUnlessUniqMaxSmallestOrientable,
   NULL
};


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
	 TermIsVar(handle->lterm) &&
	 TermIsVar(handle->rterm))
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
	 TermIsVar(handle->lterm) &&
	 TermIsVar(handle->rterm))
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
	 assert(EqnIsOriented(handle));

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
   int index;

   assert(name);

   index = StringIndex(name, LiteralSelectionFunNames);
   
   if(index>=0)
   {
      return litsel_fun_array[index];
   }
   return NULL;
}


/*-----------------------------------------------------------------------
//
// Function: GetLitSelName()
//
//   Given a LiteralSelectionFun, return the corresponding name.
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

   for(i=0; litsel_fun_array[i]; i++)
   {
      if(litsel_fun_array[i] == fun)
      {
	 res = LiteralSelectionFunNames[i];
	 assert(res);
	 break;
      }
   }
   assert(res);
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: SelectNoLiterals()
//
//   Unselect all literals.
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

   return;
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
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no)
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
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no)
   {
      Eqn_p handle = clause->literals;

      while(handle)
      {
	 EqnSetProp(handle, EPIsSelected);
	 handle = handle->next;
      }
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
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no)
   {
      Eqn_p handle = ClauseFindNegPureVarLit(clause);
      
      if(handle)
      {
	 EqnSetProp(handle, EPIsSelected);
      }
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
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no)
   {
      Eqn_p handle = ClauseFindNegPureVarLit(clause);

      if(handle)
      {
	 clause_select_pos(clause);
      }
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
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no)
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
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no)
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
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no)
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
   if(clause->neg_lit_no)
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
   assert(ocb);
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);
   
   if(clause->neg_lit_no)
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
   assert(ocb);
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);
   
   if(clause->neg_lit_no)
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
   assert(ocb);
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);
   
   if(clause->neg_lit_no)
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
   assert(ocb);
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);
   
   if(clause->neg_lit_no)
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
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no)
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
      assert(selected);
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
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no)
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
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no)
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
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no)
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
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no)
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
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no)
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
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no)
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
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no)
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
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no)
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
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no)
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
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no)
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
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no)
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
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no)
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
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no)
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
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no)
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
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no)
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
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no)
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
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no)
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
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no==0)
   {
      return;
   }
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
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);


   if(clause->neg_lit_no==0)
   {
      return;
   }
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
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no==0)
   {
      return;
   }
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
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no==0)
   {
      return;
   }
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
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no==0)
   {
      return;
   }
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
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no==0)
   {
      return;
   }
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
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no==0)
   {
      return;
   }
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
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no==0)
   {
      return;
   }
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
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no==0)
   {
      return;
   }
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
   assert(ocb);
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no==0)
   {
      return;
   }
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
   assert(ocb);
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no==0)
   {
      return;
   }
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
   assert(ocb);
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no==0)
   {
      return;
   }
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
   assert(ocb);
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no==0)
   {
      return;
   }
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
   assert(ocb);
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no==0)
   {
      return;
   }
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
   assert(ocb);
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no==0)
   {
      return;
   }
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
   assert(ocb);
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no==0)
   {
      return;
   }
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
   assert(ocb);
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no==0)
   {
      return;
   }
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

   assert(ocb);
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no==0)
   {
      return;
   }
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

   assert(ocb);
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no==0)
   {
      return;
   }
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
   if(clause->neg_lit_no)
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
   if(clause->neg_lit_no)
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
   if(clause->neg_lit_no==0)
   {
      return;
   }
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
   if(clause->neg_lit_no==0)
   {
      return;
   }
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
   if(clause->neg_lit_no)
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
   if(clause->neg_lit_no)
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

   assert(ocb);
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);
   
   if(clause->neg_lit_no==0)
   {
      return;
   }
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
   
   assert(ocb);
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);
   
   if(clause->neg_lit_no==0)
   {
      return;
   }
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

   assert(ocb);
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);
   
   if(clause->neg_lit_no==0)
   {
      return;
   }
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

   assert(ocb);
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);
   
   if(clause->neg_lit_no==0)
   {
      return;
   }
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
//   negative literals are extendet type literals, select nothing.
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

   assert(ocb);
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);
   
   if(clause->neg_lit_no==0)
   {
      return;
   }
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

   assert(ocb);
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);
   
   if(clause->neg_lit_no==0)
   {
      return;
   }
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
   if(clause->neg_lit_no>0)
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
	    cur_ground = undefined;
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
		  cur_ground = EqnIsGround(handle);
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
	    if(cur_ground == undefined)
	    {
	       cur_ground = EqnIsGround(handle);
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
   if(clause->neg_lit_no>0)
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
	    cur_ground = undefined;
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
		  cur_ground = EqnIsGround(handle);
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
	    if(cur_ground == undefined)
	    {
	       cur_ground = EqnIsGround(handle);
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
   if(clause->neg_lit_no>0)
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
	    cur_ground = undefined;
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
		  cur_ground = EqnIsGround(handle);
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
	    if(cur_ground == undefined)
	    {
	       cur_ground = EqnIsGround(handle);
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
   if(clause->neg_lit_no>0)
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
	    cur_ground = undefined;
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
		  cur_ground = EqnIsGround(handle);
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
	    if(cur_ground == undefined)
	    {
	       cur_ground = EqnIsGround(handle);
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
   assert(ocb);
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no==0)
   {
      return;
   }
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
   assert(ocb);
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no==0)
   {
      return;
   }
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
   if(clause->neg_lit_no)
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
   if(clause->neg_lit_no)
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
   assert(ocb);
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no==0)
   {
      return;
   }
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
   assert(ocb);
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no==0)
   {
      return;
   }
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
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);
   
   if(clause->neg_lit_no)
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
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);
   
   if(clause->neg_lit_no)
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
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);
   
   if(clause->neg_lit_no)
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
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);
   
   if(clause->neg_lit_no)
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
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);
   
   if(clause->neg_lit_no)
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
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);
   
   if(clause->neg_lit_no)
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
   assert(ocb);
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no==0)
   {
      return;
   }
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
   assert(ocb);
   assert(clause);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(clause->neg_lit_no==0)
   {
      return;
   }
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



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/









