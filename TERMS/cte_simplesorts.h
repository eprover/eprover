/*-----------------------------------------------------------------------

File  : cte_simplesorts.h

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Data structure and function interfaces for managing simple, disjoint
  sorts.

  Copyright 2007 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sat Sep 15 01:33:52 EDT 2007
    New

-----------------------------------------------------------------------*/

#ifndef CTE_SIMPLESORTS

#define CTE_SIMPLESORTS

#include <clb_stringtrees.h>
#include <cio_scanner.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* Build-in sorts for the many-sorted logic E is being moved to. Note
 * that the system relies on the fact that the system-defined sorts are
 * inserted in a specific order.
 *
 * User sorts are integers bigger than STPredefined. */



#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
