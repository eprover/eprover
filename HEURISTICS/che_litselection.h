/*-----------------------------------------------------------------------

File  : che_litselection.h

Author: Stephan Schulz

Contents

  Functions for selection certain literals (and hence superposition
  strategies).

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
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

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

extern char* LiteralSelectionFunNames[];

LiteralSelectionFun GetLitSelFun(char* name);
char*               GetLitSelName(LiteralSelectionFun fun);

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

void SelectMin2Infpos(OCB_p ocb, Clause_p clause);
void PSelectMin2Infpos(OCB_p ocb, Clause_p clause);

void SelectComplexExceptUniqMaxPosHorn(OCB_p ocb, Clause_p clause);
void PSelectComplexExceptUniqMaxPosHorn(OCB_p ocb, Clause_p clause);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





