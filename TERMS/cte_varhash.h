/*-----------------------------------------------------------------------

File  : cte_varhash.h

Author: Stephan Schulz

Contents

  Data structures for hashing and traversing variable occurences.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed Jul 22 04:53:43 MET DST 1998
    New
<2> Sun Jul 13 02:33:42 CEST 2003
    Added code for using PDArrays instead of slow and complicated
    VarHashes...I'm reasonably stupid on occasion.

-----------------------------------------------------------------------*/

#ifndef CTE_VARHASH

#define CTE_VARHASH


#include <cte_termvars.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


/* We sometimes need to collect sets of variables and information
   about them, e.g. in checking the KBO variable condition. For small
   sets (e.g. variables in a term), this can be done rather
   efficiently in a hash. */

#define VAR_HASH_SIZE 16 /* 2^n */
#define VAR_HASH_MASK (VAR_HASH_SIZE - 1)

typedef struct var_hash_entry_cell
{
   Term_p                     key;
   long                       val;
   struct var_hash_entry_cell *next;
}VarHashEntryCell, *VarHashEntry_p;

typedef struct var_hash_cell
{
   VarHashEntry_p hash[VAR_HASH_SIZE];
}VarHashCell, *VarHash_p;




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


#define VarHashEntryCellAlloc() \
      (VarHashEntryCell*)SizeMalloc(sizeof(VarHashEntryCell))
#define VarHashEntryCellFree(junk) \
      SizeFree(junk, sizeof(VarHashEntryCell))

static inline VarHashEntry_p VarHashEntryAlloc(Term_p var, long value);
void           VarHashEntryListFree(VarHashEntry_p list);

#define VarHashCellAlloc() \
      (VarHashCell*)SizeMalloc(sizeof(VarHashCell))
#define VarHashCellFree(junk) \
      SizeFree(junk, sizeof(VarHashCell))

static inline VarHash_p VarHashAlloc(void);
void           VarHashFree(VarHash_p junk);
int            VarHashFunction(Term_p var);
VarHashEntry_p VarHashListFind(VarHashEntry_p list, Term_p var);
long           VarHashAddValue(VarHash_p hash, Term_p var, long
                value);
void VarHashAddVarDistrib(VarHash_p hash, Term_p term, DerefType
           deref, long add);
static inline VarHashEntry_p VarHashFind(VarHash_p hash, Term_p
                    var);

void PDArrayAddVarDistrib(PDArray_p array, Term_p term, DerefType
           deref, long add);



/*---------------------------------------------------------------------*/
/*                 Inline Functions                                    */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: VarHashEntryAlloc()
//
//   Allocate an initialized hash entry cell.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static inline VarHashEntry_p VarHashEntryAlloc(Term_p var, long
                     value)
{
   VarHashEntry_p handle = VarHashEntryCellAlloc();

   handle->key = var;
   handle->val = value;
   handle->next = NULL;

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: VarHashAlloc()
//
//   Allocate an initialized variable hash.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static inline VarHash_p VarHashAlloc(void)
{
   VarHash_p handle;
   int       i;

   handle = VarHashCellAlloc();
   for(i=0; i<VAR_HASH_SIZE; i++)
   {
      handle->hash[i] = NULL;
   }
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: VarHashFind()
//
//   Return the entry for var in hash (NULL if non-existant).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static inline VarHashEntry_p VarHashFind(VarHash_p hash, Term_p
                    var)
{
   int i = VarHashFunction(var);

   return VarHashListFind(hash->hash[i], var);
}


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





