/*-----------------------------------------------------------------------

File  : clb_dstrings.h

Author: Stephan Schulz

  Declarations for dynamic, arbitrary length strings
  (i.e. 0-terminated arrays of characters). The conversion between
  DStrs and C-strings is as simple and efficient as possible. This
  implementation is optimized for strings with a certain behaviour,
  usually experienced when reading input: Most strings are fairly
  small, and for each set of strings the length distribution is fairly
  similar.

  DStrings can also be used as a primitive form of reference-counted
  strings - functions for obtaining counted reference, releasing it,
  and delayed destruction of a still referenced object are
  provided. The use of these functions is optional and requires user
  discipline!

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: Aug 15 17:01:33 MET DST 1997 - New

-----------------------------------------------------------------------*/

#ifndef CLB_DSTRINGS

#define CLB_DSTRINGS

#include <clb_memory.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct dstrcell
{
   char* string;
   long  len;
   long  mem;
   long  refs;
}DStrCell, *DStr_p;

#define DSTRGROW 64

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

extern char NullStr[];

#define DStrCellAlloc() (DStrCell*)SizeMalloc(sizeof(DStrCell))
#define DStrCellFree(junk)         SizeFree(junk, sizeof(DStrCell))

DStr_p DStrAlloc(void);
void   DStrFree(DStr_p junk);

char*   DStrAppendStr(DStr_p strdes, const char* newpart);
char*   DStrAppendChar(DStr_p strdes, char newch);
char*   DStrAppendBuffer(DStr_p strdes, char* buf, int len);
char*   DStrAppendInt(DStr_p strdes, long newpart);
char*   DStrAppendStrArray(DStr_p strdes, char* array[], char*
                           separator);
#define DStrAppendDStr(strdes, str)             \
        DStrAppendStr((strdes), DStrView(str))
char    DStrDeleteLastChar(DStr_p strdes);
#define DStrLastChar(strdes) (DStrLen(strdes)?DStrView(strdes)[DStrLen(strdes)-1]:'\0')
char*   DStrView(DStr_p strdes);
char*   DStrAddress(DStr_p strdes, int index);
char*   DStrCopy(DStr_p strdes);
char*   DStrCopyCore(DStr_p strdes);
char*   DStrSet(DStr_p strdes, char* string);
long    DStrLen(DStr_p strdes);
void    DStrReset(DStr_p strdes);
void    DStrMinimize(DStr_p strdes);
char*   DStrFGetS(DStr_p strdes, FILE* fp);

#define DStrGetRef(strdes)     (((strdes)?((strdes)->refs++):0),strdes)
#define DStrReleaseRef(strdes) if(strdes){DStrFree(strdes);}

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
