/*-----------------------------------------------------------------------

File  : cio_scanner.c

Author: Stephan Schulz

Contents
 
  Implementation of the scanner.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Sun Aug 31 13:31:42 MET DST 1997
    New

-----------------------------------------------------------------------*/

#include "cio_scanner.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

static TokenRepCell token_print_rep[] =
{
   {NoToken,      "No token (probably EOF)"},
   {WhiteSpace,   "White space (spaces, tabs, newlines...)"},
   {Comment,      "Comment"},
   {Ident,        "Identifier not terminating in a number"},
   {Idnum,        "Identifier terminating in a number"},
   {String,       "String enclosed in \"\" or ''"},
   {PosInt,       "Integer (sequence of digits)"},
   {OpenBracket,  "Opening bracket ('(')"},
   {CloseBracket, "Closing bracket (')')"},
   {OpenCurly,    "Opening curly brace ('{')"},
   {CloseCurly,   "Closing curly brace ('}')"},
   {OpenSquare,   "Opening square brace ('[')"},
   {CloseSquare,  "Closing square brace (']')"},
   {LesserSign,   "\"Lesser than\" sign ('<')"},
   {GreaterSign,  "\"Greater than\" sign ('>')"},
   {EqualSign,    "Equal sign ('=')"},
   {TildeSign,    "Tilde ('~')"},
   {Exclamation,  "Exclamation mark ('!')"},
   {QuestionMark, "Question mark ('?')"},
   {Comma,        "Comma (',')"},
   {Semicolon,    "Semicolon (';')"},
   {Colon,        "Colon (':')"},
   {Hyphen,       "Hyphen ('-')"},
   {Plus,         "Plus sign ('+')"},
   {Mult,         "Multiplication sign ('*')"},
   {Fullstop,     "Fullstop ('.')"},
   {Dollar,       "Dollar sign ('$')"},
   {Pipe,         "Vertical bar ('|')"},
   {Slash,        "Slash ('/')"},
   {Ampersand,    "Ampersand ('&')"},
   {NoToken,      NULL}

};

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: scan_white()
//
//   Scan a continous sequence of white space characters. 
//
// Global Variables: -
//
// Side Effects    : Reads input, changes the scanner state, may cause
//                   memory operations by by calling DStrAppendChar().
//
/----------------------------------------------------------------------*/

static void scan_white(Scanner_p in)
{
   AktToken(in)->tok = WhiteSpace;
   while(isspace(CurrChar(in)))
   {
      DStrAppendChar(AktToken(in)->literal, CurrChar(in));
      NextChar(in);
   }
}


/*-----------------------------------------------------------------------
//
// Function: scan_ident()
//
//   Scan an identifier, d.h. an ident or an idnum.
//
// Global Variables: -
//
// Side Effects    : As for scan_white()
//
/----------------------------------------------------------------------*/

static void scan_ident(Scanner_p in)
{
   long numstart = 0,
        i;
   
   for(i=0; isidchar(CurrChar(in)); i++)
   {
      if(!numstart && isdigit(CurrChar(in)))
      {
	 numstart = i;
      }
      else if(!isdigit(CurrChar(in)))
      {
	 numstart = 0;
      }
      DStrAppendChar(AktToken(in)->literal, CurrChar(in));
      NextChar(in);
   }
   if(numstart)
   {
      AktToken(in)->tok = Idnum;
      AktToken(in)->numval =
	 strtol(DStrView(AktToken(in)->literal)+numstart, NULL, 10);
      /* Errors are intentionally ignored to allow arbitrary
	 identifiers */
   }
   else
   {
      AktToken(in)->tok = Ident;
      AktToken(in)->numval = 0;
   }      
}


/*-----------------------------------------------------------------------
//
// Function: void scan_int()
//
//   Scan an unsigned integer, i.e. a sequence of digits. 
//
// Global Variables: -
//
// Side Effects    : As for scan_white(), error if int is to big for
//                   unsigned long 
//
/----------------------------------------------------------------------*/

