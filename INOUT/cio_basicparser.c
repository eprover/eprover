/*-----------------------------------------------------------------------

  File  : cio_basicparser.c

  Author: Stephan Schulz

  Contents

  Parsing routines for useful C build-in Datatypes not covered by the
  scanner.

  Copyright 1998-2020 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Sat Jul  5 02:28:25 MET DST 1997

  -----------------------------------------------------------------------*/


#include <stdlib.h>
#include "cio_basicparser.h"
#include "cio_scanner.h"


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: ParseBool()
//
//   Parse and return a Boolean value (true/false).
//
// Global Variables: -
//
// Side Effects    : Input
//
/----------------------------------------------------------------------*/

ScannerBoolResult ParseBool(Scanner_p in)
{
   ScannerBoolResult result;
   bool res = false;

   if (false == _CheckInpId(in, "true|false")) 
   {
      MAKE_ERR(result, res)
   }
   if(TestInpId(in, "true"))
   {
      res = true;
   }
   NextToken(in);

   MAKE_OK(result, res)
}


/*-----------------------------------------------------------------------
//
// Function: ParseIntMax()
//
//   Parses a (possibly negative) Integer, defined as an optional "-",
//   followed by a sequence of digits. Returns the value or gives an
//   error on overflow.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

ParseIntMaxResult ParseIntMax(Scanner_p in)
{
   ParseIntMaxResult result;
   intmax_t value = 0;

   if(TestInpTok(in, Hyphen))
   {
      NextToken(in);
      if (false == _CheckInpTokNoSkip(in, PosInt)) 
      {
         MAKE_ERR(result, value)
      }
      value = - strtoimax(DStrView(AktToken(in)->literal), NULL, 10);
   }
   else
   {
      if (false == _CheckInpTok(in, PosInt)) 
      {
         MAKE_ERR(result, value)
      }
      value = - strtoimax(DStrView(AktToken(in)->literal), NULL, 10);
   }
   NextToken(in);

   MAKE_OK(result, value)
}




/*-----------------------------------------------------------------------
//
// Function: ParseIntLimited()
//
//   Parses a (possibly negative) Integer, defined as an optional "-",
//   followed by a sequence of digits. Returns the value or gives an
//   error on overflow.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

ScannerLongResult ParseIntLimited(Scanner_p in, long lower, long upper)
{
   ScannerLongResult result;
   long value = 0;

   if(TestInpTok(in, Hyphen))
   {
      NextToken(in);
      if (false == _CheckInpTokNoSkip(in, PosInt)) 
      {
         MAKE_ERR(result, value)
      }
      if((AktToken(in)->numval-1) > LONG_MAX)
      {
         _CreateTokenError(in, "Long integer underflow", TokenError);
      }
      value = -AktToken(in)->numval;
   }
   else
   {
      if (false == _CheckInpTok(in, PosInt)) 
      {
         MAKE_ERR(result, value)
      }
      if(AktToken(in)->numval > LONG_MAX)
      {
         _CreateTokenError(in, "Long integer overflow", TokenError);
      }
      value = AktToken(in)->numval;
   }
   if(!((value >= lower) && (value <= upper)))
   {
      _CreateTokenError(in, "Long integer out of expected range", TokenError);
   }
   NextToken(in);

   MAKE_OK(result, value)
}



/*-----------------------------------------------------------------------
//
// Function: ParseInt()
//
//   Parses a (possibly negative) (long) Integer, defined as an
//   optional "-", //   followed by a sequence of digits. Returns the
//   value or gives an error on overflow.
//
// Global Variables: -
//
// Side Effects    : Input is read (and checked)
//
/----------------------------------------------------------------------*/

ScannerLongResult ParseInt(Scanner_p in)
{
   return ParseIntLimited(in, LONG_MIN, LONG_MAX);
}



