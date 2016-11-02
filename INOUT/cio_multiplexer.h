/*-----------------------------------------------------------------------

File  : cio_multiplexer.h

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Code for handling several communication channels.

  Copyright 2011 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Fri Mar 11 21:00:09 CET 2011
    New

-----------------------------------------------------------------------*/

#ifndef CIO_MULTIPLEXER

#define CIO_MULTIPLEXER


#include <cio_network.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


typedef struct tcp_channel_cell
{
   int             sock;
   PQueue_p        in;
   PQueue_p        out;
}TCPChannelCell, *TCPChannel_p;




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


/*
#define XXXCellAlloc()    (XXXCell*)SizeMalloc(sizeof(XXXCell))
#define XXXCellFree(junk) SizeFree(junk, sizeof(XXXCell))

XXX_p  XXXAlloc();
void   XXXFree(XXX_p junk);
*/


#define TCPChannelCellAlloc()    (TCPChannelCell*)SizeMalloc(sizeof(TCPChannelCell))
#define TCPChannelCellFree(junk) SizeFree(junk, sizeof(TCPChannelCell))

TCPChannel_p  TCPChannelAlloc(int sock);
void          TCPChannelFree(TCPChannel_p junk);
void          TCPChannelClose(TCPChannel_p channel);
#define       TCPChannelHasOutMsg(channel) (!PQueueEmpty(channel->out))
bool          TCPChannelHasInMsg(TCPChannel_p channel);
#define       TCPChannelGetInMsg(channel) (PQueueGetNextP((channel)->in))
void          TCPChannelSendMsg(TCPChannel_p channel, TCPMsg_p msg);
void          TCPChannelSendStr(TCPChannel_p channel, char* str);

MsgStatus     TCPChannelRead(TCPChannel_p channel);
MsgStatus     TCPChannelWrite(TCPChannel_p channel);



#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





