/*-----------------------------------------------------------------------

File  : cle_examplerep.h

Author: Stephan Schulz

Contents

  Data structures and functions to associate names, numbers and
  features with a proof problem.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Mon Jul 26 18:30:59 MET DST 1999
    New

-----------------------------------------------------------------------*/

#ifndef CLE_EXAMPLEREP

#define CLE_EXAMPLEREP

#include <clb_simple_stuff.h>
#include <clb_stringtrees.h>
#include <cle_numfeatures.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* An example is represented by a name (which also points to the
   original input file stored somewhere), a number (used in the
   annotations to identify the example) and the features which are
   used to identify similar exampes */

typedef struct examplerepcell
{
   long       ident;
   char*      name;
   Features_p features;
}ExampleRepCell, *ExampleRep_p;


/* We want to deal with sets of example representations */

typedef struct examplesetcell
{
   long      count; /* For ident generation, _not_ number of examples
           */
   NumTree_p ident_index;
   StrTree_p name_index;
}ExampleSetCell, *ExampleSet_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define ExampleRepCellAlloc() (ExampleRepCell*)SizeMalloc(sizeof(ExampleRepCell))
#define ExampleRepCellFree(junk) SizeFree(junk, sizeof(ExampleRepCell))

void          ExampleRepFree(ExampleRep_p junk);
void          ExampleRepPrint(FILE* out, ExampleRep_p rep);
ExampleRep_p  ExampleRepParse(Scanner_p in);

#define ExampleSetCellAlloc() (ExampleSetCell*)SizeMalloc(sizeof(ExampleSetCell))
#define ExampleSetCellFree(junk) SizeFree(junk, sizeof(ExampleSetCell))

ExampleSet_p  ExampleSetAlloc(void);
void          ExampleSetFree(ExampleSet_p junk);
ExampleRep_p  ExampleSetFindName(ExampleSet_p set, char* name);
bool          ExampleSetInsert(ExampleSet_p set, ExampleRep_p rep);
ExampleRep_p  ExampleSetExtract(ExampleSet_p set, ExampleRep_p rep);
bool          ExampleSetDeleteId(ExampleSet_p set, long ident);
bool          ExampleSetDeleteName(ExampleSet_p set, char* name);
void          ExampleSetPrint(FILE* out, ExampleSet_p set);
long          ExampleSetParse(Scanner_p in, ExampleSet_p set);
long          ExampleSetSelectByDist(PStack_p results, ExampleSet_p
                 set, Features_p target, double
                 pred_w, double func_w, double
                 *weights, long sel_no, double
                 set_part, double dist_part);
#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





