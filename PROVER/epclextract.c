/*-----------------------------------------------------------------------

File  : epclextract.c

Author: Stephan Schulz

Contents
 
  Read a PCL protocol and print all steps that are needed to print
  "proof" or "final" steps.

  Copyright 2002 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Tue Jul  9 19:15:05 MEST 2002
    New

-----------------------------------------------------------------------*/

#include <stdio.h>
#include <cio_commandline.h>
#include <cio_output.h>
#include <cio_tempfile.h>
#include <cio_signals.h>
#include <pcl_protocol.h>
#include <pcl_miniprotocol.h>

/*---------------------------------------------------------------------*/
/*                  Data types                                         */
/*---------------------------------------------------------------------*/

#define NAME    "epclextract"
#define VERSION "0.2"

typedef enum
{
   OPT_NOOPT=0,
   OPT_HELP,
   OPT_VERSION,
   OPT_VERBOSE,
   OPT_FAST,
   OPT_COMPETITION,
   OPT_OUTPUT,
   OPT_SILENT,
   OPT_OUTPUTLEVEL
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

   {OPT_FAST,
    'f', "fast-extract",
    NoArg, NULL,
    "Do a fast extract. With this option the program understands only "
    "a subset of PCL and assumes that all \"proof\" and \"final\" "
    "steps are at the end of the protokoll."},

   {OPT_COMPETITION,
    'c', "competition-framing",
    NoArg, NULL,
    "Print special \"begin\" and \"end\"comments around the proof "
    "object, as requiered by the CASC MIX* class."},

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

char       *outname    = NULL;
long       time_limit  = 10;
char       *executable = NULL;
bool       fast_extract = false,
           comp_frame = false;


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
   long            steps;
   int             i;

   assert(argv[0]);

   InitOutput();
   InitError(NAME);
   atexit(TempFileCleanup);

   ESignalSetup(SIGTERM);
   ESignalSetup(SIGINT);
   
   /* We need consistent name->var mappings here because we
      potentially read the compressed input format. */
   ClausesHaveLocalVariables = false;

   state = process_options(argc, argv);

   GlobalOut = OutOpen(outname);
   
   if(state->argc ==  0)
   {
      CLStateInsertArg(state, "-");
   }
   steps = 0;
   if(fast_extract)
   {
      PCLMiniProt_p prot;

      prot = PCLMiniProtAlloc();   
      for(i=0; state->argv[i]; i++)
      {
	 in = CreateScanner(StreamTypeFile, state->argv[i] , true, NULL);
	 ScannerSetFormat(in, TPTPFormat);
	 steps+=PCLMiniProtParse(in, prot);
	 CheckInpTok(in, NoToken);
	 DestroyScanner(in); 
      }
      VERBOUT2("PCL input read\n");
      
      PCLMiniProtMarkProofClauses(prot, true);

      if(comp_frame)
      {
	 fprintf(GlobalOut, "# Proof status evidence starts here.\n");
      }
      PCLMiniProtPrintProofClauses(GlobalOut,prot);
      if(comp_frame)
      {
	 fprintf(GlobalOut, "# Proof status evidence ends here.\n");
      }
#ifdef FAST_EXIT
      exit(0);
#endif
      PCLMiniProtFree(prot);
   }
   else
   {
      PCLProt_p prot;
      
      prot = PCLProtAlloc();
      for(i=0; state->argv[i]; i++)
      {
	 in = CreateScanner(StreamTypeFile, state->argv[i] , true, NULL);
	 ScannerSetFormat(in, TPTPFormat);
	 steps+=PCLProtParse(in, prot);
	 CheckInpTok(in, NoToken);
	 DestroyScanner(in); 
      }
      VERBOUT2("PCL input read\n");
      
      PCLProtMarkProofClauses(prot);
      if(comp_frame)
      {
	 fprintf(GlobalOut, "# Proof status evidence starts here.\n");
      }
      PCLProtPrintProofClauses(GlobalOut,prot);
      if(comp_frame)
      {
	 fprintf(GlobalOut, "# Proof status evidence ends here.\n");
      }
#ifdef FAST_EXIT
      exit(0);
#endif
      PCLProtFree(prot);
   }
   CLStateFree(state);
   
   fflush(GlobalOut);
   OutClose(GlobalOut);
   
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
	    break;
      case OPT_VERSION:
	    printf(NAME " " VERSION "\n");
	    exit(NO_ERROR);
	    break;
      case OPT_FAST:
	    fast_extract = true;
	    break;
      case OPT_COMPETITION:
	    comp_frame = true;
	    break;
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
"Usage: " NAME " [options] [files]\n"
"\n"
"Read an PCL2 protocol and print the steps necessary for proving"
"the clauses in \"proof\" or \"final\" steps.\n"
"\n");
   PrintOptions(stdout, opts);
   fprintf(out, "\n\
Copyright 2002-2003 by Stephan Schulz, " STS_MAIL "\n\
\n\
This program is a part of the support structure for the E equational\n\
theorem prover. You can find the latest version of the E distribution\n\
as well as additional information at\n"
E_URL
"\n\
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


