/*-----------------------------------------------------------------------

File  : cio_scanner.h

Author: Stephan Schulz

Contents

  Datatypes for the scanner: TokenType, TokenCell, TokenRepCell

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Thu Aug 28 01:48:03 MET DST 1997
    New

-----------------------------------------------------------------------*/

#ifndef CIO_SCANNER

#define CIO_SCANNER

#include <cio_streams.h>
#include <clb_stringtrees.h>
#include <ctype.h>
#include <limits.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


/* Possible token type. This used to be a nice enum, but that
   restriced the set to no more than 32 distinct tokens. As a n
   unsigned long long, we can accomodate at least 64 tokens.

   Note that it is possible to define additional classes consisting of
   more than one particular token (see e.g. SkipToken). If this list
   is extended, you also need to extend token_print_rep[] in
   cio_scanner.c. */

typedef unsigned long long TokenType;

#define NoToken       1LL
#define WhiteSpace    (2*NoToken)
#define Comment       (2*WhiteSpace)
#define Ident         (2*Comment)
#define Idnum         (2*Ident)
#define SemIdent      (2*Idnum)
#define String        (2*SemIdent)
#define SQString      (2*String)
#define PosInt        (2*SQString)
#define OpenBracket   (2*PosInt)
#define CloseBracket  (2*OpenBracket)
#define OpenCurly     (2*CloseBracket)
#define CloseCurly    (2*OpenCurly)
#define OpenSquare    (2*CloseCurly)
#define CloseSquare   (2*OpenSquare)
#define LesserSign    (2*CloseSquare)
#define GreaterSign   (2*LesserSign)
#define EqualSign     (2*GreaterSign)
#define NegEqualSign  (2*EqualSign)
#define TildeSign     (2*NegEqualSign)
#define Exclamation   (2*TildeSign)
#define UnivQuantor   (Exclamation)
#define QuestionMark  (2*Exclamation)
#define ExistQuantor  (QuestionMark)
#define Comma         (2*QuestionMark)
#define Semicolon     (2*Comma)
#define Colon         (2*Semicolon)
#define Hyphen        (2*Colon)
#define Plus          (2*Hyphen)
#define Mult          (2*Plus)
#define Fullstop      (2*Mult)
#define Dollar        (2*Fullstop)
#define Slash         (2*Dollar)
#define Pipe          (2*Slash)
#define FOFOr         (Pipe)
#define Ampersand     (2*Pipe)
#define FOFAnd        (Ampersand)
#define FOFLRImpl     (2*Ampersand)
#define FOFRLImpl     (2*FOFLRImpl)
#define FOFEquiv      (2*FOFRLImpl)
#define FOFXor        (2*FOFEquiv)
#define FOFNand       (2*FOFXor)
#define FOFNor        (2*FOFNand)
#define Application   (2*FOFNor)
#define Carret        (2*Application)
#define LambdaQuantor (Carret)
#define LetToken      (2*LambdaQuantor)
#define IteToken      (2*LetToken)


#define SkipToken     (WhiteSpace | Comment)
#define Identifier    (Ident | Idnum)
#define Name          (Identifier | String)
//                                                                                     FOOL additions
#define FOFBinOp      (FOFAnd|FOFOr|FOFLRImpl|FOFRLImpl|FOFEquiv|FOFXor|FOFNand|FOFNor|EqualSign|NegEqualSign)
#define FOFAssocOp    (FOFAnd|FOFOr)



/* If your application parses multiple format you can use this to
   distinguish them: */

typedef enum
{
   LOPFormat,
   TPTPFormat,
   TSTPFormat,
   AutoFormat
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
   uintmax_t     numval;      /* Numerical value (if any) of the token */
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
   DStr_p      default_dir; /* Directory we read from, if any */
   IOFormat    format;
   DStr_p      accu; /* Place for Multi-Token constructs or messages */
   bool        ignore_comments; /* Comments can be skipped
                                 * completely. If not set, comments
                                 * are accumulated (but never
                                 * delivered as tokens) */
   char*       include_key; /* An Identifier,  e.g. "include" */
   TokenCell   tok_sequence[MAXTOKENLOOKAHEAD]; /* Need help? Bozo! */
   int         current; /* Pointer to current token in tok_sequence */
   char*       include_pos; /* If created by "include", by which one? */
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

#define  ScannerGetFormat(scanner)        ((scanner)->format)

#define  ScannerGetDefaultDir(scanner) DStrView((scanner)->default_dir)

#define isstartidchar(ch)  (isalpha(ch) || (ch) == '_')
#define isidchar(ch)       (isalnum(ch) || (ch) == '_')
#define ischar(ch)         ((ch)!=EOF)
#define isstartcomment(ch) ((ch)=='#' || (ch)=='%')

char*     PosRep(StreamType type, DStr_p file, long line, long column);
char*     TokenPosRep(Token_p token);
char*     DescribeToken(TokenType token);
void      PrintToken(FILE* out, Token_p token);

Scanner_p CreateScanner(StreamType type, char *name, bool
                        ignore_comments, char *default_dir, bool fail);
void      DestroyScanner(Scanner_p  junk);

void      ScannerSetFormat(Scanner_p scanner, IOFormat fmt);


#define TOKENREALPOS(pos) ((pos) % MAXTOKENLOOKAHEAD)
#define AktToken(in) (&((in)->tok_sequence[(in)->current]))
#define AktTokenType(in) (AktToken(in)->tok)
#define LookToken(in,look) \
    (&((in)->tok_sequence[TOKENREALPOS((in)->current+(look))]))

bool TestTok(Token_p akt, TokenType toks);
bool TestId(Token_p akt, char* ids);
bool TestIdnum(Token_p akt, char* ids);

#define TestInpTok(in, toks)  TestTok(AktToken(in), (toks))
#define TestInpId(in, ids)    TestId(AktToken(in), (ids))
#define TestInpIdnum(in, ids) TestIdnum(AktToken(in), (ids))
#define TestInpNoSkip(in)     (!(AktToken(in)->skipped))
#define TestInpTokNoSkip(in, toks) \
        (TestInpNoSkip(in) && TestInpTok(in, toks))

void AktTokenError(Scanner_p in, char* msg, bool syserr);
void AktTokenWarning(Scanner_p in, char* msg);

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

Scanner_p ScannerParseInclude(Scanner_p in, StrTree_p *name_selector,
                              StrTree_p *skip_includes);

#ifdef ENABLE_LFHO
#define PARSE_OPTIONAL_AV_PENALTY(in, var_name) \
if(TestInpTok((in), Comma)) \
{ \
   AcceptInpTok((in), Comma); \
   var_name = ParseFloat((in)); \
}
#else
#define PARSE_OPTIONAL_AV_PENALTY(in, var_name) /* relax */
#endif

#define APP_VAR_MULT_DEFAULT 1

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
