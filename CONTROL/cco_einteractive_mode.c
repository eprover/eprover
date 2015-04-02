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
#define END_OF_BLOCK_TOCKEN "GO"

// Defining Success messages

// Defining Failure messages

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/

void print_to_outstream(char* message, FILE* fp, int sock_fd);


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/



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
   DStr_p jobname = DStrAlloc();
   bool done = false;
   Scanner_p in;
   ClauseSet_p cset;
   FormulaSet_p fset;
   long         wct_limit=30;

   int oldsock,sock_fd;

   char* message;
   char* input_command;
   char buffer[256];

   if(spec->per_prob_limit)
   {
      wct_limit = spec->per_prob_limit;
   }


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
   }

   while(!done)
   {
      DStrReset(input);
      message = "# Enter job, 'help' or 'quit', followed by 'go.' on a line of its own:\n";
      print_to_outstream(message, fp, sock_fd);


      // TODO : Refactor this to read one command at a time
      if(sock_fd != -1)
      {
        TCPReadTextBlock(input, sock_fd, "go.\n");
      }
      else
      {
        ReadTextBlock(input, stdin, "go.\n");
      }
      // END TODO

      in = CreateScanner(StreamTypeUserString, 
                         DStrView(input),
                         true, 
                         NULL);
      ScannerSetFormat(in, TSTPFormat);

      if(TestInpId(in, STAGE_COMMAND))
      {
      }
      else if(TestInpId(in, UNSTAGE_COMMAND))
      {
      }
      else if(TestInpId(in, REMOVE_COMMAND))
      {
      }
      else if(TestInpId(in, DOWNLOAD_COMMAND))
      {
      }
      else if(TestInpId(in, ADD_COMMAND))
      {
      }
      else if(TestInpId(in, RUN_COMMAND))
      {
      }
      else if(TestInpId(in, LIST_COMMAND))
      {
      }
      else if(TestInpId(in, HELP_COMMAND))
      {
        print_to_outstream(help_message, fp, sock_fd);
      }
      else if(TestInpId(in, QUIT_COMMAND))
      {
         done = true;
      }
      else
      {
         DStrReset(jobname);
         if(TestInpId(in, "job"))
         {
            AcceptInpId(in, "job");
            DStrAppendDStr(jobname, AktToken(in)->literal);
            AcceptInpTok(in, Identifier);
            AcceptInpTok(in, Fullstop);
         }
         else
         {
            DStrAppendStr(jobname, "unnamed_job");            
         }

         fprintf(stdout, "%s", DStrView(jobname));
         fflush(stdout);

         sprintf(buffer, "\n# Processing started for %s\n", DStrView(jobname));
         message = buffer;

         print_to_outstream(message, fp, sock_fd);


         cset = ClauseSetAlloc();
         fset = FormulaSetAlloc();
         FormulaAndClauseSetParse(in, cset, fset, ctrl->terms, 
                                  NULL, 
                                  &(ctrl->parsed_includes));

         // cset and fset are handed over to BatchProcessProblem and are
         // freed there (via StructFOFSpecBacktrackToSpec()).
         (void)BatchProcessProblem(spec, 
                                   wct_limit,
                                   ctrl,
                                   DStrView(jobname),
                                   cset,
                                   fset,
                                   fp,
                                   sock_fd);
         sprintf(buffer, "\n# Processing finished for %s\n\n", DStrView(jobname));
         message = buffer;
         print_to_outstream(message, fp, sock_fd);
      }
      DestroyScanner(in);
   }
   DStrFree(jobname);
   DStrFree(input);
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

