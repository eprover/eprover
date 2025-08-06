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
#include <clb_simple_stuff.h>
#include <cio_tempfile.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


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
#define MAX_CORES 8


typedef struct e_pctrl_set_cell
{
   NumTree_p procs;  /* Indexed by fileno() */
   char      buffer[EPCTRL_BUFSIZE];
}EPCtrlSetCell, *EPCtrlSet_p;

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define SZS_THEOREM_STR    COMCHARRAW" SZS status Theorem"
#define SZS_CONTRAAX_STR   COMCHARRAW" SZS status ContradictoryAxioms"
#define SZS_UNSAT_STR      COMCHARRAW" SZS status Unsatisfiable"
#define SZS_SATSTR_STR     COMCHARRAW" SZS status Satisfiable"
#define SZS_COUNTERSAT_STR COMCHARRAW" SZS status CounterSatisfiable"
#define SZS_GAVEUP_STR     COMCHARRAW" SZS status GaveUp"
#define SZS_FAILURE_STR    COMCHARRAW" Failure:"


#define E_OPTIONS_BASE " --print-pid -s -R  --memory-limit=2048 --proof-object "
#define E_OPTIONS "--satauto-schedule --assume-incompleteness"


extern char* PRResultTable[];

#define EPCtrlCellAlloc()    (EPCtrlCell*)SizeMalloc(sizeof(EPCtrlCell))
#define EPCtrlCellFree(junk) SizeFree(junk, sizeof(EPCtrlCell))

EPCtrl_p EPCtrlAlloc(char* name);
void     EPCtrlFree(EPCtrl_p junk);

EPCtrl_p ECtrlCreate(char* prover, char* name,
                     char* extra_options,
                     long cpu_limit, char* file);

EPCtrl_p ECtrlCreateGeneric(char* prover, char* name,
                            char* options, char* extra_options,
                            long cpu_limit, char* file);
void     EPCtrlCleanup(EPCtrl_p ctrl, bool delete_file1);

bool EPCtrlGetResult(EPCtrl_p ctrl,
                     char* buffer,
                     long buf_size);

#define EPCtrlSetCellAlloc()    (EPCtrlSetCell*)SizeMalloc(sizeof(EPCtrlSetCell))
#define EPCtrlSetCellFree(junk) SizeFree(junk, sizeof(EPCtrlSetCell))

EPCtrlSet_p EPCtrlSetAlloc(void);
void        EPCtrlSetFree(EPCtrlSet_p junk, bool delete_files);
void        EPCtrlSetAddProc(EPCtrlSet_p set, EPCtrl_p proc);
EPCtrl_p    EPCtrlSetFindProc(EPCtrlSet_p set, int fd);
void        EPCtrlSetDeleteProc(EPCtrlSet_p set,
                                EPCtrl_p proc, bool delete_file);
#define     EPCtrlSetEmpty(set) ((set)->procs==NULL)
#define     EPCtrlSetCardinality(set) NumTreeNodes((set)->procs)

int         EPCtrlSetFDSet(EPCtrlSet_p set, fd_set *rd_fds);

EPCtrl_p    EPCtrlSetGetResult(EPCtrlSet_p set, bool delete_files);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
