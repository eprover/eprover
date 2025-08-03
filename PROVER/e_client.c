/*-----------------------------------------------------------------------

File  : e_client.c

Author: Stephan Schulz

Contents

  Parse a problem specification, connect to the e_server, and have it
  trie to solve it.

  Copyright 2011 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Mon Feb 21 13:24:04 CET 2011
    New (but borrowing from e_server)

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

#define NAME         "e_client"

typedef enum
{
   OPT_NOOPT=0,
   OPT_HELP,
   OPT_VERSION,
   OPT_VERBOSE,
   OPT_OUTPUT,
   OPT_SERVER,
   OPT_PORT,
   OPT_OUTPUTLEVEL,
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

   {OPT_SERVER,
    'S', "server",
    ReqArg, NULL,
    "Specify the address of the server. The default is 'localhost'."},

   {OPT_PORT,
    'P', "service-port",
    ReqArg, NULL,
    "Specify the port to use for the deduction service. The default"
    " is to use 3666"},

   {OPT_PORT,
    'P', "port",
    ReqArg, NULL,
    "Specify the port to use for the deduction service."},


   {OPT_NOOPT,
    '\0', NULL,
    NoArg, NULL,
    NULL}
};
IOFormat parse_format = TSTPFormat;
char     *outname    = NULL;
char     *server     = "localhost";
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
// Function: tcp_msg_wait()
//
//   Blockingly read messages off the provided socket until the
//   expected reply has been read. Dump communication to GlobalOut.
//
// Global Variables: -
//
// Side Effects    : IO, memory
//
/----------------------------------------------------------------------*/

void tcp_msg_wait(int sock, char* reply)
{
   char *msg;

   while(true)
   {
      msg = TCPStringRecvX(sock);
      if(msg)
      {
         fprintf(GlobalOut, COMCHAR" Server: %s\n", msg);
         if(strcmp(msg, reply)==0)
         {
            FREE(msg);
            break;
         }
         FREE(msg);
      }
      else
      {
         TmpErrno = errno;
         SysError("Connection to server failed", SYS_ERROR);
      }
   }
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
   int              i;
   DStr_p           problem;
   DStr_p           cmd;
   int              sock;

   assert(argv[0]);

   InitIO(NAME);
   DocOutputFormat = tstp_format;
   OutputFormat = TSTPFormat;

   state = process_options(argc, argv);

   OpenGlobalOut(outname);

   if(state->argc ==  0)
   {
      CLStateInsertArg(state, "-");
   }

   problem = DStrAlloc();
   cmd     = DStrAlloc();
   for(i=0; state->argv[i]; i++)
   {
      FileLoad(state->argv[i],problem);
   }
   //fprintf(GlobalOut, "Problem:\n%s", DStrView(problem));

   sock = CreateClientSock(server, port);

   TCPStringSendX(sock, "hello");
   tcp_msg_wait(sock, "ready");
   TCPStringSendX(sock, "add");
   TCPStringSendX(sock, DStrView(problem));
   TCPStringSendX(sock, "prove");
   tcp_msg_wait(sock, "result");


   /* Backtracking is automatic on termination of the connection */


   close(sock);
   DStrFree(cmd);
   DStrFree(problem);
   CLStateFree(state);

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
      case OPT_SERVER:
            server = arg;
            break;
      case OPT_PORT:
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
Read an problem specification, connect to the E deduction server, \n\
and try to have the problem solved.\n\
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
