/*-----------------------------------------------------------------------

File  : cio_network.c

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Helper code for TCP connections and "message" based communication
  over TCP (each message corresponds to a transaction request and is
  packaged as a message to allow parsing in whole).


  Copyright 2011 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1>     New

-----------------------------------------------------------------------*/

#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>

#include "cio_network.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

#define TCP_BACKLOG 10
#define TCP_BUF_SIZE 1025

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/



/*-----------------------------------------------------------------------
//
// Function: create_server_sock_nofail()
//
//   Try to create a bound server socket. Return -1 on failure (with
//   errno describing the failed operation), the socket identifier on
//   success.
//
// Global Variables: -
//
// Side Effects    : Creates and binds the socket.
//
/----------------------------------------------------------------------*/

static int create_server_sock_nofail(int port)
{
   int sock = socket(PF_INET,SOCK_STREAM, IPPROTO_TCP);
   int res, err;
   struct sockaddr_in addr;

   if(sock == -1)
   {
      return -1;
   }

   addr.sin_family = AF_INET;
   addr.sin_port = htons(port);
   addr.sin_addr.s_addr = INADDR_ANY;
   memset(&addr.sin_zero, 0, sizeof(addr.sin_zero));

   int yes = 1;
   res = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
   if(res != -1)
   {
      res = bind(sock, (struct sockaddr *)&addr, sizeof(addr));
   }
   if(res == -1)
   {
      err = errno;
      close(sock);
      errno = err;
      return -1;
   }
   return sock;
}



/*-----------------------------------------------------------------------
//
// Function: create_client_sock_nofail()
//
//   Try to create a client socket connected to the provided host
//   (trying all addresses it resolves to). Return the socket
//   identifier on success, -1 on failure. If the host cannot be
//   resolved, *gai_err is set to the getaddrinfo() error code,
//   otherwise it is set to 0 and errno describes the failure.
//
// Global Variables: -
//
// Side Effects    : Creates and connects the socket.
//
/----------------------------------------------------------------------*/

static int create_client_sock_nofail(char* host, int port, int *gai_err)
{
   int sock = -1;
   int res, err = 0;
   struct addrinfo hints, *addr, *iter;
   char portstr[16];

   snprintf(portstr, sizeof(portstr), "%d", port);
   memset(&hints, 0, sizeof(hints));
   hints.ai_family = PF_UNSPEC;
   hints.ai_socktype = SOCK_STREAM;
   *gai_err = getaddrinfo(host, portstr, &hints, &addr);
   if(*gai_err)
   {
      return -1;
   }
   for(iter=addr; iter; iter = iter->ai_next)
   {
      sock = socket(iter->ai_family, iter->ai_socktype,
                    iter->ai_protocol);
      if(sock < 0)
      {
         err = errno;
         continue;
      }
      res = connect(sock, iter->ai_addr, iter->ai_addrlen);
      if(res == 0)
      {
         break;
      }
      err = errno;
      close(sock);
      sock = -1;
   }
   freeaddrinfo(addr);
   if(sock < 0)
   {
      errno = err;
   }
   return sock;
}




/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/



/*-----------------------------------------------------------------------
//
// Function: TCPMsgAlloc()
//
//   Allocate an initialized TCP message cell.
//
// Global Variables: -
//
// Side Effects    : Memory management
//
/----------------------------------------------------------------------*/