static void scan_int(Scanner_p in)
{
   AktToken(in)->tok = PosInt;

   while(isdigit( CurrChar(in)))
   {
      DStrAppendChar(AktToken(in)->literal, CurrChar(in));
      NextChar(in);
   }
   errno = 0;
   AktToken(in)->numval =
      strtol(DStrView(AktToken(in)->literal), NULL, 10);
   /* strtoul is not available on all systems....*/

   if(errno)
   {
      TmpErrno = errno;
      
      AktTokenError(in, "Cannot translate integer", true);
   }
}


/*-----------------------------------------------------------------------
//
// Function: scan_line_comment()
//
//   Scan a comment starting with # or %.
//
// Global Variables: -
//
// Side Effects    : As scan_white()
//
/----------------------------------------------------------------------*/

static void scan_line_comment(Scanner_p in)
{
   AktToken(in)->tok = Comment;

   while(CurrChar(in) != '\n')
   {
      DStrAppendChar(AktToken(in)->literal, CurrChar(in));
      NextChar(in);
   }
   DStrAppendChar(AktToken(in)->literal, CurrChar(in));
   NextChar(in);
}


/*-----------------------------------------------------------------------
//
// Function: void scan_C_comment()
//
//   Scan a comment in C-Style.
//
// Global Variables: -
//
// Side Effects    : As scan_white()
//
/----------------------------------------------------------------------*/

static void scan_C_comment(Scanner_p in)
{   
   AktToken(in)->tok = Comment;
   
   while(!((CurrChar(in) == '*') && (LookChar(in,1) == '/')))
   {
      DStrAppendChar(AktToken(in)->literal, CurrChar(in));
      NextChar(in);
   }
   DStrAppendChar(AktToken(in)->literal, CurrChar(in));
   NextChar(in);
   DStrAppendChar(AktToken(in)->literal, CurrChar(in));
   NextChar(in);
}


/*-----------------------------------------------------------------------
//
// Function: scan_string()
//
//   Scan a string (enclosed in ""). Only the value of the strings,
//   not the delimiter is stored.
//
// Global Variables: -
//
// Side Effects    : As scan_white()
//
/----------------------------------------------------------------------*/

static void scan_string(Scanner_p in, char delim)
{
   AktToken(in)->tok = String;
   
   DStrAppendChar(AktToken(in)->literal, CurrChar(in));
   NextChar(in);
   while(CurrChar(in) != delim)
   {
      if(!isprint(CurrChar(in)))
      {
	 AktTokenError(in, 
		       "Non-printable character in string constant",
		       false);
      }
      DStrAppendChar(AktToken(in)->literal, CurrChar(in));
      NextChar(in);
   }
   DStrAppendChar(AktToken(in)->literal, CurrChar(in));
   NextChar(in);
}


/*-----------------------------------------------------------------------
//
// Function: scan_token()
//
//   Scans a token into AktToken(in). Does _not_ move the
//   AktToken-pointer - this is done only for real (i.e. non white,
//   non-comment) tokens in the function NextToken().
//   The function assumes that *AktToken(in) is an initialized
//   TokenCell which does not contain any outside references.
//
//
// Global Variables: -
//
// Side Effects    : Reads input, may cause memory operations via
//                   DStr-Functions. 
//
/----------------------------------------------------------------------*/

