/*-----------------------------------------------------------------------

File  : cte_functypes.h

Author: Stephan Schulz

Contents

  Simple, widely used functions for dealing with function symbols and
  operators.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sun Nov  9 23:09:33 MET 1997
    New

-----------------------------------------------------------------------*/

#ifndef CTE_FUNCTYPES

#define CTE_FUNCTYPES

#include <clb_simple_stuff.h>
#include <cio_basicparser.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* Data type repesenting the various types of encodings for function
 * symols (including constants) and predicates. */

typedef enum
{
   FSNone,
   FSIdentVar,         /* Ident, starts with capital letter or _ */
   FSIdentFreeFun,     /* Ident, starts with Lower case letter or SQString */
   FSIdentInt,         /* Integer */
   FSIdentFloat,       /* Floating point number */
   FSIdentRational,    /* Rational number */
   FSIdentInterpreted, /* SemIdent */
   FSIdentObject       /* String "in double quotes" */
}FuncSymbType;

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

/* Function symbols in terms are represented by positive numbers,
   variables by negative numbers. This alias allows clearer
   specifications. */

typedef long FunCode;

extern TokenType FuncSymbToken;
extern TokenType FuncSymbStartToken;

FuncSymbType FuncSymbParse(Scanner_p in, DStr_p id);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
