/*-----------------------------------------------------------------------

File  : cco_batch_spec.h

Author: Stephan Schulz (schulz@eprover.org)

Contents
 
  Data types and code for dealing with CASC-2010 LTB batch
  specifications. It's unclear if this will ever be useful...
  
  Copyright 2010 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Mon Jun 28 21:46:06 CEST 2010
    New

-----------------------------------------------------------------------*/

#ifndef CCO_BATCH_SPEC

#define CCO_BATCH_SPEC

#include <cio_tempfile.h>
#include <ccl_formulafunc.h>
#include <ccl_sine.h>
#include <cco_proc_ctrl.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef enum
{
   BONone,
   BODesired,
   BORequired
}BOOutputType;


/* Describe a batch specification file as used in CASC-J5. Note that
 * that char* values are part of this data structure and are free'd
 * with it. */

typedef struct batch_spec_cell
{
   char         *executable;
   char         *pexec;
   char*        category;     /* Just a name */
   long         per_prob_time;/* Wall clock, in seconds */
   long         total_time;   /* Wall clock, in seconds */
   PStack_p     includes;     /* Names of include files (char*) */
   IOFormat     format;
   BOOutputType res_assurance;
   BOOutputType res_proof;
   BOOutputType res_model;
   BOOutputType res_answer;
   BOOutputType res_list_fof;
   PStack_p     source_files; /* Input files (char*) */
   PStack_p     dest_files;   /* Output files (char*) */
}BatchSpecCell, *BatchSpec_p;


/* Complex, multi-file problem specification data structure - holding
 * information about all the includes parsed so they are parsed at
 * most once. */

typedef struct strtuctured_FOF_spec_cell
{
   Sig_p         sig;
   TB_p          terms;
   PStack_p      clause_sets;
   PStack_p      formula_sets;
   StrTree_p     parsed_includes;
   PStackPointer shared_ax_sp;
   GenDistrib_p  f_distrib;
}StructFOFSpecCell, *StructFOFSpec_p;




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define BatchSpecCellAlloc()    (BatchSpecCell*)SizeMalloc(sizeof(BatchSpecCell))
#define BatchSpecCellFree(junk) SizeFree(junk, sizeof(BatchSpecCell))

BatchSpec_p BatchSpecAlloc(char* executable, char* pexec, IOFormat format);
void        BatchSpecFree(BatchSpec_p spec);
void        BatchSpecPrint(FILE* out, BatchSpec_p spec);

BatchSpec_p BatchSpecParse(Scanner_p in, char* executable, char* pexec, IOFormat format);

#define StructFOFSpecCellAlloc()    (StructFOFSpecCell*)SizeMalloc(sizeof(StructFOFSpecCell))
#define StructFOFSpecCellFree(junk) SizeFree(junk, sizeof(StructFOFSpecCell))

StructFOFSpec_p StructFOFSpecAlloc(void);
void           StructFOFSpecFree(StructFOFSpec_p ctrl);


StructFOFSpec_p StructFOFSpecAlloc(void);


long           StructFOFSpecParseAxioms(StructFOFSpec_p ctrl, 
                                       PStack_p axfiles, 
                                       IOFormat parse_format);
#define       StructFOFSpecResetShared(ctrl) (ctrl)->shared_ax_sp = 0

void           StructFOFSpecInitDistrib(StructFOFSpec_p ctrl);
long           StructFOFSpecInit(BatchSpec_p spec, StructFOFSpec_p ctrl);

void StructFOFSpecAddProblem(StructFOFSpec_p ctrl, 
                             ClauseSet_p clauses, 
                             FormulaSet_p formulas); 

void StructFOFSpecBacktrackToSpec(StructFOFSpec_p ctrl);


long StructFOFSpecGetProblem(StructFOFSpec_p ctrl,
                             AxFilter_p filter,
                             PStack_p res_clauses, 
                             PStack_p res_formulas);

bool BatchProcessProblem(BatchSpec_p spec, 
                         StructFOFSpec_p ctrl, 
                         char* source, char* dest);

bool BatchProcessProblems(BatchSpec_p spec, 
                          StructFOFSpec_p ctrl);



#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





