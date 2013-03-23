/*-----------------------------------------------------------------------

File  : e_axfilter.c

Author: Stephan Schulz

Contents

Parse a problem specification and a filter setup, and produce output
files corresponding to each filter.

  Copyright 2011 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Mon Feb 21 13:24:04 CET 2011
    New (but borrowing from LTB runner)

-----------------------------------------------------------------------*/

#include <clb_defines.h>
#include <cio_commandline.h>
#include <cio_output.h>
#include <ccl_relevance.h>
#include <cio_signals.h>
#include <ccl_formulafunc.h>
#include <cco_batch_spec.h>
#include <ccl_sine.h>
#include <e_version.h>


/*---------------------------------------------------------------------*/
/*                  Data types                                         */
/*---------------------------------------------------------------------*/

#define NAME         "e_axfilter"

typedef enum
{
   OPT_NOOPT=0,
   OPT_HELP,
   OPT_VERSION,
   OPT_VERBOSE,
   OPT_OUTPUT,
   OPT_FILTER,
   OPT_DUMP_FILTER,
   OPT_PRINT_STATISTICS,
   OPT_SILENT,
   OPT_OUTPUTLEVEL,
   OPT_LOP_PARSE,
   OPT_LOP_FORMAT,
   OPT_TPTP_PARSE,
   OPT_TPTP_FORMAT,
   OPT_TSTP_PARSE,
   OPT_TSTP_FORMAT,   
   OPT_DUMMY
}OptionCodes;



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


OptCell opts[] =
{
   {OPT_HELP, 
    'h', "help", 
    NoArg, NULL,
    "Print a short description of program usage and options."},

   {OPT_VERSION,
    'V', "version",
    NoArg, NULL,
    "Print the version number of the prover. Please include this"
    " with all bug reports (if any)."},

   {OPT_VERBOSE, 
    'v', "verbose", 
    OptArg, "1",
    "Verbose comments on the progress of the program. This differs "
    "from the output level (below) in that technical information is "
    "printed to stderr, while the output level determines which "
    "logical manipulations of the clauses are printed to stdout."},
   
   {OPT_OUTPUT,
    'o', "output-file",
    ReqArg, NULL,
    "Redirect output into the named file (this affects only some "
    "output, as most is written to automatically generated files "
    "based on the input and filter names."},

   {OPT_SILENT,
    's', "silent",
    NoArg, NULL,
    "Equivalent to --output-level=0."},

   {OPT_OUTPUTLEVEL,
    'l', "output-level",
    ReqArg, NULL,
    "Select an output level, greater values imply more verbose "
    "output."},  

   {OPT_FILTER, 
    'f', "filter",
    ReqArg, NULL,
    "Specify the filter definition file. If not set, the system "
    "will uses the built-in default."},
   
   {OPT_DUMP_FILTER,
    'd', "dump-filter",
    NoArg, NULL,
     "Print the filter definition in force."},
   
   {OPT_LOP_PARSE,
    '\0', "lop-in",
    NoArg, NULL,
    "Parse input in E-LOP, not the default TPTP-3 format."},

   {OPT_LOP_FORMAT,
    '\0', "lop-format",
    NoArg, NULL,
    "Equivalent to --lop-in."},

   {OPT_TPTP_PARSE,
    '\0', "tptp-in",
    NoArg, NULL,
    "Parse TPTP-2 format instead of E-LOP (but note that includes are "
    "handled according to TPTP-3 semantics)."},

   {OPT_TPTP_FORMAT,
    '\0', "tptp-format",
    NoArg, NULL,
    "Equivalent to --tptp-in."},

   {OPT_TPTP_PARSE,
    '\0', "tptp2-in",
    NoArg, NULL,
    "Synonymous with --tptp-in."},

   {OPT_TPTP_FORMAT,
    '\0', "tptp2-format",
    NoArg, NULL,
    "Synonymous with --tptp-in."},

   {OPT_TSTP_PARSE,
    '\0', "tstp-in",
    NoArg, NULL,
    "Parse TPTP-3 format instead of E-LOP (Note that TPTP-3 syntax "
    "is still under development, and the version in E may not be "
    "fully conforming at all times. E works on all TPTP 5.4.0 FOF "
    "and CNF input files (including includes)."},
   
   {OPT_TSTP_FORMAT,
    '\0', "tstp-format",
    NoArg, NULL,
    "Equivalent to --tstp-in."},

   {OPT_TSTP_PARSE,
    '\0', "tptp3-in",
    NoArg, NULL,
    "Synonymous with --tstp-in."},

   {OPT_TSTP_FORMAT,
    '\0', "tptp3-format",
    NoArg, NULL,
    "Synonymous with --tstp-in."},

   {OPT_NOOPT,
    '\0', NULL,
    NoArg, NULL,
    NULL}
};
IOFormat          parse_format = TSTPFormat;
char              *outname    = NULL;
char              *filtername = NULL;
bool              dumpfilter  = false;

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/

