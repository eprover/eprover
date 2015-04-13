/*-----------------------------------------------------------------------

File  : cco_einteractive_mode.c

Author: Stephan Schulz (schulz@eprover.org)

Contents
 
  Code for parsing and handling the server's interactive mode.

  Copyright 2015 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

-----------------------------------------------------------------------*/

#include "cco_einteractive_mode.h"


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

char* help_message = "\
# Enter a job, 'help' or 'quit'. Finish any action with 'go.' on a line\n\
# of its own. A job consists of an optional job name specifier of the\n\
# form 'job <ident>.', followed by a specification of a first-order\n\
# problem in TPTP-3 syntax (including any combination of 'cnf', 'fof' and\n\
# 'include' statements. The system then tries to solve the specified\n\
# problem (including the constant background theory) and prints the\n\
# results of this attempt.\n";


// Defining commands used
#define STAGE_COMMAND "STAGE"
#define UNSTAGE_COMMAND "UNSTAGE"
#define REMOVE_COMMAND "REMOVE"
#define DOWNLOAD_COMMAND "DOWNLOAD"
#define ADD_COMMAND "ADD"
#define RUN_COMMAND "RUN"
#define LIST_COMMAND "LIST"
#define HELP_COMMAND "HELP"
#define QUIT_COMMAND "QUIT"
#define END_OF_BLOCK_TOKEN "GO\n"

// Defining Success messages
#define OK_SUCCESS_MESSAGE "XXX ok\n"

// Defining Failure messages
#define ERR_ERROR_MESSAGE "XXX Err\n"

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/

void print_to_outstream(char* message, FILE* fp, int sock_fd);


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/


char* run_command(InteractiveSpec_p interactive,
                 DStr_p jobname,
                  DStr_p input_axioms)
{

  Scanner_p job_scanner;
  ClauseSet_p cset;
  FormulaSet_p fset;
  char* message;
  char buffer[256];
  long         wct_limit=30;

  if(interactive->spec->per_prob_limit)
  {
    wct_limit = interactive->spec->per_prob_limit;
  }


  fprintf(stdout, "%s", DStrView(jobname));
  fflush(stdout);

  sprintf(buffer, "\n# Processing started for %s\n", DStrView(jobname));
  message = buffer;

  print_to_outstream(message, interactive->fp, interactive->sock_fd);

  job_scanner = CreateScanner(StreamTypeUserString, 
      DStrView(input_axioms),
      true, 
      NULL);
  ScannerSetFormat(job_scanner, TSTPFormat);
  cset = ClauseSetAlloc();
  fset = FormulaSetAlloc();
  FormulaAndClauseSetParse(job_scanner, cset, fset, interactive->ctrl->terms, 
      NULL, 
      &(interactive->ctrl->parsed_includes));

  // cset and fset are handed over to BatchProcessProblem and are
  // freed there (via StructFOFSpecBacktrackToSpec()).
  (void)BatchProcessProblem(interactive->spec, 
      wct_limit,
      interactive->ctrl,
      DStrView(jobname),
      cset,
      fset,
      interactive->fp,
      interactive->sock_fd);
  sprintf(buffer, "\n# Processing finished for %s\n\n", DStrView(jobname));
  message = buffer;
  print_to_outstream(message, interactive->fp, interactive->sock_fd);

  DestroyScanner(job_scanner);
  return OK_SUCCESS_MESSAGE;
}


