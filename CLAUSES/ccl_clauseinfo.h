/*-----------------------------------------------------------------------

File  : ccl_clauseinfo.h

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Datatype and basic functions for storing and handling clause
  information that few clauses carry (probably just input
  clauses). This is not stored in the clause (or formula) data types,
  because it would eat  up to much memory (remember, there are
  millions of clauses)

  Copyright 2004-1017 by the authors.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes


-----------------------------------------------------------------------*/

#ifndef CCL_CLAUSEINFO

#define CCL_CLAUSEINFO

#include <clb_memory.h>
#include <clb_dstrings.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct clause_info_cell
{
   char *name;   /* In the input file, if any */
   char *source; /* File name, if any */
   long line;
   long column;
}ClauseInfoCell, *ClauseInfo_p;



/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define ClauseInfoCellAlloc()    (ClauseInfoCell*)SizeMalloc(sizeof(ClauseInfoCell))
#define ClauseInfoCellFree(junk) SizeFree(junk, sizeof(ClauseInfoCell))

ClauseInfo_p ClauseInfoAlloc(char* name, char* source, long line, long column);
#define      ClauseInfoAllocEmpty() ClauseInfoAlloc(NULL, NULL, -1, -1)
void         ClauseInfoFree(ClauseInfo_p info);
void         ClauseSourceInfoPrint(FILE* out, ClauseInfo_p info,
                                   char *inf_lit, char* delim);
#define ClauseSourceInfoPrintTSTP(out, info) \
        ClauseSourceInfoPrint((out), (info), "file", "'")

#define ClauseSourceInfoPrintPCL(out, info) \
        ClauseSourceInfoPrint((out), (info), "initial", "\"")
long    ClauseInfoGetIdNameSpace(ClauseInfo_p info);
long    ClauseInfoGetIdCounter(ClauseInfo_p info);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
