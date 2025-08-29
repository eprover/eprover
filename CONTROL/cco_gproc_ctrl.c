/*-----------------------------------------------------------------------

  File  : cco_gproc_ctrl.c

  Author: Stephan Schulz (schulz@eprover.org)

  Contents

  Code for controlling multiple processes (in particular fork()ed
  instanced of E. Derived from cco_proc_ctrl.c but generalized (oder
  specialized?) for fork()ed subprocesses, not popen()ed external
  processes.

  Copyright 2021 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Sat Dec 11 23:31:39 CET 2021

-----------------------------------------------------------------------*/

#include "cco_gproc_ctrl.h"
#include <sys/wait.h>


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


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
// Function: EPGCtrlAlloc()
//
//   Allocate an initialized EPGCtrlCell.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

EGPCtrl_p EGPCtrlAlloc(int cores)
{
   EGPCtrl_p ctrl = EGPCtrlCellAlloc();

   ctrl->name        = NULL;
   ctrl->pid         = 0;
   ctrl->fileno      = -1;
   ctrl->exit_status = 0;
   ctrl->cpu_limit   = 0;
   ctrl->cores       = cores;
   ctrl->result      = PRNoResult;
   ctrl->output      = DStrAlloc();

   //printf("Allocating: %p\n", ctrl);
   return ctrl;
}


/*-----------------------------------------------------------------------
//
// Function: EGPCtrlFree()
//
//   Free a EPCtrlCell.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void EGPCtrlFree(EGPCtrl_p junk)
{
   //printf("Freeing: %p\n", junk);
   DStrFree(junk->output);
   FREE(junk->name);
   EGPCtrlCellFree(junk);
}



/*-----------------------------------------------------------------------
//
// Function: EGPCtrlCleanup()
//
//   Clean up: Kill process, close pipe,
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void EGPCtrlCleanup(EGPCtrl_p ctrl)
{
   int raw_status;
   int respid = -1;

   if(ctrl->pid)
   {
      if(kill(ctrl->pid, SIGTERM) == 0)
      {
         //while(respid == -1)  // Overkill?
         {
            UNUSED(respid = waitpid(ctrl->pid, &raw_status, 0));
         }
      }
      ctrl->pid = 0;
   }
   if(ctrl->fileno != -1)
   {
      close(ctrl->fileno);
   }
}



/*-----------------------------------------------------------------------
//
// Function: EGCtrlCreate()
//
//   Fork the process and establish a pipe from child to
//   parent. Returns NULL in the the child, a pointer to a new
//   EGPclCtrl-Block wrapping that pipe in the parent.
//
// Global Variables: -
//
// Side Effects    : Yes ;-)
//
/----------------------------------------------------------------------*/

