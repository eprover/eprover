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
ScannerBoolResult ParseBool(Scanner_p in);

DEFINE_RESULT(ParseIntMaxResult, intmax_t);
ParseIntMaxResult ParseIntMax(Scanner_p in);

ScannerLongResult ParseInt(Scanner_p in);

ScannerLongResult ParseIntLimited(Scanner_p in, long lower, long upper);

DEFINE_RESULT(ParseUIntMaxResult, uintmax_t);
ParseUIntMaxResult ParseUIntMax(Scanner_p in);

ScannerDoubleResult ParseFloat(Scanner_p in);

DEFINE_RESULT(ParseNumStringResult, StrNumType);
ParseNumStringResult ParseNumString(Scanner_p in);

ScannerLongResult DDArrayParse(Scanner_p in, DDArray_p array, bool brackets);

ScannerCharPResult ParseFilename(Scanner_p in);

ScannerCharPResult ParsePlainFilename(Scanner_p in);

ScannerCharPResult ParseBasicInclude(Scanner_p in);

ScannerCharPResult ParseDottedId(Scanner_p in);

bool _ConsumeDottedId(Scanner_p in, char* expected);

ScannerCharPResult ParseContinous(Scanner_p in);

bool _ParseSkipParenthesizedExpr(Scanner_p in);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
