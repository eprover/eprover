/*-----------------------------------------------------------------------

File  : cio_network.h

Author: Stephan Schulz (schulz@eprover.org)

Contents
 
  Helper code for TCP connections and "message" based communication
  over TCP (each message corresponds to a transaction request and is
  packages as a message to allow parsing in whole).

  Copyright 2011 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Wed Mar  9 22:24:40 CET 2011
    New

-----------------------------------------------------------------------*/

#ifndef CIO_NETWORK

#define CIO_NETWORK

#include <clb_dstrings.h>
#include <clb_pqueue.h>

/* send_msg(char* msg) -> len, bytes*/
/* char* recv_msg()...*/

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


typedef struct tcp_msg_cell
{
   DStr_p content;
   int length;
   int transmission_count;
}TCPMsgCell, *TCPMsg_p;



typedef struct tcp_channel_cell
{
   int sock;
   PQueue_p messages;   
}TCPChannelCell, *TCPChannel_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define TCP_BACKLOG 10

#define TCPMsgCellAlloc()    (TCPMsgCell*)SizeMalloc(sizeof(TCPMsgCell))
#define TCPMsgCellFree(junk) SizeFree(junk, sizeof(TCPMsgCell))


TCPMsg_p  TCPMsgAlloc();
void   TCPMsgFree(TCPMsg_p junk);


TCPMsg_p TCPMessagePack(char* str);
char*    TCPMessageUnpack(TCPMsg_p msg);

int      TCPMessageSend(int sock, TCPMsg_p msg);
int      TCPMessageRecv(int sock, TCPMsg_p msg);

int  CreateServerSock(int port);
void Listen(int sock);



#endif


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





