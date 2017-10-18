/*-----------------------------------------------------------------------

File  : che_axiomscan.h

Author: Stephan Schulz

Contents

  Declarations for functions recognizing certain axioms (e.g. AC
  axioms).

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1>     New

-----------------------------------------------------------------------*/

#ifndef CHE_AXIOMSCAN_H

#define CHE_AXIOMSCAN_H

#include <ccl_clausesets.h>
#include <cle_indexfunctions.h>



/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


FunCode DetectCommutativity(Clause_p clause);
FunCode DetectAssociativity(Clause_p clause);
bool    ClauseScanAC(Sig_p sig, Clause_p clause);
bool    ClauseSetScanAC(Sig_p sig, ClauseSet_p set);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/






