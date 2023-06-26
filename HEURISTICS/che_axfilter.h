/*-----------------------------------------------------------------------

File  : che_axfilter.h

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Definitions dealing with the description of axiom set filters based
  on relevancy/SinE principles. This only deals with their parameters
  and specifications. The real code is (for now) in CONTROL and knows
  nothing about this ;-).

  Copyright 2011 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Wed Feb 23 00:17:20 CET 2011
    New

-----------------------------------------------------------------------*/

#ifndef CHE_AXFILTER

#define CHE_AXFILTER

#include <clb_simple_stuff.h>
#include <cio_basicparser.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef enum
{
   AFNoFilter = 0,
   AFGSinE,    /* Generalized SinE */
   AFThreshold,  /* Pass all formulas if less then N */
   AFLambdaDefines  /* Pass only conjecture and all definitions */
}AxFilterType;


/* Type of generality measure: Number of occurences in terms or in
 * formulas (and possibly later in equations). */

typedef enum
{
   GMNoMeasure,
   GMTerms,
   GMLiterals,
   GMFormulas,
   GMPosFormula,
   GMPosLiteral,
   GMPosTerms,
   GMNegFormula,
   GMNegLiteral,
   GMNegTerms
}GeneralityMeasure;


/* Parameters for a single Axiom filter */

typedef struct
{
   char*             name;
   AxFilterType      type;
   GeneralityMeasure gen_measure;
   bool              use_hypotheses;
   double            benevolence;
   long              generosity;
   long              max_recursion_depth;
   long long         max_set_size;
   double            max_set_fraction;
   bool              add_no_symbol_axioms;
   bool              trim_implications;
   bool              defined_symbols_in_drel;
   long              threshold; // for Threshold filter only
}AxFilterCell, *AxFilter_p;


/* Sets of AxFilters */
typedef struct
{
   PStack_p set;
}AxFilterSetCell, *AxFilterSet_p;

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

extern char* AxFilterDefaultSet;

#define AxFilterCellAlloc()    (AxFilterCell*)SizeMalloc(sizeof(AxFilterCell))
#define AxFilterCellFree(junk) SizeFree(junk, sizeof(AxFilterCell))

AxFilter_p AxFilterAlloc(void);
void       AxFilterFree(AxFilter_p junk);
AxFilter_p AxFilterParse(Scanner_p in);
AxFilter_p AxFilterDefParse(Scanner_p in);
bool       AxFilterPrintBuf(char* buf, int buflen, AxFilter_p filter);
void       AxFilterPrint(FILE* out, AxFilter_p filter);
void       AxFilterDefPrint(FILE* out, AxFilter_p filter);


#define AxFilterSetCellAlloc()    (AxFilterSetCell*)SizeMalloc(sizeof(AxFilterSetCell))
#define AxFilterSetCellFree(junk) SizeFree(junk, sizeof(AxFilterSetCell))

AxFilterSet_p AxFilterSetAlloc(void);
void          AxFilterSetFree(AxFilterSet_p junk);
long          AxFilterSetParse(Scanner_p in, AxFilterSet_p set);
AxFilterSet_p AxFilterSetCreateInternal(char* str);
void          AxFilterSetPrint(FILE* out, AxFilterSet_p set);
#define       AxFilterSetElements(s) PStackGetSP((s)->set)
#define       AxFilterSetGetFilter(s, i) ((AxFilter_p)PStackElementP((s)->set,(i)))
#define       AxFilterSetAddFilter(s, f) PStackPushP((s)->set, (f))
AxFilter_p    AxFilterSetFindFilter(AxFilterSet_p set, char* name);
void          AxFilterSetAddNames(DStr_p res, AxFilterSet_p filters);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
