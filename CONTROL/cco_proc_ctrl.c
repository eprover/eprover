/*-----------------------------------------------------------------------

File  : cco_proc_ctrl.c

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Code for process control.

  Copyright 2010 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed Jul 14 15:54:29 BST 2010
    New

-----------------------------------------------------------------------*/

#include "cco_proc_ctrl.h"


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

char* PRResultTable[] =
{
   NULL,
   SZS_THEOREM_STR,
   SZS_UNSAT_STR,
   SZS_SATSTR_STR,
   SZS_COUNTERSAT_STR,
   SZS_FAILURE_STR,
   SZS_GAVEUP_STR
};


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: EPCtrlAlloc()
//
//   Allocate an initialized EPCtrlCell.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

EPCtrl_p EPCtrlAlloc(char *name)
{
   EPCtrl_p ctrl = EPCtrlCellAlloc();

   ctrl->pid        = 0;
   ctrl->pipe       = NULL;
   ctrl->input_file = 0;
   ctrl->name       = SecureStrdup(name);
   ctrl->start_time = 0;
   ctrl->prob_time  = 0;
   ctrl->result     = PRNoResult;
   ctrl->output     = DStrAlloc();

   return ctrl;
}


/*-----------------------------------------------------------------------
//
// Function: EPCtrlFree()
//
//   Free a EPCtrlCell.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void EPCtrlFree(EPCtrl_p junk)
{
   if(junk->input_file)
   {
      FREE(junk->input_file);
   }
   if(junk->name)
   {
      FREE(junk->name);
   }
   DStrFree(junk->output);
   EPCtrlCellFree(junk);
}





/*-----------------------------------------------------------------------
//
// Function: EPCtrlCleanup()
//
//   Clean up: Kill process, close pipe,
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void EPCtrlCleanup(EPCtrl_p ctrl, bool delete_file)
{
   if(ctrl->pid)
   {
      kill(ctrl->pid, SIGTERM);
      ctrl->pid = 0;
   }
   if(ctrl->pipe)
   {
      pclose(ctrl->pipe);
      ctrl->pipe = NULL;
   }
   if(delete_file && ctrl->input_file)
   {
      TempFileRemove(ctrl->input_file);
      FREE(ctrl->input_file);
      ctrl->input_file = NULL;
   }
}



/*-----------------------------------------------------------------------
//
// Function: ECtrlCreate()
//
//   Create a pipe running prover with time limit cpu_limit on
//   file. "prover" must conform to the calling conventions of E and
//   provide similar output. This takes over responsibility for the
//   string pointed to by file.
//
// Global Variables: -
//
// Side Effects    : Yes ;-)
//
/----------------------------------------------------------------------*/

EPCtrl_p ECtrlCreate(char* prover, char* name,
                     char* extra_options,
                     long cpu_limit, char* file)
{
   return ECtrlCreateGeneric(prover, name, E_OPTIONS, extra_options, cpu_limit, file);
}



/*-----------------------------------------------------------------------
//
// Function: ECtrlCreateGeneric()
//
//   Create a pipe running prover with time limit cpu_limit on
//   file. "prover" must conform to the calling conventions of E and
//   provide similar output. This takes over responsibility for the
//   string pointed to by file.
//
// Global Variables: -
//
// Side Effects    : Yes ;-)
//
/----------------------------------------------------------------------*/

