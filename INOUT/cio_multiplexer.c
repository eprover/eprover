/*-----------------------------------------------------------------------

File  : cio_multiplexer.c

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Code for convenient multiplexing of many channels.

  Copyright 2011 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Fri Mar 11 20:35:33 CET 2011
    New

-----------------------------------------------------------------------*/

#include "cio_multiplexer.h"



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
// Function: TCPChannelAlloc()
//
//   Allocate a TCPChannelCell for the provided socket.
//
// Global Variables: -
//
// Side Effects    : Memory management
//
/----------------------------------------------------------------------*/

TCPChannel_p  TCPChannelAlloc(int sock)
{
   TCPChannel_p handle = TCPChannelCellAlloc();

   handle->sock  = sock;
   handle->in    = PQueueAlloc();
   handle->out   = PQueueAlloc();

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: TCPChannelFree()
//
//   Free a TCPChannel.
//
// Global Variables: -
//
// Side Effects    : Memory management
//
/----------------------------------------------------------------------*/

void TCPChannelFree(TCPChannel_p junk)
{
   TCPMsg_p handle;

   while(!PQueueEmpty(junk->in))
   {
      handle = PQueueGetNextP(junk->in);
      TCPMsgFree(handle);
   }
   PQueueFree(junk->in);

   while(!PQueueEmpty(junk->out))
   {
      handle = PQueueGetNextP(junk->out);
      TCPMsgFree(handle);
   }
   PQueueFree(junk->out);
   if(junk->sock>=0)
   {
      close(junk->sock);
   }
   TCPChannelCellFree(junk);
}

/*-----------------------------------------------------------------------
//
// Function: TCPChannelClose()
//
//   Close a TCP channel
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void TCPChannelClose(TCPChannel_p channel)
{
   assert(channel->sock>=0);
   VERBOSE(fprintf(stderr, "Closing channel %d\n", channel->sock););
   close(channel->sock);
   channel->sock = -1;
}


/*-----------------------------------------------------------------------
//
// Function: TCPChannelHasInMsg()
//
//   Return true if there is a complete message in the channel.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool TCPChannelHasInMsg(TCPChannel_p channel)
{
   TCPMsg_p handle;

   if(PQueueEmpty(channel->in))
   {
      return false;
   }
   handle = PQueueLookP(channel->in);
   if(TCP_MSG_COMPLETE(handle))
   {
      return true;
   }
   return false;
}

/*-----------------------------------------------------------------------
//
// Function: TCPChannelSendMsg()
//
//   Add the message to the out queue.
//
// Global Variables: -
//
// Side Effects    : IO, memory
//
/----------------------------------------------------------------------*/

void TCPChannelSendMsg(TCPChannel_p channel, TCPMsg_p msg)
{
   PQueueStoreP(channel->out, msg);
}

/*-----------------------------------------------------------------------
//
// Function: TCPChannelSendStr()
//
//   Add the string as a message to the out queue.
//
// Global Variables: -
//
// Side Effects    : IO, memory
//
/----------------------------------------------------------------------*/

void TCPChannelSendStr(TCPChannel_p channel, char* str)
{
   TCPMsg_p  msg = TCPMsgPack(str);
   TCPChannelSendMsg(channel, msg);
}


/*-----------------------------------------------------------------------
//
// Function: TCPChannelRead()
//
//   Read data for one message from the socket - either the latest
//   incomplete message, or a newly allocated one.
//
// Global Variables: -
//
// Side Effects    : IO, memory.
//
/----------------------------------------------------------------------*/

MsgStatus TCPChannelRead(TCPChannel_p channel)
{
   TCPMsg_p  current;
   MsgStatus res;

   if(PQueueEmpty(channel->in))
   {
      current = TCPMsgAlloc();
      PQueueStoreP(channel->in, current);
   }
   else
   {
      current = PQueueLookLastP(channel->in);
      if(TCP_MSG_COMPLETE(current))
      {
         current = TCPMsgAlloc();
         PQueueStoreP(channel->in, current);
      }
   }
   res = TCPMsgRead(channel->sock, current);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: TCPChannelWrite()
//
//   Write the current message (if any) as far as possible without
//   blocking (i.e. try a single "write()"). If the message is
//   complete, remove it from the queue.
//
// Global Variables: -
//
// Side Effects    : IO, Memory
//
/----------------------------------------------------------------------*/

MsgStatus TCPChannelWrite(TCPChannel_p channel)
{
   TCPMsg_p current;
   MsgStatus res;

   if(PQueueEmpty(channel->in))
   {
      return NWSuccess;
   }
   current = PQueueLookP(channel->in);
   res = TCPMsgWrite(channel->sock, current);
   if(res == NWSuccess)
   {
      current = PQueueGetNextP(channel->in);
      TCPMsgFree(current);
   }
   return res;
}





/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


