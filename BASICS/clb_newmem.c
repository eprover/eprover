/*-----------------------------------------------------------------------

File  : clb_memory.c

Author: Stephan Schulz

  Memory management. The implemented routines are very simple, because
  they make use of the standard memory management for reorganization.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Thu Aug 14 10:00:35 MET DST 1997
    new

-----------------------------------------------------------------------*/

#include "clb_newmem.h"


/*-----------------------------------------------------------------------*/
/*                         Globale Variable                              */
/*-----------------------------------------------------------------------*/

/* This global variable is set whenever malloc() failed and triggered
   a free memory reorganization. User programs may examine this
   variable to take certain measures (and may reset it if they think
   that they freed significant amounts of memory). However, this is
   somewhat discouraged - do you really want your program to depend in
   pretty complex ways on uncontrolable features like the amount of
   free memory available to your process? _I_ only use it for some
   measurements ;-)  StS */

bool MemIsLow = false;


Mem_p free_mem_list[MEM_ARR_SIZE] = {NULL};

#ifdef CLB_MEMORY_DEBUG
long size_malloc_mem = 0;
long size_malloc_count = 0;
long size_free_mem = 0;
long size_free_count = 0;
long clb_free_count = 0;
long secure_malloc_count = 0;
long secure_malloc_mem = 0;
long secure_realloc_count = 0;
long secure_realloc_m_count = 0;
long secure_realloc_f_count = 0;
#endif

/*-----------------------------------------------------------------------*/
/*                  Exportierte Funktionen                               */
/*-----------------------------------------------------------------------*/



/*-----------------------------------------------------------------------
//
// Function: MemFlushFreeList()
//
//   Returns all memory kept in free_mem_list[] to the operation
//   system. This is useful if a very different memory access pattern
//   is expected (SizeFree() never reorganizes the memory
//   automatically).
//
// Global Variables: free_mem_list[]
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void MemFlushFreeList(void)
{
   VERBOUT("MemFlushFreeList() called - this is a dummy in the new memory handler\n");
}

/*-----------------------------------------------------------------------
//
// Function: SecureMalloc()
//
//  Returns a pointer to an unused memory block sized size. If
//  possible, a fresh block is allocated, if not, the
//  reorganization of free_mem_list is triggered, if still no memory
//  is available, an error will be produced.
//
// Global Variables: free_mem_list
//
// Side Effects    : Memory operations, possibly error
//
/----------------------------------------------------------------------*/

void* SecureMalloc(int size)
{
   void* handle;

   #ifdef CLB_MEMORY_DEBUG
   secure_malloc_count++;
   secure_malloc_mem += size;
   #endif

   handle = (void*)malloc(size);

   if(!handle)
   {    /* malloc has no memory left  */
      MemIsLow = true;
      MemFlushFreeList(); /* Return own freelist */

      handle = (void*)malloc(size);

      if(!handle)
      {   /*  Still nothing...*/
#ifdef PRINT_SOMEERRORS_STDOUT
    PrintRusage(stdout);
    fprintf(stdout, COMCHAR" Failure: Resource limit exceeded (memory)\n");
#endif
         Error("Out of Memory", OUT_OF_MEMORY);
      }
   }
   return handle;
}

/*-------------------------------------------------------------------------
//
// Function: SecureRealloc()
//
//   Imitates realloc, but reorganizes free_mem_list to get new memory
//   if the block has to be moved and no memory is available. Will
//   terminate with OUT_OF_MEMORY if no memory is found.
//
// Global Variables: -
//
// Side Effect     : Via SecureMalloc()
//
//-----------------------------------------------------------------------*/

