/*-----------------------------------------------------------------------

File  : che_litselection.h

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

<1> Fri May 21 22:17:06 GMT 1999
    New

-----------------------------------------------------------------------*/

#ifndef CHE_LITSELECTION

#define CHE_LITSELECTION

#include <clb_simple_stuff.h>
#include <ccl_clauses.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef void (*LiteralSelectionFun)(OCB_p ocb, Clause_p clause);

typedef struct litsel_name_fun_assoc_cell
{
   char*                name;
   LiteralSelectionFun  fun;
}LitSelNameFunAssocCell;

typedef struct lit_eval_cell
{
   Eqn_p literal;
   bool  forbidden; /* Never select this; */
   bool  exclusive; /* If this is selected, select no others */
   int   w1; /* Lexicographically compared weights */
   int   w2;
   int   w3;
}LitEvalCell, *LitEval_p;

typedef void LitWeightFun(LitEval_p, Clause_p, void*);

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define LitEvalInit(cell) \
        {(cell)->forbidden = false;cell->exclusive=true;\
        cell->w1=0;cell->w2=0;cell->w3=0;}

LiteralSelectionFun GetLitSelFun(char* name);
char*               GetLitSelName(LiteralSelectionFun fun);
void LitSelAppendNames(DStr_p str);

void SelectNoLiterals(OCB_p ocb, Clause_p clause);
void SelectNoGeneration(OCB_p ocb, Clause_p clause);
void SelectNegativeLiterals(OCB_p ocb, Clause_p clause);
void PSelectNegativeLiterals(OCB_p ocb, Clause_p clause);
void SelectFirstVariableLiteral(OCB_p ocb, Clause_p clause);
void PSelectFirstVariableLiteral(OCB_p ocb, Clause_p clause);
void SelectLargestNegativeLiteral(OCB_p ocb, Clause_p clause);
void PSelectLargestNegativeLiteral(OCB_p ocb, Clause_p clause);
void SelectSmallestNegativeLiteral(OCB_p ocb, Clause_p clause);
void PSelectSmallestNegativeLiteral(OCB_p ocb, Clause_p clause);
void SelectLargestOrientableLiteral(OCB_p ocb, Clause_p clause);
void PSelectLargestOrientableLiteral(OCB_p ocb, Clause_p clause);
void MSelectLargestOrientableLiteral(OCB_p ocb, Clause_p clause);
void SelectSmallestOrientableLiteral(OCB_p ocb, Clause_p clause);
void PSelectSmallestOrientableLiteral(OCB_p ocb, Clause_p clause);
void MSelectSmallestOrientableLiteral(OCB_p ocb, Clause_p clause);
void SelectDiffNegativeLiteral(OCB_p ocb, Clause_p clause);
void PSelectDiffNegativeLiteral(OCB_p ocb, Clause_p clause);
void SelectGroundNegativeLiteral(OCB_p ocb, Clause_p clause);
void PSelectGroundNegativeLiteral(OCB_p ocb, Clause_p clause);
void SelectOptimalLiteral(OCB_p ocb, Clause_p clause);
void PSelectOptimalLiteral(OCB_p ocb, Clause_p clause);

void SelectMinOptimalLiteral(OCB_p ocb, Clause_p clause);
void PSelectMinOptimalLiteral(OCB_p ocb, Clause_p clause);
void SelectMinOptimalNoTypePred(OCB_p ocb, Clause_p clause);
void PSelectMinOptimalNoTypePred(OCB_p ocb, Clause_p clause);
void SelectMinOptimalNoXTypePred(OCB_p ocb, Clause_p clause);
void PSelectMinOptimalNoXTypePred(OCB_p ocb, Clause_p clause);
void SelectMinOptimalNoRXTypePred(OCB_p ocb, Clause_p clause);
void PSelectMinOptimalNoRXTypePred(OCB_p ocb, Clause_p clause);

void SelectCondOptimalLiteral(OCB_p ocb, Clause_p clause);
void PSelectCondOptimalLiteral(OCB_p ocb, Clause_p clause);
void SelectAllCondOptimalLiteral(OCB_p ocb, Clause_p clause);
void PSelectAllCondOptimalLiteral(OCB_p ocb, Clause_p clause);
void SelectDepth2OptimalLiteral(OCB_p ocb, Clause_p clause);
void PSelectDepth2OptimalLiteral(OCB_p ocb, Clause_p clause);
void SelectPDepth2OptimalLiteral(OCB_p ocb, Clause_p clause);
void PSelectPDepth2OptimalLiteral(OCB_p ocb, Clause_p clause);
void SelectNDepth2OptimalLiteral(OCB_p ocb, Clause_p clause);
void PSelectNDepth2OptimalLiteral(OCB_p ocb, Clause_p clause);
void SelectNonRROptimalLiteral(OCB_p ocb, Clause_p clause);
void PSelectNonRROptimalLiteral(OCB_p ocb, Clause_p clause);
void SelectNonStrongRROptimalLiteral(OCB_p ocb, Clause_p clause);
void PSelectNonStrongRROptimalLiteral(OCB_p ocb, Clause_p clause);

