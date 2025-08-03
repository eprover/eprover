/*-----------------------------------------------------------------------

  File  : cco_einteractive_mode.c

  Author: Stephan Schulz (schulz@eprover.org), Mohamed Bassem Hasona

  Contents

  Code for parsing and handling the server's interactive mode.

  Copyright 2015, 2017 by the author.

  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Lost in the mist of time!

  -----------------------------------------------------------------------*/

#include "cco_einteractive_mode.h"


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

char* help_message = "\
"COMCHAR" Note : Block commands that are of the form of \"COMMAND <NAME> ... GO\"\n\
"COMCHAR" should have the \"COMMAND <NAME>\" and GO each on a separate line of\n\
"COMCHAR" their own. The block should be in between these two.\n\
"COMCHAR"\n\
"COMCHAR"- ADD <NAME> ... GO : Uploads a new axiom set with the name <NAME>.\n\
"COMCHAR"- LOAD <NAME>       : Loads a server-side axiom set with the name <NAME>. \n\
"COMCHAR"- STAGE <NAME>      : Stages the axiom set <NAME>.\n\
"COMCHAR"- UNSTAGE <NAME>    : Unstages the axiom set <NAME>.\n\
"COMCHAR"- REMOVE <NAME>     : Removes the axiom set <NAME> from the memory.\n\
"COMCHAR"- DOWNLOAD <NAME>   : Prints the axiom set <NAME>.\n\
"COMCHAR"- RUN <NAME> ... GO : Runs a job with the name <NAME>.\n\
"COMCHAR"- LIST              : Prints the status of the axiom sets.\n\
"COMCHAR"- HELP              : Prints the help message.\n\
"COMCHAR"- QUIT              : Closes the connection with the server.\n";


// Defining commands used
#define STAGE_COMMAND "STAGE"
#define UNSTAGE_COMMAND "UNSTAGE"
#define REMOVE_COMMAND "REMOVE"
#define DOWNLOAD_COMMAND "DOWNLOAD"
#define ADD_COMMAND "ADD"
#define LOAD_COMMAND "LOAD"
#define RUN_COMMAND "RUN"
#define LIST_COMMAND "LIST"
#define HELP_COMMAND "HELP"
#define QUIT_COMMAND "QUIT"
#define END_OF_BLOCK_TOKEN "GO\n"

// Defining Success messages
#define OK_SUCCESS_MESSAGE "200 ok : success\n"
#define OK_STAGED_MESSAGE "201 ok : staged\n"
#define OK_UNSTAGED_MESSAGE "202 ok : unstaged\n"
#define OK_REMOVED_MESSAGE "203 ok : removed\n"
#define OK_DOWNLOADED_MESSAGE "204 ok : downloaded\n"
#define OK_ADDED_MESSAGE "205 ok : added\n"
#define OK_LOADED_MESSAGE "206 ok : loaded\n"

// Defining Failure messages
#define ERR_ERROR_MESSAGE "499 Err : Something went wrong\n"
#define ERR_AXIOM_SET_NAME_TAKEN_MESSAGE "401 Err : axiom set name is taken\n"
#define ERR_SYNTAX_ERROR_MESSAGE "402 Err : syntax error\n"
#define ERR_AXIOM_SET_IS_STAGED_MESSAGE "403 Err : axiom set is staged, please unstage it first\n"
#define ERR_UNKNOWN_AXIOM_SET_MESSAGE "404 Err : unknown axiom set\n"
#define ERR_AXIOM_SET_IS_ALREADY_STAGED_MESSAGE "405 Err : axiom set is already staged\n"
#define ERR_AXIOM_SET_IS_ALREADY_UNSTAGED_MESSAGE "406 Err : axiom set is already unstaged\n"
#define ERR_UNKNOWN_COMMAND_MESSAGE "407 Err : unknown command\n"
#define ERR_NO_AXIOM_LIBRARY_ON_SERVER_MESSAGE "408 Err : no axioms library on server\n"
#define ERR_CANNOT_READ_SERVER_LIBRARY_MESSAGE "409 Err : cannot read server library\n"

#define AXIOM_SET_NAME_TOKENS String|Name|PosInt|Fullstop|Plus|Hyphen

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/

