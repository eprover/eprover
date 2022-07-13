/*-----------------------------------------------------------------------

File  : cco_sine.h

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Data types and definitions for supporting SinE-like specification
  filtering.

  Copyright 2012 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Thu May 10 08:35:26 CEST 2012
    New

-----------------------------------------------------------------------*/

#ifndef CCO_SINE

#define CCO_SINE

#include <ccl_proofstate.h>
#include <ccl_formulafunc.h>
#include <ccl_sine.h>
#include <che_rawspecfeatures.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* Complex, multi-file problem specification data structure - holding
 * information about all the includes parsed so they are parsed at
 * most once. */

typedef struct structured_FOF_spec_cell
{
   //Sig_p         sig;
   TB_p          terms;
   //GCAdmin_p     gc_terms;
   PStack_p      clause_sets;
   PStack_p      formula_sets;
   StrTree_p     parsed_includes;
   PStackPointer shared_ax_sp;
   FunCode       shared_ax_f_count;
   GenDistrib_p  f_distrib;
}StructFOFSpecCell, *StructFOFSpec_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define StructFOFSpecCellAlloc()    (StructFOFSpecCell*)SizeMalloc(sizeof(StructFOFSpecCell))
#define StructFOFSpecCellFree(junk) SizeFree(junk, sizeof(StructFOFSpecCell))

StructFOFSpec_p StructFOFSpecCreate(TB_p terms);
StructFOFSpec_p StructFOFSpecAlloc(void);

void            StructFOFSpecDestroy(StructFOFSpec_p ctrl);
void            StructFOFSpecFree(StructFOFSpec_p ctrl);

long           StructFOFSpecParseAxioms(StructFOFSpec_p ctrl,
                                       PStack_p axfiles,
                                        IOFormat parse_format,
                                        char* default_dir);
#define       StructFOFSpecResetShared(ctrl) (ctrl)->shared_ax_sp = 0

void           StructFOFSpecInitDistrib(StructFOFSpec_p ctrl, bool trim);

void StructFOFSpecAddProblem(StructFOFSpec_p ctrl,
                             ClauseSet_p clauses,
                             FormulaSet_p formulas,
                             bool trim);

void StructFOFSpecBacktrackToSpec(StructFOFSpec_p ctrl);

long StructFOFSpecCollectFCode(StructFOFSpec_p ctrl,
                               FunCode f_code,
                               PStack_p res_formulas);

long StructFOFSpecGetProblem(StructFOFSpec_p ctrl,
                             AxFilter_p filter,
                             PStack_p res_clauses,
                             PStack_p res_formulas);

long ProofStateSinE(ProofState_p state, char* filter);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