void SelectAntiRROptimalLiteral(OCB_p ocb, Clause_p clause);
void PSelectAntiRROptimalLiteral(OCB_p ocb, Clause_p clause);

void SelectNonAntiRROptimalLiteral(OCB_p ocb, Clause_p clause);
void PSelectNonAntiRROptimalLiteral(OCB_p ocb, Clause_p clause);

void SelectStrongRRNonRROptimalLiteral(OCB_p ocb, Clause_p clause);
void PSelectStrongRRNonRROptimalLiteral(OCB_p ocb, Clause_p clause);

void SelectUnlessUniqMaxSmallestOrientable(OCB_p ocb, Clause_p clause);
void PSelectUnlessUniqMaxSmallestOrientable(OCB_p ocb, Clause_p clause);

void SelectUnlessUniqMaxOptimalLiteral(OCB_p ocb, Clause_p clause);
void PSelectUnlessUniqMaxOptimalLiteral(OCB_p ocb, Clause_p clause);

void SelectUnlessPosMaxOptimalLiteral(OCB_p ocb, Clause_p clause);
void PSelectUnlessPosMaxOptimalLiteral(OCB_p ocb, Clause_p clause);

void SelectUnlessUniqPosMaxOptimalLiteral(OCB_p ocb, Clause_p clause);
void PSelectUnlessUniqPosMaxOptimalLiteral(OCB_p ocb, Clause_p clause);

void SelectUnlessUniqMaxPosOptimalLiteral(OCB_p ocb, Clause_p clause);
void PSelectUnlessUniqMaxPosOptimalLiteral(OCB_p ocb, Clause_p clause);

void SelectComplex(OCB_p ocb, Clause_p clause);
void PSelectComplex(OCB_p ocb, Clause_p clause);

void SelectComplexExceptRRHorn(OCB_p ocb, Clause_p clause);
void PSelectComplexExceptRRHorn(OCB_p ocb, Clause_p clause);

void SelectLComplex(OCB_p ocb, Clause_p clause);
void PSelectLComplex(OCB_p ocb, Clause_p clause);

void SelectMaxLComplex(OCB_p ocb, Clause_p clause);
void PSelectMaxLComplex(OCB_p ocb, Clause_p clause);

void SelectMaxLComplexNoTypePred(OCB_p ocb, Clause_p clause);
void PSelectMaxLComplexNoTypePred(OCB_p ocb, Clause_p clause);
void SelectMaxLComplexNoXTypePred(OCB_p ocb, Clause_p clause);
void PSelectMaxLComplexNoXTypePred(OCB_p ocb, Clause_p clause);

void SelectComplexPreferNEQ(OCB_p ocb, Clause_p clause);
void PSelectComplexPreferNEQ(OCB_p ocb, Clause_p clause);

void SelectComplexPreferEQ(OCB_p ocb, Clause_p clause);
void PSelectComplexPreferEQ(OCB_p ocb, Clause_p clause);

void SelectComplexExceptUniqMaxHorn(OCB_p ocb, Clause_p clause);
void PSelectComplexExceptUniqMaxHorn(OCB_p ocb, Clause_p clause);
void MSelectComplexExceptUniqMaxHorn(OCB_p ocb, Clause_p clause);

void SelectNewComplex(OCB_p ocb, Clause_p clause);
void PSelectNewComplex(OCB_p ocb, Clause_p clause);
void SelectNewComplexExceptUniqMaxHorn(OCB_p ocb, Clause_p clause);
void PSelectNewComplexExceptUniqMaxHorn(OCB_p ocb, Clause_p clause);

void SelectMinInfpos(OCB_p ocb, Clause_p clause);
void PSelectMinInfpos(OCB_p ocb, Clause_p clause);
void HSelectMinInfpos(OCB_p ocb, Clause_p clause);
void GSelectMinInfpos(OCB_p ocb, Clause_p clause);
void SelectMinInfposNoTypePred(OCB_p ocb, Clause_p clause);
void PSelectMinInfposNoTypePred(OCB_p ocb, Clause_p clause);

void SelectMin2Infpos(OCB_p ocb, Clause_p clause);
void PSelectMin2Infpos(OCB_p ocb, Clause_p clause);

