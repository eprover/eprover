/*-----------------------------------------------------------------------

File  : che_to_precgen.h

Author: Stephan Schulz

Contents

  Routines for generating precedences for term orderings

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
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
   PInvalidEntry = -1,
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
   PByInvConjFrequency, /* Make often occuring symbols small,
                         * conjecture symbols large */
   PByInvFreqConjMax,   /* Make conjecture symbols maximal, otherwise
                           use invfreq */
   PByInvFreqConjMin,   /* Make conjecture symbols mminimal, otherwise
                           use invfreq */
   PByInvFreqConstMin,  /* Make rarely occuring symbols small, except for
            constants */
   PByInvFreqHack,      /* Make constants minimal, frequent unary
                           symbols maximal, otherwise as
                           PByInvFrequency */
   PArrayOpt,           /* Special hack for theory of array with
                           conceptually typed symbols recognized by
                           name. */
   POrientAxioms,       /* My (planned) hack */
   PMinMethod = PUnaryFirst,
   PMaxMethod = POrientAxioms
}TOPrecGenMethod;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define FREQ_SEMI_INFTY 2000000 /* Bigger than any expected frequency,
             * small enough to never cause over-
             * or underflow */

extern char* TOPrecGenNames[];

#define TOGetPrecGenName(method) (TOPrecGenNames[(method)])

TOPrecGenMethod TOTranslatePrecGenMethod(char* name);

#define TOGenerateDefaultPrecedence(ocb,axioms)               \
        TOGeneratePrecedence((ocb), (axioms),NULL, PUnaryFirst)

void TOGeneratePrecedence(OCB_p ocb, ClauseSet_p axioms,
           char* predefined, TOPrecGenMethod method);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/