void print_to_outstream(char* message, FILE* fp, int sock_fd);
PStack_p get_directory_listings(DStr_p dir);
void AcceptAxiomSetName(Scanner_p in, DStr_p dest);
char* run_command(InteractiveSpec_p interactive,
                  DStr_p jobname,
                  DStr_p input_axioms);
char* add_command(InteractiveSpec_p interactive,
                  DStr_p axiomsname,
                  DStr_p input_axioms);
char* stage_command(InteractiveSpec_p interactive, DStr_p axiom_set);
char* list_command(InteractiveSpec_p interactive);
void  quit_command(InteractiveSpec_p interactive);
char* remove_command(InteractiveSpec_p interactive, DStr_p axiom_set);
char* download_command(InteractiveSpec_p interactive, DStr_p axiom_set);
char* unstage_command(InteractiveSpec_p interactive, DStr_p axiom_set);
char* load_command(InteractiveSpec_p interactive, DStr_p filename);

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function:
//
//
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

char* run_command(InteractiveSpec_p interactive,
                  DStr_p jobname,
                  DStr_p input_axioms)
{

   Scanner_p job_scanner;
   ClauseSet_p dummy;
   FormulaSet_p fset;
   char* message;
   char buffer[256];
   long wct_limit=30;
   int pid;

   if ((pid = fork()) == -1)
   {
      return ERR_ERROR_MESSAGE;
   }
   else if(pid > 0)
   {
      wait(NULL);
   }
   else if(pid == 0)
   {
      if(interactive->spec->per_prob_limit)
      {
         wct_limit = interactive->spec->per_prob_limit;
      }


      fprintf(stdout, "%s", DStrView(jobname));
      fflush(stdout);

      sprintf(buffer, "\n"COMCHAR" Processing started for %s\n", DStrView(jobname));
      message = buffer;

      print_to_outstream(message, interactive->fp, interactive->sock_fd);

      job_scanner = CreateScanner(StreamTypeUserString,
                                  DStrView(input_axioms),
                                  true,
                                  NULL, true);
      ScannerSetFormat(job_scanner, TSTPFormat);
      dummy = ClauseSetAlloc();
      fset = FormulaSetAlloc();
      FormulaAndClauseSetParse(job_scanner, fset, dummy, interactive->ctrl->terms,
                               NULL,
                               &(interactive->ctrl->parsed_includes));

      // cset and fset are handed over to BatchProcessProblem and are
      // freed there (via StructFOFSpecBacktrackToSpec()).
      (void)BatchProcessProblem(interactive->spec,
                                wct_limit,
                                interactive->ctrl,
                                DStrView(jobname),
                                dummy,
                                fset,
                                interactive->fp,
                                interactive->sock_fd,
                                true);
      sprintf(buffer, "\n"COMCHAR" Processing finished for %s\n\n", DStrView(jobname));
      message = buffer;
      print_to_outstream(message, interactive->fp, interactive->sock_fd);

      DestroyScanner(job_scanner);
      exit(0);
   }
   return OK_SUCCESS_MESSAGE;
}

