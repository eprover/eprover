/*-----------------------------------------------------------------------

  File  : cio_commandline.c

  Author: Stephan Schulz

  Contents

  Functions for handling options and recognising non-option
  arguments.

  Copyright 1998-2017 by the author.

  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created:  Wed Sep 10 00:01:33 MET DST 1997

-----------------------------------------------------------------------*/

#include "cio_commandline.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: print_start_of_str()
//
//   Print str up to the last blank character before the len's
//   character or the first newline, whichever is first, followed by a
//   newline. If there is no blank, break at
//   character number len. Returns a pointer to the first character
//   following the break, or NULL if the string was printed
//    completely.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static char* print_start_of_str(FILE* out, char* str, int len)
{
   int   i = 0;
   char* search = str;
   char* blank = NULL;

   while(*search && i<len) /* Search last blank before
               EOString/newline or len characters. */
   {
      if(*search == ' ')
      {
    blank = search;
      }
      else if(*search == '\n')
      {
    blank = search;
    i=len;
    break;
      }
      search++;
      i++;
   }
   if(i<len) /* Print remaining string */
   {
      while(*str)
      {
    putc(*str, out);
    str++;
      }
      putc('\n', out);
      return 0;
   }
   else if(blank)  /* Print string up to space */
   {
      while(str!=blank)
      {
    putc(*str, out);
    str++;
      }
      putc('\n', out);
      str++;
      return *str?str:NULL;
   }
   else /* Now have to do a hard break! */
   {
      while(str!=search)
      {
    putc(*str, out);
    str++;
      }
      putc('\n', out);
      return str;
   }
}

/*-----------------------------------------------------------------------
//
// Function: shift_array_left()
//
//   Shift a 0-terminated array of char* elements left by one,
//   dropping the first element. Return false if no element is
//   present.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static bool shift_array_left(char* array[])
{
   int i;

   if(!array[0])
   {
      return false;
   }
   for(i=0; array[i+1]; i++)
   {
      array[i] = array[i+1];
   }
   array[i] = NULL;

   return true;
}


/*-----------------------------------------------------------------------
//
// Function: find_long_opt()
//
//   Find an option entry by long name. Return NULL if not found.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static Opt_p find_long_opt(char* option, OptCell options[])
{
   int   i;
   unsigned int len;

   option+=2; /* Jump -- */
   len = 0;
   while(option[len])
   {
      if(option[len]=='=')
      {
    break;
      }
      len++;
   }
   for(i=0; options[i].option_code; i++)
   {
      if((strncmp(options[i].longopt, option, len)==0) &&
    (strlen(options[i].longopt)==len))
      {
    return &(options[i]);
      }
   }
   return NULL;
}


/*-----------------------------------------------------------------------
//
// Function: find_short_opt()
//
//   Find an option entry by short name. Return NULL if not found.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static Opt_p find_short_opt(char option, OptCell options[])
{
   int i;

   for(i=0; options[i].option_code; i++)
   {
      if(option == options[i].shortopt)
      {
    return &(options[i]);
      }
   }
   return NULL;
}


/*-----------------------------------------------------------------------
//
// Function:  process_long_option()
//
//   Process the long option that is found in state->argc[state->argi]:
//   Find the option, check for argument, set *arg to an argument,
//   update  state.
//
// Global Variables: -
//
// Side Effects    : Changes state, may cause error
//
/----------------------------------------------------------------------*/

