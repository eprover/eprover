/*-----------------------------------------------------------------------

File  : cto_kbo.h

Author: Stephan Schulz

Contents

  Definitions for implementing a Knuth-Bendix ordering.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Thu May 28 12:14:31 MET DST 1998
    New
<2> Mon Jul 13 19:16:03 MET DST 1998
    Changed checking of the variable condition - StS
<3> Fri Aug 17 00:25:01 CEST 2001
    Removed obsolete old code for KBO1, renamed everything

-----------------------------------------------------------------------*/

#ifndef CTO_KBO

#define CTO_KBO

#include <cto_ocb.h>
#include <cte_varhash.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

CompareResult KBOVarCompare(Term_p s, Term_p t, DerefType deref_s,
             DerefType deref_t);
bool          KBOVarGreater(Term_p s, Term_p t, DerefType deref_s,
             DerefType deref_t);

bool          KBOGreater(OCB_p ocb, Term_p s, Term_p t, DerefType
          deref_s, DerefType deref_t);

CompareResult KBOCompare(OCB_p ocb, Term_p t1, Term_p t2,
          DerefType deref_t1, DerefType deref_t2);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/










