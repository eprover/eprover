/*-----------------------------------------------------------------------

File  : cle_tsmio.h

Author: Stephan Schulz

Contents

  Functions for building TSMs from a knowledge base.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Tue Aug 31 13:23:14 MET DST 1999
    New

-----------------------------------------------------------------------*/

#ifndef CLE_TSMIO

#define CLE_TSMIO

#include <cle_examplerep.h>
#include <cle_tsm.h>
#include <cle_kbdesc.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


double ExampleSetPrepare(FlatAnnoSet_p flatset, AnnoSet_p annoset, double
          evalweights[], ExampleSet_p examples,  Sig_p sig,
          ClauseSet_p target, long sel_no, double set_part,
          double dist_part);

double ExampleSetFromKB(AnnoSet_p annoset, FlatAnnoSet_p flatset, bool
            flat_patterns, TB_p bank, double evalweights[],
            char* kb, Sig_p sig, ClauseSet_p target, long
            sel_no, double set_part, double dist_part);

TSMAdmin_p TSMFromKB(bool flat_patterns, double evalweights[], char*
           kb,  Sig_p sig, ClauseSet_p target, long sel_no,
           double set_part, double dist_part, IndexType
           indextype, TSMType tsmtype, long indexdepth);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





