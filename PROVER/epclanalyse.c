/*-----------------------------------------------------------------------

File  : epclanalyse.c

Author: Stephan Schulz

Contents

  Read a PCL protocol and collect and print a number of statistics on
  the protocol.

  Copyright 2002-2009 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Thu Feb 28 13:45:43 MET 2002
    New

-----------------------------------------------------------------------*/

#include <stdio.h>
#include <cio_commandline.h>
#include <cio_output.h>
#include <cio_tempfile.h>
#include <cio_signals.h>
#include <pcl_propanalysis.h>
#include <e_version.h>


/*---------------------------------------------------------------------*/
/*                  Data types                                         */
/*---------------------------------------------------------------------*/

#define NAME    "epclanalyse"

typedef enum
{
   OPT_NOOPT=0,
   OPT_HELP,
   OPT_VERSION,
   OPT_VERBOSE,
   OPT_OUTPUT,
   OPT_SILENT
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
    '\0', "version",
    NoArg, NULL,
    "Print the version number of the program."},

   {OPT_VERBOSE,
    'v', "verbose",
    OptArg, "1",
    "Verbose comments on the progress of the program."},

   {OPT_OUTPUT,
    'o', "output-file",
    ReqArg, NULL,
   "Redirect output into the named file."},

   {OPT_SILENT,
    's', "silent",
    NoArg, NULL,
    "Equivalent to --output-level=0."},

   {OPT_NOOPT,
    '\0', NULL,
    NoArg, NULL,
    NULL}
};

char *outname    = NULL;
long time_limit  = 10;
char *executable = NULL;
bool app_encode  = false;


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/

CLState_p process_options(int argc, char* argv[]);
void print_help(FILE* out);

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/


int main(int argc, char* argv[])
{
   CLState_p       state;
   Scanner_p       in;
   PCLProt_p       prot;
   int             i;
   PCLPropDataCell data;

   assert(argv[0]);

#ifdef STACK_SIZE
   INCREASE_STACK_SIZE;
#endif
   InitIO(NAME);
   atexit(TempFileCleanup);

   ESignalSetup(SIGTERM);
   ESignalSetup(SIGINT);

   OutputFormat = TPTPFormat;
   /* We need consistent name->var mappings here because we
      potentially read the compressed input format. */
   ClausesHaveLocalVariables = false;

   state = process_options(argc, argv);

   OpenGlobalOut(outname);
   prot = PCLProtAlloc();

   if(state->argc ==  0)
   {
      CLStateInsertArg(state, "-");
   }
   for(i=0; state->argv[i]; i++)
   {
      in = CreateScanner(StreamTypeFile, state->argv[i], true, NULL, true);
      ScannerSetFormat(in, TPTPFormat);
      PCLProtParse(in, prot);
      CheckInpTok(in, NoToken);
      DestroyScanner(in);
   }
   VERBOUT2("PCL input read\n");

   PCLProtPropAnalyse(prot, &data);
   PCLProtPropDataPrint(GlobalOut, &data);
   PCLProtFree(prot);

   CLStateFree(state);

   fflush(GlobalOut);
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
// Global Variables:
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
      case OPT_VERSION:
       printf(NAME " " VERSION "\n");
       exit(NO_ERROR);
      case OPT_OUTPUT:
       outname = arg;
       break;
      case OPT_SILENT:
       OutputLevel = 0;
       break;
      default:
       assert(false);
       break;
      }
   }
   return state;
}

void print_help(FILE* out)
{
   fprintf(out,
      "\n"
      "\n"
NAME " " VERSION "\n"
      "\n"
"Usage: " NAME " [options] [files]\n\
\n\
Read an PCL2 protocol and print a number of statistics about the\n\
protocol and its clauses.\n\
\n");
   PrintOptions(stdout, opts, "Options\n\n");
   fprintf(out, "\n\
Copyright (C) 2002-2009 by Stephan Schulz, " STS_MAIL "\n\
\n\
This program is a part of the support structure for the E equational\n\
theorem prover. You can find the latest version of the E distribution\n\
as well as additional information at\n\
http://wwwjessen.informatik.tu-muenchen.de/~schulz/WORK/eprover.html.\n\
\n\
This program is free software; you can redistribute it and/or modify\n\
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
Boston, MA  02111-1307 USA\n\
\n\
The original copyright holder can be contacted as\n\
\n"
STS_SNAIL
"\n");
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
