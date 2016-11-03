/*-----------------------------------------------------------------------

File  : cle_flatannoterms.h

Author: Stephan Schulz

Contents

  Terms with only an evaluation and a counter left.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Mon Aug  9 12:32:53 MET DST 1999
    New

-----------------------------------------------------------------------*/

#ifndef CLE_FLATANNOTERMS

#define CLE_FLATANNOTERMS

#include <clb_ddarrays.h>
#include <cle_annoterms.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


typedef struct flatannotermcell
{
   Term_p                  term;       /* Usually shared, but no
                 reference */
   double                  eval;
   double                  eval_weight;  /* How much is this annotation
                   worth at each subterm node?
                   This takes the place of the
                   sources field, and is
                   normalized by dividing it
                   through the term size */
   long                    sources;
   struct flatannotermcell *next;
}FlatAnnoTermCell, *FlatAnnoTerm_p;

typedef struct flatannosetcell
{
   NumTree_p      set;   /* Of FlatAnnoTerms, indexed by
             term->entry_no */
}FlatAnnoSetCell, *FlatAnnoSet_p;



/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


#define FlatAnnoTermCellAlloc() \
        (FlatAnnoTermCell*)SizeMalloc(sizeof(FlatAnnoTermCell))
#define FlatAnnoTermCellFree(junk) \
        SizeFree(junk, sizeof(FlatAnnoTermCell))

FlatAnnoTerm_p FlatAnnoTermAlloc(Term_p term, double eval, double
             eval_weight, long sources);

#define         FlatAnnoTermFree(junk) FlatAnnoTermCellFree(junk)

#define FlatAnnoSetCellAlloc() \
        (FlatAnnoSetCell*)SizeMalloc(sizeof(FlatAnnoSetCell))
#define FlatAnnoSetCellFree(junk) \
        SizeFree(junk, sizeof(FlatAnnoSetCell))

void FlatAnnoTermPrint(FILE* out, FlatAnnoTerm_p term, Sig_p sig);

FlatAnnoSet_p FlatAnnoSetAlloc(void);
void          FlatAnnoSetFree(FlatAnnoSet_p junk);
bool          FlatAnnoSetAddTerm(FlatAnnoSet_p set, FlatAnnoTerm_p
             term);
void FlatAnnoSetPrint(FILE* out, FlatAnnoSet_p set, Sig_p sig);
long FlatAnnoSetTranslate(FlatAnnoSet_p flatset, AnnoSet_p set, double
           weights[]);

long FlatAnnoSetSize(FlatAnnoSet_p fset);

long FlatAnnoTermFlatten(FlatAnnoSet_p set, FlatAnnoTerm_p term);
long FlatAnnoSetFlatten(FlatAnnoSet_p set, FlatAnnoSet_p to_flatten);

double FlatAnnoSetEvalAverage(FlatAnnoSet_p set);
double FlatAnnoSetEvalWeightedAverage(FlatAnnoSet_p set);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





