/*-----------------------------------------------------------------------

File  : edpll.c

Author: Stephan Schulz

Contents

  Read a ground problem and try to refute (or satisfy) it.

  Copyright 2003 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Thu May  1 20:40:24 CEST 2003
    New

-----------------------------------------------------------------------*/

#include <cio_commandline.h>
#include <cio_output.h>
#include <cio_signals.h>
#include <cpr_dpll.h>
#include <e_version.h>

/*---------------------------------------------------------------------*/
/*                  Data types                                         */
/*---------------------------------------------------------------------*/

#define NAME "edpll"

typedef enum
{
   OPT_NOOPT=0,
   OPT_HELP,
   OPT_VERSION,
   OPT_VERBOSE,
   OPT_OUTPUT,
   OPT_SILENT,
   OPT_OUTPUTLEVEL,
   OPT_TPTP_PARSE,
   OPT_DIMACS_PRINT,
   OPT_MEM_LIMIT,
   OPT_CPU_LIMIT,
   OPT_SOFTCPU_LIMIT,
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
    "Verbose comments on the progress of the program by printing "
    "technical information to stderr."},

   {OPT_OUTPUT,
    'o', "output-file",
    ReqArg, NULL,
   "Redirect output into the named file."},

   {OPT_SILENT,
    's', "silent",
    NoArg, NULL,
    "Equivalent to --output-level=0."},

   {OPT_OUTPUTLEVEL,
    'l', "output-level",
    ReqArg, NULL,
    "Select an output level, greater values imply more verbose "
    "output. Level 0 produces "
    "nearly no output, level 1 produces minimal additional output."
    "Higher levels are without meaning in edpll (I think)."},

   {OPT_TPTP_PARSE,
    '\0', "tptp-in",
    NoArg, NULL,
    "Parse TPTP format instead of lop (does not understand includes, "
    "as TPTP include syntax is considered harmful)."},

   {OPT_DIMACS_PRINT,
    'd', "dimacs",
    NoArg, NULL,
    "Print output in the DIMACS format suitable for many propositional "
    "provers."},

   {OPT_MEM_LIMIT,
    'm', "memory-limit",
    ReqArg, NULL,
    "Limit the memory the system may use. The argument is "
    "the allowed amount of memory in MB. This option may not work "
    "everywhere, due to broken and/or strange behaviour of setrlimit() "
    "in some UNIX implementations. It does work under all tested "
    "versions of Solaris and GNU/Linux."},

   {OPT_CPU_LIMIT,
    '\0', "cpu-limit",
    OptArg, "300",
    "Limit the cpu time the program should run. The optional argument "
    "is the CPU time in seconds. The program will terminate immediately"
    " after reaching the time limit, regardless of internal state. This"
    " option may not work "
    "everywhere, due to broken and/or strange behaviour of setrlimit() "
    "in some UNIX implementations. It does work under all tested "
    "versions of Solaris, HP-UX and GNU/Linux. As a side effect, this "
    "option will inhibit core file writing."},

   {OPT_SOFTCPU_LIMIT,
    '\0', "soft-cpu-limit",
    OptArg, "310",
    "Limit the cpu time spend in grounding. After the time expires,"
    " the prover will print an partial system."},

    {OPT_NOOPT,
    '\0', NULL,
    NoArg, NULL,
    NULL}
};

char     *outname = NULL;
IOFormat parse_format = LOPFormat;
bool     dimacs_format = false;
bool     app_encode = false;

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
   TypeBank_p      typebank;
   Sig_p           sig;
   Scanner_p       in;
   int             i;
   CLState_p       state;
   DPLLFormula_p   form;
   DPLLState_p     dpllstate;

   assert(argv[0]);
#ifdef STACK_SIZE
   INCREASE_STACK_SIZE;
#endif
   InitIO(NAME);
   ESignalSetup(SIGXCPU);

   state = process_options(argc, argv);

   OpenGlobalOut(outname);

   if(state->argc ==  0)
   {
      CLStateInsertArg(state, "-");
   }

   typebank = TypeBankAlloc();
   sig      = SigAlloc(typebank);
   form     = DPLLFormulaAlloc();
   for(i=0; state->argv[i]; i++)
   {
      in = CreateScanner(StreamTypeFile, state->argv[i] , true, NULL, true);
      ScannerSetFormat(in, parse_format);

      DPLLFormulaParseLOP(in, sig, form);
      DestroyScanner(in);
   }
   dpllstate = DPLLStateAlloc(form);

   CLStateFree(state);
   UNUSED(dpllstate); /* Stiffle warning for now */
