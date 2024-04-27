/*-----------------------------------------------------------------------

  File  : cio_scanner.c

  Author: Stephan Schulz

  Contents

  Implementation of the scanner.

  Copyright 1998-2020 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details.
  Run "eprover -h" for contact information.

  Created: Sun Aug 31 13:31:42 MET DST 1997

  -----------------------------------------------------------------------*/

#include "cio_scanner.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

/* Using X-macro TOKENS to generate TokenRepCell table. */

#define T(token, value, description) {token, description},
static TokenRepCell token_print_rep[] =
{
   TOKENS
};
static const size_t token_print_rep_size = sizeof(token_print_rep) / sizeof(token_print_rep[0]);
#undef T


/*---------------------------------------------------------------------*/
/*                            Local Types                              */
/*---------------------------------------------------------------------*/

DEFINE_RESULT(ScanTokenFollowIncludesResult, Token_p)

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
//   Scan an identifier, d.h. an ident or an idnum. Also used for
//   completing SemIdents.
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
//   Scan an unsigned integer, i.e. a sequence of digits. If this
//   cannot be parsed as an int, it will be interpreted as an
//   identifier.
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

   while(isdigit(CurrChar(in)))
   {
      DStrAppendChar(AktToken(in)->literal, CurrChar(in));
      NextChar(in);
   }
   errno = 0;
   AktToken(in)->numval =
      strtoumax(DStrView(AktToken(in)->literal), NULL, 10);

   if(errno)
   {
      //static char buff[30];
      //char* term=strncpy(buff, DStrView(AktToken(in)->literal), 29);
      //*term = '\0';
      //strtol(buff, NULL, 10);
      _CreateTokenWarning(in, "Number truncated. If this happens on 32 bit systems while parsing internal strings, it is harmless and can be ignored");
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

   while((CurrChar(in)) != '\n' && (CurrChar(in)!=EOF))
   {
      DStrAppendChar(AktToken(in)->literal, CurrChar(in));
      NextChar(in);
   }
   DStrAppendChar(AktToken(in)->literal, '\n');
   NextChar(in); /* Should be harmless even at EOF */
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
//   Scan a string (enclosed in "" or '').
//
// Global Variables: -
//
// Side Effects    : As scan_white()
//
/----------------------------------------------------------------------*/

static void scan_string(Scanner_p in, char delim)
{
   bool escape = false;
   int curr;

   AktToken(in)->tok = (delim=='\'')?SQString:String;

   DStrAppendChar(AktToken(in)->literal, CurrChar(in));
   NextChar(in);
   while(escape || ((curr = CurrChar(in)) != delim))
   {
      if(!isprint(curr) && curr<=127)
      {
         _CreateTokenError(in,
                       "Non-printable character in string constant",
                       false);
         return;
      }
      if(curr=='\\')
      {
         escape = !escape;
      }
      else
      {
         escape = false;
      }
      DStrAppendChar(AktToken(in)->literal, curr);
      NextChar(in);
   }
   DStrAppendChar(AktToken(in)->literal, curr);
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
   else if((CurrChar(in)=='$') && isidchar(LookChar(in,1)))
   {
      DStrAppendChar(AktToken(in)->literal, CurrChar(in));
      NextChar(in);
      scan_ident(in);
      if(!strcmp(DStrView(AktToken(in)->literal), "$let"))
      {
         AktToken(in)->tok = LetToken;
      }
      else if(!strcmp(DStrView(AktToken(in)->literal), "$ite"))
      {
         AktToken(in)->tok = IteToken;
      }
      else
      {
         AktToken(in)->tok = SemIdent;
      }
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
            if((LookChar(in,1) == '~' && (LookChar(in,2) == '>')))
            {
               DStrAppendChar(AktToken(in)->literal, CurrChar(in));
               NextChar(in);
               DStrAppendChar(AktToken(in)->literal, CurrChar(in));
               NextChar(in);
               AktToken(in)->tok = FOFXor;
            }
            else if(LookChar(in,1) == '=')
            {
               DStrAppendChar(AktToken(in)->literal, CurrChar(in));
               NextChar(in);
               if(LookChar(in,1) == '>')
               {
                  DStrAppendChar(AktToken(in)->literal, CurrChar(in));
                  NextChar(in);
                  AktToken(in)->tok = FOFEquiv;
               }
               else
               {
                  AktToken(in)->tok = FOFRLImpl;
               }
            }
            else
            {
               AktToken(in)->tok = LesserSign;
               break;
            }
            break;
      case '>':
            AktToken(in)->tok = GreaterSign;
            break;
      case '=':
            if(LookChar(in,1) == '>')
            {
               DStrAppendChar(AktToken(in)->literal, CurrChar(in));
               NextChar(in);
               AktToken(in)->tok = FOFLRImpl;
            }
            else
            {
               AktToken(in)->tok = EqualSign;
            }
            break;
      case '~':
            switch(LookChar(in,1))
            {
            case '|':
                  DStrAppendChar(AktToken(in)->literal, CurrChar(in));
                  NextChar(in);
                  AktToken(in)->tok = FOFNor;
                  break;
            case '&':
                  DStrAppendChar(AktToken(in)->literal, CurrChar(in));
                  NextChar(in);
                  AktToken(in)->tok = FOFNand;
                  break;

            default:
                  AktToken(in)->tok = TildeSign;
                  break;
            }
            break;
      case '!':
            if(LookChar(in,1) == '=')
            {
               DStrAppendChar(AktToken(in)->literal, CurrChar(in));
               NextChar(in);
               AktToken(in)->tok = NegEqualSign;
            }
            else
            {
               AktToken(in)->tok = Exclamation;
            }
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
      case '|':
            AktToken(in)->tok = Pipe;
            break;
      case '/':
            AktToken(in)->tok = Slash;
            break;
      case '&':
            AktToken(in)->tok = Ampersand;
            break;
      case '$':
            AktToken(in)->tok = Dollar;
            break;
      case '@':
            AktToken(in)->tok = Application;
            break;
      case '^':
            AktToken(in)->tok = Carret;
            break;
      default:
            DStrAppendChar(AktToken(in)->literal, CurrChar(in));
            /* Skip bad character. */
            NextChar(in);
            _CreateTokenError(in, "Illegal character", false);
            return AktToken(in);
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

static ScanTokenFollowIncludesResult scan_token_follow_includes(Scanner_p in)
{
   ScanTokenFollowIncludesResult result;

   scan_token(in);
   if(in->include_key && (TestInpId(in, in->include_key)))
   {
      DStr_p name = DStrAlloc();
      char*  tptp_source;

      tptp_source = getenv("TPTP");
      if(tptp_source)
      {
         DStrAppendStr(name, tptp_source);
         if(DStrLen(name) && (DStrView(name)[DStrLen(name)-1] !='/'))
         {
            DStrAppendChar(name,'/');
         }
      }

      scan_token(in);
      if (false == _CheckInpTok(in, OpenBracket)) 
      {
         DStrFree(name);
         MAKE_ERR(result, NULL)
      }

      scan_token(in);
      if (false == _CheckInpTok(in, Identifier|String|SQString)) 
      {
         DStrFree(name);
         MAKE_ERR(result, NULL)
      }
      if(TestInpTok(in, Identifier))
      {
         DStrAppendDStr(name, AktToken(in)->literal);
      }
      else
      {
         DStrAppendStr(name, DStrView(AktToken(in)->literal)+1);
         DStrDeleteLastChar(name);
      }
      OpenStackedInput(&(in->source), StreamTypeFile,
                       DStrView(name), true);
      DStrFree(name);
      scan_token_follow_includes(in);
   }
   else if(in->include_key && TestInpTok(in, NoToken))
   {
      if((in->source)->next)
      {
         CloseStackedInput(&(in->source));
         scan_token(in);
         if (false == _CheckInpTok(in, CloseBracket)) 
         {
            MAKE_ERR(result, NULL)
         }
         scan_token(in);
         if (false == _CheckInpTok(in, Fullstop)) 
         {
            MAKE_ERR(result, NULL)
         }
         scan_token_follow_includes(in);
      }
   }
   MAKE_OK(result, AktToken(in))
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
      if(!in->ignore_comments && TestInpTok(in, Comment))
      {
         DStrAppendDStr(AktToken(in)->comment, AktToken(in)->literal);
      }
      scan_token_follow_includes(in);
   }
   return AktToken(in);
}



/*-----------------------------------------------------------------------
//
// Function: compose_errmsg()
//
//    Compose position of current token and message into a DStr for
//    futher processing.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void compose_errmsg(DStr_p err, Scanner_p in, char* msg)
{
   DStrAppendStr(err, TokenPosRep(AktToken(in)));
   DStrAppendStr(err, "(just read '");
   DStrAppendDStr(err, AktToken(in)->literal);
   DStrAppendStr(err, "'): ");
   DStrAppendStr(err, msg);
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
   //printf("# str_n_element(%s, %s) = ", str, ids);

   if(strncmp(str, ids, len)==0)
   {
      ids+=len;
      if(!*ids || (*ids=='|'))
      {
         //printf("true\n");
         return true;
      }
   }
   while(*ids && (*ids != '|'))
   {
      ids++;
   }
   if(!*ids)
   {
      //printf("false\n");
      return false;
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
   char        tmp_str[MAX_ERRMSG_LEN];
   static char buff[MAX_ERRMSG_LEN+128]; // make place for numbers


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

   for(i=0; i < token_print_rep_size; i++)
   {
      if(tok & token_print_rep[i].key)
      {
         if (token_print_rep[i].rep) 
         {
            DStrAppendStr(res, found ? " or " : "");
            DStrAppendStr(res, token_print_rep[i].rep);
            found = true;
         }
      }
   }
   if(!found)
   {
      DStrAppendStr(res, "Unknown token (this should not happen)");
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
   fprintf(out, "Literal:  %s\n", token->literal?DStrView(token->literal):"");
   fprintf(out, "Numval:   %6" PRIuMAX "   Skipped:  %s\n", token->numval,
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

Scanner_p CreateScanner(StreamType type, char *name, bool
                        ignore_comments, char *default_dir, bool fail)
{
   Scanner_p handle;
   Stream_p  stream;
   char      *tmp_name;

   handle = ScannerCellAlloc();
   handle->source = NULL;
   handle->default_dir = DStrAlloc();
   handle->accu = DStrAlloc();
   handle->ignore_comments = ignore_comments;
   handle->include_key = NULL;
   handle->format = LOPFormat;

   //printf("# CreateScanner(%s, %s, %d, %s, %d)\n", type, name,
   //ignore_comments, default_dir, fail);

   if((type == StreamTypeFile && strcmp(name,"-")==0)||
      (type != StreamTypeFile))
   {
      stream = OpenStackedInput(&handle->source, type, name, true);
      assert(stream);
   }
   else
   {
      assert(type == StreamTypeFile);
      if(FileNameIsAbsolute(name))
      {
         stream = OpenStackedInput(&handle->source, type, name, fail);
         tmp_name = FileNameDirName(name);
         DStrAppendStr(handle->default_dir, tmp_name);
         FREE(tmp_name);
         //assert(stream);
      }
      else
      {
         DStr_p full_file_name = DStrAlloc();

         if(default_dir)
         {
            DStrAppendStr(handle->default_dir, default_dir);
            assert(!DStrLen(handle->default_dir)||
                   DStrLastChar(handle->default_dir) =='/');
         }
         tmp_name = FileNameDirName(name);
         DStrAppendStr(handle->default_dir, tmp_name);
         assert(DStrLen(handle->default_dir)==0 ||
                DStrLastChar(handle->default_dir) =='/');
         FREE(tmp_name);
         tmp_name = FileNameBaseName(name);
         DStrAppendStr(full_file_name,
                       DStrView(handle->default_dir));
         DStrAppendStr(full_file_name,
                       tmp_name);
         FREE(tmp_name);
         stream = OpenStackedInput(&handle->source, type,
                                   DStrView(full_file_name), fail&&!TPTP_dir);
         fflush(stdout);
         if(!stream&&TPTP_dir)
         {
            assert(TPTP_dir);
            DStrSet(handle->default_dir, TPTP_dir);
            tmp_name = FileNameDirName(name);
            DStrAppendStr(handle->default_dir, tmp_name);
            FREE(tmp_name);
            tmp_name = FileNameBaseName(name);
            DStrSet(full_file_name,
                    DStrView(handle->default_dir));
            DStrAppendStr(full_file_name,
                          tmp_name);
            FREE(tmp_name);
            stream = OpenStackedInput(&handle->source, type,
                                      DStrView(full_file_name), fail);
         }
         DStrFree(full_file_name);
      }
   }
   if(!stream)
   {
      DStrFree(handle->default_dir);
      DStrFree(handle->accu);
      ScannerCellFree(handle);
      return NULL;
   }

   for(handle->current = 0; handle->current < MAXTOKENLOOKAHEAD;
       handle->current++)
   {
      handle->tok_sequence[handle->current].tok = NoToken;
      handle->tok_sequence[handle->current].literal = DStrAlloc();
      handle->tok_sequence[handle->current].comment = DStrAlloc();
      handle->tok_sequence[handle->current].source      = NULL;
      handle->tok_sequence[handle->current].stream_type = NULL;
      scan_real_token(handle);
   }
   handle->current = 0;
   handle->include_pos = NULL;
   handle->panic_mode = false;
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
   DStrFree(junk->default_dir);
   DStrFree(junk->accu);
   if(junk->include_pos)
   {
      FREE(junk->include_pos);
   }
   ScannerCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: ScannerSetFormat()
//
//   Set the format of the scanner (in particular, guess a format if
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void ScannerSetFormat(Scanner_p scanner, IOFormat fmt)
{
   if(fmt == AutoFormat)
   {
      if(TestInpId(scanner, "fof|cnf|tff|thf|tcf|include"))
      {
         //printf("# TSTP!\n");
         fmt = TSTPFormat;
      }
      else if(TestInpId(scanner, "input_clause|input_formula"))
      {
         //printf("# TPTP!\n");
         fmt = TPTPFormat;
      }
      else
      {
         //printf("# LOP!\n");
         fmt = LOPFormat;
      }
   }
   scanner->format = fmt;
}


/*-----------------------------------------------------------------------
//
// Function: TestTok()
//
//   Compares the type of the given token with a list of possible
//   tokens. Possibilities are values of type TokenType, possibly
//   combined with the bitwise or operator '|'. The test is true if
//   the given token matches at least one type from the list.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool TestTok(Token_p akt, TokenType toks)
{
   /* PrintToken(stdout, akt); */
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

static void _token_error_at(Scanner_p in, char* msg, TokenErrorType err_type) 
{
   assert(in);
   assert(msg);

   DStr_p err = DStrAlloc();
   compose_errmsg(err, in, msg);

   /* Free the literal, it will be replaced by err. */
   DStrFree(AktToken(in)->literal);

   AktToken(in)->tok = ErrorToken;
   AktToken(in)->error.error_message = err;
   AktToken(in)->error.err_type = err_type;
}

void _CreateTokenError(Scanner_p in, char* msg, bool syserr) 
{
   assert(in);
   assert(msg);

   _token_error_at(in, msg, syserr ? TokenSysError : TokenSyntaxError);
}

void _CreateTokenWarning(Scanner_p in, char* msg) 
{
   assert(in);
   assert(msg);

   _token_error_at(in, msg, TokenWarning);
}

void _ParseError(Scanner_p in, char* msg, bool syserr) 
{
   assert(in);
   assert(msg);

   /* Do not print errors if in panic mode. */
   if (true == in->panic_mode) 
   {
      return;
   }

   /* Create error message. */
   DStr_p err = DStrAlloc();
   compose_errmsg(err, in, msg);

   if(syserr)
   {
      /* System errors are critical enough to exit. */
      SysError(DStrView(err), SYNTAX_ERROR);
   }
   else
   {
      /* "Normal" parse errors get printed. No exit. */
      _PrintError(DStrView(err), SYNTAX_ERROR);
   }

   /* Free error message after printing. Enter panic mode. */
   in->panic_mode = true;
   DStrFree(err);
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

bool _CheckInpTok(Scanner_p in, TokenType toks)
{
   assert(in);

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
      _ParseError(in, DStrView(in->accu), false);
      return false;
   }

   return true;
}

bool _ConsumeInpTok(Scanner_p in, TokenType toks) 
{
   assert(in);

   if (!_CheckInpTok(in, toks)) 
   {
      return false;
   }

   NextToken(in);
   return true;
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

bool _CheckInpTokNoSkip(Scanner_p in, TokenType toks)
{
   assert(in);

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
      _ParseError(in, DStrView(in->accu), false);
      return false;
   }

   return _CheckInpTok(in, toks);
}

bool _ConsumeInpTokNoSkip(Scanner_p in, TokenType toks) 
{
   assert(in);

   if (!_CheckInpTokNoSkip(in, toks)) 
   {
      return false;
   }

   NextToken(in);
   return true;
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

bool _CheckInpId(Scanner_p in, char* ids)
{
   assert(in);
   assert(ids);

   if(!TestInpId(in, ids))
   {
      char* tmp;

      DStrSet(in->accu, "Identifier (");
      DStrAppendStr(in->accu, ids);
      DStrAppendStr(in->accu, ") expected, but ");
      tmp = DescribeToken(AktToken(in)->tok);
      DStrAppendStr(in->accu, tmp);
      FREE(tmp);
      DStrAppendStr(in->accu, "('");
      DStrAppendStr(in->accu, DStrView(AktToken(in)->literal));
      DStrAppendStr(in->accu, "') read ");
      _ParseError(in, DStrView(in->accu), false);
      return false;
   }

   return true;
}

bool _ConsumeInpId(Scanner_p in, char* ids) 
{
   assert(in);
   assert(ids);

   if (!_CheckInpId(in, ids)) 
   {
      return false;
   }

   NextToken(in);
   return true;
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

   /*printf("Current token:\n");
   PrintToken(stdout, AktToken(in));
   printf("Next token:\n");
   PrintToken(stdout, LookToken(in,1));
   printf("SuperNext token:\n");
   PrintToken(stdout, LookToken(in,2));
   printf("\n");*/
}


/*-----------------------------------------------------------------------
//
// Function: ScannerParseInclude()
//
//   Parse a TPTP-Style include statement. Return a scanner for the
//   included file, and put (optional) selected names into
//   name_selector. If the file name is in skip_includes, skip the
//   rest and return NULL.
//
// Global Variables: -
//
// Side Effects    : Reads input.
//
/----------------------------------------------------------------------*/

ScannerParseIncludeResult ScannerParseInclude(Scanner_p in, 
                                              StrTree_p *name_selector,
                                              StrTree_p *skip_includes)
{
   /* Initialize result. */
   ScannerParseIncludeResult result;
   Scanner_p new_scanner = NULL;

   char* name;
   char* pos_rep;

   pos_rep = SecureStrdup(TokenPosRep(AktToken(in)));
   if (false == _ConsumeInpId(in, "include")) 
   {
      FREE(pos_rep);
      MAKE_ERR(result, NULL)
   }

   if (false == _ConsumeInpTok(in, OpenBracket)) 
   {
      FREE(pos_rep);
      MAKE_ERR(result, NULL)
   }

   if (false == _CheckInpTok(in, SQString)) 
   {
      FREE(pos_rep);
      MAKE_ERR(result, NULL)
   }

   name = DStrCopyCore(AktToken(in)->literal);

   if(!StrTreeFind(skip_includes, name))
   {
      new_scanner = CreateScanner(StreamTypeFile, name,
                                         in->ignore_comments,
                                         ScannerGetDefaultDir(in),
                                         true);
      ScannerSetFormat(new_scanner, ScannerGetFormat(in));
      new_scanner->include_pos = pos_rep;
   }
   else
   {
      FREE(pos_rep);
   }
   FREE(name);
   NextToken(in);

   if(TestInpTok(in, Comma))
   {
      IntOrP dummy;

      dummy.i_val = 0;
      NextToken(in);
      if (false == _ConsumeInpTok(in, Name|PosInt|OpenSquare))
      {
         if (NULL != new_scanner) 
         {
            DestroyScanner(new_scanner);
         }
      MAKE_ERR(result, NULL)
      }

      if(TestInpTok(in, Name|PosInt))
      {
         StrTreeStore(name_selector, DStrView(AktToken(in)->literal),
                      dummy, dummy);
         NextToken(in);
      }
      else
      {
         if (false == _ConsumeInpTok(in, OpenSquare)) 
         {
            if (NULL != new_scanner) 
            {
               DestroyScanner(new_scanner);
            }
            MAKE_ERR(result, NULL)
         }

         if(!TestInpTok(in, CloseSquare))
         {
            StrTreeStore(name_selector, DStrView(AktToken(in)->literal),
                         dummy, dummy);
            if (false == _ConsumeInpTok(in, Name|PosInt))
            {
               if (NULL != new_scanner) 
               {
                  DestroyScanner(new_scanner);
               }
               MAKE_ERR(result, NULL)
            }
            while(TestInpTok(in, Comma))
            {
               NextToken(in);
               StrTreeStore(name_selector, DStrView(AktToken(in)->literal),
                            dummy, dummy);
               if (false == _ConsumeInpTok(in, Name|PosInt))
               {
                  if (NULL != new_scanner) 
                  {
                     DestroyScanner(new_scanner);
                  }
                  MAKE_ERR(result, NULL)
               }
            }
         }
         else /* Empty list - insert full dummy */
         {
            dummy.i_val = 1;
            StrTreeStore(name_selector, "** Not a legal name**",
                         dummy, dummy);

         }
         if (false == _ConsumeInpTok(in, CloseSquare)) 
         {
            if (NULL != new_scanner) 
            {
               DestroyScanner(new_scanner);
            }
            MAKE_ERR(result, NULL)
         }
      }
   }
   if (false == _ConsumeInpTok(in, CloseBracket))
   {
      if (NULL != new_scanner) 
      {
         DestroyScanner(new_scanner);
      }
      MAKE_ERR(result, NULL)
   }
   
   if (false == _ConsumeInpTok(in, Fullstop)) 
   {
      if (NULL != new_scanner) 
      {
         DestroyScanner(new_scanner);
      }
      MAKE_ERR(result, NULL)
   }

   MAKE_OK(result, new_scanner)
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