void SelectComplexExceptUniqMaxPosHorn(OCB_p ocb, Clause_p clause);
void PSelectComplexExceptUniqMaxPosHorn(OCB_p ocb, Clause_p clause);

void SelectDiversificationLiterals(OCB_p ocb, Clause_p clause);
void SelectDiversificationPreferIntoLiterals(OCB_p ocb, Clause_p clause);
void SelectMaxLComplexG(OCB_p ocb, Clause_p clause);
void SelectMaxLComplexAvoidPosPred(OCB_p ocb, Clause_p clause);
void SelectMaxLComplexAvoidAppVar(OCB_p ocb, Clause_p clause);
void SelectMaxLComplexStronglyAvoidAppVar(OCB_p ocb, Clause_p clause);
void SelectMaxLComplexPreferAppVar(OCB_p ocb, Clause_p clause);
void SelectMaxLComplexAPPNTNp(OCB_p ocb, Clause_p clause);
void SelectMaxLComplexAPPNoType(OCB_p ocb, Clause_p clause);
void SelectMaxLComplexAvoidPosUPred(OCB_p ocb, Clause_p clause);
void SelectComplexG(OCB_p ocb, Clause_p clause);
void SelectComplexAHP(OCB_p ocb, Clause_p clause);
void PSelectComplexAHP(OCB_p ocb, Clause_p clause);

void SelectNewComplexAHP(OCB_p ocb, Clause_p clause);
void PSelectNewComplexAHP(OCB_p ocb, Clause_p clause);

void SelectComplexAHPExceptRRHorn(OCB_p ocb, Clause_p clause);
void PSelectComplexAHPExceptRRHorn(OCB_p ocb, Clause_p clause);

void SelectNewComplexAHPExceptRRHorn(OCB_p ocb, Clause_p clause);
void PSelectNewComplexAHPExceptRRHorn(OCB_p ocb, Clause_p clause);

void SelectNewComplexAHPExceptUniqMaxHorn(OCB_p ocb, Clause_p clause);
void PSelectNewComplexAHPExceptUniqMaxHorn(OCB_p ocb, Clause_p clause);
void SelectNewComplexAHPNS(OCB_p ocb, Clause_p clause);
void SelectVGNonCR(OCB_p ocb, Clause_p clause);

void SelectCQArEqLast(OCB_p ocb, Clause_p clause);
void SelectCQArEqFirst(OCB_p ocb, Clause_p clause);
void SelectCQIArEqLast(OCB_p ocb, Clause_p clause);
void SelectCQIArEqFirst(OCB_p ocb, Clause_p clause);
void SelectCQAr(OCB_p ocb, Clause_p clause);
void SelectCQIAr(OCB_p ocb, Clause_p clause);
void SelectCQArNpEqFirst(OCB_p ocb, Clause_p clause);
void SelectCQIArNpEqFirst(OCB_p ocb, Clause_p clause);

void SelectGrCQArEqFirst(OCB_p ocb, Clause_p clause);
void SelectCQGrArEqFirst(OCB_p ocb, Clause_p clause);

void SelectCQArNTEqFirst(OCB_p ocb, Clause_p clause);
void SelectCQIArNTEqFirst(OCB_p ocb, Clause_p clause);
void SelectCQArNTNpEqFirst(OCB_p ocb, Clause_p clause);
void SelectCQIArNTNpEqFirst(OCB_p ocb, Clause_p clause);
void SelectCQArNXTEqFirst(OCB_p ocb, Clause_p clause);
void SelectCQIArNXTEqFirst(OCB_p ocb, Clause_p clause);

void SelectCQArNTNp(OCB_p ocb, Clause_p clause);
void SelectCQIArNTNp(OCB_p ocb, Clause_p clause);
void SelectCQArNT(OCB_p ocb, Clause_p clause);
void SelectCQIArNT(OCB_p ocb, Clause_p clause);
void SelectCQArNp(OCB_p ocb, Clause_p clause);
void SelectCQIArNp(OCB_p ocb, Clause_p clause);

void SelectCQArNpEqFirstUnlessPDom(OCB_p ocb, Clause_p clause);
void SelectCQArNTEqFirstUnlessPDom(OCB_p ocb, Clause_p clause);

void SelectCQPrecW(OCB_p ocb, Clause_p clause);
void SelectCQIPrecW(OCB_p ocb, Clause_p clause);
void SelectCQPrecWNTNp(OCB_p ocb, Clause_p clause);
void SelectCQIPrecWNTNp(OCB_p ocb, Clause_p clause);



#endif



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





