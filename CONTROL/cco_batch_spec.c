/*-----------------------------------------------------------------------

File  : cco_batch_spec.c

Author: Stephan Schulz (schulz@eprover.org)

Contents
 
  CASC-J5 batch specification file.

  Copyright 2010 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Tue Jun 29 04:41:18 CEST 2010
    New

-----------------------------------------------------------------------*/

#include "cco_batch_spec.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: BatchSpecAlloc()
//
//   Allocate an empty, initialized batch spec file.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

BatchSpec_p BatchSpecAlloc(void)
{
   BatchSpec_p handle = BatchSpecCellAlloc();

   handle->category      = NULL;
   handle->per_prob_time = 0;
   handle->total_time    = 0;
   handle->includes      = PStackAlloc();
   handle->source_files  = PStackAlloc();
   handle->dest_files    = PStackAlloc();

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: BatchSpecFree()
//
//   Free a batch spec structure with all information.
//
// Global Variables: -
//
// Side Effects    : Memory management
//
/----------------------------------------------------------------------*/

void BatchSpecFree(BatchSpec_p spec)
{
   char* str;

   FREE(spec->category);
   
   while(!PStackEmpty(spec->includes))
   {
      str = PStackPopP(spec->includes);
      FREE(str);
   }
   PStackFree(spec->includes);

   while(!PStackEmpty(spec->source_files))
   {
      str = PStackPopP(spec->source_files);
      FREE(str);
   }
   PStackFree(spec->source_files);

   while(!PStackEmpty(spec->dest_files))
   {
      str = PStackPopP(spec->dest_files);
      FREE(str);
   }
   PStackFree(spec->dest_files);

   BatchSpecCellFree(spec);
}


/*-----------------------------------------------------------------------
//
// Function: BatchSpecPrint()
//
//   Print a BatchSpec cell in the original form (or as close as I can
//   make it).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void BatchSpecPrint(FILE* out, BatchSpec_p spec)
{
   PStackPointer i;

   fprintf(out, "%% SZS start BatchConfiguration\n");
   fprintf(out, "division.category %s\n", spec->category);
   fprintf(out, "limit.time.problem.wc %ld\n", spec->per_prob_time);
   fprintf(out, "limit.time.overall.wc %ld\n", spec->total_time);
   fprintf(out, "%% SZS end BatchConfiguration\n");
   fprintf(out, "%% SZS start BatchIncludes\n");

   for(i=0; i<PStackGetSP(spec->includes); i++)
   {
      fprintf(out, "include('%s;).\n", 
              (char*)PStackElementP(spec->includes, i));
   }
   fprintf(out, "%% SZS end BatchIncludes\n");
   fprintf(out, "%% SZS start BatchProblems\n");

   for(i=0; i<PStackGetSP(spec->source_files); i++)
   {
      fprintf(out, "%s %s\n", 
              (char*)PStackElementP(spec->source_files, i),
              (char*)PStackElementP(spec->dest_files, i));
   }
   fprintf(out, "%% SZS end BatchProblems\n");
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