static Token_p scan_token(Scanner_p in)
{
   DStrReset(AktToken(in)->literal);
   DStrReleaseRef(AktToken(in)->source);

   AktToken(in)->source      = DStrGetRef(Source(in));
   AktToken(in)->stream_type = SourceType(in);
   AktToken(in)->line        = CurrLine(in);
   AktToken(in)->column      = CurrColumn(in);

   if(!ischar(CurrChar(in)))
   {
      AktToken(in)->tok = NoToken;
   }
   else if(isspace(CurrChar(in)))
   {
      scan_white(in);
   }
   else if(isstartidchar(CurrChar(in)))
   {
      scan_ident(in);
   }
   else if(isdigit(CurrChar(in)))
   {
      scan_int(in);
   }
   else if(isstartcomment(CurrChar(in)))
   {
      scan_line_comment(in);
   }
   else if(CurrChar(in)=='/' && LookChar(in,1) == '*')
   {
      scan_C_comment(in);
   }
   else if((CurrChar(in)=='"') || (CurrChar(in)=='\''))
   {
      scan_string(in, CurrChar(in));
   }
   else
   {
      switch(CurrChar(in))
      {
      case '(':
	 AktToken(in)->tok = OpenBracket;
	 break;
      case ')':
	 AktToken(in)->tok = CloseBracket;
	 break;
      case '{':
	 AktToken(in)->tok = OpenCurly;
	 break;
      case '}':
	 AktToken(in)->tok = CloseCurly;
	 break;
      case '[':
	 AktToken(in)->tok = OpenSquare;
	 break;
      case ']':
	 AktToken(in)->tok = CloseSquare;
	 break;
      case '<':
	 AktToken(in)->tok = LesserSign;
	 break;
      case '>':
	 AktToken(in)->tok = GreaterSign;
	 break;
      case '=':
	 AktToken(in)->tok = EqualSign;
	 break;
      case '~':
	 AktToken(in)->tok = TildeSign;
	 break;
      case '!':
	 AktToken(in)->tok = Exclamation;
	 break;
      case '?':
	 AktToken(in)->tok = QuestionMark;
	 break;
      case ',':
	 AktToken(in)->tok = Comma;
	 break;
      case ';':
	 AktToken(in)->tok = Semicolon;
	 break;
       case ':':
	 AktToken(in)->tok = Colon;
	 break;
       case '-':
	 AktToken(in)->tok = Hyphen;
	 break;
      case '+':
	 AktToken(in)->tok = Plus;
	 break;
      case '*':
	 AktToken(in)->tok = Mult;
	 break;
        case '.':
	 AktToken(in)->tok = Fullstop;
	 break;
      case '$':
	    AktToken(in)->tok = Dollar;
	    break;
      case '|':
	    AktToken(in)->tok = Pipe;
	    break;
      case '/':
	    AktToken(in)->tok = Slash;
	    break;
      case '&':
	    AktToken(in)->tok = Ampersand;
	    break;
      default:
	    DStrAppendChar(AktToken(in)->literal, CurrChar(in));
	    AktTokenError(in, "Illegal character", false);
	    break;
      }
      DStrAppendChar(AktToken(in)->literal, CurrChar(in));
      NextChar(in);
   }
   return AktToken(in);
}


/*-----------------------------------------------------------------------
//
// Function: scan_token_follow_includes()
//
//   Scan a token, follow include directives and pop back empty input
/    streams. 
//
// Global Variables: -
//
// Side Effects    : Read input, manipulate streams.
//
/----------------------------------------------------------------------*/

Token_p scan_token_follow_includes(Scanner_p in)
{
   scan_token(in);
   if(in->include_key && (TestInpId(in, in->include_key)))
   {
      scan_token(in);
      CheckInpTok(in, WhiteSpace);
      scan_token(in);
      CheckInpTok(in, Name);
      OpenStackedInput(&(in->source), StreamTypeFile,
		       DStrView(AktToken(in)->literal));
      scan_token_follow_includes(in);
   }
   else if(in->include_key && TestInpTok(in, NoToken))
   {
      if((in->source)->next)
      {
	 CloseStackedInput(&(in->source));
	 scan_token_follow_includes(in);
      }      
   }
   return AktToken(in);
}

/*-----------------------------------------------------------------------
//
// Function: scan_real_token()
//
//   Scan tokens until a real token (i.e. not a SkipToken has been
//   scanned.  
//
// Global Variables: -
//
// Side Effects    : Reads input, manipulates streams.
//
/----------------------------------------------------------------------*/