char* add_command(InteractiveSpec_p interactive,
                 DStr_p axiomsname,
                  DStr_p input_axioms)
{

  Scanner_p axioms_scanner;
  ClauseSet_p cset;
  FormulaSet_p fset;
  AxiomSet_p axiom_set;
  PStackPointer i;
  AxiomSet_p    handle;

  axioms_scanner = CreateScanner(StreamTypeUserString, 
      DStrView(input_axioms),
      true, 
      NULL);
  ScannerSetFormat(axioms_scanner, TSTPFormat);
  cset = ClauseSetAlloc();
  fset = FormulaSetAlloc();
  FormulaAndClauseSetParse(axioms_scanner, cset, fset, interactive->ctrl->terms, 
      NULL, 
      &(interactive->ctrl->parsed_includes));
  DestroyScanner(axioms_scanner);
  DStrAppendDStr(cset->identifier, axiomsname);
  DStrAppendDStr(fset->identifier, axiomsname);

  axiom_set = AxiomSetAlloc(cset, fset, 0);

  int name_taken = 0;
  for(i=0; i<PStackGetSP(interactive->axiom_sets); i++)
  {
    handle = PStackElementP(interactive->axiom_sets, i);
    if(strcmp( DStrView(axiomsname), DStrView(handle->cset->identifier)) == 0 )
    {
      name_taken = 1;
      break;
    }
  }
  if(!name_taken)
  {
    PStackPushP(interactive->axiom_sets, axiom_set);
    return OK_SUCCESS_MESSAGE;
  }
  else
  {
    AxiomSetFree(axiom_set);
    ClauseSetFree(cset);
    FormulaSetFree(fset);
    return ERR_ERROR_MESSAGE;
  }
}

char* stage_command(InteractiveSpec_p interactive, DStr_p axiom_set)
{
  PStackPointer i;
  AxiomSet_p    handle;

  for(i=0; i<PStackGetSP(interactive->axiom_sets); i++)
  {
    handle = PStackElementP(interactive->axiom_sets, i);
    if(strcmp( DStrView(axiom_set), DStrView(handle->cset->identifier)) == 0 )
    {
      if( handle->staged )
      {
        return ERR_ERROR_MESSAGE;
      }
      else
      {
        PStackPushP(interactive->ctrl->clause_sets, handle->cset);
        PStackPushP(interactive->ctrl->formula_sets, handle->fset);
        StructFOFSpecAddProblem(interactive->ctrl, handle->cset, handle->fset);
        handle->staged = 1;
        interactive->ctrl->shared_ax_sp = PStackGetSP(interactive->ctrl->clause_sets);
        return OK_SUCCESS_MESSAGE;
      }
    }
  }
  return ERR_ERROR_MESSAGE;
}


