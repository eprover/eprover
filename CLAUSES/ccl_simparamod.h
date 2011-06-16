/*-----------------------------------------------------------------------

File  : ccl_simparamod.h

Author: Stephan Schulz

Contents
 
  Code for implementing simulataneous paramodulation. Also contains
  some stuff to make standard paramodulation cleaner.

Copyright 1998-2011 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Thu Apr 29 01:20:55 CEST 2004
    New

-----------------------------------------------------------------------*/

#ifndef CCL_SIMPARAMOD

#define CCL_SIMPARAMOD



/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


bool ClausePosCheckPMFromConstraint(ClausePos_p from_pos);
bool ClausePosCheckPMIntoConstraint(ClausePos_p into_pos);

Clause_p ClauseSimParamod(TB_p bank, ClausePos_p from_pos, 
                          Clause_pos_p into);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