static Token_p scan_real_token(Scanner_p in)
{
   AktToken(in)->skipped = false;
   DStrReset(AktToken(in)->comment);
   
   scan_token_follow_includes(in);
   
   while(TestTok(AktToken(in), SkipToken))
   {
      AktToken(in)->skipped = true;
      if(!in->ignore_comments)
      {
	 DStrAppendDStr(AktToken(in)->comment, AktToken(in)->literal);
      }
      scan_token_follow_includes(in);
   }   
   return AktToken(in);
}


/*-----------------------------------------------------------------------
//
// Function: str_n_element()
//
//   Test whether the len lenght start of str is contained in the set
//   id of strings (encoded in a single string with elements separated
//   by |). 
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool str_n_element(char* str, char* ids, int len)
{
   if(strncmp(str, ids, len)==0)
   {
      ids+=len;
      if(!*ids || (*ids=='|'))
      {
	 return true;
      }
   }
   else
   {
      while(*ids && (*ids != '|'))
      {
	 ids++;
      }
      if(!*ids)
      {
	 return false;
      }
   }
   ids++;

   return str_n_element(str, ids, len);
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: PosRep()
//
//   Return a pointer to a description of a position in a file. The
//   description is valid until the function is called the next time.
//
// Global Variables: -
//
// Side Effects    : Sets static variable
//
/----------------------------------------------------------------------*/

char* PosRep(StreamType type, DStr_p source, long line, long column)
{
   static char buff[MAX_ERRMSG_LEN];
   char        tmp_str[MAX_ERRMSG_LEN];


   if(type == StreamTypeFile)
   {
      assert(strlen(DStrView(source))<=MAXPATHLEN);
      
      sprintf(buff, "%s:%ld:(Column %ld):",
	      DStrView(source), line, column);
   }
   else
   {
      tmp_str[0] = '\0';
      strcat(tmp_str, type);
      strcat(tmp_str, ": \"");
      strncat(tmp_str, DStrView(source), MAXPATHLEN-4);
      if(strlen(DStrView(source))>MAXPATHLEN-4)
      {
	 strcat(tmp_str, "...");
      }
      strcat(tmp_str, "\"");
      sprintf(buff, "%s:%ld:(Column %ld):", tmp_str, line, column);
   }
   
   return buff;
}


/*-----------------------------------------------------------------------
//
// Function: TokenPosRep()
//
//   Return a pointer to a description of the position of a token. The
//   description is valid until the function or PosRep() is called the
//   next time. 
//
// Global Variables: -
//
// Side Effects    : By PosRep()
//
/----------------------------------------------------------------------*/

char* TokenPosRep(Token_p token)
{
  return PosRep(token->stream_type, token->source, token->line,
		token->column);
}


/*-----------------------------------------------------------------------
//
// Function: DescribeToken()
//
//   Return a pointer to a description of the set of tokens described
//   by tok. The caller has to free the space of this description!
//
// Global Variables: token_print_rep
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

char* DescribeToken(TokenType tok)
{
   char*  help;
   int    i;
   DStr_p res = DStrAlloc();
   bool   found = false;

   for(i=0; token_print_rep[i].rep; i++)
   {
      if(tok & token_print_rep[i].key)
      {
	 DStrAppendStr(res, found ? " or " : "");
	 DStrAppendStr(res, token_print_rep[i].rep);
	 found = true;
      }
   }
   if(!found)
   {
      DStrAppendStr(res, token_print_rep[0].rep);
   }
   help = DStrCopy(res);
   DStrFree(res);
   
   return help;
}
   
/*-----------------------------------------------------------------------
//
// Function: PrintToken()
//
//   Print a token (probably only for debugging purposes...
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void PrintToken(FILE* out, Token_p token)
{
   char* handle;

   handle = DescribeToken(token->tok);
   fprintf(out, "Token:    %d = %s\n", (int)token->tok, handle);
   FREE(handle);
   fprintf(out, "Position: %s   ", TokenPosRep(token));
   fprintf(out, "Literal:  %s\n", DStrView(token->literal));
   fprintf(out, "Numval:   %6lu   Skipped:  %s\n", token->numval,
	   token->skipped ? "true" : "false");
   fprintf(out, "Comment:  %s\n", DStrView(token->comment));
}


/*-----------------------------------------------------------------------
//
// Function: CreateScanner()
//
//   Create a new, initialized scanner from which tokens can be read
//   immediately.
//
// Global Variables: -
//
// Side Effects    : Opens files, memory operations, reads input.
//
/----------------------------------------------------------------------*/