EGPCtrl_p EGPCtrlCreate(char *name, int cores, rlim_t cpu_limit)
{
   EGPCtrl_p res = NULL;
   int       pipefd[2];
   pid_t     childpid;

   if (pipe(pipefd) <0)
   {
      SysError("pipe failed", SYS_ERROR);
      exit(EXIT_FAILURE);
   }
   fprintf(GlobalOut, COMCHAR" Starting %s with %jus (%d) cores\n", name, (uintmax_t)cpu_limit, cores);

   if((childpid = fork()) <0 )
   {
      SysError("fork failed", SYS_ERROR);
   }

   if(childpid == 0)
   {  // child process
      signal(SIGTERM, SIG_DFL);
      dup2(pipefd[1], STDOUT_FILENO);
      close(pipefd[0]);
      close(pipefd[1]);
      GlobalOut = stdout;
      VERBOUTARG("New subprocess ", name);
      if(cpu_limit)
      {
         SetSoftRlimitErr(RLIMIT_CPU, cpu_limit, "RLIMIT_CPU (subprocess)");
      }
   }
   else
   {  /// parent
      close(pipefd[1]);
      VERBOUTARG("Started subprocess ", name);
      res            = EGPCtrlAlloc(cores);
      res->pid       = childpid;
      res->fileno    = pipefd[0];
      res->cpu_limit = cpu_limit;
      res->name      = SecureStrdup(name);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: EGPCtrlGetResult()
//
//   Read data from the connected subprocess. If that has terminated,
//   determine the status and record in in the block. Return
//   true. Otherwise return false.
//
// Global Variables: -
//
// Side Effects    : Reads input, may set result state in ctrl.
//
/----------------------------------------------------------------------*/

bool EGPCtrlGetResult(EGPCtrl_p ctrl, char *buffer, long buf_size)
{
   int len, raw_status;
   int respid = -1;

   len = read(ctrl->fileno, buffer, (size_t)(buf_size-1));
   if(len == -1)
   {
      SysError("read() failed", SYS_ERROR);
   }
   buffer[len] = '\0';

   // fprintf(stdout, COMCHAR" %s: [%s].\n", ctrl->name, buffer);

   if(len)
   {
      DStrAppendStr(ctrl->output, buffer);
      return false;
   }
   else
   {
      if(strstr(DStrView(ctrl->output), SZS_THEOREM_STR))
      {
         ctrl->result = PRTheorem;
      }
      else if(strstr(DStrView(ctrl->output), SZS_CONTRAAX_STR))
      {
         ctrl->result = PRTheorem;
      }
      else if(strstr(DStrView(ctrl->output), SZS_UNSAT_STR))
      {
         ctrl->result = PRUnsatisfiable;
      }
      else if(strstr(DStrView(ctrl->output), SZS_SATSTR_STR))
      {
         ctrl->result = PRSatisfiable;
      }
      else if(strstr(DStrView(ctrl->output), SZS_COUNTERSAT_STR))
      {
         ctrl->result = PRCounterSatisfiable;
      }
      else
      {
         ctrl->result = PRFailure;
      }
      while(respid == -1)
      {
         respid = waitpid(ctrl->pid, &raw_status, 0);
      }
      if(WIFEXITED(raw_status))
      {
         ctrl->exit_status = WEXITSTATUS(raw_status);
      }
      else
      {
         ctrl->exit_status = -1; //Aborted or killed (timeout)
      }
      fprintf(GlobalOut, COMCHAR" %s with pid %d completed with status %d\n",
              ctrl->name, ctrl->pid, ctrl->exit_status);
      ctrl->pid = 0;
      return true;
   }
}




/*-----------------------------------------------------------------------
//
// Function: EGPCtrlSetAlloc()
//
//   Allocate an empty EGPCtrlCell.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

EGPCtrlSet_p EGPCtrlSetAlloc(void)
{
   EGPCtrlSet_p handle = EGPCtrlSetCellAlloc();

   handle->cores_reserved = 0;
   handle->procs          = NULL;

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: EGPCtrlSetFree()
//
//   Free an EPCtrlSet(), including the payload. Will clean up the
//   processes.
//
// Global Variables: -
//
// Side Effects    : Will terminate processes, memory, ...
//
/----------------------------------------------------------------------*/

void EGPCtrlSetFree(EGPCtrlSet_p junk, bool kill_proc)
{
   while(junk->procs)
   {
      EGPCtrlSetDeleteProc(junk, junk->procs->val1.p_val, kill_proc);
   }
   EGPCtrlSetCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: EGPCtrlSetAddProc()
//
//   Add a process to the process set.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void EGPCtrlSetAddProc(EGPCtrlSet_p set, EGPCtrl_p proc)
{
   IntOrP tmp;

   tmp.p_val = proc;
   NumTreeStore(&(set->procs), proc->fileno, tmp, tmp);
   set->cores_reserved += proc->cores;
}


/*-----------------------------------------------------------------------
//
// Function: EGPCtrlSetFindProc()
//
//   Find the process associated with fd.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

EGPCtrl_p EGPCtrlSetFindProc(EGPCtrlSet_p set, int fd)
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
// Function: EGPCtrlSetDeleteProc()
//
//   Delete a process from the set.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void EGPCtrlSetDeleteProc(EGPCtrlSet_p set, EGPCtrl_p proc, bool kill_proc)
{
   NumTree_p cell;

   cell = NumTreeExtractEntry(&(set->procs), proc->fileno);
   if(cell)
   {
      if(kill_proc)
      {
         EGPCtrlCleanup(cell->val1.p_val);
      }
      set->cores_reserved -= proc->cores;
      EGPCtrlFree(cell->val1.p_val);
      NumTreeCellFree(cell);
   }
}



/*-----------------------------------------------------------------------
//
// Function: EGPCtrlSetFDSet()
//
//   Set all file descriptor bits of the set in the fd_set data
//   structure. Return the largest one.
//
// Global Variables: -
//
// Side Effects    : Memory (only temporary)
//
/----------------------------------------------------------------------*/

int EGPCtrlSetFDSet(EGPCtrlSet_p set, fd_set *rd_fds)
{
   PStack_p trav_stack;
   int maxfd = 0;
   EGPCtrl_p handle;
   NumTree_p cell;

   trav_stack = NumTreeTraverseInit(set->procs);
   while((cell = NumTreeTraverseNext(trav_stack)))
   {
      handle = cell->val1.p_val;
      FD_SET(handle->fileno, rd_fds);
      maxfd = MAX(maxfd, handle->fileno);
   }
   NumTreeTraverseExit(trav_stack);

   return maxfd;
}


/*-----------------------------------------------------------------------
//
// Function: EGPCtrlSetGetResult()
//
//
//
// Global Variables: -
//
// Side Effects    : Reads data from subprocesses, etc.
//
/----------------------------------------------------------------------*/

EGPCtrl_p EGPCtrlSetGetResult(EGPCtrlSet_p set)
{
   bool eof;
   fd_set readfds, writefds, errorfds;
   int maxfd = 0,i;
   EGPCtrl_p handle, res = NULL;
   struct timeval waittime;
   int sel_success;

   FD_ZERO(&readfds);
   FD_ZERO(&writefds);
   FD_ZERO(&errorfds);
   waittime.tv_sec  = 0;
   waittime.tv_usec = 500000;

   maxfd = EGPCtrlSetFDSet(set, &readfds);

   //ELog("Before select (TERM: %d)\n", SigTermCaught);
   sel_success = select(maxfd+1, &readfds, &writefds, &errorfds, &waittime);
   //ELog("After select (TERM: %d)\n", SigTermCaught);
   if(sel_success !=-1)
   {
      for(i=0; i<= maxfd && !res; i++)
      {
         //ELog("Loop %d\n", i);
         if(FD_ISSET(i, &readfds))
         {
            //ELog("Readable:\n", i);
            handle = EGPCtrlSetFindProc(set, i);
            eof = EGPCtrlGetResult(handle, set->buffer, EGPCTRL_BUFSIZE);
            if(eof)
            {
               switch(handle->result)
               {
               case PRNoResult:
                     /* No result (yet) -> should not really happen! */
                     assert(false);
                     break;
               case PRSatisfiable:
               case PRCounterSatisfiable:
               case PRTheorem:
               case PRUnsatisfiable:
                     res = handle;
                     //printf("res: %p\n", res);
                     break;
               case PRFailure:
                     /* Process terminates, but no result determined -> Remove it*/
                     EGPCtrlSetDeleteProc(set, handle, true);
                     break;
               default:
                     assert(false && "Impossible ProverResult");
               }
            }
         }
      }
   }
   //ELog("After Loop (TERM: %d)\n", SigTermCaught);
   return res;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
