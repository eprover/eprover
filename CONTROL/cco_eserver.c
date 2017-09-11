/*-----------------------------------------------------------------------

File  : cco_eserver.c

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Control code for realising the E server.

  Copyright 2011 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Thu Mar 17 01:08:00 CET 2011
    New

-----------------------------------------------------------------------*/

#include "cco_eserver.h"



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
// Function: EServerAlloc()
//
//   Allocate an initialized EServer cell. This is not yet listening.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

EServer_p EServerAlloc(void)
{
   EServer_p handle = EServerCellAlloc();

   handle->listening = -1;
   handle->sessions  = PQueueAlloc();

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: EServerFree()
//
//   Free an EServer.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void EServerFree(EServer_p junk)
{
}


/*-----------------------------------------------------------------------
//
// Function: EServerReset()
//
//   Close all communication channels and delete their queues.
//
// Global Variables: -
//
// Side Effects    : Memory operations, drops connections
//
/----------------------------------------------------------------------*/

void EServerReset(EServer_p server)
{
}


/*-----------------------------------------------------------------------
//
// Function: EServerListen()
//
//   Switch the server to listening mode on the given port
//   number. Return success (true) or failure.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool EServerListen(EServer_p server, int port)
{
   assert(server->listening == -1);

   server->listening = CreateServerSock(port);
   if(server->listening == -1)
   {
      return false;
   }
   Listen(server->listening);

   return true;
}


/*-----------------------------------------------------------------------
//
// Function: EServerAccept()
//
//   Accept a new connection on the listening port and queue it in the
//   connection queue. This assumes that there is a pending connection
//   (e.g. indicated via select). Return success or failure.
//
// Global Variables: -
//
// Side Effects    : Creates sock and connection, memory management.
//
/----------------------------------------------------------------------*/

bool EServerAccept(EServer_p server)
{
   struct sockaddr addr;
   socklen_t       addr_len = sizeof(addr);
   int             sock;
   ESession_p      session;

   assert(server->listening != -1);

   sock = accept(server->listening, &addr, &addr_len);

   if(sock == -1)
   {
      TmpErrno = errno;
      SysWarning("Failure to accept connection");
      return false;
   }
   session = ESessionAlloc(sock);
   PQueueStoreP(server->sessions, session);

   return true;
}


/*-----------------------------------------------------------------------
//
// Function: EServerInitFDSet()
//
//   Set the fd bits for all file descriptors relevant to the
//   server. Return largest fd.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int EServerInitFDSet(EServer_p server,
                     fd_set *rd_fds,
                     fd_set *wr_fds)
{
   long index;
   int tmp, max_fd = 0;
   ESession_p handle;

   for(index = PQueueTailIndex(server->sessions);
       index != -1;
       index =  PQueueIncIndex(server->sessions, index))
   {
      handle = PQueueElementP(server->sessions, index);
      tmp = ESessionInitFDSet(handle, rd_fds, wr_fds);
      max_fd = MAX(max_fd, tmp);
   }
   max_fd = MAX(max_fd, server->listening);
   FD_SET(server->listening, rd_fds);

   return max_fd;
}






/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