#ifndef FAST_EXIT
   DPLLStateFree(dpllstate);
   SigFree(sig);
   TypeBankFree(typebank);
#endif
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
// Global Variables: opts, Verbose, TermPrologArgs,
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
   struct rlimit limit = {RLIM_INFINITY, RLIM_INFINITY};
   rlim_t mem_limit = 0;

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
       printf("classify_problem " VERSION "\n");
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
      case OPT_TPTP_PARSE:
       parse_format = TPTPFormat;
       break;
      case OPT_DIMACS_PRINT:
       dimacs_format = true;
       break;
      case OPT_MEM_LIMIT:
            if(strcmp(arg, "Auto")==0)
            {
               long tmpmem =  GetSystemPhysMemory();
               mem_limit = 0.8*tmpmem;

               if(tmpmem==-1)
               {
                  Error("Cannot find physical memory automatically. "
                        "Give explicit value to --memory-limit", OTHER_ERROR);
               }
               mem_limit = MEGA*mem_limit;
               VERBOSE(fprintf(stderr,
                               "Physical memory determined as %ld MB\n"
                               "Memory limit set to %lld MB\n",
                               tmpmem,
                               (long long)mem_limit););
            }
            else
            {
               mem_limit = MEGA*CLStateGetIntArg(handle, arg);
            }
       break;
      case OPT_CPU_LIMIT:
       HardTimeLimit = CLStateGetIntArg(handle, arg);
       if(SoftTimeLimit != RLIM_INFINITY)
       {
          if(HardTimeLimit<=SoftTimeLimit)
          {
        Error("Hard time limit has to be larger than soft"
         "time limit", USAGE_ERROR);
          }
       }
       break;
      case OPT_SOFTCPU_LIMIT:
       SoftTimeLimit = CLStateGetIntArg(handle, arg);
       if(HardTimeLimit != RLIM_INFINITY)
       {
          if(HardTimeLimit<=SoftTimeLimit)
          {
        Error("Soft time limit has to be smaller than hard"
         "time limit", USAGE_ERROR);
          }
       }
       break;
      default:
    assert(false);
    break;
      }
   }
   if((HardTimeLimit!=RLIM_INFINITY)||(SoftTimeLimit!=RLIM_INFINITY))
   {
      if(getrlimit(RLIMIT_CPU, &limit))
      {
    TmpErrno = errno;
    SysError("Unable to get system cpu time limit", SYS_ERROR);
      }
      SystemTimeLimit = limit.rlim_max;
      if(SoftTimeLimit!=RLIM_INFINITY)
      {
    limit.rlim_max = SystemTimeLimit; /* Redundant, but clearer */
    limit.rlim_cur = SoftTimeLimit;
    TimeLimitIsSoft = true;
      }
      else
      {
    limit.rlim_max = SystemTimeLimit;
    limit.rlim_cur = HardTimeLimit;
    TimeLimitIsSoft = false;
      }
      if(setrlimit(RLIMIT_CPU, &limit))
      {
    TmpErrno = errno;
    SysError("Unable to set cpu time limit", SYS_ERROR);
      }
      limit.rlim_max = RLIM_INFINITY;
      limit.rlim_cur = 0;

      if(setrlimit(RLIMIT_CORE, &limit))
      {
    TmpErrno = errno;
    SysError("Unable to prevent core dumps", SYS_ERROR);
      }
   }
   SetMemoryLimit(mem_limit);

   return state;
}


void print_help(FILE* out)
{
   fprintf(out, "\n\
\n\
"NAME " " VERSION "\n\
\n\
Usage: " NAME " [options] [files]\n\
\n\
Read a set of ground clauses and try to refute (or satisfy) it.\n\
Not completed yet!\n\
\n");
   PrintOptions(stdout, opts, "Options\n\n");
   fprintf(out, "\n\
Copyright (C) 2003 by Stephan Schulz, " STS_MAIL" \n\
\n\
This program is a part of the support structure for the E equational\n\
theorem prover. You can find the latest version of the E distribution\n\
as well as additional information at\n"
E_URL
"\n\n\
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
