/*-----------------------------------------------------------------------

File  : che_axfilter.h

Author: Stephan Schulz (schulz@eprover.org)

Contents
 
  Definitions dealing with the description of axiom set filters based
  on relevancy/SinE principles.

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



/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef enum 
{
   AFNoFilter = 0,
   AFSineE
}AxFilterType;


/* Type of generality measure: Number of occurences in terms or in
 * formulas (and possibly later in equations). */

typedef enum
{
   GMTerms,
   GMFormulas
}GeneralityMeasure;


/* Parameters for a single Axiom filter */

typedef struct
{
   AxFilterType      type;
   GeneralityMeasure gen_measure;
   long              max_recursion_depth;
   long long         max_set_size;
   float             max_set_fraction;   
}AxFilterCell, *AxFilter_p;


/* Sets of AxFilters */
typedef struct
{
   PStack_p set;
}AxFilterSetCell, *AxFilterSet_p;

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define AxFilterCellAlloc()    (AxFilterCell*)SizeMalloc(sizeof(AxFilterCell))
#define AxFilterCellFree(junk) SizeFree(junk, sizeof(AxFilterCell))

AxFilter_p AxFilterAlloc();
void       AxFilterFree(AxFilter_p junk);
AxFilter_p AxFilterParse(Scanner_p in);
void       AxFilterPrint(FILE* out, AxFilter_p filter);


#define AxFilterSetCellAlloc()    (AxFilterSetCell*)SizeMalloc(sizeof(AxFilterSetCell))
#define AxFilterSetCellFree(junk) SizeFree(junk, sizeof(AxFilterSetCell))

AxFilter_p AxFilterSetAlloc();
void       AxFilterSetFree(AxFilterSet_p junk);



#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