CLState_p process_options(int argc, char* argv[]);
void print_help(FILE* out);

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: filter_problem()
//
//   Given a structured problem data structure, an axfilter, and the
//   core name of the output, apply the filter to the problem and
//   write the result into a properly named file (which is determined
//   from the core name and the filter name).
//
// Global Variables: -
//
// Side Effects    : I/O, Memory operations
//
/----------------------------------------------------------------------*/

void filter_problem(StructFOFSpec_p ctrl, 
                    AxFilter_p filter, 
                    char* corename)
{
   DStr_p   filename = DStrAlloc();
   PStack_p formulas, clauses;
   FILE     *fp;

   DStrAppendStr(filename, corename);
   DStrAppendChar(filename, '_');
   DStrAppendStr(filename, filter->name);
   DStrAppendStr(filename, ".p");
   
   formulas = PStackAlloc();
   clauses  = PStackAlloc();

   StructFOFSpecGetProblem(ctrl, 
                           filter, 
                           clauses,
                           formulas);

   fprintf(GlobalOut, "# Filter: %s goes into file %s\n", 
           filter->name,
           DStrView(filename));

   fp = fopen(DStrView(filename), "w");
   PStackClausePrintTSTP(fp, clauses);
   PStackFormulaPrintTSTP(fp, formulas);
   fclose(fp);

   PStackFree(clauses);
   PStackFree(formulas);
   DStrFree(filename);
}


/*-----------------------------------------------------------------------
//
// Function: main()
//
//   Main function of the program.
//
// Global Variables: Yes
//
// Side Effects    : All
//
/----------------------------------------------------------------------*/

int main(int argc, char* argv[])
{
   CLState_p        state;
   StructFOFSpec_p  ctrl;
   PStack_p         prob_names = PStackAlloc();
   int              i;
   AxFilterSet_p    filters;
   Scanner_p        in;
   DStr_p           corename;
   char             *tname;

   assert(argv[0]);
   
   InitIO(NAME);
   DocOutputFormat = tstp_format;
   OutputFormat = TSTPFormat;

   state = process_options(argc, argv);


   OpenGlobalOut(outname);

   if(filtername)
   {
      filters = AxFilterSetAlloc();
      in = CreateScanner(StreamTypeFile, filtername, true, NULL);
      AxFilterSetParse(in, filters);
      DestroyScanner(in);
   }
   else
   {
      filters = AxFilterSetCreateInternal(AxFilterDefaultSet);
   }
   if(dumpfilter)
   {
      AxFilterSetPrint(GlobalOut, filters);
   }

   if(state->argc < 1)
   {
      Error("Usage: e_axfilter <problem> [<options>]\n", USAGE_ERROR);
   }    
   
   for(i=0; state->argv[i]; i++)
   {
      PStackPushP(prob_names,  state->argv[i]);      
   }
   /* Base name is the stripped base of the first argument */
   tname = FileNameStrip(state->argv[0]);
   corename = DStrAlloc();
   DStrAppendStr(corename, tname);
   FREE(tname);
   
   ctrl = StructFOFSpecAlloc();
   StructFOFSpecParseAxioms(ctrl, prob_names, parse_format);
   StructFOFSpecInitDistrib(ctrl);
   StructFOFSpecResetShared(ctrl);

   for(i=0; i<AxFilterSetElements(filters); i++)
   {
      /* SigPrint(stdout,ctrl->sig); */

      filter_problem(ctrl, 
                     AxFilterSetGetFilter(filters,i),
                     DStrView(corename));
   }

   StructFOFSpecFree(ctrl);
   DStrFree(corename);
   AxFilterSetFree(filters);
   CLStateFree(state);
   PStackFree(prob_names);

   OutClose(GlobalOut);
   ExitIO();
#ifdef CLB_MEMORY_DEBUG
   MemFlushFreeList();
   MemDebugPrintStats(stdout);
#endif
  
   return 0;
}


