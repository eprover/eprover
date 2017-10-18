/*-----------------------------------------------------------------------

File  : cco_esession.c

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Code for handling a single E server connection.

  Copyright 2011 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Fri Apr 22 15:23:07 CEST 2011
    New

-----------------------------------------------------------------------*/

#include "cco_esession.h"



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
// Function: ESessionAlloc()
//
//   Allocate an initialized ESession cell. This is not yet listening.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

ESession_p ESessionAlloc(int sock)
{
   ESession_p handle = ESessionCellAlloc();

   handle->state   = ESNoState;
   handle->channel = TCPChannelAlloc(sock);
   handle->running = NULL;

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: ESessionFree()
//
//   Free an ESession.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void ESessionFree(ESession_p junk)
{
   TCPChannelFree(junk->channel);
   if(junk->running)
   {
      EPCtrlSetFree(junk->running, true);
   }
   ESessionCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: ESessionInitFDSet()
//
//   Set the fd bits for all file descriptors relevant to the
//   server. Return largest fd.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int ESessionInitFDSet(ESession_p session,
                     fd_set *rd_fds,
                     fd_set *wr_fds)
{
   int tmp, max_fd;

   if(session->state==ESNoState || session->state==ESStale)
   {
      return 0;
   }
   assert(session->channel->sock >=0);

   max_fd = session->channel->sock;

   FD_SET(session->channel->sock, rd_fds);
   if(TCPChannelHasOutMsg(session->channel))
   {
      FD_SET(session->channel->sock, wr_fds);
   }
   if(session->running)
   {
      tmp = EPCtrlSetFDSet(session->running, rd_fds);
      max_fd = MAX(max_fd, tmp);
   }
   return max_fd;
}


/*-----------------------------------------------------------------------
//
// Function: ESessionDoIO()
//
//   Perform I/O on all connections of the session.
//
// Global Variables: -
//
// Side Effects    : IO, memory
//
/----------------------------------------------------------------------*/

void ESessionDoIO(ESession_p session,
                  fd_set *rd_fds,
                  fd_set *wr_fds)
{
   MsgStatus res;

   if(session->running)
   {
      /* Do I/O for the controlled processes */
   }
   if(session->state==ESNoState || session->state==ESStale)
   {
      return;
   }
   assert(session->channel->sock >=0);
   if(FD_ISSET(session->channel->sock, rd_fds))
   {
      res = TCPChannelRead(session->channel);
      switch(res)
      {
      case NWConnClosed:
      case NWError:
            TCPChannelClose(session->channel);
            session->state = ESStale;
            break;
      case NWSuccess:
            TCPChannelSendStr(session->channel, "wait");
            break;
      default:
            break;
      }
   }
   if(session->state==ESNoState || session->state==ESStale)
   {
      return;
   }
   if(FD_ISSET(session->channel->sock, wr_fds))
   {
      res = TCPChannelWrite(session->channel);
      switch(res)
      {
      case NWConnClosed:
      case NWError:
            TCPChannelClose(session->channel);
            session->state = ESStale;
            break;
      default:
            break;
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: ESessionProcessCmds()
//
//   Process the messages stored in the input queue of the channel.
//
// Global Variables: -
//
// Side Effects    : IO
//
/----------------------------------------------------------------------*/

void ESessionProcessCmds(ESession_p session)
{
   TCPMsg_p msg;
   char     *str;

   while(TCPChannelHasInMsg(session->channel))
   {
      msg = TCPChannelGetInMsg(session->channel);
      str = TCPMsgUnpack(msg);
      printf("Received: %s\n", str);
      FREE(str);
   }
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
