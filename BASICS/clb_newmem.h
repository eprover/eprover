/*-----------------------------------------------------------------------

File  : clb_newmem.h

Author: Stephan Schulz

  This module implements a simple general purpose memory management
  stystem that is efficient for problems with a very regular memory
  access pattern (like most theorem provers). In addition to the
  groundwork it also implements secure versions of standard functions
  making use of memory allocation. This is less elegant, but in most
  cases more efficient, than the old system.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed Aug 13 21:56:20 MET DST 1997
    New

-----------------------------------------------------------------------*/

#ifndef CLB_NEWMEM

#define CLB_NEWMEM

#include <string.h>
#include "clb_verbose.h"
#include "clb_os_wrapper.h"

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* Administrate deallocated memory blocks */

typedef struct memcell
{
   struct memcell* next;
#ifndef NDEBUG
   unsigned long   test;
#endif
}MemCell, *Mem_p;

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define MEM_ARR_SIZE     8192
#define MEM_ALIGN        16
#define MEM_CHUNKLIMIT   (4096/MEM_ALIGN) /* Objects smaller than this are
                   allocated in sets of MEM_MULTIPLIER */
#define MEM_MULTIPLIER   1024
#define MEM_FREE_PATTERN 0xFAFBFAFA
#define MEM_RSET_PATTERN 0x00000000

/* For estimating the real memory consumption of a data type - the
   default may be way of for some memory managers, but should be
   reasonably ok for many situations. If CONSTANT_MEM_ESTIMATE is on,
   a very rough but machine-independent estimate is used. */

#ifdef CONSTANT_MEM_ESTIMATE
#define MEMSIZE(type) "There is a bug in the code! Everything has to work with constants."
#else
#define MEMSIZE(type) (sizeof(type)+sizeof(void*))
#endif


extern bool MemIsLow;
extern Mem_p free_mem_list[];

void* SizeMallocReal(int size);
void  SizeFreeReal(void* junk, int size);

#ifndef NDEBUG
#define SizeFree(junk, size) SizeFreeReal(junk, size); junk=NULL
#define SizeMalloc(size)     SizeMallocReal(size)
#define ENSURE_NULL(junk)    junk=NULL
#else
#define SizeFree(junk, size) SizeFreeReal(junk, size);
#define SizeMalloc(size)     SizeMallocReal(size)
#define ENSURE_NULL(junk)    /* Only defined in debug mode */
#endif

void  MemFlushFreeList(void);
void* SecureMalloc(int size);
void* SecureRealloc(void *ptr, int size);
void  MemAddNewChunk(int mem_index);
char* SecureStrdup(const char* source);
char* SecureStrndup(const char* source, size_t n);
#define FREE(junk) assert(junk);free(junk)

long* IntArrayAlloc(int size);
#define IntArrayFree(array, size) SizeFree(array, size*sizeof(long))

#ifdef CLB_MEMORY_DEBUG
void MemDebugPrintStats(FILE* out);
extern long size_malloc_mem;
extern long size_malloc_count;
extern long size_free_mem;
extern long size_free_count;
extern long clb_free_count;
extern long secure_malloc_count;
extern long secure_malloc_mem;
extern long secure_realloc_count;
extern long secure_realloc_m_count;
extern long secure_realloc_f_count;
#undef FREE
#define FREE(junk) assert(junk); clb_free_count++; free(junk); junk=NULL
#endif

/*-------------------------------------------------------------------------
  If you want to have a special Allocator and Deallocator for a
  datatype just copy the following templates to your .h-file and fill
  them in... The allocated structures will not be initialized - you
  need to write a function build on top of the macros if you want more
  functionality in you Allocator.

#define DataCellAlloc() (DataCell*)SizeMalloc(sizeof(DataCell))
#define DataCellFree(junk)         SizeFree(junk, sizeof(DataCell))

-------------------------------------------------------------------------*/

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