/*-----------------------------------------------------------------------
//
// Function:
//
//
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

char* add_command(InteractiveSpec_p interactive,
                  DStr_p axiomsname,
                  DStr_p input_axioms)
{

   Scanner_p axioms_scanner;
   ClauseSet_p dummy;
   FormulaSet_p fset;
   AxiomSet_p axiom_set;
   PStackPointer i;
   AxiomSet_p    handle;
   char *file;
   FILE *fp;

   file = TempFileName();
   fp   = SecureFOpen(file, "w");
   fprintf(fp, "%s", DStrView(input_axioms));
   SecureFClose(fp);

   axioms_scanner = CreateScanner(StreamTypeFile,
                                  file,
                                  true,
                                  NULL, true);
   ScannerSetFormat(axioms_scanner, TSTPFormat);
   dummy = ClauseSetAlloc();
   fset = FormulaSetAlloc();
   FormulaAndClauseSetParse(axioms_scanner, fset, dummy, interactive->ctrl->terms,
                            NULL,
                            &(interactive->ctrl->parsed_includes));
   DestroyScanner(axioms_scanner);
   DStrAppendDStr(dummy->identifier, axiomsname);
   DStrAppendDStr(fset->identifier, axiomsname);

   axiom_set = AxiomSetAlloc(dummy, fset, input_axioms, 0);

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
      return OK_ADDED_MESSAGE;
   }
   else
   {
      AxiomSetFree(axiom_set);
      return ERR_AXIOM_SET_NAME_TAKEN_MESSAGE;
   }
}

/*-----------------------------------------------------------------------
//
// Function:
//
//
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

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
            return ERR_AXIOM_SET_IS_ALREADY_STAGED_MESSAGE;
         }
         else
         {
            StructFOFSpecAddProblem(interactive->ctrl, handle->cset, handle->fset, false);
            handle->staged = 1;
            interactive->ctrl->shared_ax_sp = PStackGetSP(interactive->ctrl->clause_sets);
            return OK_STAGED_MESSAGE;
         }
      }
   }
   return ERR_UNKNOWN_AXIOM_SET_MESSAGE;
}


/*-----------------------------------------------------------------------
//
// Function:
//
//
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

char* list_command(InteractiveSpec_p interactive)
{
   PStackPointer i;
   AxiomSet_p    handle;
   PStack_p staged, unstaged;
   char buffer[256];
   PStack_p files;
   DStr_p dummy;

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
         handle = PStackElementP(staged, i);
         sprintf(buffer, "  %s\n", DStrView(handle->cset->identifier));
         print_to_outstream(buffer, interactive->fp, interactive->sock_fd);
      }
   }

   if( PStackGetSP(unstaged) > 0 )
   {
      print_to_outstream("Unstaged :\n", interactive->fp, interactive->sock_fd);
      for(i=0; i<PStackGetSP(unstaged); i++){
         handle = PStackElementP(unstaged, i);
         sprintf(buffer, "  %s\n", DStrView(handle->cset->identifier));
         print_to_outstream(buffer, interactive->fp, interactive->sock_fd);
      }
   }

   if( PStackGetSP(staged) == 0 && PStackGetSP(unstaged) == 0 )
   {
      print_to_outstream("No Axiom Sets currently in memory.\n",
                         interactive->fp, interactive->sock_fd);
   }
   PStackFree(staged);
   PStackFree(unstaged);

   print_to_outstream("On Disk :\n", interactive->fp, interactive->sock_fd);
   if(DStrLen(interactive->server_lib))
   {
      files = get_directory_listings(interactive->server_lib);
      if (files == NULL)
      {
         print_to_outstream("\tCould not open current directory.\n",
                            interactive->fp, interactive->sock_fd);
      }
      else
      {
         while(!PStackEmpty(files))
         {
            dummy = PStackPopP(files);
            sprintf(buffer, "\t%s\n", DStrView(dummy));
            print_to_outstream(buffer, interactive->fp, interactive->sock_fd);
            DStrFree(dummy);
         }
         PStackFree(files);
      }
   }
   else
   {
      print_to_outstream("\tNo axioms directory was specified on server startup.\n",
                         interactive->fp, interactive->sock_fd);
   }
   return OK_SUCCESS_MESSAGE;
}

/*-----------------------------------------------------------------------
//
// Function:
//
//
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void quit_command(InteractiveSpec_p interactive)
{
   PStack_p spare_stack;
   AxiomSet_p    axiom_set_handle;
   PStackPointer i;
   DStr_p dummy;

   spare_stack = PStackAlloc();
   for(i=0; i < PStackGetSP(interactive->axiom_sets); i++)
   {
      axiom_set_handle = PStackElementP(interactive->axiom_sets, i);
      if( axiom_set_handle->staged )
      {
         PStackPushP(spare_stack, axiom_set_handle->cset->identifier);
      }
   }

   while(!PStackEmpty(spare_stack))
   {
      dummy = PStackPopP(spare_stack);
      unstage_command(interactive, dummy);
   }
   PStackFree(spare_stack);
}

/*-----------------------------------------------------------------------
//
// Function:
//
//
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

char* remove_command(InteractiveSpec_p interactive, DStr_p axiom_set)
{
   AxiomSet_p    handle;
   PStack_p spare_stack;
   spare_stack = PStackAlloc();
   handle = NULL;
   int found = 0;

   while(!PStackEmpty(interactive->axiom_sets))
   {
      handle = PStackPopP(interactive->axiom_sets);
      if(strcmp( DStrView(axiom_set), DStrView(handle->cset->identifier)) == 0 )
      {
         if( handle->staged )
         {
            return ERR_AXIOM_SET_IS_STAGED_MESSAGE;
         }
         else
         {
            found = 1;
            AxiomSetFree(handle);
            break;
         }
      }
      else
      {
         PStackPushP(spare_stack, handle);
      }
   }
   while(!PStackEmpty(spare_stack))
   {
      handle = PStackPopP(spare_stack);
      PStackPushP(interactive->axiom_sets, handle);
   }
   PStackFree(spare_stack);
   if( !found )
   {
      return ERR_UNKNOWN_AXIOM_SET_MESSAGE;
   }
   else
   {
      return OK_REMOVED_MESSAGE;
   }
}


/*-----------------------------------------------------------------------
//
// Function:
//
//
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

char* download_command(InteractiveSpec_p interactive, DStr_p axiom_set)
{
   PStackPointer i;
   AxiomSet_p    handle;

   for(i=0; i < PStackGetSP(interactive->axiom_sets); i++)
   {
      handle = PStackElementP(interactive->axiom_sets, i);
      if(strcmp( DStrView(axiom_set), DStrView(handle->cset->identifier)) == 0 )
      {
         print_to_outstream(DStrView(handle->raw_data),
                            interactive->fp, interactive->sock_fd);
         return OK_DOWNLOADED_MESSAGE;
      }
   }

   // Axiom Set Not Found
   return ERR_UNKNOWN_AXIOM_SET_MESSAGE;
}

/*-----------------------------------------------------------------------
//
// Function:
//
//
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

char* unstage_command(InteractiveSpec_p interactive, DStr_p axiom_set)
{
   PStackPointer i;
   AxiomSet_p    axiom_set_handle;
   PStack_p cspare_stack, fspare_stack;
   fspare_stack = PStackAlloc();
   cspare_stack = PStackAlloc();
   FormulaSet_p fhandle;
   ClauseSet_p chandle;
   int found = 0;

   for(i=0; i < PStackGetSP(interactive->axiom_sets); i++)
   {
      axiom_set_handle = PStackElementP(interactive->axiom_sets, i);
      if(strcmp( DStrView(axiom_set), DStrView(axiom_set_handle->cset->identifier)) == 0 )
      {
         if( !axiom_set_handle->staged )
         {
            return ERR_AXIOM_SET_IS_ALREADY_UNSTAGED_MESSAGE;
         }
         else
         {
            axiom_set_handle->staged = 0;
            found = 1;
         }
      }
   }

   if( !found )
   {
      return ERR_UNKNOWN_AXIOM_SET_MESSAGE;
   }

   assert( PStackGetSP(interactive->ctrl->clause_sets) ==
           PStackGetSP(interactive->ctrl->formula_sets) );
   found = 0;

   while(!PStackEmpty(interactive->ctrl->clause_sets))
   {
      chandle = PStackPopP(interactive->ctrl->clause_sets);
      fhandle = PStackPopP(interactive->ctrl->formula_sets);
      assert( strcmp( DStrView(chandle->identifier), DStrView(fhandle->identifier)) == 0 );

      if(strcmp( DStrView(axiom_set), DStrView(chandle->identifier)) == 0 )
      {
         GenDistribAddFormulaSet(interactive->ctrl->f_distrib, fhandle, false, -1);
         GenDistribAddClauseSet(interactive->ctrl->f_distrib, chandle, -1);
         found = 1;
         break;
      }
      else
      {
         PStackPushP(cspare_stack, chandle);
         PStackPushP(fspare_stack, fhandle);
      }
   }

   while(!PStackEmpty(fspare_stack))
   {
      fhandle = PStackPopP(fspare_stack);
      chandle = PStackPopP(cspare_stack);
      PStackPushP(interactive->ctrl->formula_sets, fhandle);
      PStackPushP(interactive->ctrl->clause_sets, chandle);
   }
   interactive->ctrl->shared_ax_sp = PStackGetSP(interactive->ctrl->clause_sets);
   PStackFree(fspare_stack);
   PStackFree(cspare_stack);

   if( !found )
   {
      return ERR_UNKNOWN_AXIOM_SET_MESSAGE;
   }
   else
   {
      return OK_UNSTAGED_MESSAGE;
   }
}

/*-----------------------------------------------------------------------
//
// Function:
//
//
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

char* load_command(InteractiveSpec_p interactive, DStr_p filename)
{
   PStack_p files;
   DStr_p handle, file_content;
   char *ret;
   int found;

   if(DStrLen(interactive->server_lib))
   {
      found = 0;
      files = get_directory_listings(interactive->server_lib);
      if(files == NULL)
      {
         return ERR_CANNOT_READ_SERVER_LIBRARY_MESSAGE;
      }
      else
      {
         while(!PStackEmpty(files))
         {
            handle = PStackPopP(files);
            if(strcmp(DStrView(handle), DStrView(filename)) == 0)
            {
               found = 1;
            }
            DStrFree(handle);
         }
         PStackFree(files);
         if(found)
         {
            handle = DStrAlloc();
            file_content = DStrAlloc();
            DStrAppendDStr(handle, interactive->server_lib);
            DStrAppendStr(handle, "/");
            DStrAppendDStr(handle, filename);
            FileLoad(DStrView(handle), file_content);
            DStrFree(handle);
            ret = add_command(interactive, filename, file_content);
            if( strcmp(ret, OK_ADDED_MESSAGE) == 0 )
            {
               ret = OK_LOADED_MESSAGE;
            }
            DStrFree(file_content);
            return ret;
         }
         else
         {
            return ERR_UNKNOWN_AXIOM_SET_MESSAGE;
         }
      }

   }
   else
   {
      return ERR_NO_AXIOM_LIBRARY_ON_SERVER_MESSAGE;
   }
}


/*-----------------------------------------------------------------------
//
// Function:
//
//
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void print_to_outstream(char* message, FILE* fp, int sock_fd)
{
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

/*-----------------------------------------------------------------------
//
// Function: get_directory_listings()
//
//    Open a directory and return a newly created stack of freshly
//    allocated DStrs containing the names of regular files in the
//    directory.
//
// Global Variables: -
//
// Side Effects    : I/O
//
/----------------------------------------------------------------------*/


