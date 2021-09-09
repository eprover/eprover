/*-----------------------------------------------------------------------

File  : cio_streams.h

Author: Stephan Schulz

Contents

  Definitions for a stream type, i.e. an object associated with a file
  pointer (and possibly a file name), allowing read operations,
  arbitrary look-aheads, and maintaining line and column numbers for
  error messages.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed Aug 20 00:02:32 MET DST 1997
    New

-----------------------------------------------------------------------*/

#ifndef CIO_STREAMS

#define CIO_STREAMS

#include <cio_initio.h>
#include <cio_fileops.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

#define MAXLOOKAHEAD 64


/* Streams can read either from a file or from several predefined
   classes of strings. We use a StreamType 'virtual type' to denote
   this, because we can thus code the string type in a convenient
   way. */

typedef char* StreamType;


/* The following data structure describes the state of a (named) input
   stream with lookahead-capability. Streams are stackable, with new
   data being read from the top of the stack. The empty stack is a
   NULL-valued pointer of type Inpstack_p! */


typedef struct streamcell
{
   struct streamcell* next;
   DStr_p             source;
   StreamType         stream_type; /* Only constant strings allowed
                                      here! */
   long               string_pos;
   FILE*              file;
   bool               eof_seen;
   long               line;
   long               column;
   int                buffer[MAXLOOKAHEAD];
   int                current;
}StreamCell, *Stream_p, **Inpstack_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

extern const StreamType StreamTypeFile;
extern const StreamType StreamTypeInternalString;
extern const StreamType StreamTypeUserString;
extern const StreamType StreamTypeOptionString;


#define StreamCellAlloc() (StreamCell*)SizeMalloc(sizeof(StreamCell))
#define StreamCellFree(junk)         SizeFree(junk, sizeof(StreamCell))

Stream_p CreateStream(StreamType type, char* source, bool fail);
void     DestroyStream(Stream_p stream);

#define  STREAMREALPOS(pos) ((pos) % MAXLOOKAHEAD)

#define  StreamLookChar(stream, look)\
         (assert((look)<MAXLOOKAHEAD),\
     (stream)->buffer[STREAMREALPOS((stream)->current+(look))])
#define  StreamCurrChar(stream) ((stream)->buffer[(stream)->current])
#define  StreamCurrLine(stream)   ((stream)->line)
#define  StreamCurrColumn(stream) ((stream)->column)

int      StreamNextChar(Stream_p stream);

Stream_p OpenStackedInput(Inpstack_p stack, StreamType type,
           char* source, bool fail);
void     CloseStackedInput(Inpstack_p stack);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