EPCtrl_p ECtrlCreateGeneric(char* prover, char* name,
                            char* options, char* extra_options,
                            long cpu_limit, char* file)
{
   DStr_p   cmd = DStrAlloc();
   EPCtrl_p res;
   char     line[180];
   char*    ret;

   DStr_p procname = DStrAlloc();
   DStrAppendStr(procname, name);
   DStrAppendStr(procname, " => ");
   DStrAppendStr(procname, options);
   res = EPCtrlAlloc(DStrView(procname));
   DStrFree(procname);

   DStrAppendStr(cmd, prover);
   DStrAppendStr(cmd, E_OPTIONS_BASE);
   DStrAppendStr(cmd, options);
   DStrAppendStr(cmd, " ");
   DStrAppendStr(cmd, extra_options);
   DStrAppendStr(cmd, " --cpu-limit=");
   DStrAppendInt(cmd, cpu_limit);
   DStrAppendStr(cmd, " ");
   DStrAppendStr(cmd, file);
   //printf(COMCHAR" Command: %s\n", DStrView(cmd));

   res->prob_time  = cpu_limit;
   res->start_time = GetSecTime();
   res->input_file = file;
   //printf(COMCHAR" Executing: %s\n", DStrView(cmd));
   res->pipe = popen(DStrView(cmd), "r");
   if(!res->pipe)
   {
      TmpErrno = errno;
      SysError("Cannot start eprover subprocess", SYS_ERROR);
   }
   res->fileno = fileno(res->pipe);
   ret = fgets(line, 180, res->pipe);
   if(!ret || ferror(res->pipe))
   {
      Error("Cannot read eprover PID line", OTHER_ERROR);
   }
   // fprintf(GlobalOut, COMCHAR" Line = %s", line);
   if(!strstr(line, COMCHAR" Pid: "))
   {
      Error("Cannot get eprover PID", OTHER_ERROR);
   }
   res->pid = atoi(line+7);
   DStrAppendStr(res->output, line);

   DStrFree(cmd);
   //printf(COMCHAR" eprover subprocess started\n");
   return res;
}






/*-----------------------------------------------------------------------
//
// Function: EPCtrlGetResult()
//
//   Try to read a line from the E process. If successful, try to
//   extract a result state. Return true if the E process terminated
//   (i.e. the read returns 0), false otherwise.
//
// Global Variables: -
//
// Side Effects    : Reads input, may set result state in ctrl.
//
/----------------------------------------------------------------------*/

bool EPCtrlGetResult(EPCtrl_p ctrl, char* buffer, long buf_size)
{
   char* l;

   l=fgets(buffer, buf_size, ctrl->pipe);

   if(l)
   {
      DStrAppendStr(ctrl->output, l);

      if(strstr(buffer, SZS_THEOREM_STR))
      {
         ctrl->result = PRTheorem;
      }
      else if(strstr(buffer, SZS_CONTRAAX_STR))
      {
         ctrl->result = PRTheorem;
      }
      else if(strstr(buffer, SZS_UNSAT_STR))
      {
         ctrl->result = PRUnsatisfiable;
      }
      else if(strstr(buffer, SZS_SATSTR_STR))
      {
         ctrl->result = PRSatisfiable;
      }
      else if(strstr(buffer, SZS_COUNTERSAT_STR))
      {
         ctrl->result = PRCounterSatisfiable;
      }
      return false;
   }
   else
   {
      if(ctrl->result == PRNoResult)
      {
         ctrl->result = PRFailure;
      }
      return true;
   }
}


/*-----------------------------------------------------------------------
//
// Function: EPCtrlSetAlloc()
//
//   Allocate an empty EPCtrlCell.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

EPCtrlSet_p EPCtrlSetAlloc(void)
{
   EPCtrlSet_p handle = EPCtrlSetCellAlloc();

   handle->procs     = NULL;

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: EPCtrlSetFree()
//
//   Free an EPCtrlSet(), including the payload.Will clean up the
//   processes.
//
// Global Variables: -
//
// Side Effects    : Will terminate processes, memory, ...
//
/----------------------------------------------------------------------*/