TCPMsg_p  TCPMsgAlloc(void)
{
   TCPMsg_p res = TCPMsgCellAlloc();

   res->content            = DStrAlloc();
   res->len                = -1;
   res->transmission_count = 0;

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: TCPMsgFree()
//
//   Free a TCP message cell.
//
// Global Variables: -
//
// Side Effects    : Memory management.
//
/----------------------------------------------------------------------*/

void TCPMsgFree(TCPMsg_p junk)
{
   DStrFree(junk->content);
   TCPMsgCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: TCPMsgPack()
//
//   Take a string and convert it into a newly allocated TCP Msg.
//
// Global Variables: -
//
// Side Effects    : Memory allocation
//
/----------------------------------------------------------------------*/

TCPMsg_p TCPMsgPack(char* str)
{
   uint32_t len;
   TCPMsg_p handle = TCPMsgAlloc();

   len = strlen(str)+sizeof(len);
   DStrAppendStr(handle->content, "0000");
   *((uint32_t*)DStrAddress(handle->content, 0)) =  htonl(len);
   DStrAppendStr(handle->content, str);
   handle->len = len;

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: TCPMsgUnpack()
//
//   Given a TCP message, return the string and destroy the
//   container. If this ever becomes measurable, we can make this
//   faster by avoiding the copy...
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

char* TCPMsgUnpack(TCPMsg_p msg)
{
   char *res = SecureStrdup(DStrAddress(msg->content, sizeof(uint32_t)));
   TCPMsgFree(msg);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: TCPMsgWrite()
//
//   Send the message over the socket. Return NWError, NWIncomplete,
//   or NWSuccess depending on wether the transmission was partial,
//   complete or a failure.
//
// Global Variables: -
//
// Side Effects    : IO, memory.
//
/----------------------------------------------------------------------*/

MsgStatus TCPMsgWrite(int sock, TCPMsg_p msg)
{
   int remaining, res;

   remaining = msg->len-msg->transmission_count;

   res = write(sock,
              DStrAddress(msg->content, msg->transmission_count),
              remaining);
   if(res < 0 )
   {
      return NWError;
   }
   msg->transmission_count+=res;
   if(!TCP_MSG_COMPLETE(msg))
   {
      return NWIncomplete;
   }
   return NWSuccess;
}


/*-----------------------------------------------------------------------
//
// Function: TCPMsgRead()
//
//   Receive a (partial) TCP message, i.e. do at most one read() for
//   the header (length field) and at most one for the payload. Return
//   NWIncomplete if the message is not yet complete, NWSuccess if it
//   is, NWConnClosed if the connection was closed, and NWError on
//   read errors or a malformed length field. The payload may contain
//   arbitrary bytes, but TCPMsgUnpack() treats it as a C string.
//
// Global Variables: -
//
// Side Effects    : IO, memory
//
/----------------------------------------------------------------------*/

MsgStatus TCPMsgRead(int sock, TCPMsg_p msg)
{
   char     buffer[TCP_BUF_SIZE];
   uint32_t len;
   ssize_t  res;

   /* Handle header */
   if(msg->transmission_count < (int)sizeof(uint32_t))
   {
      res = read(sock,
                 msg->len_buf+msg->transmission_count,
                 sizeof(uint32_t)-msg->transmission_count);
      if(res < 0)
      {
         return NWError;
      }
      if(res == 0)
      {
         return NWConnClosed;
      }
      msg->transmission_count += res;
      if(msg->transmission_count < (int)sizeof(uint32_t))
      {
         return NWIncomplete;
      }
      memcpy(&len, msg->len_buf, sizeof(uint32_t));
      len = ntohl(len);
      if(len < sizeof(uint32_t) || len > INT_MAX)
      {
         return NWError;
      }
      msg->len = len;
      DStrAppendBuffer(msg->content, msg->len_buf, sizeof(uint32_t));
      if(TCP_MSG_COMPLETE(msg))
      {  /* Empty payload */
         return NWSuccess;
      }
   }
   /* Rest */
   len = MIN(TCP_BUF_SIZE, msg->len - msg->transmission_count);
   res = read(sock, buffer, len);
   if(res < 0)
   {
      return NWError;
   }
   if(res == 0)
   {
      return NWConnClosed;
   }
   DStrAppendBuffer(msg->content, buffer, res);
   msg->transmission_count += res;

   if(!TCP_MSG_COMPLETE(msg))
   {
      return NWIncomplete;
   }
   return NWSuccess;
}



/*-----------------------------------------------------------------------
//
// Function: TCPMsgSend()
//
//   Send the message over the connection represented
//   by socket. This will block until transmission is complete. Return
//   status.
//
// Global Variables: -
//
// Side Effects    : Network traffic
//
/----------------------------------------------------------------------*/

MsgStatus TCPMsgSend(int sock, TCPMsg_p msg)
{
   int res = 0;

   while(res != NWSuccess)
   {
      res = TCPMsgWrite(sock, msg);
      if(res==NWError)
      {
         break;
      }
   }
   return res;
}




/*-----------------------------------------------------------------------
//
// Function: TCPMsgRecv()
//
//   Receive and return a message and return it. This will block until
//   transmission terminates.
//
// Global Variables: -
//
// Side Effects    : Memory
//
/----------------------------------------------------------------------*/

TCPMsg_p TCPMsgRecv(int sock, MsgStatus *res)
{
   TCPMsg_p msg = TCPMsgAlloc();

   *res = 0;

   while(*res != NWSuccess)
   {
      *res = TCPMsgRead(sock, msg);
      if(*res == NWError || *res == NWConnClosed)
      {
         break;
      }
   }
   return msg;
}




/*-----------------------------------------------------------------------
//
// Function: TCPStringSend()
//
//   Send the string as a TCP message over the connection represented
//   by socket. This will block until transmission is complete. Return
//   status.
//
// Global Variables: -
//
// Side Effects    : Network traffic
//
/----------------------------------------------------------------------*/

MsgStatus TCPStringSend(int sock, char* str, bool err)
{
   TCPMsg_p msg = TCPMsgPack(str);
   MsgStatus res = TCPMsgSend(sock, msg);

   if(err && res!=NWSuccess)
   {
      SysError("Could not send string message", SYS_ERROR);
   }
   TCPMsgFree(msg);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: TCPStringRecv()
//
//   Receive a message string, unpack it, and return the message
//   content (or NULL on failure).
//
// Global Variables: -
//
// Side Effects    : Memory
//
/----------------------------------------------------------------------*/

char* TCPStringRecv(int sock, MsgStatus* res, bool err)
{
   TCPMsg_p msg = TCPMsgRecv(sock, res);

   if(*res != NWSuccess)
   {
      if(err)
      {
         SysError("Could not receive string message", SYS_ERROR);
      }
      TCPMsgFree(msg);
      return NULL;
   }
   return TCPMsgUnpack(msg);
}


/*-----------------------------------------------------------------------
//
// Function: TCPStringSendX()
//
//   Send a string, fail on error.
//
// Global Variables: -
//
// Side Effects    : Indirect
//
/----------------------------------------------------------------------*/

void TCPStringSendX(int sock, char* str)
{
   TCPStringSend(sock, str, true);
}


/*-----------------------------------------------------------------------
//
// Function: TCPStringRecvX()
//
//   Read and return a string.
//
// Global Variables: -
//
// Side Effects    : Indirect
//
/----------------------------------------------------------------------*/

char* TCPStringRecvX(int sock)
{
   MsgStatus res;

   return TCPStringRecv(sock, &res, true);
}


/*-----------------------------------------------------------------------
//
// Function: CreateServerSock()
//
//   Create a server socket bound to the given port and return
//   it. Fail with error message if the port cannot be creates.
//
// Global Variables: -
//
// Side Effects    : Creates and binds socket
//
/----------------------------------------------------------------------*/

int CreateServerSock(int port)
{
   int sock = create_server_sock_nofail(port);

   if(sock==-1)
   {
      TmpErrno = errno;
      SysError("Cannot create socket for port %d", SYS_ERROR, port);
   }
   return sock;
}


/*-----------------------------------------------------------------------
//
// Function: Listen()
//
//   Thin wrapper around listen() terminating with an error message if
//   it fails.
//
// Global Variables: -
//
// Side Effects    : Socket operation
//
/----------------------------------------------------------------------*/

void Listen(int sock)
{
   int res = listen(sock, TCP_BACKLOG);

   if(res == -1)
   {
      TmpErrno = errno;
      SysError("Failed to switch socket %d to listening",
               SYS_ERROR, sock);
   }
}


/*-----------------------------------------------------------------------
//
// Function: CreateClientSock()
//
//   Create a socket connected to the given host and port. Return sock
//   or terminate with error on fail.
//
// Global Variables: TmpErrno
//
// Side Effects    : Creates and connects socket, may terminate with
//                   error.
//
/----------------------------------------------------------------------*/

int CreateClientSock(char* host, int port)
{
   int gai_err;
   int sock = create_client_sock_nofail(host, port, &gai_err);

   if(gai_err)
   {
      Error("Could not resolve address %s (%s)", SYS_ERROR,
            host, gai_strerror(gai_err));
   }
   if(sock == -1)
   {
      TmpErrno = errno;
      SysError("Could not connect to %s, port %d", SYS_ERROR, host, port);
   }
   return sock;
}




/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