/*-----------------------------------------------------------------------
//
// Function: ParseUIntMax()
//
//   Parses an uintmax-Integer, a sequence of digits. Returns the
//   value or gives an error on overflow.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

ParseUIntMaxResult ParseUIntMax(Scanner_p in)
{
   ParseUIntMaxResult result;
   uintmax_t value = 0;

   if (false == _CheckInpTok(in, PosInt)) 
   {
      MAKE_ERR(result, value)
   }

   value = AktToken(in)->numval;
   NextToken(in);

   MAKE_OK(result, value)
}



/*-----------------------------------------------------------------------
//
// Function: ParseFloat()
//
//   Parse a float in x.yEz format (optional negative and so on...)
//
// Global Variables: -
//
// Side Effects    : Input, may cause error
//
/----------------------------------------------------------------------*/

#ifndef ALLOW_COMMA_AS_DECIMAL_DOT
#define DECIMAL_DOT Fullstop
#else
#define DECIMAL_DOT Fullstop|Comma
#endif

ScannerDoubleResult ParseFloat(Scanner_p in)
{
   ScannerDoubleResult result;
   double value = 0.0;

   DStrReset(in->accu);

   if(TestInpTok(in, Hyphen|Plus))
   {
      DStrAppendDStr(in->accu, AktToken(in)->literal);
      NextToken(in);
      if (false == _CheckInpTokNoSkip(in, PosInt)) 
      {
         DStrReset(in->accu);
         MAKE_ERR(result, value)
      }
   }
   else
   {
      if (false == _CheckInpTok(in, PosInt)) 
      {
         DStrReset(in->accu);
         MAKE_ERR(result, value)
      }
   }
   DStrAppendDStr(in->accu, AktToken(in)->literal);
   NextToken(in);

   /* Parsed [-]123 so far */

   if(TestInpNoSkip(in)&&TestInpTok(in, DECIMAL_DOT))
   {
      DStrAppendChar(in->accu, '.');
      if (false == _ConsumeInpTokNoSkip(in, DECIMAL_DOT)) 
      {
         DStrReset(in->accu);
         MAKE_ERR(result, value)
      }
      DStrAppendDStr(in->accu,  AktToken(in)->literal);
      if (false == _ConsumeInpTokNoSkip(in, PosInt)) 
      {
         DStrReset(in->accu);
         MAKE_ERR(result, value)
      }
   }

   /* Parsed -123.1123 so far */

   if(TestInpNoSkip(in)&&TestInpId(in, "e|E"))
   {
      DStrAppendDStr(in->accu,  AktToken(in)->literal);
      NextToken(in); /* Skip E */
      DStrAppendDStr(in->accu,  AktToken(in)->literal);
      if (false == _ConsumeInpTokNoSkip(in, Hyphen|Plus)) /* Eat - */
      {
         DStrReset(in->accu);
         MAKE_ERR(result, value)
      }
      DStrAppendDStr(in->accu,  AktToken(in)->literal);
      if (false == _ConsumeInpTokNoSkip(in, PosInt)) 
      {
         DStrReset(in->accu);
         MAKE_ERR(result, value)
      }
   }
   errno = 0;
   value = strtod(DStrView(in->accu), NULL);

   if(errno)
   {
      TmpErrno = errno;
      DStrReset(in->accu);
      _CreateTokenError(in, "Cannot translate double", TokenError);
      MAKE_ERR(result, value)
   }

   MAKE_OK(result, value)
}


/*-----------------------------------------------------------------------
//
// Function: ParseNumString()
//
//   Parse a (possibly signed) number (Integer, Rational, or Float)
//   and return the most specific type compatible with it. The number
//   is not evaluated, but its ASCII representation is stored in
//   in->accu.
//
// Global Variables: -
//
// Side Effects    : Reads input, DStr handling may cause memory
//                   operations.
//
/----------------------------------------------------------------------*/

