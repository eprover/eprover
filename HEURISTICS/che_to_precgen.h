/*-----------------------------------------------------------------------

File  : che_to_precgen.h

Author: Stephan Schulz

Contents
 
  Routines for generating precedences for term orderings

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Fri Oct 16 17:01:23 MET DST 1998
    New

-----------------------------------------------------------------------*/

#ifndef CHE_TO_PRECGEN

#define CHE_TO_PRECGEN

#include <che_fcode_featurearrays.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef enum
{
   PNoMethod = 0,       /* Nothing */
   PUnaryFirst,         /* My hack ;-) */
   PUnaryFirstFreq,     /* My new hack ;-) */
   PArity,              /* Sort by arity */
   PInvArity,           /* Sort by inverse arity */
   PConstMax,           /* Sort by arity, but constants first (Default
			   for SPASS) */
   PInvArConstMin,      /* Sort by inverse arity, but make constants
			   minimal */
   PByFrequency,        /* Make often occuring symbols big */
   PByInvFrequency,     /* Make often occuring symbols small */
   PByInvFreqConstMin,  /* Make often occuring symbols big, except for
			   constants */
   POrientAxioms,       /* My (planned) hack */
   PMinMethod = PUnaryFirst,
   PMaxMethod = POrientAxioms
}TOPrecGenMethod;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


extern char* TOPrecGenNames[];

#define TOGetPrecGenName(method) \
        (assert((method)>=PNoMethod), assert((method)<=PMaxMethod), \
	 TOPrecGenNames[(method)])

TOPrecGenMethod TOTranslatePrecGenMethod(char* name);

#define TOGenerateDefaultPrecedence(ocb) \
        TOGeneratePrecedence((ocb), NULL, PUnaryFirst)

void TOGeneratePrecedence(OCB_p ocb, ClauseSet_p axioms,
			  char* predefined, TOPrecGenMethod method);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/






