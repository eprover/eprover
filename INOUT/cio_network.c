/*-----------------------------------------------------------------------

File  : cio_network.c

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

<1>     New

-----------------------------------------------------------------------*/

#include <netinet/in.h>
#include <sys/socket.h>

#include "cio_network.h"



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
// Function: create_server_sock_nofail()
//
//   Try to create a bound server socket. Return -1 on failure, the
//   socket identifier on success.
//
// Global Variables: -
//
// Side Effects    : Creates and binds the socket.
//
/----------------------------------------------------------------------*/

int create_server_sock_nofail(int port)
{
   int sock = socket(PF_INET,SOCK_STREAM, IPPROTO_IP);
   int res;
   struct sockaddr_in addr;

   if(sock == -1)
   {
      return -1;
   }   
   addr.sin_family = AF_INET;
   addr.sin_port = htons(port);
   addr.sin_addr.s_addr = INADDR_ANY;

   res = bind(sock, (struct sockaddr *)&addr, sizeof(addr));

   if(res == -1)
   {
      return -1;
   }
   return sock;
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/



TCPMsg_p  TCPMsgAlloc();
void   TCPMsgFree(TCPMsg_p junk);


TCPMsg_p TCPMessagePack(char* str);
char*    TCPMessageUnpack(TCPMsg_p msg);

int      TCPMessageSend(int sock, TCPMsg_p msg);
int      TCPMessageRecv(int sock, TCPMsg_p msg);



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
               SYS_ERROR, socket);      
   }
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


