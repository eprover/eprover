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
   more than one particular token (see e.g. SkipToken). */


/* All Tokens are defined using X-macros (https://en.wikipedia.org/wiki/X_macro). 
   T is T(token, value, description) where:
      token: Name of token.
      value: Numeric value of token.
      description: Description of token used in token_print_rep.
                   If no description is needed, use NULL. */
typedef unsigned long long TokenType;

#define TOKENS \
   T(NoToken       , 1LL,                  "No token (probably EOF)") \
   T(WhiteSpace    , 2*NoToken,            "White space (spaces, tabs, newlines...)") \
   T(Comment       , 2*WhiteSpace,         "Comment") \
   T(Ident         , 2*Comment,            "Identifier not terminating in a number") \
   T(Idnum         , 2*Ident,              "Identifier terminating in a number") \
   T(SemIdent      , 2*Idnum,              "Interpreted function/predicate name ('$name')") \
   T(String        , 2*SemIdent,           "String enclosed in double quotes (\"\")") \
   T(SQString      , 2*String,             "String enclosed in single quote ('')") \
   T(PosInt        , 2*SQString,           "Integer (sequence of decimal digits) convertible to an 'unsigned long'") \
   T(OpenBracket   , 2*PosInt,             "Opening bracket ('(')") \
   T(CloseBracket  , 2*OpenBracket,        "Closing bracket (')')") \
   T(OpenCurly     , 2*CloseBracket,       "Opening curly brace ('{')") \
   T(CloseCurly    , 2*OpenCurly,          "Closing curly brace ('}')") \
   T(OpenSquare    , 2*CloseCurly,         "Opening square brace ('[')") \
   T(CloseSquare   , 2*OpenSquare,         "Closing square brace (']')") \
   T(LesserSign    , 2*CloseSquare,        "\"Lesser than\" sign ('<')") \
   T(GreaterSign   , 2*LesserSign,         "\"Greater than\" sign ('>')") \
   T(EqualSign     , 2*GreaterSign,        "Equal Predicate/Sign ('=')") \
   T(NegEqualSign  , 2*EqualSign,          "Negated Equal Predicate ('!=')") \
   T(TildeSign     , 2*NegEqualSign,       "Tilde ('~')") \
   T(Exclamation   , 2*TildeSign,          "Exclamation mark ('!')") \
   T(UnivQuantor   , Exclamation,          NULL) \
   T(QuestionMark  , 2*Exclamation,        "Question mark ('?')") \
   T(ExistQuantor  , QuestionMark,         NULL) \
   T(Comma         , 2*QuestionMark,       "Comma (',')") \
   T(Semicolon     , 2*Comma,              "Semicolon (';')") \
   T(Colon         , 2*Semicolon,          "Colon (':')") \
   T(Hyphen        , 2*Colon,              "Hyphen ('-')") \
   T(Plus          , 2*Hyphen,             "Plus sign ('+')") \
   T(Mult          , 2*Plus,               "Multiplication sign ('*')") \
   T(Fullstop      , 2*Mult,               "Fullstop ('.')") \
   T(Dollar        , 2*Fullstop,           "Dollar sign ('$')") \
   T(Slash         , 2*Dollar,             "Slash ('/')") \
   T(Pipe          , 2*Slash,              "Vertical bar ('|')") \
   T(FOFOr         , Pipe,                 NULL) \
   T(Ampersand     , 2*Pipe,               "Ampersand ('&')") \
   T(FOFAnd        , Ampersand,            NULL) \
   T(FOFLRImpl     , 2*Ampersand,          "Implication/LRArrow ('=>')") \
   T(FOFRLImpl     , 2*FOFLRImpl,          "Back Implicatin/RLArrow ('<=')") \
   T(FOFEquiv      , 2*FOFRLImpl,          "Equivalence/Double arrow ('<=>')") \
   T(FOFXor        , 2*FOFEquiv,           "Negated Equivalence/Xor ('<~>')") \
   T(FOFNand       , 2*FOFXor,             "Nand ('~&')") \
   T(FOFNor        , 2*FOFNand,            "Nor ('~|')") \
   T(Application   , 2*FOFNor,             "Application ('@')") \
   T(Carret        , 2*Application,        "Lambda ('^')") \
   T(LambdaQuantor , Carret,               NULL) \
   T(LetToken      , 2*LambdaQuantor,      "Let ('$let')") \
   T(IteToken      , 2*LetToken,           "Ite ('$ite')") \
   T(ErrorToken    , 2*IteToken,           NULL) \
   T(SkipToken     , WhiteSpace | Comment, NULL) \
   T(Identifier    , Ident | Idnum,        NULL) \
   T(Name          , Identifier | String,  NULL) \
   T(FOFBinOp      , FOFAnd|FOFOr|FOFLRImpl|FOFRLImpl|FOFEquiv|FOFXor|FOFNand|FOFNor|EqualSign|NegEqualSign, NULL) \
   T(FOFAssocOp    , FOFAnd|FOFOr, NULL) 

/* Generate the tokens as const values. */
#define T(token, value, description) static const TokenType token = value;
TOKENS
#undef T

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
   DStr_p        literal;      /* Verbatim copy of input for the token  */
   uintmax_t     numval;      /* Numerical value (if any) of the token */
   DStr_p        comment;     /* Accumulated preceding comments        */
   bool          skipped;     /* Was this token preceded by SkipSpace? */
   DStr_p        source;      /* Ref. to the input stream source       */
   StreamType    stream_type; /* File or string? */
   long          line;        /* Position in this stream               */
   long          column;      /*  "               "                    */

}TokenCell, *Token_p;

typedef enum 
{
   TokenError,
   TokenWarning,
   TokenSysError
}TokenErrorType;

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
   bool        panic_mode;
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

void _CreateTokenError(Scanner_p in, char* msg, TokenErrorType error_type);
void _CreateTokenWarning(Scanner_p in, char* msg);
void _ParseError(Scanner_p in, char* msg, TokenErrorType error_type);

bool _CheckInpTok(Scanner_p in, TokenType toks);
bool _CheckInpTokNoSkip(Scanner_p in, TokenType toks);
bool _CheckInpId(Scanner_p in, char* ids);
bool _ConsumeInpTok(Scanner_p in, TokenType toks);
bool _ConsumeInpTokNoSkip(Scanner_p in, TokenType toks);
bool _ConsumeInpId(Scanner_p in, char* ids);

void NextToken(Scanner_p in);

#define DEFINE_RESULT(name, return_type) \
typedef struct \
{ \
   bool result; \
   return_type ret; \
} name

#define ERR(r, v) r.result = false; r.ret = v
#define MAKE_ERR(r, v) ERR(r, v); return r;
#define IS_ERR(r) false == r.result

#define OK(r, v) r.result = true; r.ret = v
#define MAKE_OK(r, v) OK(r, v); return r;
#define IS_OK(r) false != r.result

/* Some basic result types. */
DEFINE_RESULT(ScannerBoolResult, bool);
DEFINE_RESULT(ScannerLongResult, long);
DEFINE_RESULT(ScannerDoubleResult, double);
DEFINE_RESULT(ScannerCharPResult, char*);

DEFINE_RESULT(ScannerParseIncludeResult, Scanner_p);
ScannerParseIncludeResult ScannerParseInclude(Scanner_p in, 
                                              StrTree_p *name_selector,
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