Scanner_p CreateScanner(StreamType type, char* name, bool
			ignore_comments, char* include_key)
{
   Scanner_p handle;

   handle = ScannerCellAlloc();
   handle->source = NULL;
   handle->accu = DStrAlloc();
   handle->ignore_comments = ignore_comments;
   handle->include_key = include_key;
   handle->format = LOPFormat;

   OpenStackedInput(&handle->source, type, name);
   
   for(handle->current = 0; handle->current < MAXTOKENLOOKAHEAD;
	  handle->current++)
   {
      handle->tok_sequence[handle->current].literal = DStrAlloc();
      handle->tok_sequence[handle->current].comment = DStrAlloc();
      handle->tok_sequence[handle->current].source      = NULL;
      handle->tok_sequence[handle->current].stream_type = NULL;
      scan_real_token(handle);
      /* PrintToken(stdout,AktToken(handle));*/
   }
   handle->current = 0;

   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: DestroyScanner()
//
//   Ensure that the scanner is disposed of cleanly, all files are
//   closed and all memory/references are released.
//
// Global Variables: -
//
// Side Effects    : Closes file, may cause error if input stack is
//                   not empty
//
/----------------------------------------------------------------------*/

void DestroyScanner(Scanner_p  junk)
{
   assert(junk);
   for(junk->current = 0; junk->current < MAXTOKENLOOKAHEAD;
	  junk->current++)
   {
      DStrFree(junk->tok_sequence[junk->current].literal);
      DStrFree(junk->tok_sequence[junk->current].comment);
      DStrReleaseRef(junk->tok_sequence[junk->current].source);
   }
   assert(junk->source);
   CloseStackedInput(&junk->source);
   assert(!junk->source);
   DStrFree(junk->accu);
   ScannerCellFree(junk);
}

/*-----------------------------------------------------------------------
//
// Function: TestTok()
//
//   Compares the type of the given token with a list of possible
//   tokens. Possibilities are values of type TokenType, possibly
//   combined with the biwise or operator '|'. The test is true if the
//   given token matches at least one type from the list.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool TestTok(Token_p akt, TokenType toks)
{
   /* PrintToken(stdout, akt);*/
   return (akt->tok & toks)!=0;
}

/*-----------------------------------------------------------------------
//
// Function: TestId()
//
//   Test whether a given token is of type identifier and is one of
//   a set of possible alternatives. This set is given as a single
//   C-String, alternatives are separated by the '|' character.
//
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool TestId(Token_p akt, char* ids)
{
   if(!TestTok(akt, Identifier))
   {
      return false;
   }
   return str_n_element(DStrView(akt->literal), ids,
			DStrLen(akt->literal));
}


/*-----------------------------------------------------------------------
//
// Function: TestIdNum()
//
//   As TestId(), but take only the non-numerical-part into account.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool TestIdnum(Token_p akt, char* ids)
{
   int  i, len=0;
   char c;

   if(!TestTok(akt, Idnum))
   {
      return false;
   }
   for(i=0; (c=DStrView(akt->literal)[i]); i++)
   {
      if(!len && isdigit(DStrView(akt->literal)[i]))
      {
	 len = i;
      }
      else if(!isdigit(DStrView(akt->literal)[i]))
      {
	 len = 0;
      }
   }
   return str_n_element(DStrView(akt->literal), ids, len);
}



/*-----------------------------------------------------------------------
//
// Function: AktTokenError() 
//
//   Produce a syntax error at the current token with the given
//   message. 
//
// Global Variables: -
//
// Side Effects    : Terminates programm
//
/----------------------------------------------------------------------*/

