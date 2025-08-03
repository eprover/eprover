/*-----------------------------------------------------------------------

  File  : epclextract.c

  Author: Stephan Schulz

  Contents

  Read a PCL protocol and print all steps that are needed to print
  "proof", "final", or "extract" steps.

  Copyright 2002-2022 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Tue Jul  9 19:15:05 MEST 2002

  -----------------------------------------------------------------------*/

#include <stdio.h>
#include <cio_commandline.h>
#include <cio_output.h>
#include <cio_tempfile.h>
#include <cio_signals.h>
#include <pcl_protocol.h>
#include <pcl_miniprotocol.h>
#include <e_version.h>


/*---------------------------------------------------------------------*/
/*                  Data types                                         */
/*---------------------------------------------------------------------*/

#define NAME    "epclextract"

typedef enum
{
   OPT_NOOPT=0,
   OPT_HELP,
   OPT_VERSION,
   OPT_VERBOSE,
   OPT_FAST,
   OPT_PASS_COMMENTS,
   OPT_TSTP_PRINT,
   OPT_COMPETITION,
   OPT_NO_EXTRACT,
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
    'V', "version",
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
    "steps are at the end of the protocoll."},

   {OPT_PASS_COMMENTS,
    'C', "forward-comments",
    NoArg, NULL,
    "Pass comments found in the input through to the output while"
    " reading input."},

   {OPT_COMPETITION,
    'c', "competition-framing",
    NoArg, NULL,
    "Print special \"begin\" and \"end\"comments around the proof "
    "object, as required by the CASC MIX* class."},

   {OPT_NO_EXTRACT,
    'n', "no-extract",
    NoArg, NULL,
    "Don't extract, print back all steps (actually, it treats all "
    "steps as proof steps). Useful as a syntax checker, or if you want"
    " to convert PCL to TSTP with the next option."},

   {OPT_TSTP_PRINT,
    '\0', "tstp-out",
    NoArg, NULL,
    "Print proof protocol in TSTP syntax (default is PCL)."},

   {OPT_TSTP_PRINT,
    '\0', "tptp3-out",
    NoArg, NULL,
    "Synonymous with --tstp-out."},

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
   comp_frame = false,
   no_extract = false,
   pass_comments = false,
   app_encode = false;
OutputFormatType output_format = pcl_format;

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
   int             i;
   bool            empty_clause = false;
   PCLMiniProt_p   mprot = NULL;
   PCLProt_p       prot = NULL;

   assert(argv[0]);

#ifdef STACK_SIZE
   INCREASE_STACK_SIZE;
#endif
   SupportShellPCL = true;
   InitIO(NAME);
   atexit(TempFileCleanup);

   ESignalSetup(SIGTERM);
   ESignalSetup(SIGINT);

   /* We need consistent name->var mappings here because we
      potentially read the compressed input format. */
   ClausesHaveLocalVariables = false;

   state = process_options(argc, argv);

   OpenGlobalOut(outname);

   if(state->argc ==  0)
   {
      CLStateInsertArg(state, "-");
   }
   if(fast_extract)
   {
      mprot = PCLMiniProtAlloc();
   }
   else
   {
      prot = PCLProtAlloc();
   }
   for(i=0; state->argv[i]; i++)
   {
      in = CreateScanner(StreamTypeFile, state->argv[i], !pass_comments,
                         NULL, true);
      ScannerSetFormat(in, TPTPFormat);
      if(fast_extract)
      {
         PCLMiniProtParse(in, mprot);
      }
      else
      {
         PCLProtParse(in, prot);
      }
      CheckInpTok(in, NoToken);
      DestroyScanner(in);
   }
   VERBOUT2("PCL input read\n");
   fflush(GlobalOut);

   if(fast_extract)
   {
      if(no_extract)
      {
         PCLMiniProtSetClauseProp(mprot, PCLIsProofStep);
      }
      else
      {
         empty_clause = PCLMiniProtMarkProofClauses(mprot, true);
      }
   }
   else
   {
      if(no_extract)
      {
         PCLProtSetProp(prot, PCLIsProofStep);
      }
      else
      {
         empty_clause = PCLProtMarkProofClauses(prot);
      }
   }
   if(comp_frame)
   {
      if(no_extract)
      {
         fprintf(GlobalOut, COMCHAR" SZS output start Derivation.\n");
      }
      else if(empty_clause)
      {
         fprintf(GlobalOut, COMCHAR" SZS output start CNFRefutation.\n");
      }
      else
      {
         fprintf(GlobalOut, COMCHAR" SZS output start Saturation.\n");
      }
   }
   if(fast_extract)
   {
      PCLMiniProtPrintProofClauses(GlobalOut,mprot, output_format);
   }
   else
   {
      PCLProtPrintProofClauses(GlobalOut,prot, output_format);
   }
   if(comp_frame)
   {
      if(no_extract)
      {
         fprintf(GlobalOut, COMCHAR" SZS output end Derivation.\n");
      }
      else if(empty_clause)
      {
         fprintf(GlobalOut, COMCHAR" SZS output end CNFRefutation\n");
      }
      else
      {
         fprintf(GlobalOut, COMCHAR" SZS output end Saturation.\n");
      }
   }
#ifdef FAST_EXIT
   exit(0);
#else
   if(fast_extract)
   {
      PCLMiniProtFree(mprot);
   }
   else
   {
      PCLProtFree(prot);
   }
   CLStateFree(state);

   fflush(GlobalOut);
   OutClose(GlobalOut);
   ExitIO();

#ifdef CLB_MEMORY_DEBUG
   MemFlushFreeList();
   MemDebugPrintStats(stdout);
#endif

   return 0;
#endif
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
      case OPT_FAST:
            fast_extract = true;
            break;
      case OPT_PASS_COMMENTS:
            pass_comments = true;
            break;
      case OPT_COMPETITION:
            comp_frame = true;
            break;
      case OPT_NO_EXTRACT:
            no_extract = true;
            break;
      case OPT_TSTP_PRINT:
            output_format = tstp_format;
            OutputFormat = TSTPFormat;
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
           " the clauses in \"proof\", \"final\", or \"extract\" steps.\n"
           "\n");
   PrintOptions(stdout, opts, "Options\n\n");
   fprintf(out, "\n\n" E_FOOTER);
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
