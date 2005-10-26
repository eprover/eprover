/*-----------------------------------------------------------------------

File  : cto_orderings.h

Author: Stephan Schulz

Contents
 
  Generic Interface to the term comparison routines.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Mon May  4 23:24:41 MET DST 1998
    New

-----------------------------------------------------------------------*/

#ifndef CTO_ORDERINGS

#define CTO_ORDERINGS

#include <cto_lpo.h>
#include <cto_lpo_debug.h>
/* #include <cto_rpo.h> */
#include <cto_kbo.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


bool          TOGreater(OCB_p ocb, Term_p s, Term_p t, DerefType
			deref_s, DerefType deref_t); 
CompareResult TOCompare(OCB_p ocb, Term_p s, Term_p t, DerefType
			deref_s, DerefType deref_t);

CompareResult TOCompareSymbolParse(Scanner_p in);
PStackPointer TOSymbolComparisonChainParse(Scanner_p in, OCB_p ocb);
PStackPointer TOPrecedenceParse(Scanner_p in, OCB_p ocb);
void          TOSymbolWeightParse(Scanner_p in, OCB_p ocb);
long          TOWeightsParse(Scanner_p in, OCB_p ocb);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
