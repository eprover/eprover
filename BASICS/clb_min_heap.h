/*-----------------------------------------------------------------------

File  : clb_min_heap.h

Author: Petar Vukmirovic

Contents

  Simple minimum heap implementation.

Copyright 1998-2022 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> di  4 jan 2022 14:29:14 CET
-----------------------------------------------------------------------*/

#include <clb_pstacks.h>

#ifndef CLB_MIN_HEAPS
#define CLB_MIN_HEAPS

struct MinHeap;
typedef struct MinHeap* MinHeap_p;

typedef void (*SetIndexFun)(void*, int);
typedef int (*CmpFun)(IntOrP*, IntOrP*);

MinHeap_p MinHeapAllocWithIndex(CmpFun, SetIndexFun);
#define  MinHeapAlloc(f) (MinHeapAllocWithIndex(f, NULL))

long   MinHeapSize(MinHeap_p);

void MinHeapAddP(MinHeap_p, void*);
void MinHeapAddInt(MinHeap_p, long);

IntOrP MinHeapPopMin(MinHeap_p);
#define MinHeapPopMinP(m) (MinHeapPopMin(m).p_val)
#define MinHeapPopMinInt(m) (MinHeapPopMin(m).i_val)

void MinHeapUpdateElement(MinHeap_p h, long idx);
void MinHeapRemoveElement(MinHeap_p h, long idx);
void MinHeapDecrKey(MinHeap_p, long);
void MinHeapIncrKey(MinHeap_p, long);

void MinHeapFree(MinHeap_p);

void DBGPrintHeap(FILE* out, MinHeap_p h, bool as_ptr);

#endif
