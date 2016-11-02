/*-----------------------------------------------------------------------

File  : cco_esession.h

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Code and data structures representing a single session
  (i.e. connection to the user and all processes run on behalf of this
  user).

  Copyright 2011 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Fri Apr 22 15:08:31 CEST 2011
    New

-----------------------------------------------------------------------*/

#ifndef CCO_ESESSION

#define CCO_ESESSION

#include <netinet/in.h>
#include <cio_multiplexer.h>
#include <cco_proc_ctrl.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef enum
{
   ESNoState,  /* Default */
   ESWaiting,  /* Connected, not processing */
   ESActive,   /* Connected and processing */
   ESStale     /* Disconnected, will be removed */
}ESessionState;


typedef struct esession_cell
{
   ESessionState state;
   TCPChannel_p  channel;
   EPCtrlSet_p   running;
}ESessionCell, *ESession_p;



/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/
/*
#define XXXCellAlloc()    (XXXCell*)SizeMalloc(sizeof(XXXCell))
#define XXXCellFree(junk) SizeFree(junk, sizeof(XXXCell))

XXX_p  XXXAlloc();
void   XXXFree(XXX_p junk);
*/

#define ESessionCellAlloc()    (ESessionCell*)SizeMalloc(sizeof(ESessionCell))
#define ESessionCellFree(junk) SizeFree(junk, sizeof(ESessionCell))

ESession_p  ESessionAlloc(int sock);
void        ESessionFree(ESession_p junk);

int         ESessionInitFDSet(ESession_p session,
                              fd_set *rd_fds,
                              fd_set *wr_fds);

#define     ESessionSetState(session, state) (session)->state = state

void        ESessionDoIO(ESession_p session,
                         fd_set *rd_fds,
                         fd_set *wr_fds);

void        ESessionProcessCmds(ESession_p session);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





