/*-----------------------------------------------------------------------

File  : e_server.c

Author: Stephan Schulz

Contents

  Parse a problem specification and a filter setup, and offer
  deduction in the specification as a service via a TCP port.

  Copyright 2011 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Mon Feb 21 13:24:04 CET 2011
    New (but borrowing from LTB runner)

-----------------------------------------------------------------------*/

#include <sys/select.h>
#include <netinet/in.h>
#include <clb_defines.h>
#include <cio_commandline.h>
#include <cio_output.h>
#include <cio_network.h>
#include <ccl_relevance.h>
#include <cio_signals.h>
#include <ccl_formulafunc.h>
#include <cco_batch_spec.h>
#include <ccl_sine.h>
#include <e_version.h>


/*---------------------------------------------------------------------*/
/*                  Data types                                         */
/*---------------------------------------------------------------------*/

#define NAME         "e_server"

typedef enum
{
   OPT_NOOPT=0,
   OPT_HELP,
   OPT_VERSION,
   OPT_VERBOSE,
   OPT_OUTPUT,
   OPT_FILTER,
   OPT_PROVER,
   OPT_SERVICE_PORT,
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

   {OPT_PROVER,
    'p', "prover",
    ReqArg, NULL,
    "Specify the prover binary to use. The default is 'eprover', "
    "and initially, only E is supported. This option does accept "
    "absolute and relative paths."},

   {OPT_SERVICE_PORT,
    'P', "service-port",
    ReqArg, NULL,
    "Specify the port to use for the deduction service."},

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
    "fully conforming at all times. E works on all TPTP 4.1.0 input "
    "files (including includes)."},

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
IOFormat parse_format = TSTPFormat;
char     *outname    = NULL;
char     *filtername = NULL;
char     *prover     = "eprover";
int      port        = 3666;
bool     app_encode = false;


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

   assert(argv[0]);

   InitIO(NAME);
   DocOutputFormat = tstp_format;
   OutputFormat = TSTPFormat;

   state = process_options(argc, argv);


   OpenGlobalOut(outname);

   if(state->argc < 1)
   {
      Error("Usage: e_server <domain-spec> [<options>]\n", USAGE_ERROR);
   }

   if(filtername)
   {
      filters = AxFilterSetAlloc();
      in = CreateScanner(StreamTypeFile, filtername, true, NULL, true);
      AxFilterSetParse(in, filters);
      DestroyScanner(in);
   }
   else
   {
      filters = AxFilterSetCreateInternal(AxFilterDefaultSet);
   }
   for(i=0; state->argv[i]; i++)
   {
      PStackPushP(prob_names,  state->argv[i]);
   }

   ctrl = StructFOFSpecAlloc();
   StructFOFSpecParseAxioms(ctrl, prob_names, parse_format, NULL);
   StructFOFSpecInitDistrib(ctrl, false);
   StructFOFSpecResetShared(ctrl);

   /* Do stuff */

   {
      int sock = CreateServerSock(port);
      int conn = -1;
      fd_set rfds, wfds, xfds;
      int res, tmp;
      struct sockaddr addr;
      socklen_t       addr_len = sizeof(addr);
      char *msg;
      MsgStatus msg_stat;

      Listen(sock);

      while(true)
      {
         printf("Main loop\n");
         FD_ZERO(&rfds);
         FD_ZERO(&wfds);
         FD_ZERO(&xfds);
         FD_SET(sock, &rfds);
         if(conn!=-1)
         {
            FD_SET(conn, &rfds);
         }
         res = select(MAX(sock, conn)+1, &rfds, &wfds, &xfds, NULL);
         if(res != -1)
         {
            if((conn != -1) && FD_ISSET(conn, &rfds))
            {
               msg = TCPStringRecv(conn, &msg_stat, false);
               if(!msg)
               {
                  if(msg_stat == NWError)
                  {
                     printf("Read error\n");
                  }
                  else
                  {
                     printf("Connection closed\n");
                  }
                  close(conn);
                  conn = -1;
               }
               else
               {
                  printf("Received: %s\n", msg);
                  FREE(msg);
                  TCPStringSendX(conn, "wait");
                  TCPStringSendX(conn, "ready");
               }
            }
            if(FD_ISSET(sock, &rfds))
            {
               tmp = accept(sock, &addr, &addr_len);
               if(conn==-1)
               {
                  printf("Accepted %d\n", tmp);
                  conn = tmp;
               }
               else
               {
                  close(tmp);
               }
            }
         }
         else
         {
            perror("Something weird");
         }
      }
   }
   /* Done */

   StructFOFSpecFree(ctrl);
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
      case OPT_VERSION:
       fprintf(stdout, "E " VERSION " " E_NICKNAME "\n");
       exit(NO_ERROR);
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
      case OPT_PROVER:
            prover = arg;
            break;
      case OPT_SERVICE_PORT:
            port  = CLStateGetIntArg(handle, arg);
            if(port<0 || port>65535)
            {
               Error("Port numbers must be between 0 and 65535", USAGE_ERROR);
            }
            else if(port < IPPORT_RESERVED)
            {
               Warning("Port numbers less than %d require root level access",
                       IPPORT_RESERVED);
            }
            break;
      case OPT_LOP_PARSE:
      case OPT_LOP_FORMAT:
       parse_format = LOPFormat;
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
   fprintf(out, "\n\
E " VERSION " \"" E_NICKNAME "\"\n\
\n\
Usage: " NAME " [options] [files]\n\
\n\
Read an problem specification and offer deduction in the the structure\n\
described by the specification as a service.  All input formats (LOP,\n\
TPTP-2 and TPTP-3 are supported for the original specification, \n\
however, only TPTP-3 is used for the service. TPTP-3 is also the \n\
default format. Important options allow specificatio of the filters\n\
to use for proof attemtps, the dervice port, and the binary of the\n\
prover to use.\n\
\n");
   PrintOptions(stdout, opts, "Options:\n\n");
   fprintf(out, "\n\
Copyright (C) 2011 by Stephan Schulz, " STS_MAIL "\n\
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
