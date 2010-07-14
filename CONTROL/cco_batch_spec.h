/*-----------------------------------------------------------------------

File  : cco_batch_spec.h

Author: Stephan Schulz (schulz@eprover.org)

Contents
 
  Data types and code for dealing with CASC-2010 LTB batch
  specifications. It's unclear if this will ever be useful...
  
  Copyright 2010 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Mon Jun 28 21:46:06 CEST 2010
    New

-----------------------------------------------------------------------*/

#ifndef CCO_BATCH_SPEC

#define CCO_BATCH_SPEC

#include <ccl_formulafunc.h>
#include <ccl_sine.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* Describe a batch specification file as used in CASC-J5. Note that
 * that char* values are part of this data structure and are free'd
 * with it. */

typedef struct batch_spec_cell
{
   char* category;        /* Just a name */
   long  per_prob_time;   /* Wall clock, in seconds */
   long  total_time;      /* Wall clock, in seconds */
   PStack_p includes;     /* Names of include files (char*) */
   PStack_p source_files; /* Input files (char*) */
   PStack_p dest_files;   /* Output files (char*) */
}BatchSpecCell, *BatchSpec_p;


/* Batch control data structure - holding information about all the
 * includes. */

typedef struct batch_control_cell
{
   Sig_p         sig;
   TB_p          terms;
   PStack_p      clause_sets;
   PStack_p      formula_sets;
   StrTree_p     parsed_includes;
   PStackPointer shared_ax_sp;
   GenDistrib_p  f_distrib;
}BatchControlCell, *BatchControl_p;




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define BatchSpecCellAlloc()    (BatchSpecCell*)SizeMalloc(sizeof(BatchSpecCell))
#define BatchSpecCellFree(junk) SizeFree(junk, sizeof(BatchSpecCell))

BatchSpec_p BatchSpecAlloc(void);
void        BatchSpecFree(BatchSpec_p spec);
void        BatchSpecPrint(FILE* out, BatchSpec_p spec);

BatchSpec_p BatchSpecParse(Scanner_p in);

#define BatchControlCellAlloc()    (BatchControlCell*)SizeMalloc(sizeof(BatchControlCell))
#define BatchControlCellFree(junk) SizeFree(junk, sizeof(BatchControlCell))

BatchControl_p BatchControlAlloc(void);
void           BatchControlFree(BatchControl_p ctrl);
long           BatchControlInitSpec(BatchSpec_p spec, BatchControl_p ctrl);
void           BatchControlInitDistrib(BatchControl_p ctrl);
long           BatchControlInit(BatchSpec_p spec, BatchControl_p ctrl);

void BatchControlAddProblem(BatchControl_p ctrl, 
                            ClauseSet_p clauses, 
                            FormulaSet_p formulas); 

void BatchControlBacktrackToSpec(BatchControl_p ctrl);


long BatchControlGetProblem(BatchControl_p ctrl,
                            GeneralityMeasure gen_measure,
                            double            benevolence,
                            PStack_p          res_clauses, 
                            PStack_p          res_formulas);

bool BatchProcessProblem(BatchSpec_p spec, 
                         BatchControl_p ctrl, 
                         char* source, char* dest);

bool BatchProcessProblems(BatchSpec_p spec, 
                          BatchControl_p ctrl);



#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





