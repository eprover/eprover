/*-----------------------------------------------------------------------

File  : cio_scanner.h

Author: Stephan Schulz

Contents
 
  Datatypes for the scanner: TokenType, TokenCell, TokenRepCell

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Thu Aug 28 01:48:03 MET DST 1997
    New

-----------------------------------------------------------------------*/

#ifndef CIO_SCANNER

#define CIO_SCANNER

#include "cio_streams.h"
#include <ctype.h>
#include <limits.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


/* Possible token. There can be no more than 32 distinct tokens
   (although it is possible to define additional classes consisting of
   more than one particular token (see e.g. SkipToken). If this list
   is extended, you also need to extend token_print_rep[] in
   cio_scanner.c. */

typedef enum
{
   NoToken      = 1,
   WhiteSpace   = 2*NoToken,
   Comment      = 2*WhiteSpace,
   SkipToken    = WhiteSpace | Comment,
   Ident        = 2*Comment,
   Idnum        = 2*Ident,
   Identifier   = Ident | Idnum,
   String       = 2*Idnum,
   Name         = Identifier | String,
   PosInt       = 2*String,
   OpenBracket  = 2*PosInt,
   CloseBracket = 2*OpenBracket,
   OpenCurly    = 2*CloseBracket,
   CloseCurly   = 2*OpenCurly,
   OpenSquare   = 2*CloseCurly,
   CloseSquare  = 2*OpenSquare,
   LesserSign   = 2*CloseSquare,
   GreaterSign  = 2*LesserSign,
   EqualSign    = 2*GreaterSign,
   TildeSign    = 2*EqualSign,
   Exclamation  = 2*TildeSign,
   AllQuantor   = Exclamation,
   QuestionMark = 2*Exclamation,
   ExistQuantor = QuestionMark,
   Comma        = 2*QuestionMark,
   Semicolon    = 2*Comma,
   Colon        = 2*Semicolon,
   Hyphen       = 2*Colon,
   Plus         = 2*Hyphen,
   Mult         = 2*Plus,
   Fullstop     = 2*Mult,
   Dollar       = 2*Fullstop,
   Pipe         = 2*Dollar,
   Slash        = 2*Pipe,
   Ampersand    = 2*Slash
}TokenType;


/* If your application parses multiple format you can use this to
   distinguish them: */

typedef enum 
{
   LOPFormat,
   TPTPFormat,
   TSTPFormat,
}IOFormat;


typedef struct tokenrepcell
{
   TokenType key;
   char*     rep;
}TokenRepCell, *TokenRep_p;


typedef struct tokencell
{
   TokenType     tok;         /* Type for AcceptTok(), TestTok() ...   */
   DStr_p        literal;     /* Verbatim copy of input for the token  */
   unsigned long numval;      /* Numerical value (if any) of the token */
   DStr_p        comment;     /* Accumulated preceding comments        */
   bool          skipped;     /* Was this token preceded by SkipSpace? */
   DStr_p        source;      /* Ref. to the input stream source       */   
   StreamType    stream_type; /* File or string? */
   long          line;        /* Position in this stream               */
   long          column;      /*  "               "                    */
   
}TokenCell, *Token_p;

#define MAXTOKENLOOKAHEAD 4

typedef struct scannercell
{
   Stream_p    source;  /* Input stack from which to read */
   IOFormat    format;
   DStr_p      accu; /* Place for Multi-Token constructs or messages */
   bool        ignore_comments; /* Comments can be skipped completely */
   char*       include_key; /* An Identifier,  e.g. "include" */
   TokenCell   tok_sequence[MAXTOKENLOOKAHEAD]; /* Need help? Bozo! */
   int         current; /* Pointer to current token in tok_sequence */
}ScannerCell, *Scanner_p;

    
/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define TokenCellAlloc()      (TokenCell*)SizeMalloc(sizeof(TokenCell))
#define TokenCellFree(junk)   SizeFree(junk, sizeof(TokenCell))
#define ScannerCellAlloc()    (ScannerCell*)SizeMalloc(sizeof(ScannerCell))
#define ScannerCellFree(junk) SizeFree(junk, sizeof(ScannerCell))

#define  Source(scanner)       (((scanner)->source)->source)
#define  SourceType(scanner)   (((scanner)->source)->stream_type)
#define  LookChar(scanner, look) StreamLookChar((scanner)->source, look)
#define  CurrChar(scanner)       StreamCurrChar((scanner)->source)
#define  CurrLine(scanner)       StreamCurrLine((scanner)->source)
#define  CurrColumn(scanner)     StreamCurrColumn((scanner)->source)
#define  NextChar(scanner)       StreamNextChar((scanner)->source)

#define  ScannerSetFormat(scanner, fmt) ((scanner)->format = (fmt))
#define  ScannerGetFormat(scanner)        ((scanner)->format)

#define isstartidchar(ch)  (isalpha(ch) || (ch) == '_')
#define isidchar(ch)       (isalnum(ch) || (ch) == '_')
#define ischar(ch)         ((ch)!=EOF)
#define isstartcomment(ch) ((ch)=='#' || (ch)=='%')

char*     PosRep(StreamType type, DStr_p file, long line, long column);
char*     TokenPosRep(Token_p token);
char*     DescribeToken(TokenType token);
void      PrintToken(FILE* out, Token_p token);

Scanner_p CreateScanner(StreamType type, char* name, bool
			ignore_comments, char* include_key);
void      DestroyScanner(Scanner_p  junk);

#define TOKENREALPOS(pos) ((pos) % MAXTOKENLOOKAHEAD)
#define AktToken(in) (&((in)->tok_sequence[(in)->current]))
#define LookToken(in,look) \
    (&((in)->tok_sequence[TOKENREALPOS((in)->current+(look))]))

bool TestTok(Token_p akt, TokenType toks);
bool TestId(Token_p akt, char* ids);
bool TestIdnum(Token_p akt, char* ids);

#define TestInpTok(in, toks)  TestTok(AktToken(in), (toks))
#define TestInpId(in, ids)    TestId(AktToken(in), (ids))
#define TestInpIdnum(in, ids) TestIdnum(AktToken(in), (ids))
#define TestInpNoSkip(in)     (!(AktToken(in)->skipped))

VOLATILE void AktTokenError(Scanner_p in, char* msg, bool syserr);

void CheckInpTok(Scanner_p in, TokenType toks);
void CheckInpTokNoSkip(Scanner_p in, TokenType toks);
void CheckInpId(Scanner_p in, char* ids);


#define AcceptInpTok(in, toks) CheckInpTok((in), (toks));\
                               NextToken(in)
#define AcceptInpTokNoSkip(in, toks) \
                               CheckInpTokNoSkip((in), (toks));\
                               NextToken(in)
#define AcceptInpId(in, ids)   CheckInpId((in), (ids));\
                               NextToken(in)

void NextToken(Scanner_p in);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