char* list_command(InteractiveSpec_p interactive)
{
  PStackPointer i;
  AxiomSet_p    handle;
  PStack_p staged, unstaged;
  char buffer[256];

  staged = PStackAlloc();
  unstaged = PStackAlloc();

  for(i=0; i<PStackGetSP(interactive->axiom_sets); i++)
  {
    handle = PStackElementP(interactive->axiom_sets, i);
    if(handle->staged)
    {
      PStackPushP(staged, handle);
    }
    else
    {
      PStackPushP(unstaged, handle);
    }
  }
  
  if( PStackGetSP(staged) > 0 )
  {
    print_to_outstream("Staged :\n", interactive->fp, interactive->sock_fd);
    for(i=0; i<PStackGetSP(staged); i++){
      handle = PStackElementP(interactive->axiom_sets, i);
      sprintf(buffer, "  %s\n", DStrView(handle->cset->identifier));
      print_to_outstream(buffer, interactive->fp, interactive->sock_fd);
    }
  }

  if( PStackGetSP(unstaged) > 0 )
  {
    print_to_outstream("Unstaged :\n", interactive->fp, interactive->sock_fd);
    for(i=0; i<PStackGetSP(unstaged); i++){
      handle = PStackElementP(interactive->axiom_sets, i);
      sprintf(buffer, "  %s\n", DStrView(handle->cset->identifier));
      print_to_outstream(buffer, interactive->fp, interactive->sock_fd);
    }
  }

  if( PStackGetSP(staged) == 0 && PStackGetSP(unstaged) == 0 )
  {
    print_to_outstream("No Axiom Sets currently in memory.\n", interactive->fp, interactive->sock_fd);
  }

  return OK_SUCCESS_MESSAGE;
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/



/*-----------------------------------------------------------------------
//
// Function: InteractiveSpecAlloc()
//
//   Allocate an initialized interactive spec structure.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

InteractiveSpec_p InteractiveSpecAlloc(BatchSpec_p spec,
                                       StructFOFSpec_p ctrl,
                                       FILE* fp,
                                       int sock_fd)
{
   InteractiveSpec_p handle = InteractiveSpecCellAlloc();
   handle->spec = spec;
   handle->ctrl = ctrl;
   handle->fp = fp;
   handle->sock_fd = sock_fd;
   handle->axiom_sets = PStackAlloc();
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: InteractiveSpecFree()
//
//   Free an interactive spec structure. The BatchSpec struct and StructFOFSpec are not freed.
//
// Global Variables: -
//
// Side Effects    : Memory management
//
/----------------------------------------------------------------------*/

void InteractiveSpecFree(InteractiveSpec_p spec)
{
  PStackPointer i;
  AxiomSet_p   handle;

  for(i=0; i<PStackGetSP(spec->axiom_sets); i++)
  {
    handle = PStackElementP(spec->axiom_sets, i);
    AxiomSetFree(handle);
  }
  PStackFree(spec->axiom_sets);
  InteractiveSpecCellFree(spec);
}


/*-----------------------------------------------------------------------
//
// Function: AxiomSetAlloc()
//
//   Allocate an initialized axiom set structure.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

AxiomSet_p AxiomSetAlloc(ClauseSet_p cset,
                         FormulaSet_p fset,
                         int staged)
{
   AxiomSet_p handle = AxiomSetCellAlloc();
   handle->cset = cset;
   handle->fset = fset;
   handle->staged = 0;
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: AxiomSetFree()
//
//   Free an interactive spec structure. The BatchSpec struct and StructFOFSpec are not freed.
//
// Global Variables: -
//
// Side Effects    : Memory management
//
/----------------------------------------------------------------------*/

void AxiomSetFree(AxiomSet_p axiomset)
{
  ClauseSetFree(axiomset->cset);
  FormulaSetFree(axiomset->fset);
  AxiomSetCellFree(axiomset);
}


/*-----------------------------------------------------------------------
//
// Function: BatchProcessInteractive()
//
//   Perform interactive processing of problems relating to the batch
//   processing spec in spec and the axiom sets stored in ctrl.
//
// Global Variables: -
//
// Side Effects    : I/O, blocks on reading fp, initiates processing.
//
/----------------------------------------------------------------------*/

void BatchProcessInteractive(BatchSpec_p spec, 
                             StructFOFSpec_p ctrl, 
                             FILE* fp, 
                             int port)
{
   DStr_p input   = DStrAlloc();
   DStr_p dummyStr = DStrAlloc();
   InteractiveSpec_p interactive;
   bool done = false;
   Scanner_p in;

   int oldsock,sock_fd;

   char* dummy;
   DStr_p input_command = DStrAlloc();

   if(port != -1)
   {
     struct sockaddr cli_addr;
     socklen_t       cli_len;
     oldsock = CreateServerSock(port);
     Listen(oldsock);
     sock_fd = accept(oldsock, &cli_addr, &cli_len);
     if (sock_fd < 0)
     {
       SysError("Error on accepting connection", SYS_ERROR);
     }
     fp = NULL;
   }else{
      sock_fd = -1;
      oldsock = -1;
   }

  interactive = InteractiveSpecAlloc(spec, ctrl, fp, sock_fd);

   while(!done)
   {
      DStrReset(input);
      DStrReset(input_command);
      /*message = "";*/
      /*print_to_outstream(message, fp, sock_fd);*/
      if( sock_fd != -1)
      {
        dummy = TCPStringRecvX(sock_fd);
        DStrAppendBuffer(input, dummy, strlen(dummy));
      }
      else
      {
        print_to_outstream("Not implemented yet for stdout", fp, sock_fd);
        break;
      }

      in = CreateScanner(StreamTypeUserString, 
                         DStrView(input),
                         true, 
                         NULL);
      ScannerSetFormat(in, TSTPFormat);

      if(TestInpId(in, STAGE_COMMAND))
      {
        AcceptInpId(in, STAGE_COMMAND);
        DStrReset(dummyStr);
        DStrAppendDStr(dummyStr, AktToken(in)->literal);
        AcceptInpTok(in, Identifier);
        print_to_outstream(stage_command(interactive, dummyStr), fp, sock_fd);
      }
      else if(TestInpId(in, UNSTAGE_COMMAND))
      {
        AcceptInpId(in, UNSTAGE_COMMAND);
        dummy = "Should unstage : ";
        DStrAppendBuffer(input_command, dummy, strlen(dummy));
        DStrAppendDStr(input_command, AktToken(in)->literal);
        dummy = "\n";
        DStrAppendBuffer(input_command, dummy, strlen(dummy));
        print_to_outstream(DStrView(input_command), fp, sock_fd);
        AcceptInpTok(in, Identifier);
        print_to_outstream(OK_SUCCESS_MESSAGE, fp, sock_fd);
      }
      else if(TestInpId(in, REMOVE_COMMAND))
      {
        AcceptInpId(in, REMOVE_COMMAND);
        dummy = "Should remove : ";
        DStrAppendBuffer(input_command, dummy, strlen(dummy));
        DStrAppendDStr(input_command, AktToken(in)->literal);
        dummy = "\n";
        DStrAppendBuffer(input_command, dummy, strlen(dummy));
        print_to_outstream(DStrView(input_command), fp, sock_fd);
        AcceptInpTok(in, Identifier);
        print_to_outstream(OK_SUCCESS_MESSAGE, fp, sock_fd);
      }
      else if(TestInpId(in, DOWNLOAD_COMMAND))
      {
        AcceptInpId(in, DOWNLOAD_COMMAND);
        dummy = "Should download : ";
        DStrAppendBuffer(input_command, dummy, strlen(dummy));
        DStrAppendDStr(input_command, AktToken(in)->literal);
        dummy = "\n";
        DStrAppendBuffer(input_command, dummy, strlen(dummy));
        print_to_outstream(DStrView(input_command), fp, sock_fd);
        AcceptInpTok(in, Identifier);
        print_to_outstream(OK_SUCCESS_MESSAGE, fp, sock_fd);
      }
      else if(TestInpId(in, ADD_COMMAND))
      {
        AcceptInpId(in, ADD_COMMAND);
        DStrReset(dummyStr);
        DStrAppendDStr(dummyStr, AktToken(in)->literal);
        AcceptInpTok(in, Identifier);
        DStrReset(input);
        if(sock_fd != -1)
        {
          TCPReadTextBlock(input, sock_fd, END_OF_BLOCK_TOKEN);
        }
        else
        {
          ReadTextBlock(input, stdin, END_OF_BLOCK_TOKEN);
        }

        print_to_outstream(add_command(interactive, dummyStr, input), fp, sock_fd);
      }
      else if(TestInpId(in, RUN_COMMAND))
      {
        AcceptInpId(in, RUN_COMMAND);
        DStrReset(dummyStr);
        DStrAppendDStr(dummyStr, AktToken(in)->literal);
        AcceptInpTok(in, Identifier);
        DStrReset(input);
        if(sock_fd != -1)
        {
          TCPReadTextBlock(input, sock_fd, END_OF_BLOCK_TOKEN);
        }
        else
        {
          ReadTextBlock(input, stdin, END_OF_BLOCK_TOKEN);
        }
        print_to_outstream(run_command(interactive, dummyStr, input), fp, sock_fd);
      }
      else if(TestInpId(in, LIST_COMMAND))
      {
        AcceptInpId(in, LIST_COMMAND);
        print_to_outstream(list_command(interactive), fp, sock_fd);
      }
      else if(TestInpId(in, HELP_COMMAND))
      {
        AcceptInpId(in, HELP_COMMAND);
        print_to_outstream(help_message, fp, sock_fd);
        print_to_outstream(OK_SUCCESS_MESSAGE, fp, sock_fd);
      }
      else if(TestInpId(in, QUIT_COMMAND))
      {
        AcceptInpId(in, QUIT_COMMAND);
        done = true;
      }
      else
      {
        print_to_outstream(ERR_ERROR_MESSAGE, fp, sock_fd);
      }
      DestroyScanner(in);
   }
   DStrFree(dummyStr);
   DStrFree(input);
   DStrFree(input_command);
   InteractiveSpecFree(interactive);
   if( sock_fd != -1 )
   {
     shutdown(sock_fd,SHUT_RDWR);
     shutdown(oldsock,SHUT_RDWR);
     close(sock_fd);
     close(oldsock);
   }
}


void print_to_outstream(char* message, FILE* fp, int sock_fd){
  if(sock_fd != -1)
  {
    TCPStringSendX(sock_fd, message);
  }
  else
  {
    fprintf(fp, "%s", message);
    fflush(fp);
  }
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/