ParseNumStringResult ParseNumString(Scanner_p in)
{
   ParseNumStringResult result;
   StrNumType value = SNInteger;
   DStr_p accumulator = in->accu;

   DStrReset(accumulator);

   if(TestInpTok(in, Hyphen|Plus))
   {
      DStrAppendDStr(accumulator, AktToken(in)->literal);
      NextToken(in);
      if (false == _CheckInpTokNoSkip(in, PosInt)) 
      {
         DStrReset(accumulator);
         MAKE_ERR(result, value)
      }
   }
   else
   {
      if (false == _CheckInpTok(in, PosInt)) 
      {
         DStrReset(accumulator);
         MAKE_ERR(result, value)
      }
   }
   DStrAppendDStr(accumulator, AktToken(in)->literal);
   NextToken(in);

   if(TestInpTokNoSkip(in, Slash))
   {
      DStrAppendChar(accumulator, '/');
      NextToken(in);

      if(TestInpTok(in, Hyphen|Plus))
      {
         DStrAppendDStr(accumulator, AktToken(in)->literal);
         NextToken(in);
      }
      if(TestInpTok(in, PosInt))
      {
         if(atol(DStrView(AktToken(in)->literal)) == 0l)
         {
            DStrReset(accumulator);
            _CreateTokenError(in, "Denominator in rational number cannot be 0", TokenError);
            MAKE_ERR(result, value)
         }
      }
      DStrAppendDStr(accumulator,  AktToken(in)->literal);
      if (false == _ConsumeInpTokNoSkip(in, PosInt)) 
      {
         DStrReset(accumulator);
         MAKE_ERR(result, value)
      }

      value = SNRational;
   }
   else
   {
      if(TestInpTokNoSkip(in, DECIMAL_DOT) &&
         TestTok(LookToken(in,1), PosInt) &&
         !(LookToken(in,1)->skipped))
      {
         DStrAppendChar(accumulator, '.');
         if (false == _ConsumeInpTokNoSkip(in, DECIMAL_DOT)) 
         {
            DStrReset(accumulator);
            MAKE_ERR(result, value)
         }
         DStrAppendDStr(accumulator,  AktToken(in)->literal);
         if (false == _ConsumeInpTokNoSkip(in, PosInt)) 
         {
            DStrReset(accumulator);
            MAKE_ERR(result, value)
         }
         value = SNFloat;
      }
      if(TestInpNoSkip(in))
      {
         if(TestInpId(in, "e|E"))
         {
            DStrAppendStr(accumulator,  "e");
            NextToken(in); /* Skip E */
            DStrAppendDStr(accumulator,  AktToken(in)->literal);
            if (false == _ConsumeInpTokNoSkip(in, Hyphen|Plus)) /* Eat - */
            {
               DStrReset(accumulator);
               MAKE_ERR(result, value)
            }
            DStrAppendDStr(accumulator,  AktToken(in)->literal);
            if (false == _ConsumeInpTokNoSkip(in, PosInt)) 
            {
               DStrReset(accumulator);
               MAKE_ERR(result, value)
            }
            value = SNFloat;
         }
         else if(TestInpIdnum(in, "e|E"))
         {
            DStrAppendDStr(accumulator,  AktToken(in)->literal);
            if (false == _ConsumeInpTokNoSkip(in, Idnum)) 
            {
               DStrReset(accumulator);
               MAKE_ERR(result, value)
            }
            value = SNFloat;
         }
      }
   }
   
   MAKE_OK(result, value)
}


/*-----------------------------------------------------------------------
//
// Function: DDArrayParse()
//
//   Parse a coma-delimited list of double values into array. If
//   brackets is true, expect the list to be enclosed into (). Return
//   the number of values parsed.
//
// Global Variables: -
//
// Side Effects    : Reads input
//
/----------------------------------------------------------------------*/

ScannerLongResult DDArrayParse(Scanner_p in, DDArray_p array, bool brackets)
{
   ScannerLongResult result;
   long i=0;
   ScannerDoubleResult temp;

   if(brackets)
   {
      if (false == _ConsumeInpTok(in, OpenBracket)) 
      {
         MAKE_ERR(result, i)
      }
   }

   if(TestInpTok(in, Hyphen|Plus|PosInt))
   {
      temp = ParseFloat(in);
      if (IS_ERR(temp)) 
      {
         MAKE_ERR(result, i)
      }

      DDArrayAssign(array, i, temp.ret);
      i++;

      while(TestInpTok(in, Comma))
      {
         NextToken(in); /* We know it's a comma */

         temp = ParseFloat(in);
         if (IS_ERR(temp)) 
         {
            MAKE_ERR(result, i)
         }

         DDArrayAssign(array, i, temp.ret);
         i++;
      }
   }
   if(brackets)
   {
      if (false == _ConsumeInpTok(in, CloseBracket)) 
      {
         MAKE_ERR(result, i)
      }
   }

   MAKE_OK(result, i)
}


