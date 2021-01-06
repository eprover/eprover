/*-----------------------------------------------------------------------

  File  : cio_basicparser.h

  Author: Stephan Schulz

  Contents

  Parsing routines for useful C build-in ans some general CLIB
  datatypes not covered by the scanner.

  Copyright 1998-2020 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Mon Sep  8 16:34:11 MET DST 1997

  -----------------------------------------------------------------------*/

#ifndef CIO_BASICPARSER

#define CIO_BASICPARSER

#include <clb_ddarrays.h>
#include <clb_pstacks.h>
#include <cio_scanner.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef enum
{
   SNNoNumber,
   SNInteger,
   SNRational,
   SNFloat
}StrNumType;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

bool       ParseBool(Scanner_p in);
intmax_t   ParseIntMax(Scanner_p in);
long       ParseInt(Scanner_p in);
long       ParseIntLimited(Scanner_p in, long lower, long upper);
uintmax_t  ParseUIntMax(Scanner_p in);
double     ParseFloat(Scanner_p in);
StrNumType ParseNumString(Scanner_p in);
long       DDArrayParse(Scanner_p in, DDArray_p array, bool brackets);
char*      ParseFilename(Scanner_p in);
char*      ParsePlainFilename(Scanner_p in);
char*      ParseBasicInclude(Scanner_p in);
char*      ParseDottedId(Scanner_p in);
void       AcceptDottedId(Scanner_p in, char* expected);
char*      ParseContinous(Scanner_p in);

void       ParseSkipParenthesizedExpr(Scanner_p in);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
