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
      fprintf(out, "include('%s').\n", 
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


/*-----------------------------------------------------------------------
//
// Function: BatchSpecParse()
//
//   Parse a batch specification file. This is somewhat wonky - the
//   spec file syntax is not really well-defined, and what we know
//   about them is that comments and newlines are significant for the
//   structure. This just ignores those and hopes for the best.
//
// Global Variables: -
//
// Side Effects    : Input, memory
//
/----------------------------------------------------------------------*/

BatchSpec_p BatchSpecParse(Scanner_p in)
{
   BatchSpec_p handle = BatchSpecAlloc();
   char *dummy;
   
   dummy = ParseDottedId(in);
   if(strcmp(dummy, "division.category")!= 0)
   {
      Error("Expected \"division.category\"\n", SYNTAX_ERROR);
   }
   FREE(dummy);
   handle->category = ParseDottedId(in);
   
   dummy = ParseDottedId(in);
   if(strcmp(dummy, "limit.time.problem.wc")!= 0)
   {
      Error("Expected \"limit.time.problem.wc\"\n", SYNTAX_ERROR);
   }
   FREE(dummy);
   handle->per_prob_time = ParseInt(in);

   dummy = ParseDottedId(in);
   if(strcmp(dummy, "limit.time.overall.wc")!= 0)
   {
      Error("Expected \"limit.time.overall.wc\"\n", SYNTAX_ERROR);
   }
   FREE(dummy);
   handle->total_time = ParseInt(in);

   while(TestInpId(in, "include"))
   {
      dummy = ParseBasicInclude(in);
      PStackPushP(handle->includes, dummy);
   }
   
   while(!TestInpTok(in, NoToken))
   {
      dummy = ParseFilename(in);
      PStackPushP(handle->source_files, dummy);
      dummy = ParseFilename(in);
      PStackPushP(handle->dest_files, dummy);

   }      
   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: BatchControlAlloc()
//
//   Allocate a BatchControl data structure.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

BatchControl_p BatchControlAlloc(void)
{
   BatchControl_p handle = BatchControlCellAlloc();

   handle->sig             = SigAlloc();
   SigInsertFOFCodes(handle->sig);
   handle->terms           = TBAlloc(handle->sig);
   handle->clause_sets     = PStackAlloc();
   handle->formula_sets    = PStackAlloc();
   handle->parsed_includes = NULL;

   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: BatchControlFree()
//
//   Free a BatchControl data structure.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void BatchControlFree(BatchControl_p ctrl)
{
   FormulaSet_p fset;
   ClauseSet_p  cset;

   while(!PStackEmpty(ctrl->clause_sets))
   {
      cset = PStackPopP(ctrl->clause_sets);
      ClauseSetFree(cset);
   }
   PStackFree(ctrl->clause_sets);

   while(!PStackEmpty(ctrl->formula_sets))
   {
      fset = PStackPopP(ctrl->formula_sets);
      FormulaSetFree(fset);
   }   
   PStackFree(ctrl->formula_sets);

   SigFree(ctrl->sig);
   ctrl->terms->sig = NULL;
   TBFree(ctrl->terms);
   StrTreeFree(ctrl->parsed_includes);
   
   BatchControlCellFree(ctrl);
}


/*-----------------------------------------------------------------------
//
// Function: BatchControlInit()
//
//   Initialize a BatchControllCell by parsing all the include files
//   in spec.
//
// Global Variables: -
//
// Side Effects    : I/O, memory operations
//
/----------------------------------------------------------------------*/

long BatchControlInit(BatchSpec_p spec, BatchControl_p ctrl)
{
   PStackPointer i;
   char*        iname;
   FormulaSet_p fset;
   ClauseSet_p  cset;
   Scanner_p    in;
   long         res = 0;
   IntOrP       dummy;

   for(i=0; i<PStackGetSP(spec->includes); i++)
   {
      iname = PStackElementP(spec->includes, i);
      if(!StrTreeFind(&(ctrl->parsed_includes), iname))
      {
         in = CreateScanner(StreamTypeFile, iname, true, NULL);
         ScannerSetFormat(in, TSTPFormat);

         fprintf(GlobalOut, "# Parsing %s\n", iname);
         cset = ClauseSetAlloc();
         fset = FormulaSetAlloc();
         res += FormulaAndClauseSetParse(in, cset, fset, ctrl->terms, 
                                         NULL, 
                                         &(ctrl->parsed_includes));
         PStackPushP(ctrl->clause_sets, cset);
         PStackPushP(ctrl->formula_sets, fset);
         StrTreeStore(&(ctrl->parsed_includes), iname, dummy, dummy);
         
         ClauseSetPrint(stdout, cset, true);
         FormulaSetPrint(stdout, fset, true);
         
         DestroyScanner(in);
      }
   }
   return res;
}




/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