/*-----------------------------------------------------------------------
//
// Function: ParseFilename()
//
//   Parse a filename and return
//   it. Note that we only allow reasonably "normal" filenames or
//   strings, i.e. not spaces, non-printables, most meta-charachters,
//   or quotes.
//
// Global Variables: -
//
// Side Effects    : Read input, allocates space (which must be freed
//                   by the caller)
//
/----------------------------------------------------------------------*/

#define PLAIN_FILE_TOKENS String|Name|PosInt|Fullstop|Plus|Hyphen|EqualSign

ScannerCharPResult ParseFilename(Scanner_p in)
{
   ScannerCharPResult result;
   bool first_tok = true;

   DStrReset(in->accu);

   while((first_tok || TestInpNoSkip(in)) &&
         TestInpTok(in, PLAIN_FILE_TOKENS|Slash|Mult))
   {
      DStrAppendDStr(in->accu, AktToken(in)->literal);
      NextToken(in);
      first_tok = false;
   }

   MAKE_OK(result, SecureStrdup(DStrView(in->accu)))
}


/*-----------------------------------------------------------------------
//
// Function: ParsePlainFileName()
//
//   Parse a local file name (without /) and return it. The caller has
//   to free the allocated memory!
//
// Global Variables: -
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

ScannerCharPResult ParsePlainFilename(Scanner_p in)
{
   ScannerCharPResult result;
   bool first_tok = true;
   DStrReset(in->accu);

   while((first_tok || TestInpNoSkip(in)) &&
         TestInpTok(in, PLAIN_FILE_TOKENS|Slash))
   {
      DStrAppendDStr(in->accu, AktToken(in)->literal);
      NextToken(in);
      first_tok = false;
   }

   MAKE_OK(result, SecureStrdup(DStrView(in->accu)))
}

/*-----------------------------------------------------------------------
//
// Function: ParseBasicInclude()
//
//   Parse a basic TPTP-3 include (without optional selector),
//   return the file name (which the caller has to free).
//
// Global Variables: -
//
// Side Effects    : Input, memory allocation.
//
/----------------------------------------------------------------------*/

ScannerCharPResult ParseBasicInclude(Scanner_p in)
{
   ScannerCharPResult result;
   char* value = NULL;

   if (false == _ConsumeInpId(in, "include")) 
   {
      MAKE_ERR(result, value)
   }
   
   if (false == _ConsumeInpTok(in, OpenBracket)) 
   {
      MAKE_ERR(result, value)
   }
   
   if (false == _CheckInpTok(in, SQString)) 
   {
      MAKE_ERR(result, value)
   }
   value = DStrCopyCore(AktToken(in)->literal);
   NextToken(in);

   if (false == _ConsumeInpTok(in, CloseBracket)) 
   {
      FREE(value);
      value = NULL;
      MAKE_ERR(result, value)
   }
   
   if (false == _ConsumeInpTok(in, Fullstop)) 
   {
      FREE(value);
      value = NULL;
      MAKE_ERR(result, value)
   }

   MAKE_OK(result, value)
}


/*-----------------------------------------------------------------------
//
// Function: ParseDottedId()
//
//   Parse a sequence id1.id2.id2 ... and return it as a string.
//
// Global Variables: -
//
// Side Effects    : Input, memory ops.
//
/----------------------------------------------------------------------*/

ScannerCharPResult ParseDottedId(Scanner_p in)
{
   ScannerCharPResult result;
   DStrReset(in->accu);

   DStrAppendDStr(in->accu, AktToken(in)->literal);
   if (false == _ConsumeInpTok(in, Identifier|PosInt)) 
   {
      DStrReset(in->accu);
      MAKE_ERR(result, NULL)
   }

   while(TestInpNoSkip(in) && TestInpTok(in, Fullstop))
   {
      DStrAppendDStr(in->accu, AktToken(in)->literal);
      if (false == _ConsumeInpTok(in, Fullstop))   
      {
         DStrReset(in->accu);
         MAKE_ERR(result, NULL)
      }
      
      DStrAppendDStr(in->accu, AktToken(in)->literal);
      if (false == _ConsumeInpTok(in, Identifier|PosInt))
      {
         DStrReset(in->accu);
         MAKE_ERR(result, NULL)
      }
   }

   MAKE_OK(result, SecureStrdup(DStrView(in->accu)))
}


