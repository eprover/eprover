/*-----------------------------------------------------------------------

  File  : cio_streams.c

  Author: Stephan Schulz

  Contents

  Implementation of the stream datatype for look-aheadable input.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: Sat Jul  5 02:28:25 MET DST 1997

  -----------------------------------------------------------------------*/

#include "cio_streams.h"


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

/* This variables are really constants and are used to select the input
   method for a stream. Default is file type (with error messages
   described by e.g. the GNU coding guidelines), for string input
   streams the StreamType is used in the error message. */

const StreamType StreamTypeFile = NULL;
const StreamType StreamTypeInternalString =
   "Internal (programmer-defined) string - if you see this, you"
   " encountered a bug";
const StreamType StreamTypeUserString =
   "Parsing a user provided string";
const StreamType StreamTypeOptionString =
   "Parsing a user given option argument";

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: read_char()
//
//   Read a character and return it. Return an infinite sequence of
//   EOFs after the end of file.
//
// Global Variables: -
//
// Side Effects    : May read a character
//
/----------------------------------------------------------------------*/

static int read_char(Stream_p stream)
{
   int ch;

   if(stream->eof_seen)
   {
      ch = EOF;
   }
   else
   {
      if(stream->stream_type!=StreamTypeFile)
      {
         ch = (int)DStrView(stream->source)[stream->string_pos];
         if(ch)
         {
            stream->string_pos++;
         }
         else
         {
            ch = EOF;
            stream->eof_seen = true;
         }
      }
      else
      {
         ch = getc(stream->file);
         if(ch == EOF)
         {
            stream->eof_seen = true;
         }
      }
   }
   return ch;
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: CreateStream()
//
//   Create a stream associated with the file name. Both the
//   NULL-pointer and the name "-" are taken to mean stdin.
//
// Global Variables: -
//
// Side Effects    : May terminate with an error message
//
/----------------------------------------------------------------------*/

Stream_p CreateStream(StreamType type, char* source, bool fail)
{
   Stream_p handle;
   int      i;

   handle = StreamCellAlloc();

   handle->source = DStrAlloc();
   handle->stream_type = type;

   if(type == StreamTypeFile)
   {
      /* Interprete source as a file name ! */

      if(!source || !strcmp(source,"-"))
      {
         DStrSet(handle->source, "<stdin>");
         handle->file = stdin;
         /* handle->dir  == "" */
      }
      else
      {
         DStrSet(handle->source, source);
         handle->file = InputOpen(source, fail);
         if(!handle->file)
         {
            DStrFree(handle->source);
            StreamCellFree(handle);
            return NULL;
         }
      }
      VERBOUTARG("Opened ", DStrView(handle->source));
   }
   else
   {
      /* Interprete source as a string to read from! */

      DStrSet(handle->source, source);
      handle->string_pos = 0;
   }
   handle->next       = NULL;
   handle->eof_seen   = false;
   handle->line       = 1;
   handle->column     = 1;
   handle->current    = 0;

   for(i=0; i<MAXLOOKAHEAD; i++)
   {
      handle->buffer[i] = read_char(handle);
   }

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: DestroyStream()
//
//   Free all resources (memory, file handle) associated with the
//   stream.
//
// Global Variables: -
//
// Side Effects    : Memory operations, file operations
//
/----------------------------------------------------------------------*/

void DestroyStream(Stream_p stream)
{
   if(stream->stream_type == StreamTypeFile)
   {
      if(stream->file != stdin)
      {
         if(fclose(stream->file) != 0)
         {
            TmpErrno = errno;
            sprintf(ErrStr, "Cannot close file %s", DStrView(stream->source));
            SysError(ErrStr, FILE_ERROR);
         }
      }
      VERBOUTARG("Closing ", DStrView(stream->source));
   }
   DStrFree(stream->source);
   StreamCellFree(stream);
}


/*-----------------------------------------------------------------------
//
// Function: StreamNextChar()
//
//   Move the current window on the input stream one character
//   forward. Return the new CurrChar().
//
// Global Variables: -
//
// Side Effects    : Reads one character from the input, update the
//                   stream information about the current position.
//
/----------------------------------------------------------------------*/

int StreamNextChar(Stream_p stream)
{
   if(StreamCurrChar(stream) == '\n')
   {
      stream->line++;
      stream->column = 1;
   }
   else
   {
      stream->column++;
   }
   stream->current=STREAMREALPOS(stream->current+1);
   stream->buffer[STREAMREALPOS(stream->current+MAXLOOKAHEAD-1)]
      = read_char(stream);

   return StreamCurrChar(stream);
}


/*-----------------------------------------------------------------------
//
// Function: OpenStackedInput()
//
//   Open a new input stream and put it on top of the stack. All
//   further input from this stack is read from the new top of the
//   stack.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Stream_p OpenStackedInput(Inpstack_p stack, StreamType type, char* source, bool fail)
{
   Stream_p handle;

   handle = CreateStream(type, source, fail);
   if(handle)
   {
      handle->next = *stack;
      *stack = handle;
   }
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: CloseStackedInput()
//
//   Pop the top from the input stack and destroy the associated
//   stream.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void CloseStackedInput(Inpstack_p stack)
{
   Stream_p handle;

   assert(*stack);
   handle = *stack;
   *stack = handle->next;
   DestroyStream(handle);
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