VOLATILE void AktTokenError(Scanner_p in, char* msg, bool syserr)
{
   DStr_p err = DStrAlloc();
   DStrReset(err);
   DStrAppendStr(err, TokenPosRep(AktToken(in)));
   DStrAppendStr(err, "(just read '");
   DStrAppendDStr(err, AktToken(in)->literal);
   DStrAppendStr(err, "'): ");
   DStrAppendStr(err, msg);
   if(syserr)
   {
      SysError(DStrView(err), SYNTAX_ERROR);
   }
   else
   {
      Error(DStrView(err), SYNTAX_ERROR);
   }
   DStrFree(err); /* Just for symmetry reasons */
}


/*-----------------------------------------------------------------------
//
// Function: CheckInpTok()
//
//   Check whether AktTok(in) is of one of the desired types. Produce
//   error if not.
//
// Global Variables: -
//
// Side Effects    : Memory operations, may terminate program.
//
/----------------------------------------------------------------------*/

void CheckInpTok(Scanner_p in, TokenType toks)
{
   if(!TestInpTok(in, toks))
   {
      char* tmp;
      
      DStrReset(in->accu);
      tmp = DescribeToken(toks);
      DStrAppendStr(in->accu, tmp);
      FREE(tmp);
      DStrAppendStr(in->accu, " expected, but ");
      tmp = DescribeToken(AktToken(in)->tok);
      DStrAppendStr(in->accu, tmp);
      FREE(tmp);
      DStrAppendStr(in->accu, " read ");
      AktTokenError(in, DStrView(in->accu), false);
   }
}


/*-----------------------------------------------------------------------
//
// Function: CheckInpTokNoSkip()
//
//   As CheckInpTok(), but produce an error if SkipTokens were
//   present. 
//
// Global Variables: -
//
// Side Effects    : As CheckInpTok()
//
/----------------------------------------------------------------------*/

void CheckInpTokNoSkip(Scanner_p in, TokenType toks)
{
   if(AktToken(in)->skipped)
   {
      char* tmp;

      DStrReset(in->accu);
      tmp = DescribeToken(toks);
      DStrAppendStr(in->accu, tmp);
      FREE(tmp);
      DStrAppendStr(in->accu, " expected, but ");
      tmp = DescribeToken(SkipToken);
      DStrAppendStr(in->accu, tmp);
      FREE(tmp);
      DStrAppendStr(in->accu, " read ");
      AktTokenError(in, DStrView(in->accu), false);
   }
   CheckInpTok(in, toks);
}


/*-----------------------------------------------------------------------
//
// Function:  CheckInpId()
//
//   Check whether AktToken(in) is an identifier with the desired
//   value. Produce error if not.
//
// Global Variables: -
//
// Side Effects    : Memory operations, may terminate program.
//
/----------------------------------------------------------------------*/

void CheckInpId(Scanner_p in, char* ids)
{
   if(!TestInpId(in, ids))
   {
      char* tmp;

      DStrReset(in->accu);
      DStrAppendStr(in->accu, "Identifier (");
      DStrAppendStr(in->accu, ids);
      DStrAppendStr(in->accu, ") expected, but ");
      tmp = DescribeToken(AktToken(in)->tok);
      DStrAppendStr(in->accu, tmp);
      FREE(tmp);
      DStrAppendStr(in->accu, "('");
      DStrAppendStr(in->accu, DStrView(AktToken(in)->literal));
      DStrAppendStr(in->accu, "') read ");
      AktTokenError(in, DStrView(in->accu), false);
   }
}

/*-----------------------------------------------------------------------
//
// Function: NextToken()
//
//   Read a new token, switch to the next token in the queue.
//
// Global Variables: -
//
// Side Effects    : Only by calling scan_real_token()
//
/----------------------------------------------------------------------*/

void NextToken(Scanner_p in)
{
   scan_real_token(in);
   in->current = TOKENREALPOS(in->current+1);
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