void EPCtrlSetFree(EPCtrlSet_p junk, bool delete_files)
{
   NumTree_p cell;

   while(junk->procs)
   {
      cell = NumTreeExtractRoot(&(junk->procs));
      EPCtrlCleanup(cell->val1.p_val, delete_files);
      EPCtrlFree(cell->val1.p_val);
      NumTreeCellFree(cell);
   }
   EPCtrlSetCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: EPCtrlSetAddProc()
//
//   Add a process to the process set.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void EPCtrlSetAddProc(EPCtrlSet_p set, EPCtrl_p proc)
{
   IntOrP tmp;

   tmp.p_val = proc;
   NumTreeStore(&(set->procs), proc->fileno, tmp, tmp);
}


/*-----------------------------------------------------------------------
//
// Function: EPCtrlSetFindProc()
//
//   Find the process associated with fd.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

EPCtrl_p EPCtrlSetFindProc(EPCtrlSet_p set, int fd)
{
   NumTree_p cell;

   cell = NumTreeFind(&(set->procs), fd);

   if(cell)
   {
      return cell->val1.p_val;
   }
   return NULL;
}


/*-----------------------------------------------------------------------
//
// Function: EPCtrlSetDeleteProc()
//
//   Delete a process from the set.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void EPCtrlSetDeleteProc(EPCtrlSet_p set, EPCtrl_p proc, bool delete_file)
{
   NumTree_p cell;

   cell = NumTreeExtractEntry(&(set->procs), proc->fileno);
   if(cell)
   {
      EPCtrlCleanup(cell->val1.p_val, delete_file);
      EPCtrlFree(cell->val1.p_val);
      NumTreeCellFree(cell);
   }
}


/*-----------------------------------------------------------------------
//
// Function: EPCtrlSetFDSet()
//
//   Set all file descriptor bits of the set in the fd_set data
//   structure. Return the largest one.
//
// Global Variables: -
//
// Side Effects    : Memory (only temporary)
//
/----------------------------------------------------------------------*/

int EPCtrlSetFDSet(EPCtrlSet_p set, fd_set *rd_fds)
{
   PStack_p trav_stack;
   int maxfd = 0;
   EPCtrl_p handle;
   NumTree_p cell;

   trav_stack = NumTreeTraverseInit(set->procs);
   while((cell = NumTreeTraverseNext(trav_stack)))
   {
      handle = cell->val1.p_val;
      FD_SET(handle->fileno, rd_fds);
      maxfd = handle->fileno;
   }
   NumTreeTraverseExit(trav_stack);

   return maxfd;
}


/*-----------------------------------------------------------------------
//
// Function: EPCtrlSetGetResult()
//
//
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

EPCtrl_p EPCtrlSetGetResult(EPCtrlSet_p set, bool delete_files)
{
   bool eof;
   fd_set readfds, writefds, errorfds;
   int maxfd = 0,i;
   EPCtrl_p handle, res = NULL;
   struct timeval waittime;
   int sel_success;

   FD_ZERO(&readfds);
   FD_ZERO(&writefds);
   FD_ZERO(&errorfds);
   waittime.tv_sec  = 0;
   waittime.tv_usec = 500000;

   maxfd = EPCtrlSetFDSet(set, &readfds);

   sel_success = select(maxfd+1, &readfds, &writefds, &errorfds, &waittime);

   if(sel_success !=-1)
   {
      for(i=0; i<=maxfd; i++)
      {
         if(FD_ISSET(i, &readfds))
         {
            handle = EPCtrlSetFindProc(set, i);
            eof = EPCtrlGetResult(handle, set->buffer, EPCTRL_BUFSIZE);
            if(eof)
            {
               switch(handle->result)
               {
               case PRNoResult:
                     break;
               case PRTheorem:
               case PRUnsatisfiable:
                     res = handle;
                     break;
               case PRSatisfiable:
               case PRCounterSatisfiable:
               case PRFailure:
                     /* Process terminates, but no proof found -> Remove it*/
                     fprintf(GlobalOut, COMCHAR" No proof found by %s\n",
                             handle->name);

                     EPCtrlSetDeleteProc(set, handle, delete_files);
                     break;
               default:
                     assert(false && "Impossible ProverResult");
               }
            }
         }
      }
   }
   return res;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
