/*-----------------------------------------------------------------------

  File  : clb_plocalstacks.h

  Author: Martin Möhrmann

  Contents

  Stack implementation with macros that use local (automatic)
  variables. The responsibility to ensurce space is delegeted to the
  user for the simple push operation. There are macro-functions to
  push all arguments of a term.

  Copyright 2016 by the author.

  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Sat Jun 4 20:30:20 2016

  -----------------------------------------------------------------------*/

#ifndef CLB_PLOCALSTACKS

#define CLB_PLOCALSTACKS

#include <clb_memory.h>


#define PLOCALSTACK_DEFAULT_SIZE 64


#define PLocalStackInit(stack) PLocalStackInitWithSize(stack, PLOCALSTACK_DEFAULT_SIZE)

#define PLocalStackInitWithSize(stack, num) size_t stack##_##size = num; \
   void* *stack##_##data = SizeMalloc(stack##_##size * sizeof(void*));  \
   size_t stack##_##current = 0

#define PLocalStackFree(junk)  SizeFree(junk##_##data, junk##_##size * sizeof(void*));

#define PLocalStackEnsureSpace(stack, space) do{                        \
      if(UNLIKELY((stack##_##current+space) >= stack##_##size))         \
      {                                                                 \
         stack##_##size = PLocalStackGrow(&stack##_##data, stack##_##size, space); \
      }                                                                 \
   }while(0)

#define PLocalStackEmpty(stack)     (stack##_##current == 0)
#define PLocalStackTop(stack)       (stack##_##current)
#define PLocalStackPop(stack)       (stack##_##data[--stack##_##current])
#define PLocalStackPush(stack, val) (stack##_##data[stack##_##current++] = val)

#define PLocalStackPushTermArgsReversed(stack, term) do{                \
      PLocalStackEnsureSpace(stack,term->arity);                        \
      for(long i = term->arity-1; i>=0; i--)                            \
      {                                                                 \
         PLocalStackPush(stack, term->args[i]);                         \
      }                                                                 \
   }while(0)

#define PLocalStackPushTermArgs(stack, term) do{                        \
      PLocalStackEnsureSpace(stack,term->arity);                        \
      for(size_t i = 0; i < term->arity; i++)                           \
      {                                                                 \
         PLocalStackPush(stack, term->args[i]);                         \
      }                                                                 \
   }while(0)

#define PLOCALSTACK_TAG_BITS 2
#define PLOCALSTACK_TAG_MASK (uintptr_t)((1<<PLOCALSTACK_TAG_BITS)-1)
#define PLOCALSTACK_VAL_MASK (uintptr_t)(~PLOCALSTACK_TAG_MASK)

#define PLocalTaggedStackInit(stack)  PLocalStackInit(stack)
#define PLocalTaggedStackFree(junk)   PLocalStackFree(junk)
#define PLocalTaggedStackEmpty(stack) PLocalStackEmpty(stack)

#define PLocalTaggedStackPushTermArgsReversed(stack, term, tag) do{     \
      const long arity = term->arity;                                   \
      PLocalTaggedStackEnsureSpace(stack,arity);                        \
      for(long i = arity-1; i>=0; i--)                                  \
      {                                                                 \
         PLocalTaggedStackPush(stack, term->args[i], tag);              \
      }                                                                 \
   }while(0)

#define PLocalTaggedStackPushTermArgs(stack, term, tag) do{             \
      const long arity = term->arity;                                   \
      PLocalTaggedStackEnsureSpace(stack, arity);                       \
      for(size_t i = 0; i < arity; i++)                                 \
      {                                                                 \
         PLocalTaggedStackPush(stack, term->args[i], tag);              \
      }                                                                 \
   }while(0)

#ifdef TAGGED_POINTERS

#define PLocalTaggedStackEnsureSpace(stack, space) PLocalStackEnsureSpace(stack, space)

#define PLocalTaggedStackPop(stack, val, tag)      do{                  \
      val = stack##_##data[--stack##_##current];                        \
      tag = (uintptr_t)val & PLOCALSTACK_TAG_MASK;                      \
      val = (void*)((uintptr_t)val & PLOCALSTACK_VAL_MASK);             \
   }while(0)

#define PLocalTaggedStackPush(stack, val, tag)     do{                  \
      assert(((void*)((uintptr_t)val&PLOCALSTACK_VAL_MASK)) == val);    \
      assert(((uintptr_t)tag&PLOCALSTACK_TAG_MASK) == tag);             \
      stack##_##data[stack##_##current++] = (void*)(((uintptr_t)val) | tag); \
   }while(0)                                                            \

#else

#define PLocalTaggedStackEnsureSpace(stack, space) PLocalStackEnsureSpace(stack, (2*(space)))

#define PLocalTaggedStackPop(stack, val, tag)      do{                  \
      tag = (__typeof__(tag))stack##_##data[--stack##_##current];       \
      val = stack##_##data[--stack##_##current];                        \
   }while(0)

#define PLocalTaggedStackPush(stack, val, tag)     do{                  \
      assert(sizeof(tag) <= sizeof(void*));                             \
      assert(((__typeof__(tag))((void*)tag)) == tag);                   \
      stack##_##data[stack##_##current++] = val;                        \
      stack##_##data[stack##_##current++] = (void*)tag;                 \
   }while(0)

#endif

__attribute__ ((noinline)) size_t PLocalStackGrow(void** *data, size_t size, size_t space);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
