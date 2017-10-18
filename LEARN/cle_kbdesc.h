/*-----------------------------------------------------------------------

File  : cle_kbdesc.h

Author: Stephan Schulz

Contents

  Data types and functions for representing the knowledge base.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Fri Jul 16 20:12:05 MET DST 1999
    New

-----------------------------------------------------------------------*/

#ifndef CLE_KB

#define CLE_KB

#include <cle_examplerep.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

#define KB_VERSION "0.20dev"

typedef struct kbdesccell
{
   char         *version;
   double       neg_proportion;
   long         fail_neg_examples;
}KBDescCell, *KBDesc_p;


#define KB_ANNOTATION_NO 7

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


#define KBDescCellAlloc() (KBDescCell*)SizeMalloc(sizeof(KBDescCell))
#define KBDescCellFree(junk) SizeFree(junk, sizeof(KBDescCell))

KBDesc_p KBDescAlloc(char* version, double neg_prop, long
           neg_examples);
void     KBDescFree(KBDesc_p desc);

void     KBDescPrint(FILE* out, KBDesc_p desc);
KBDesc_p KBDescParse(Scanner_p in);

char*    KBFileName(DStr_p name, char *basename, char* file);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





