/*-----------------------------------------------------------------------

File  : pcl_lemmas.c

Author: Stephan Schulz

Contents

  Data structures and functions for the detection of lemmas in PCL2
  protocol. 

  Copyright 2003 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Mon Jun 16 13:06:20 CEST 2003
    New

-----------------------------------------------------------------------*/

#include "pcl_lemmas.h"



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
// Function: LemmaParamAlloc()
//
//   Allocate an initialized parameter block for the lemma detection
//   algorithm. 
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

LemmaParam_p LemmaParamAlloc(void)
{
   LemmaParam_p handle = LemmaParamCellAlloc();

   handle->size_w       = LEMMA_SIZE_W;
   handle->act_pm_w     = LEMMA_ACT_PM_W;
   handle->o_gen_w      = LEMMA_O_GEN_W;
   handle->act_simpl_w  = LEMMA_ACT_SIMPL_W;
   handle->pas_simpl_w  = LEMMA_PAS_SIMPL_W;
   handle->subsum_w     = LEMMA_SUBSUM_W;
   handle->proof_tree_w = LEMMA_PROOF_TREE_W;
   handle->proof_dag_w  = LEMMA_PROOF_DAG_W;

   return handle;
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


