/*-----------------------------------------------------------------------

File  : pcl_lemmas.h

Author: Stephan Schulz

Contents

  Definition for dealing with lemmas in PCL protocols. 


  Copyright 1998-2003 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Sun Jun 15 22:47:43 CEST 2003
    New

-----------------------------------------------------------------------*/

#ifndef PCL_LEMMAS

#define PCL_LEMMAS

#include <pcl_protocol.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


/* Lemma rating is as follows: 

   size   = StandardWeight(lemma)
   actpm  = references as active partner in paramod
   o_gen  = references from other generating inferences
   act_simpl = references from active simplification
   pas_simpl = references from being simplified
   subsum = references from subsumption (will probably not always be
              available) 
   proof_tree = size of proof tree (unfolded)
   proof_dag  = size of proof tree seen as a dag  


   (actpm*actpm_w + 
   o_gen*o_gen_w + 
   act_simpl*act_simpl_w + 
   pas_simpl*pas_simpl_w +
   subsum*subsum_w)
   *
   (proof_tree*proof_tree_w+
   proof_dag*proof_dag_w)
   /
   size*size_w

   Large is good! */

typedef struct lemma_param_cell
{
   double size_w;   
   double act_pm_w;
   double o_gen_w;
   double act_simpl_w;
   double pas_simpl_w;
   double subsum_w;
   double proof_tree_w;
   double proof_dag_w;
}LemmaParamCell, *LemmaParam_p;

#define LEMMA_SIZE_W       1
#define LEMMA_ACT_PM_W     2
#define LEMMA_O_GEN_W      1
#define LEMMA_ACT_SIMPL_W  2
#define LEMMA_PAS_SIMPL_W  1
#define LEMMA_SUBSUM_W     0 /* Not in most protocols anyway */
#define LEMMA_PROOF_TREE_W 0 /* Less important for search */
#define LEMMA_PROOF_DAG_W  1 /* More important for search */


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


#define LemmaParamCellAlloc() (LemmaParamCell*)SizeMalloc(sizeof(LemmaParamCell))
#define LemmaParamCellFree(junk)        SizeFree(junk, sizeof(LemmaParamCell))

LemmaParam_p LemmaParamAlloc(void);
#define LemmaParamFree(cell) LemmaParamCellFree(cell)

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





