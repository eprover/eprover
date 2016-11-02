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
#include <cio_simplestuff.h>
#include <ccl_formulafunc.h>
#include <ccl_sine.h>
#include <cco_sine.h>
#include <cco_proc_ctrl.h>
#include <cio_network.h>


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
   IOFormat     format;
   char*        category;     /* Just a name */
   char*        train_dir;
   bool         ordered;
   BOOutputType res_assurance;
   BOOutputType res_proof;
   BOOutputType res_model;
   BOOutputType res_answer;
   BOOutputType res_list_fof;
   long         per_prob_limit;  /* Wall clock, in seconds */
   long         total_wtc_limit; /* Wall clock, in seconds */
   PStack_p     includes;        /* Names of include files (char*) */
   PStack_p     source_files; /* Input files (char*) */
   PStack_p     dest_files;   /* Output files (char*) */
}BatchSpecCell, *BatchSpec_p;




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define BatchSpecCellAlloc()    (BatchSpecCell*)SizeMalloc(sizeof(BatchSpecCell))
#define BatchSpecCellFree(junk) SizeFree(junk, sizeof(BatchSpecCell))

BatchSpec_p BatchSpecAlloc(char* executable, IOFormat format);
void        BatchSpecFree(BatchSpec_p spec);
void        BatchSpecPrint(FILE* out, BatchSpec_p spec);


long        BatchStructFOFSpecInit(BatchSpec_p spec, StructFOFSpec_p ctrl);

BatchSpec_p BatchSpecParse(Scanner_p in, char* executable,
                           char* category, char* train_dir,
                           IOFormat format);

#define BatchSpecProblemNo(spec) PStackGetSP((spec)->source_files)


bool BatchProcessProblem(BatchSpec_p spec,
                         long wct_limit,
                         StructFOFSpec_p ctrl,
                         char* jobname,
                         ClauseSet_p cset,
                         FormulaSet_p fset,
                         FILE* out,
                         int sock_fd);

bool BatchProcessFile(BatchSpec_p spec,
                      long wct_limit,
                      StructFOFSpec_p ctrl,
                      char* source, char* dest);

long BatchProcessProblems(BatchSpec_p spec,
                          StructFOFSpec_p ctrl,
                          long total_wtc_limit,
                          char* dest_dir);

void BatchProcessInteractive(BatchSpec_p spec,
                             StructFOFSpec_p ctrl,
                             FILE* fp);



#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