/*-----------------------------------------------------------------------
//
// Function: process_options()
//
//   Read and process the command line option, return (the pointer to)
//   a CLState object containing the remaining arguments.
//
// Global Variables: opts, Verbose, TBPrintInternalInfo 
//
// Side Effects    : Sets variables, may terminate with program
//                   description if option -h or --help was present
//
/----------------------------------------------------------------------*/

CLState_p process_options(int argc, char* argv[])
{
   Opt_p handle;
   CLState_p state;
   char*  arg;

   state = CLStateAlloc(argc,argv);
   
   while((handle = CLStateGetOpt(state, &arg, opts)))
   {
      switch(handle->option_code)
      {
      case OPT_VERBOSE:
	    Verbose = CLStateGetIntArg(handle, arg);
	    break;
      case OPT_HELP: 
	    print_help(stdout);
	    exit(NO_ERROR);
	    break;
      case OPT_VERSION:
	    fprintf(stdout, NAME " " VERSION " " E_NICKNAME "\n");
	    exit(NO_ERROR);
	    break;
      case OPT_OUTPUT:
	    outname = arg;
	    break;
      case OPT_SILENT:
	    OutputLevel = 0;
	    break;
      case OPT_OUTPUTLEVEL:
	    OutputLevel = CLStateGetIntArg(handle, arg);
	    break;
      case OPT_FILTER:
            filtername = arg;
            break;
      case OPT_DUMP_FILTER:
            dumpfilter = true;
            break;
      case OPT_LOP_PARSE:
      case OPT_LOP_FORMAT:
	    parse_format = LOPFormat;	    
	    break;
      case OPT_TPTP_PARSE:
      case OPT_TPTP_FORMAT:
	    parse_format = TPTPFormat;	    
	    break;
      case OPT_TSTP_PARSE:
      case OPT_TSTP_FORMAT:
	    parse_format = TSTPFormat;
	    break;
      default:
	    assert(false && "Unknown option");
	    break;
      }
   }
   return state;
}

void print_help(FILE* out)
{
   fprintf(out, "\n"
NAME " " VERSION " \"" E_NICKNAME "\"\n\
\n\
Usage: " NAME " [options] [files]\n\
\n\
This problem applies SinE-like goal-directed filters to a problem\n\
specification to generate reduced problem specifications that are easier\n\
to handle for the prover, but still are likely to contain the necessary\n\
axioms for a proof (if one exists). The program reads a problem\n\
specification and a filter specification, and produces one reduced\n\
specifiation for each filter given. Note that while all input \n\
formats (LOP, TPTP-2 and TPTP-3 are supported, output is only and\n\
automatically supported in TPTP-3, and the default input format is\n\
TPTP-3. Also note that unlike most of the other tools in the E\n\
distribution, this program does not support pipe-based input and output,\n\
since it uses file names generated from the input file name and filter\n\
names to store the different result files\n\
\n");
   PrintOptions(stdout, opts, "Options:\n\n");
   fprintf(out, "\n\
"STS_COPYRIGHT", " STS_MAIL "\n\
\n\
You can find the latest version of E and additional information at\n"
E_URL
"\n\n"
"This program is free software; you can redistribute it and/or modify\n\
it under the terms of the GNU General Public License as published by\n\
the Free Software Foundation; either version 2 of the License, or\n\
(at your option) any later version.\n\
\n\
This program is distributed in the hope that it will be useful,\n\
but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n\
GNU General Public License for more details.\n\
\n\
You should have received a copy of the GNU General Public License\n\
along with this program (it should be contained in the top level\n\
directory of the distribution in the file COPYING); if not, write to\n\
the Free Software Foundation, Inc., 59 Temple Place, Suite 330,\n\
Boston, MA  02111-1307 USA\n"
"\n\
The original copyright holder can be contacted as\n\
\n"
STS_SNAIL
"\n");

}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


