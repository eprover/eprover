/*-----------------------------------------------------------------------

File  : cco_proc_ctrl.h

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Code for running E as a separate process within other programs. This
  is only a first draft - there probably will be a much better version
  eventually ;-).

  Copyright 2010 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed Jul 14 11:45:55 BST 2010
    New

-----------------------------------------------------------------------*/

#ifndef CCO_PROC_CTRL

#define CCO_PROC_CTRL

#include <sys/select.h>
#include <signal.h>
#include <clb_numtrees.h>
#include <cio_tempfile.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef enum
{
   PRNoResult = 0,
   PRTheorem,
   PRUnsatisfiable,
   PRSatisfiable,
   PRCounterSatisfiable,
   PRFailure,
   PRGaveUp
}ProverResult;


typedef struct e_pctrl_cell
{
   pid_t        pid;
   int          fileno;
   FILE*        pipe;
   char*        input_file;
   char*        name;
   long long    start_time;
   long         prob_time;
   ProverResult result;
   DStr_p       output;
}EPCtrlCell, *EPCtrl_p;

#define EPCTRL_BUFSIZE 200

typedef struct e_pctrl_set_cell
{
   NumTree_p procs;  /* Indexed by fileno() */
   char      buffer[EPCTRL_BUFSIZE];
}EPCtrlSetCell, *EPCtrlSet_p;

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define SZS_THEOREM_STR    "# SZS status Theorem"
#define SZS_UNSAT_STR      "# SZS status Unsatisfiable"
#define SZS_SATSTR_STR     "# SZS status Satisfiable"
#define SZS_COUNTERSAT_STR "# SZS status CounterSatisfiable"
#define SZS_GAVEUP_STR     "# SZS status GaveUp"
#define SZS_FAILURE_STR    "# Failure:"


#define E_OPTIONS "--print-pid -s -xAuto -tAuto --free-numbers -R --answers=1 \
--assume-incompleteness --memory-limit=1024 \
--proof-object --cpu-limit="


extern char* PRResultTable[];

#define EPCtrlCellAlloc()    (EPCtrlCell*)SizeMalloc(sizeof(EPCtrlCell))
#define EPCtrlCellFree(junk) SizeFree(junk, sizeof(EPCtrlCell))

EPCtrl_p EPCtrlAlloc(char* name);
void     EPCtrlFree(EPCtrl_p junk);

EPCtrl_p ECtrlCreate(char* prover, char* name,
                     char* extra_options,
                     long cpu_limit, char* file);

void     EPCtrlCleanup(EPCtrl_p ctrl);

bool EPCtrlGetResult(EPCtrl_p ctrl,
                     char* buffer,
                     long buf_size);

#define EPCtrlSetCellAlloc()    (EPCtrlSetCell*)SizeMalloc(sizeof(EPCtrlSetCell))
#define EPCtrlSetCellFree(junk) SizeFree(junk, sizeof(EPCtrlSetCell))

EPCtrlSet_p EPCtrlSetAlloc(void);
void        EPCtrlSetFree(EPCtrlSet_p junk);
void        EPCtrlSetAddProc(EPCtrlSet_p set, EPCtrl_p proc);
EPCtrl_p    EPCtrlSetFindProc(EPCtrlSet_p set, int fd);
void        EPCtrlSetDeleteProc(EPCtrlSet_p set, EPCtrl_p proc);
#define     EPCtrlSetEmpty(set) ((set)->procs==NULL)

int         EPCtrlSetFDSet(EPCtrlSet_p set, fd_set *rd_fds);

EPCtrl_p    EPCtrlSetGetResult(EPCtrlSet_p set);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