static Opt_p process_long_option(CLState_p state, char** arg,
           OptCell options[])
{
   Opt_p handle;
   char* eq_sign;
   DStr_p err = DStrAlloc();

   if(!(handle = find_long_opt(state->argv[state->argi], options)))
   {
      DStrAppendStr(err, "Unknown Option: ");
      DStrAppendStr(err, state->argv[state->argi]);
      DStrAppendStr(err," (Use -h for a list of valid options)");
      Error(DStrView(err), USAGE_ERROR);
   }

   eq_sign = strchr(state->argv[state->argi], '=');

   switch(handle->type)
   {
   case NoArg:
      if(eq_sign)
      {
    DStrAppendStr(err, state->argv[state->argi]);
    DStrAppendStr(err, " does not accept an argument!");
    Error(DStrView(err), USAGE_ERROR);
      }
      *arg = NULL;
      break;
   case OptArg:
      if(eq_sign)
      {
    *arg = eq_sign+1;
      }
      else
      {
    assert(handle->arg_default);
    *arg = handle->arg_default;
      }
      break;
   case ReqArg:
      if(!eq_sign)
      {
    DStrAppendStr(err, state->argv[state->argi]);
    DStrAppendStr(err, " requires an argument!");
    Error(DStrView(err), USAGE_ERROR);
      }
      *arg = eq_sign+1;
      break;
   default:
      assert(false);
      break;
   }
   shift_array_left(&(state->argv[state->argi]));
   state->argc--;
   DStrFree(err);

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: process_short_option()
//
//   Process the short option that is found in
//   state->argc[state->argi][state->sc_opt_c]: Find the option, check
//   for argument, set *arg to an argument, update state.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

static Opt_p process_short_option(CLState_p state, char** arg,
            OptCell options[])
{
   Opt_p  handle;
   DStr_p err = DStrAlloc();
   char*  optstr = state->argv[state->argi];

   if(!(handle = find_short_opt(optstr[state->sc_opt_c], options)))
   {
      DStrAppendStr(err, "Unknown Option: -");
      DStrAppendChar(err, optstr[state->sc_opt_c]);
      DStrAppendStr(err, " (processing ");
      DStrAppendStr(err, optstr);
      DStrAppendStr(err, ")");
      DStrAppendStr(err," (Use -h for a list of valid options)");
      Error(DStrView(err), USAGE_ERROR);
   }
   *arg = NULL;
   switch(handle->type)
   {
   case OptArg:
    assert(handle->arg_default);
    *arg = handle->arg_default;
    /* Fall-through intentional! */
   case NoArg:
    state->sc_opt_c++;
    if(!optstr[state->sc_opt_c])
    {
       state->sc_opt_c = 0;
       shift_array_left(&(state->argv[state->argi]));
       state->argc--;
    }
    break;
   case ReqArg:
    if(state->sc_opt_c!=1)
    {
       DStrAppendStr(err, optstr);
       DStrAppendStr(err, ": POSIX forbids the aggregation of"
           " options which take arguments (but you"
           " probably only forgot the second hyphen for"
           " a long GNU-style option)");
       Error(DStrView(err), USAGE_ERROR);
    }
    if(optstr[state->sc_opt_c+1])
    {
       *arg=&optstr[state->sc_opt_c+1];
    }
    else
    {
       shift_array_left(&(state->argv[state->argi]));
       state->argc--;
       if(!state->argv[state->argi])
       {
          DStrAppendChar(err, '-');
          DStrAppendChar(err, optstr[state->sc_opt_c]);
          DStrAppendStr(err, " requires an argument");
          Error(DStrView(err), USAGE_ERROR);
       }
       *arg = state->argv[state->argi];
    }
    state->sc_opt_c = 0;
    shift_array_left(&(state->argv[state->argi]));
    state->argc--;
    break;
   default:
    assert(false);
    break;
   }
   DStrFree(err);
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: append_option_desc()
//
//   Append a description of the given option to the DStr.
//
// Global Variables: -
//
// Side Effects    : Changes the string.
//
/----------------------------------------------------------------------*/

static void append_option_desc(DStr_p string, Opt_p option)
{
   assert(option->shortopt || option->longopt);

   if(option->shortopt)
   {
      DStrAppendChar(string, '-');
      DStrAppendChar(string, option->shortopt);
   }
   if(option->shortopt && option->longopt)
   {
      DStrAppendStr(string, " or ");
   }
   if(option->longopt)
   {
      DStrAppendStr(string, "--");
      DStrAppendStr(string, option->longopt);
   }
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: CLStateAlloc()
//
//   Allocate initialized Structure for the description of a
//   (partially processed) command line.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

CLState_p CLStateAlloc(int argc, char* argv[])
{
   int i;
   CLState_p handle = CLStateCellAlloc();

   handle->sc_opt_c = 0;
   handle->argi = 0;
   handle->argc = argc;
   handle->argsize = argc+2; /* Allocate one entry extra for
            default inserting of "-" */
   handle->argv = SecureMalloc(handle->argsize * sizeof(char*));
   for(i=0; i<argc; i++)
   {
      handle->argv[i] = argv[i];
   }
   handle->argv[argc] = NULL;

   shift_array_left(handle->argv);
   handle->argc--;
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: CLStateFree()
//
//   Free a CLStateCell.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void CLStateFree(CLState_p junk)
{
   assert(junk);
   FREE(junk->argv);
   CLStateCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: CLStateInsertArg()
//
//   Insert an additional argument at the end of state->argv, realloc
//   for more memory if necessary. Return new state->argc value. arg
//   is expected to be const, it is not copied!
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

int CLStateInsertArg(CLState_p state, char* arg)
{
   if(state->argsize == state->argc+2)
   {
      state->argsize++;
      state->argv = SecureRealloc(state->argv, state->argsize *
               sizeof(char*));
   }
   state->argv[state->argc] = arg;
   state->argc++;
   state->argv[state->argc] = NULL;

   return state->argc;
}

/*-----------------------------------------------------------------------
//
// Function: CLStateGetOpt()
//
//   Return a pointer to the next unprocessed option, set arg to point
//   to the argument (if present) or the default (if present).
//
// Global Variables: -
//
// Side Effects    : Updates state by removing options and arguments and
//                   adjusting counters accordingly.
//
/----------------------------------------------------------------------*/

Opt_p CLStateGetOpt(CLState_p state, char** arg, OptCell options[])
{
   while(state->argv[state->argi])
   {
      if((state->argv[state->argi][0]=='-') &&
    (state->argv[state->argi][1]!='\0'))
      {
    break;
      }
      state->argi++;
   }
   if(!state->argv[state->argi])
   {
      return NULL;
   }
   if(strcmp(state->argv[state->argi], "--")==0)
   {
      shift_array_left(&(state->argv[state->argi]));
      state->argc--;
      while(state->argv[state->argi])
      {
    state->argi++;
      }
      return NULL;
   }
   if(strncmp(state->argv[state->argi], "--", 2)==0)
   {
      return process_long_option(state, arg, options);
   }
   if(!state->sc_opt_c)
   {
      state->sc_opt_c = 1;
   }
   return process_short_option(state, arg, options);
}


/*-----------------------------------------------------------------------
//
// Function: CLStateGetFloatArg()
//
//   Return the numerical value of the argument if it is a well-formed
//   (double) float, print an error message otherwise.
//
// Global Variables: -
//
// Side Effects    : May terminate program
//
/----------------------------------------------------------------------*/

double CLStateGetFloatArg(Opt_p option, char* arg)
{
   double ret;
   char*  eoarg;

   errno = 0;

   ret = strtod(arg, &eoarg);

   if(errno || *eoarg)
   {
      DStr_p err = DStrAlloc();
      TmpErrno = errno;
      append_option_desc(err, option);
      DStrAppendStr(err, " expects float instead of '");
      DStrAppendStr(err, arg);
      DStrAppendChar(err, '\'');
      if(TmpErrno)
      {
    SysError(DStrView(err), USAGE_ERROR);
      }
      else
      {
    Error(DStrView(err), USAGE_ERROR);
      }
      DStrFree(err);
   }
   return ret;
}


/*-----------------------------------------------------------------------
//
// Function: CLStateGetIntArg()
//
//   Return the numerical value of the argument if it is a well-formed
//   long, print an error message otherwise.
//
// Global Variables: -
//
// Side Effects    : May terminate program
//
/----------------------------------------------------------------------*/

long CLStateGetIntArg(Opt_p option, char* arg)
{
   long ret;
   char* eoarg;

   errno = 0;

   ret = strtol(arg, &eoarg, 10);

   if(errno || *eoarg)
   {
      DStr_p err = DStrAlloc();
      TmpErrno = errno;
      append_option_desc(err, option);
      DStrAppendStr(err, " expects integer instead of '");
      DStrAppendStr(err, arg);
      DStrAppendChar(err, '\'');
      if(TmpErrno)
      {
         SysError(DStrView(err), USAGE_ERROR);
      }
      else
      {
         Error(DStrView(err), USAGE_ERROR);
      }
      DStrFree(err); /* Symmetry */
   }
   return ret;
}

/*-----------------------------------------------------------------------
//
// Function: CLStateGetIntArgCheckRange()
//
//   Return the numerical value of the argument if it is a well-formed
//   long in the proper range, print an error message otherwise.
//
// Global Variables: -
//
// Side Effects    : May terminate program
//
/----------------------------------------------------------------------*/

long CLStateGetIntArgCheckRange(Opt_p option, char* arg, long lower, long upper)
{
   long ret = CLStateGetIntArg(option, arg);
   if(ret<lower || ret > upper)
   {
      DStr_p err = DStrAlloc();
      DStrAppendStr(err, "Option ");
      append_option_desc(err, option);
      DStrAppendStr(err, " expects integer argument from {");
      DStrAppendInt(err, lower);
      DStrAppendStr(err, "...");
      DStrAppendInt(err, upper);
      DStrAppendStr(err, "} but got ");
      DStrAppendStr(err, arg);
      Error(DStrView(err), USAGE_ERROR);
      DStrFree(err); /* Symmetry */
   }
   return ret;
}


/*-----------------------------------------------------------------------
//
// Function: CLStateGetBoolArg()
//
//   Return the boolean value of the argument if it is either 'true'
//   or 'false'  long, print an error message otherwise.
//
// Global Variables: -
//
// Side Effects    : May terminate program
//
/----------------------------------------------------------------------*/

bool CLStateGetBoolArg(Opt_p option, char* arg)
{
   DStr_p err;

   if(strcmp(arg, "true")==0)
   {
      return true;
   }
   else if(strcmp(arg, "false")==0)
   {
      return false;
   }

   err = DStrAlloc();
   append_option_desc(err, option);
   DStrAppendStr(err, " expects 'true' or 'false' instead of '");
   DStrAppendStr(err, arg);
   DStrAppendChar(err, '\'');
   Error(DStrView(err), USAGE_ERROR);
   DStrFree(err); /* Symmetry */
   return false; /* Just to stiffle warings */
}


/*-----------------------------------------------------------------------
//
// Function:  PrintOption()
//
//   Print the formatted description of an option (generated from the
//   information in an OptCell() to the desired stream.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void PrintOption(FILE* out, Opt_p option)
{
   char*  l_argdesc = "";
   char*  desc;
   DStr_p optdesc = DStrAlloc();

   switch(option->type)
   {
   case NoArg:
      break;
   case OptArg:
      l_argdesc = "[=<arg>]";
      break;
   case ReqArg:
      l_argdesc = "=<arg>";
      break;
   default:
      assert(false);
      break;
   }

   assert(option->longopt || option->shortopt);

   if(option->shortopt)
   {
      fprintf(out, "   -%c%s\n", option->shortopt,
         option->type == ReqArg ? " <arg>" : "");
   }
   if(option->longopt)
   {
      fprintf(out, "  --%s%s\n", option->longopt, l_argdesc);
   }

   DStrAppendStr(optdesc, option->desc);

   if(option->type ==OptArg)
   {
      assert(option->longopt);

      if(option->shortopt)
      {
    DStrAppendStr(optdesc,
             " The short form or the long form without"
             " the optional argument is equivalent to --");
      }
      else
      {
    DStrAppendStr(optdesc,
             " The option without"
             " the optional argument is equivalent to --");
      }

      DStrAppendStr(optdesc, option->longopt);
      DStrAppendChar(optdesc, '=');
      DStrAppendStr(optdesc, option->arg_default);
      DStrAppendStr(optdesc, ".");
   }

   desc = DStrView(optdesc);
   while(desc)
   {
      fprintf(out, "    ");
      desc = print_start_of_str(out, desc, FORMAT_WIDTH-4);
   }
   fprintf(out, "\n");
   DStrFree(optdesc);
}


/*-----------------------------------------------------------------------
//
// Function: PrintOptions()
//
//   Print the whole option array (terminated by an OptCell with type
//   NoOption.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void PrintOptions(FILE* out, OptCell option[], char* header)
{
   int i;

   if(header)
   {
      fprintf(out, "%s", header);
   }

   for(i=0; option[i].option_code; i++)
   {
      PrintOption(out, &option[i]);
   }
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