void* SecureRealloc(void *ptr, int size)
{
   void* handle;

#ifdef CLB_MEMORY_DEBUG
   secure_realloc_count++;
   if(ptr && !size)
   {
      secure_realloc_f_count++;
   }
   else if(!ptr && size)
   {
      secure_realloc_m_count++;
   }
#endif
   /* SunOS realloc() is broken, so here is a stupid workaround...*/

   handle = ptr?realloc(ptr,size):malloc(size);
   if(!handle && size!=0)
   {
      MemFlushFreeList();
      handle = ptr?realloc(ptr,size):malloc(size);
      if(!handle)
      {   /*  Still nothing...*/
#ifdef PRINT_SOMEERRORS_STDOUT
    PrintRusage(stdout);
    fprintf(stdout, COMCHAR" Failure: Resource limit exceeded (memory)\n");
#endif
    Error("Out of Memory", OUT_OF_MEMORY);
      }
   }
   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: SizeMallocReal()
//
//   Returns a block of memory sized size using the internal
//   free-list. This block is not freeable with free(), but otherwise
//   should behave like a normal malloc'ed block.
//
// Global Variables: free_mem_list[]
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void* SizeMallocReal(int size)
{
   Mem_p   handle;
   int    mem_index;

   size = MAX(size, sizeof(MemCell));

   mem_index = (size+MEM_ALIGN-1)/MEM_ALIGN;

   assert(mem_index > 0);

   if(mem_index<MEM_ARR_SIZE)
   {
      if(!free_mem_list[mem_index])
      {
    if(size < MEM_CHUNKLIMIT)
    {
       MemAddNewChunk(mem_index);
    }
    else
    {
       free_mem_list[mem_index] = SecureMalloc(size);
       free_mem_list[mem_index]->next = NULL;
       assert((free_mem_list[mem_index]->test = MEM_FREE_PATTERN, true));
    }
      }
      assert(free_mem_list[mem_index]->test = MEM_FREE_PATTERN);
      assert((free_mem_list[mem_index]->test = MEM_RSET_PATTERN, true));
      handle = (void*)(free_mem_list[mem_index]);
      assert(handle);
      free_mem_list[mem_index] = free_mem_list[mem_index]->next;
   }
   else
   {
      handle = SecureMalloc(size);
   }
   #ifdef CLB_MEMORY_DEBUG
   size_malloc_mem+=size;
   size_malloc_count++;
   #endif
   #ifdef CLB_MEMORY_DEBUG2
   printf("\nBlock %p A: size %d\n", handle, size);
   #endif
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: SizeFreeReal()
//
//  Returns a block sized size. Note: size has to be exact - you
//  should only give blocks to SizeFree() that have been allocated
//  with malloc(size) or SizeMalloc(size). Giving blocks that are to
//  big wastes memory, blocks that are to small will result in more
//  serious trouble (segmentation faults).
//
// Global Variables: free_mem_list[]
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void SizeFreeReal(void* junk, int size)
{
   int    mem_index;

   assert(junk!=NULL);
   #ifdef CLB_MEMORY_DEBUG2
   printf("\nBlock %p D: size %d\n", junk, size);
   #endif

   size = MAX(size, sizeof(MemCell));
   mem_index = (size+MEM_ALIGN-1)/MEM_ALIGN;

   if(mem_index<MEM_ARR_SIZE)
   {
      ((Mem_p)junk)->next = free_mem_list[mem_index];
      free_mem_list[mem_index] = (Mem_p)junk;
      assert(free_mem_list[mem_index]->test != MEM_FREE_PATTERN);
      assert(free_mem_list[mem_index]->test = MEM_FREE_PATTERN);
   }
   else
   {
      FREE(junk);
   }
   #ifdef CLB_MEMORY_DEBUG
   size_free_mem+=size;
   size_free_count++;
   #endif
}


/*-----------------------------------------------------------------------
//
// Function: MemAddNewChunk()
//
//   Allocate a block of size MEM_MULTIPLIER*mem_index*MEM_ALIGN and split
//   it into MEM_MULTIPLIER blocks linked into
//   free_mem_list[index]. In other words, fill up the list of fresh
//   blocks of a given size.
//
// Global Variables: free_mem_list
//
// Side Effects    : Memory allocation, changes free_mem_list[index]
//
/----------------------------------------------------------------------*/

void MemAddNewChunk(int mem_index)
{
   Mem_p handle, old;
   char* block;
   int   i, size;

   old   = free_mem_list[mem_index];
   size  = mem_index*MEM_ALIGN;
   block = SecureMalloc(size*MEM_MULTIPLIER);
   free_mem_list[mem_index] = (Mem_p)block;

   for(i=0; i< MEM_MULTIPLIER; i++)
   {
      handle        = (Mem_p)block;
      assert(handle->test = MEM_FREE_PATTERN);
      block        += size;
      handle->next  = (Mem_p)block;
   }
   handle->next = old;
}

/*-----------------------------------------------------------------------
//
// Function: SecureStrdup()
//
//   Implements the functionality of strdup, but uses SecureMalloc()
/    for the memory handling.
//
// Global Variables: -
//
// Side Effects    : By SecureMalloc()
//
/----------------------------------------------------------------------*/

char* SecureStrdup(const char* source)
{
   char* handle;

   handle = (char*)SecureMalloc(strlen(source)+1);
   strcpy(handle,source);

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: SecureStrndup()
//
//   Implements the functionality of GNU strndup, but uses
//   SecureMalloc() for the memory handling (creates a NULL-terminated
//   copy of the string or the first n bytes of it).
//
// Global Variables: -
//
// Side Effects    : By SecureMalloc()
//
/----------------------------------------------------------------------*/

char* SecureStrndup(const char* source, size_t n)
{
   char* handle;
   size_t len;

   assert(source);
   assert(n>=0);

   len = strlen(source);

   if(len > n)
   {
      handle = SecureMalloc(n+1);
      strncpy(handle,source, n);
      handle[n]='\0';
   }
   else
   {
      handle = SecureStrdup(source);
   }
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: IntArrayAlloc()
//
//   Return a pointer to  a freshly allocated, 0-initialized block of
//   longs.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

long* IntArrayAlloc(int size)
{
   long *res;
   int i;

   res = SizeMalloc(size*sizeof(long));

   for(i=0; i<size; i++)
   {
      res[i]=0;
   }
   return res;
}




#ifdef CLB_MEMORY_DEBUG

/*-----------------------------------------------------------------------
//
// Function: MemDebugPrintStats()
//
//   Print information about allocated and deallocated memory.
//
// Global Variables: size_malloc_mem, size_malloc_count,
//                   size_free_mem, size_free_count
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void MemDebugPrintStats(FILE* out)
{
   fprintf(out,
      "\n"COMCHAR" -------------------------------------------------\n");
   fprintf(out,
      COMCHAR" Total SizeMalloc()ed memory: %ld Bytes (%ld requests)\n",
      size_malloc_mem, size_malloc_count);
   fprintf(out,
      COMCHAR" Total SizeFree()ed   memory: %ld Bytes (%ld requests)\n",
      size_free_mem, size_free_count);
   fprintf(out,
      COMCHAR" New requests: %6ld (%6ld by SecureMalloc(), %6ld by SecureRealloc())\n",
      secure_malloc_count+secure_realloc_m_count,
      secure_malloc_count, secure_realloc_m_count);
   fprintf(out,
      COMCHAR" Total SecureMalloc()ed memory: %ld Bytes\n", secure_malloc_mem);
   fprintf(out,
      COMCHAR" Returned:    %6ld (%6ld by FREE(),         %6ld by SecureRealloc())\n",
      clb_free_count+secure_realloc_f_count,
      clb_free_count , secure_realloc_f_count);
   fprintf(out,
      COMCHAR" SecureRealloc(ptr): %6ld (%6ld Allocs, %6ld Frees, %6ld Reallocs)\n",
      secure_realloc_count, secure_realloc_m_count,
      secure_realloc_f_count,
      secure_realloc_count-(secure_realloc_m_count+secure_realloc_f_count));
   fprintf(out,
      COMCHAR" -------------------------------------------------\n\n");
}

#endif

/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/
