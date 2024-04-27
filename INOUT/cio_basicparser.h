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
#include <stdint.h>

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
DEFINE_RESULT(ParseBoolResult, bool)
ParseBoolResult ParseBool(Scanner_p in);

DEFINE_RESULT(ParseIntMaxResult, intmax_t)
ParseIntMaxResult ParseIntMax(Scanner_p in);

DEFINE_RESULT(ParseIntResult, long)
ParseIntResult ParseInt(Scanner_p in);

DEFINE_RESULT(ParseIntLimitedResult, long)
ParseIntLimitedResult ParseIntLimited(Scanner_p in, long lower, long upper);

DEFINE_RESULT(ParseUIntMaxResult, uintmax_t)
ParseUIntMaxResult ParseUIntMax(Scanner_p in);

DEFINE_RESULT(ParseFloatResult, double)
ParseFloatResult ParseFloat(Scanner_p in);

DEFINE_RESULT(ParseNumStringResult, StrNumType)
ParseNumStringResult ParseNumString(Scanner_p in);

DEFINE_RESULT(DDArrayParseResult, long)
DDArrayParseResult DDArrayParse(Scanner_p in, DDArray_p array, bool brackets);

DEFINE_RESULT(ParseFilenameResult, char*)
ParseFilenameResult ParseFilename(Scanner_p in);

DEFINE_RESULT(ParsePlainFilenameResult, char*)
ParsePlainFilenameResult ParsePlainFilename(Scanner_p in);

DEFINE_RESULT(ParseBasicIncludeResult, char*)
ParseBasicIncludeResult ParseBasicInclude(Scanner_p in);

DEFINE_RESULT(ParseDottedIdResult, char*)
ParseDottedIdResult ParseDottedId(Scanner_p in);

bool _ConsumeDottedId(Scanner_p in, char* expected);

DEFINE_RESULT(ParseContinousResult, char*)
ParseContinousResult ParseContinous(Scanner_p in);

bool _ParseSkipParenthesizedExpr(Scanner_p in);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