/*-----------------------------------------------------------------------
//
// Function: AcceptDottedId()
//
//   Parse a sequence id1.id2.id2..., check it against an expected
//   value, and skip it. Print error and terminate on mismatch.
//
// Global Variables: -
//
// Side Effects    : Input, memory ops.
//
/----------------------------------------------------------------------*/

bool _ConsumeDottedId(Scanner_p in, char* expected)
{
   char* posrep = TokenPosRep(AktToken(in));
   ScannerCharPResult temp = ParseDottedId(in);

   if (IS_ERR(temp)) 
   {
      return false;
   }

   if(strcmp(temp.ret, expected)!=0)
   {
      DStrReset(in->accu);
      DStrAppendStr(in->accu, posrep);
      DStrAppendStr(in->accu, " ");
      DStrAppendStr(in->accu, expected);
      DStrAppendStr(in->accu, " expected, but ");
      DStrAppendStr(in->accu, temp.ret);
      DStrAppendStr(in->accu, " read");

      FREE(temp.ret);
      _ParseError(in, DStrView(in->accu), TokenError);
      return false;
   }

   FREE(temp.ret);
   return true;
}


/*-----------------------------------------------------------------------
//
// Function: ParseContinous()
//
//   Parse a sequence of tokens with no whitespace and return the
//   result as a string.
//
// Global Variables: -
//
// Side Effects    : Input, memory ops.
//
/----------------------------------------------------------------------*/

ScannerCharPResult ParseContinous(Scanner_p in)
{
   ScannerCharPResult result;
   DStrReset(in->accu);

   DStrAppendDStr(in->accu, AktToken(in)->literal);
   NextToken(in);

   while(TestInpNoSkip(in))
   {
      DStrAppendDStr(in->accu, AktToken(in)->literal);
      NextToken(in);
   }

   MAKE_OK(result, SecureStrdup(DStrView(in->accu)))
}





/*-----------------------------------------------------------------------
//
// Function: ParseSkipParenthesizedExpr()
//
//   Skip any expression containing balanced (), [], {}. Print error
//   on missmatch. Note that no full syntax check is performed, we are
//   only interested in the different braces.
//
// Global Variables: -
//
// Side Effects    : Input
//
/----------------------------------------------------------------------*/

bool _ParseSkipParenthesizedExpr(Scanner_p in)
{
   PStack_p paren_stack = PStackAlloc();
   TokenType tok;

   if (false == _CheckInpTok(in, OpenBracket|OpenCurly|OpenSquare)) 
   {
      PStackFree(paren_stack);
      return false;
   }

   PStackPushInt(paren_stack, AktTokenType(in));
   NextToken(in);
   while(!PStackEmpty(paren_stack))
   {
      if(TestInpTok(in, OpenBracket|OpenCurly|OpenSquare))
      {
         PStackPushInt(paren_stack, AktTokenType(in));
         NextToken(in);
      }
      else if(TestInpTok(in, CloseBracket|CloseCurly|CloseSquare))
      {
         tok = PStackPopInt(paren_stack);
         switch(tok)
         {
         case OpenBracket:
               if (false == _ConsumeInpTok(in,CloseBracket)) 
               {
                  PStackFree(paren_stack);
                  return false;
               }
               break;
         case OpenCurly:
               if (false == _ConsumeInpTok(in, CloseCurly)) 
               {
                  PStackFree(paren_stack);
                  return false;
               }
               break;
         case OpenSquare:
               if (false == _ConsumeInpTok(in,CloseSquare)) 
               {
                  PStackFree(paren_stack);
                  return false;
               }
               break;
         default:
               assert(false && "Impossible value on parentheses stack");
               break;
         }
      }
      else
      {
         NextToken(in);
      }
   }
   PStackFree(paren_stack);
   return true;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
