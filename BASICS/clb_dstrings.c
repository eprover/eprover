/*-----------------------------------------------------------------------

File  : clb_dstrings.c

Author: Stephan Schulz

  Implementation of the Dynamic String functions.  

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Fri Aug 15 17:17:20 MET DST 1997
    New

-----------------------------------------------------------------------*/

#include "clb_dstrings.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

char NullStr[] = "";

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
// Function: DStrAlloc()
//
//   Return a pointer to an initialized DStrCell.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

DStr_p DStrAlloc()
{
   DStr_p handle = DStrCellAlloc();

   handle->string = NULL;
   handle->mem = 0;
   handle->len = 0;
   handle->refs = 1;

   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: DStrFree()
//
//   Decrease the reference counter. If it is equal to 0, free both
//   the DStr-Cell and the contained string. 
//
// Global Variables: -
//
// Side Effects    : Memory Operations
//
/----------------------------------------------------------------------*/

void DStrFree(DStr_p junk)
{
   assert(junk);
   assert(junk->refs >= 1);

   junk->refs--;

   if(!junk->refs)
   {
      if(junk->string)
      {
	 FREE(junk->string);
      }
      DStrCellFree(junk);
   }
}

/*-----------------------------------------------------------------------
//
// Function: DStrAppendStr()
//
//   Append a C-String to a DStr efficiently
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

char* DStrAppendStr(DStr_p strdes, char* newpart)
{
   long newlen,
        newmem;
   
   assert(strdes);
   assert(newpart);

   newlen = strlen(newpart);
   newmem = strdes->mem; 
   
   while(strdes->len+newlen >= newmem) /* I expect this loop to be
					   computed at most once in
					   the average case, so it
					   should be more efficient
					   than the direct computation
					   (which requires a
					   division. */
   {
      newmem += DSTRGROW;
   }
   if(newmem > strdes->mem)
   {
      strdes->string = SecureRealloc(strdes->string, newmem);
      strdes->mem = newmem;
      strdes->string[strdes->len] = '\0';
   }
   strcat(strdes->string, newpart);
   strdes->len += newlen;
   
   return strdes->string;
}

/*-----------------------------------------------------------------------
//
// Function: DStrAppendChar()
//
//   Append a single character to a DStr. This is the operation that
//   will probably be called with the highest frequency, so I try to
//   make it efficient.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

char* DStrAppendChar(DStr_p strdes, char newch)
{
   assert(strdes);
   assert(newch);

   if(strdes->len+1 >= strdes->mem)
   {
      strdes->string = SecureRealloc(strdes->string, strdes->len+DSTRGROW);
      strdes->mem = strdes->len+DSTRGROW;
   }
   strdes->string[strdes->len] = newch;
   strdes->len++;
   strdes->string[strdes->len] = '\0';

   return strdes->string;
}


/*-----------------------------------------------------------------------
//
// Function: DStrAppendInt()
//
//   Append the string representation of a long number to a DStr.
//
// Global Variables: ErrStr
//
// Side Effects    : By DStrAppendStr(), changes ErrStr
//
/----------------------------------------------------------------------*/

char* DStrAppendInt(DStr_p strdes, long newpart)
{
   sprintf(ErrStr, "%ld", newpart);
   return DStrAppendStr(strdes, ErrStr);
}



/*-----------------------------------------------------------------------
//
// Function: DStrAppendStrArray()
//
//   Append the elements of the NULL terminated array to str as a
//   separator separated list.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

char* DStrAppendStrArray(DStr_p strdes, char* array[], char*
			 separator)
{
   int i=0;

   if(array[0])
   {
      DStrAppendStr(strdes, array[0]);
      for(i=1; array[i]; i++)
      {
	 DStrAppendStr(strdes, separator);
	 DStrAppendStr(strdes, array[i]);
      }
   }
   return strdes->string;
}


/*-----------------------------------------------------------------------
//
// Function: DStrView()
//
//   Return a pointer to the stored C-string. This is guaranteed to stay
//   fresh as long as no other DStr-Operation is performed on the
//   string. The user is responsible for the use of this pointer - in
//   particular, write-operations on the string should not change the
//   lenght of the string, or it will become corrupted!
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

char* DStrView(DStr_p strdes)
{
   assert(strdes);
   
   if(strdes->string)
   {
      return strdes->string;
   }
   return NullStr;
}


/*-----------------------------------------------------------------------
//
// Function: DStrCopy()
//
//   Return a pointer to a copy of the stored string. The user is
//   responsible for freeing the memory (via free()/FREE()).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

char* DStrCopy(DStr_p strdes)
{
   char* handle;

   assert(strdes);
   
   if(strdes->string)
   {
      /* As we know the length, this should be more efficient than
	 using SecureStrdup() */

      handle = SecureMalloc(strdes->len+1);
      strcpy(handle,strdes->string);
      return handle;
   }
   return SecureStrdup(NullStr);
}


/*-----------------------------------------------------------------------
//
// Function: DStrSet()
//
//   Set a dstring to a given C-String.
//
// Global Variables: -
//
// Side Effects    : Via DStrAppendStr()
//
/----------------------------------------------------------------------*/

char* DStrSet(DStr_p strdes, char* string)
{
   assert(strdes);
   assert(string);

   DStrReset(strdes);
   DStrAppendStr(strdes,string);
   
   return strdes->string;
}


/*-----------------------------------------------------------------------
//
// Function: DStrLen()
//
//   Return the length of a stored string (efficiency hack...)
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long  DStrLen(DStr_p strdes)
{
   assert(strdes);

   return strdes->len;
}


/*-----------------------------------------------------------------------
//
// Function: DStrReset()
//
//   Set the string to "" efficiently (does _not_ change internal
//   memory - call this to e.g. reinitialize a string in a loop) 
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void  DStrReset(DStr_p strdes)
{
   assert(strdes);

   if(strdes->string)
   {
      strdes->string[0] = '\0';
      strdes->len = 0;
   }
}


/*-----------------------------------------------------------------------
//
// Function: DStrMinimize()
//
//  Minimize the space used to store the string in strdes. 
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void  DStrMinimize(DStr_p strdes)
{
   assert(strdes);

   if(strdes->string)
   {
      if(strdes->len)
      {
	 strdes->string = SecureRealloc(strdes->string,
					strdes->len+1);
	 strdes->mem = strdes->len+1;
      }
      else
      {
	 FREE(strdes->string);
	 strdes->mem = 0;
      }
   }
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


