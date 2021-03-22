/*-----------------------------------------------------------------------

  File  : cle_annoterms.h

  Author: Stephan Schulz

  Contents

  Terms and term sets with annotation lists.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Create: Tue Jul 20 17:22:38 MET DST 1999

  -----------------------------------------------------------------------*/

#ifndef CLE_ANNOTERMS

#define CLE_ANNOTERMS

#include <cle_annotations.h>
#include <cle_clauseenc.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct annotermcell
{
   Term_p              term;       /* Usually shared with reference*/
   Annotation_p        annotation; /* May be one, may be a list */
}AnnoTermCell, *AnnoTerm_p;

typedef struct annosetcell
{
   TB_p           terms; /* Of real terms */
   NumTree_p      set;   /* Of AnnoTerms, indexed by term->entry_no */
   PatternSubst_p subst; /* Usually shared among all terms in set */
   long           annotation_no;
}AnnoSetCell, *AnnoSet_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define AnnoTermCellAlloc() (AnnoTermCell*)SizeMalloc(sizeof(AnnoTermCell))
#define AnnoTermCellFree(junk)         SizeFree(junk, sizeof(AnnoTermCell))

AnnoTerm_p AnnoTermAlloc(Term_p term, Annotation_p annos);
AnnoTerm_p AnnoTermAllocNoRef(Term_p term, Annotation_p annos);
void       AnnoTermFree(TB_p bank, AnnoTerm_p junk);
#define    AnnoTermFreeNoRef(junk) AnnoTermCellFree(junk)
AnnoTerm_p AnnoTermParse(Scanner_p in, TB_p bank, long expected);
void       AnnoTermPrint(FILE* out, TB_p bank, AnnoTerm_p term, bool
                         fullterms);
void       AnnoTermRecToFlatEnc(TB_p bank, AnnoTerm_p term);


#define AnnoSetCellAlloc() (AnnoSetCell*)SizeMalloc(sizeof(AnnoSetCell))
#define AnnoSetCellFree(junk)         SizeFree(junk, sizeof(AnnoSetCell))

AnnoSet_p  AnnoSetAlloc(TB_p bank);
void       AnnoSetFree(AnnoSet_p junk);
void       AnnoSetFreeNoRef(AnnoSet_p junk);
bool       AnnoSetAddTerm(AnnoSet_p set, AnnoTerm_p term);
AnnoSet_p  AnnoSetParse(Scanner_p in, TB_p bank, long expected);
void       AnnoSetPrint(FILE* out, AnnoSet_p set);
bool       AnnoSetComputePatternSubst(PatternSubst_p subst, AnnoSet_p
                                      set);
long       AnnoSetRemoveByIdent(AnnoSet_p set, long set_ident);
long       AnnoSetRemoveExceptIdentList(AnnoSet_p set, PStack_p
                                        set_idents);
long       AnnoSetFlatten(AnnoSet_p set, PStack_p set_idents);
void       AnnoSetNormalizeFlatAnnos(AnnoSet_p set);

long       AnnoSetRecToFlatEnc(TB_p bank, AnnoSet_p set);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
