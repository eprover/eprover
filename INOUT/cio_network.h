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

#include <sys/socket.h>
#include <stdint.h>
#include <clb_dstrings.h>
#include <clb_pqueue.h>

/* send_msg(char* msg) -> len, bytes*/
/* char* recv_msg()...*/

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


typedef enum
{
   NWIncomplete  = 0,
   NWError       = 1,
   NWConnClosed  = 2,
   NWSuccess     = 3
}MsgStatus;

/* A single message */

typedef struct tcp_msg_cell
{
   DStr_p content; /* Includes 4 bytes initial bytes of total message
                      lengths in network byte order, and the payload. */
   int len;     /* Including length field. */
   int transmission_count; /* How many bytes have been sent/received? */
   char len_buf[sizeof(uint32_t)];
}TCPMsgCell, *TCPMsg_p;



/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


#define TCPMsgCellAlloc()    (TCPMsgCell*)SizeMalloc(sizeof(TCPMsgCell))
#define TCPMsgCellFree(junk) SizeFree(junk, sizeof(TCPMsgCell))

#define TCP_MSG_COMPLETE(msg) ((msg)->len == (msg)->transmission_count)

TCPMsg_p  TCPMsgAlloc(void);
void   TCPMsgFree(TCPMsg_p junk);


TCPMsg_p TCPMsgPack(char* str);
char*    TCPMsgUnpack(TCPMsg_p msg);

MsgStatus TCPMsgWrite(int sock, TCPMsg_p msg);
MsgStatus TCPMsgRead(int sock, TCPMsg_p msg);

MsgStatus TCPMsgSend(int sock, TCPMsg_p msg);
TCPMsg_p  TCPMsgRecv(int sock, MsgStatus *res);

MsgStatus TCPStringSend(int sock, char* str, bool err);
char*     TCPStringRecv(int sock, MsgStatus* res, bool err);

void      TCPStringSendX(int sock, char* str);
char*     TCPStringRecvX(int sock);


int  CreateServerSock(int port);
void Listen(int sock);

int  CreateClientSock(char* host, int port);

#endif


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