PStack_p get_directory_listings(DStr_p dirname)
{
   PStack_p files;
   struct dirent *de;
   DStr_p file_name;
   DIR *dir;
#ifdef __sun
   char path[PATH_MAX];
   struct stat buf;
   int len;

   if (strlcpy(path, DStrView(dirname), sizeof(path)) < sizeof(path)
       && strlcat(path, "/", sizeof(path)) < sizeof(path))
   {
      len = strlen(path);
   }
   else
   {
      return NULL;
   }
#endif

   files = PStackAlloc();

   dir = opendir(DStrView(dirname));
   if (dir == NULL)
   {
      return NULL;
   }
   else
   {
      while ((de = readdir(dir)) != NULL)
      {
         if( strcmp(de->d_name,".") == 0 || strcmp(de->d_name,"..") == 0)
         {
            continue;
         }
#ifdef __sun
         path[len] = '\0';
         if (strlcat(path, de->d_name, sizeof(path)) >= sizeof(path))
         {
            continue;
         }
         if (stat(path, &buf) != 0 || !S_ISREG(buf.st_mode))
         {
            continue;
         }
#else
         if (de->d_type != DT_REG)
         {
            continue;
         }
#endif
         file_name = DStrAlloc();
         DStrSet(file_name, de->d_name);
         PStackPushP(files, file_name);
      }
      closedir(dir);
   }
   return files;
}



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function:
//
//
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void AcceptAxiomSetName(Scanner_p in, DStr_p dest){
   while(TestInpTok(in, AXIOM_SET_NAME_TOKENS))
   {
      DStrAppendDStr(dest, AktToken(in)->literal);
      NextToken(in);
   }
}


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
   handle->server_lib = DStrAlloc();
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
   DStrFree(spec->server_lib);
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
                         DStr_p raw_data,
                         int staged)
{
   AxiomSet_p handle = AxiomSetCellAlloc();
   handle->cset = cset;
   handle->fset = fset;
   handle->staged = 0;
   handle->raw_data = DStrAlloc();
   DStrAppendDStr(handle->raw_data, raw_data);
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

void AxiomSetFree(AxiomSet_p axiom_set)
{
   ClauseSetFree(axiom_set->cset);
   FormulaSetFree(axiom_set->fset);
   DStrFree(axiom_set->raw_data);
   AxiomSetCellFree(axiom_set);
}


/*-----------------------------------------------------------------------
//
// Function: StartDeductionServer()
//
//   Run the deduction server on the specified socked. Read commands and
//   react to them.
//
// Global Variables: -
//
// Side Effects    : I/O, blocks on reading fp, initiates processing.
//
/----------------------------------------------------------------------*/

void StartDeductionServer(BatchSpec_p spec,
                          StructFOFSpec_p ctrl,
                          char* server_lib,
                          FILE* fp,
                          int sock_fd)
{
   DStr_p input   = DStrAlloc();
   DStr_p dummyStr = DStrAlloc();
   InteractiveSpec_p interactive;
   bool done = false;
   Scanner_p in;

   char* dummy;
   DStr_p input_command = DStrAlloc();

   interactive = InteractiveSpecAlloc(spec, ctrl, fp, sock_fd);
   if(server_lib)
   {
      DStrAppendStr(interactive->server_lib,server_lib);
   }

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
         FREE(dummy);
      }
      else
      {
         print_to_outstream("e_deduction_server: Server mode not implemented "
                            "yet for stdout\n", fp, sock_fd);
         break;
         //char line[256];
         //fgets(line, 254, stdin);
         //DStrAppendBuffer(input, line, strlen(line)-1);
      }

      in = CreateScanner(StreamTypeUserString,
                         DStrView(input),
                         true,
                         NULL, true);
      ScannerSetFormat(in, TSTPFormat);

      if(TestInpId(in, STAGE_COMMAND))
      {
         AcceptInpId(in, STAGE_COMMAND);
         DStrReset(dummyStr);
         AcceptAxiomSetName(in, dummyStr);
         print_to_outstream(stage_command(interactive, dummyStr), fp, sock_fd);
      }
      else if(TestInpId(in, UNSTAGE_COMMAND))
      {
         AcceptInpId(in, UNSTAGE_COMMAND);
         DStrReset(dummyStr);
         AcceptAxiomSetName(in, dummyStr);
         print_to_outstream(unstage_command(interactive, dummyStr), fp, sock_fd);
      }
      else if(TestInpId(in, REMOVE_COMMAND))
      {
         AcceptInpId(in, REMOVE_COMMAND);
         DStrReset(dummyStr);
         AcceptAxiomSetName(in, dummyStr);
         print_to_outstream(remove_command(interactive, dummyStr), fp, sock_fd);
      }
      else if(TestInpId(in, DOWNLOAD_COMMAND))
      {
         AcceptInpId(in, DOWNLOAD_COMMAND);
         DStrReset(dummyStr);
         AcceptAxiomSetName(in, dummyStr);
         print_to_outstream(download_command(interactive, dummyStr), fp, sock_fd);
      }
      else if(TestInpId(in, LOAD_COMMAND))
      {
         AcceptInpId(in, LOAD_COMMAND);
         DStrReset(dummyStr);
         AcceptAxiomSetName(in, dummyStr);
         print_to_outstream(load_command(interactive, dummyStr), fp, sock_fd);
      }
      else if(TestInpId(in, ADD_COMMAND))
      {
         AcceptInpId(in, ADD_COMMAND);
         DStrReset(dummyStr);
         AcceptAxiomSetName(in, dummyStr);
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
         quit_command(interactive);
         done = true;
      }
      else
      {
         print_to_outstream(ERR_UNKNOWN_COMMAND_MESSAGE, fp, sock_fd);
      }
      DestroyScanner(in);
   }
   DStrFree(dummyStr);
   DStrFree(input);
   DStrFree(input_command);
   InteractiveSpecFree(interactive);
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
