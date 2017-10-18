/*-----------------------------------------------------------------------

File  : cco_einteractive_mode.h

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

#ifndef CCO_EINTERACTIVE_MODE

#define CCO_EINTERACTIVE_MODE

#include <cio_simplestuff.h>
#include <ccl_formulafunc.h>
#include <cco_proc_ctrl.h>
#include <cio_network.h>
#include <clb_dstrings.h>
#include <clb_pstacks.h>
#include <cio_scanner.h>
#include <cco_batch_spec.h>
#include <dirent.h>
#include <sys/wait.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct axiom_set_cell
{
  ClauseSet_p cset;
  FormulaSet_p fset;
  int staged;
  DStr_p raw_data;
} AxiomSetCell, *AxiomSet_p;

typedef struct interactive_spec_cell
{
  BatchSpec_p spec;
  StructFOFSpec_p ctrl;
  FILE* fp;
  int sock_fd;
  PStack_p axiom_sets;
  DStr_p server_lib;
} InteractiveSpecCell, *InteractiveSpec_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define InteractiveSpecCellAlloc()    (InteractiveSpecCell*)SizeMalloc(sizeof(InteractiveSpecCell))
#define InteractiveSpecCellFree(junk) SizeFree(junk, sizeof(InteractiveSpecCell))

InteractiveSpec_p InteractiveSpecAlloc(BatchSpec_p spec,
                                       StructFOFSpec_p ctrl,
                                       FILE* fp,
                                       int sock_fd);

void InteractiveSpecFree(InteractiveSpec_p spec);


#define AxiomSetCellAlloc()    (AxiomSetCell*)SizeMalloc(sizeof(AxiomSetCell))
#define AxiomSetCellFree(junk) SizeFree(junk, sizeof(AxiomSetCell))

AxiomSet_p AxiomSetAlloc(ClauseSet_p cset,
                         FormulaSet_p fset,
                         DStr_p raw_data,
                         int staged);

void AxiomSetFree(AxiomSet_p axiomset);

void StartDeductionServer(BatchSpec_p spec,
                          StructFOFSpec_p ctrl,
                          char* server_lib,
                          FILE* fp,
                          int sock_fd);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
