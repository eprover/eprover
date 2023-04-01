/*-----------------------------------------------------------------------

  File  : cco_gproc_ctrl.h

  Author: Stephan Schulz (schulz@eprover.org)

  Contents

  Code for handling forked processes and IPC. This is derived from
  cco_proc_ctrl.h, but not suitable for external processess started
  via popen(), but for fork()ed subprocesses.

  Copyright 2021 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Sat Dec 11 23:06:19 CET 2021

-----------------------------------------------------------------------*/

#ifndef CCO_GPROC_CTRL

#define CCO_GPROC_CTRL

#include <cio_signals.h>
#include <cco_proc_ctrl.h>
#include <unistd.h>
#include <sys/wait.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


typedef struct e_gpctrl_cell
{
   char         *name;
   pid_t        pid;
   int          fileno;
   int          exit_status;
   rlim_t       cpu_limit;
   int          cores;
   ProverResult result;
   DStr_p       output;
}EGPCtrlCell, *EGPCtrl_p;


#define EGPCTRL_BUFSIZE 1024

typedef struct e_gpctrl_set_cell
{
   int       cores_reserved;
   NumTree_p procs;  /* Indexed by fileno() */
   char      buffer[EGPCTRL_BUFSIZE];
}EGPCtrlSetCell, *EGPCtrlSet_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


#define EGPCtrlCellAlloc()    (EGPCtrlCell*)SizeMalloc(sizeof(EGPCtrlCell)+0)
#define EGPCtrlCellFree(junk) SizeFree(junk, sizeof(EGPCtrlCell)+0)

EGPCtrl_p EGPCtrlAlloc(int cores);
void      EGPCtrlFree(EGPCtrl_p junk);

EGPCtrl_p EGPCtrlCreate(char* name, int cores, rlim_t cpu_limit);

void      EGPCtrlCleanup(EGPCtrl_p ctrl);

bool      EGPCtrlGetResult(EGPCtrl_p ctrl,
                           char* buffer,
                           long buf_size);

#define EGPCtrlSetCellAlloc()    (EGPCtrlSetCell*)SizeMalloc(sizeof(EGPCtrlSetCell))
#define EGPCtrlSetCellFree(junk) SizeFree(junk, sizeof(EGPCtrlSetCell))

EGPCtrlSet_p EGPCtrlSetAlloc(void);
void         EGPCtrlSetFree(EGPCtrlSet_p junk, bool kill_proc);
void         EGPCtrlSetAddProc(EGPCtrlSet_p set, EGPCtrl_p proc);
EGPCtrl_p    EGPCtrlSetFindProc(EGPCtrlSet_p set, int fd);
void         EGPCtrlSetDeleteProc(EGPCtrlSet_p set, EGPCtrl_p proc, bool kill_proc);
#define      EGPCtrlSetEmpty(set) ((set)->procs==NULL)
#define      EGPCtrlSetCardinality(set) NumTreeNodes((set)->procs)
#define      EGPCtrlSetCoresReserved(set) ((set)->cores_reserved)

int          EGPCtrlSetFDSet(EGPCtrlSet_p set, fd_set *rd_fds);

EGPCtrl_p    EGPCtrlSetGetResult(EGPCtrlSet_p set);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
