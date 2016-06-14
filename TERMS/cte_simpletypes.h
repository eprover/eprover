/*-----------------------------------------------------------------------

File  : cte_simpletypes.c

Author: Simon Cruanes (simon.cruanes@inria.fr)

Contents

  Implementation of simple types for the TSTP TFF format

  Copyright 2013 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sat Jul  6 09:45:14 CEST 2013
    New

-----------------------------------------------------------------------*/

#ifndef CTE_SIMPLETYPES

#define CTE_SIMPLETYPES

#include "cio_scanner.h"
#include "cte_simplesorts.h"

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* Representation of a simple type. A simple type can be either a
 * constant type, or a function type whose arguments are constants
 * (no argument can be a function). */
typedef struct typecell
{
    SortType            domain_sort; /* domain sort, ie the sort returned by a function */
    int                 arity;       /* arity of the function type */
    SortType*           args;        /* type of the function's arguments */

    struct typecell*    lson;        /* for sharing types */
    struct typecell*    rson;
}TypeCell, *Type_p;

/* Table to store and share types */
typedef struct
{
    SortTable_p  sort_table;             /* sorts */
    unsigned int size;                   /* number of types */
    Type_p       root;                   /* root of the tree of types */
}TypeTableCell, *TypeTable_p;

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define TypeCellAlloc()    (TypeCell*)SizeMalloc(sizeof(TypeCell))
#define TypeCellFree(junk) SizeFree(junk, sizeof(TypeCell))
#define TypeTableCellAlloc()    (TypeTableCell*)SizeMalloc(sizeof(TypeTableCell))
#define TypeTableCellFree(junk) SizeFree(junk, sizeof(TypeTableCell))
#define TypeArgumentAlloc(i) (SortType*) (SizeMalloc(i * sizeof(SortType)))
#define TypeArgumentFree(junk, i)  SizeFree(junk, i*sizeof(SortType))

#define TypeIsConstant(ty) (ty->arity == 0)
#define TypeIsFunction(ty) (ty->arity > 0)
#define TypeEqual(type1, type2) (TypeCompare((type1), (type2)) == 0)

TypeTable_p     TypeTableAlloc(SortTable_p sort_table);
void            TypeTableFree(TypeTable_p junk);
int             TypeCompare(Type_p t1, Type_p t2);
Type_p          TypeNewConstant(TypeTable_p table, SortType sort);
Type_p          TypeNewFunction(TypeTable_p table, SortType sort,
                                int arity, SortType *args);
Type_p          TypeCopyWithReturn(TypeTable_p table, Type_p source,
                                   SortType new_domain);
void            TypePrintTSTP(FILE *out, TypeTable_p table, Type_p type);
Type_p          TypeParseTSTP(Scanner_p in, TypeTable_p table);

AVL_TRAVERSE_DECLARATION(Type, Type_p)
#define TypeTraverseExit(stack) PStackFree(stack)

#endif
