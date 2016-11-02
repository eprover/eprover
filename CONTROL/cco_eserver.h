/*-----------------------------------------------------------------------

File  : cco_eserver.h

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

#ifndef CCO_ESERVER

#define CCO_ESERVER

#include <netinet/in.h>
#include <cio_multiplexer.h>
#include <cco_esession.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


typedef struct eserver_cell
{
   int         listening;  /* Socket for connections */
   PQueue_p    sessions;   /* Connections - head is active */
}EServerCell, *EServer_p;



/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/
/*
#define XXXCellAlloc()    (XXXCell*)SizeMalloc(sizeof(XXXCell))
#define XXXCellFree(junk) SizeFree(junk, sizeof(XXXCell))

XXX_p  XXXAlloc();
void   XXXFree(XXX_p junk);
*/

#define EServerCellAlloc()    (EServerCell*)SizeMalloc(sizeof(EServerCell))
#define EServerCellFree(junk) SizeFree(junk, sizeof(EServerCell))

EServer_p  EServerAlloc(void);
void       EServerFree(EServer_p junk);

void       EServerReset(EServer_p server);
bool       EServerListen(EServer_p server, int port);

bool       EServerAccept(EServer_p server);

int        EServerInitFDSet(EServer_p server,
                            fd_set *rd_fds,
                            fd_set *wr_fds);



#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





