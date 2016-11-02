/*-----------------------------------------------------------------------

File  : cto_kbolin.h

Author: Stephan Schulz

Contents

  Definitions for implementing a linear time implementation of the
  Knuth-Bendix ordering. The implementation is based in the ideas
  presented in [Loechner:JAR-2006] (Bernd Loechner, "Things to Know
  when Implementing KBO", JAR 36(4):289-310, 2006.

  Copyright 2010 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Mon Feb 15 14:41:04 EET 2010
    New (from cto_kbo.h)

-----------------------------------------------------------------------*/

#ifndef CTO_KBOLIN

#define CTO_KBOLIN

#include <cto_ocb.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

bool          KBO6Greater(OCB_p ocb, Term_p s, Term_p t, DerefType
                          deref_s, DerefType deref_t);

CompareResult KBO6Compare(OCB_p ocb, Term_p t1, Term_p t2,
                          DerefType deref_t1, DerefType deref_t2);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/










