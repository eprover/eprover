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

#include "cte_simpletypes.h"

/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/



/*-----------------------------------------------------------------------
//
// Function: TypeAlloc
// Allocate a new type cell, with the given base sort and arity.
//
//
// Global Variables:
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/
Type_p TypeAlloc(SortType domain, int arity)
{
    Type_p res;

    res = TypeCellAlloc();
    res->domain_sort = domain;
    res->lson = NULL;
    res->rson = NULL;
    res->args = NULL;

    res->arity = arity;
    if (arity)
    {
        res->args = TypeArgumentAlloc(arity);
    }

    return res;
}


/*-----------------------------------------------------------------------
//
// Function: TypeFree
// free memory of the given type cell
//
// Global Variables:
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/
void TypeFree(Type_p junk)
{
    junk->lson = NULL;
    junk->rson = NULL;

    if (junk->arity)
    {
        TypeArgumentFree(junk->args, junk->arity);
    }

    TypeCellFree(junk);
}

/*-----------------------------------------------------------------------
//
// Function: splay_type
// performs the splay operation at the root of tree.
//
//
// Global Variables:
//
// Side Effects    : modifies tree
//
/----------------------------------------------------------------------*/
static Type_p splay_type(Type_p tree, Type_p splay)
{
   Type_p   left, right, tmp;
   TypeCell new;
   int       cmpres;

   if (!tree)
   {
      return tree;
   }

   new.lson = NULL;
   new.rson = NULL;
   left = &new;
   right = &new;

   for (;;)
   {
      cmpres = TypeCompare(splay, tree);
      if (cmpres < 0)
      {
         if(!tree->lson)
         {
            break;
         }
         if(TypeCompare(splay, tree->lson) < 0)
         {
            tmp = tree->lson;
            tree->lson = tmp->rson;
            tmp->rson = tree;
            tree = tmp;
            if (!tree->lson)
            {
               break;
            }
         }
         right->lson = tree;
         right = tree;
         tree = tree->lson;
      }
      else if(cmpres > 0)
      {
         if (!tree->rson)
         {
            break;
         }
         if(TypeCompare(splay, tree->rson) > 0)
         {
            tmp = tree->rson;
            tree->rson = tmp->lson;
            tmp->lson = tree;
            tree = tmp;
            if (!tree->rson)
            {
               break;
            }
         }
         left->rson = tree;
         left = tree;
         tree = tree->rson;
      }
      else
      {
         break;
      }
   }
   left->rson = tree->lson;
   right->lson = tree->rson;
   tree->lson = new.rson;
   tree->rson = new.lson;

   return tree;
}

/*-----------------------------------------------------------------------
//
// Function: TypeTreeFind()
//
//   Find a entry in the type tree
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Type_p TypeTreeFind(Type_p *root, Type_p key)
{
   if(*root)
   {
      *root = splay_type(*root, key);
      if(TypeEqual(*root, key))
      {
         return *root;
      }
   }
   return NULL;
}


/*-----------------------------------------------------------------------
//
// Function: TypeTreeInsert()
//
//   Insert a type in the tree. If the type already exists,
//   return pointer to existing entry as usual, otherwise return NULL.
//
// Global Variables: -
//
// Side Effects    : Changes tree
//
/----------------------------------------------------------------------*/

Type_p TypeTreeInsert(Type_p *root, Type_p new)
{
   int cmpres;

   if (!*root)
   {
      new->lson = new->rson = NULL;
      *root = new;
      return NULL;
   }
   *root = splay_type(*root, new);

   cmpres = TypeCompare(new, *root);

   if (cmpres < 0)
   {
      new->lson = (*root)->lson;
      new->rson = *root;
      (*root)->lson = NULL;
      *root = new;
      return NULL;
   }
   else if(cmpres > 0)
   {
      new->rson = (*root)->rson;
      new->lson = *root;
      (*root)->rson = NULL;
      *root = new;
      return NULL;
   }
   return *root;
}


/*-----------------------------------------------------------------------
//
// Function: TypeTreeExtract()
//
//   Remove a type cell from the tree and return a pointer to it.
//
// Global Variables: -
//
// Side Effects    : Changes tree
//
/----------------------------------------------------------------------*/

Type_p TypeTreeExtract(Type_p *root, Type_p key)
{
   Type_p x, cell;

   if (!(*root))
   {
      return NULL;
   }
   *root = splay_type(*root, key);
   if(TypeEqual(key, (*root)))
   {
      if (!(*root)->lson)
      {
         x = (*root)->rson;
      }
      else
      {
         x = splay_type((*root)->lson, key);
         x->rson = (*root)->rson;
      }
      cell = *root;
      cell->lson = cell->rson = NULL;
      *root = x;
      return cell;
   }
   return NULL;
}


/*-----------------------------------------------------------------------
//
// Function: parse_sort_list
//   Parses a list of sorts, separated by "*" into the given array.
//   Will also consume the opening and trailing parenthesis.
//
//   It returns the number of parsed sorts.
//
// Global Variables: -
//
// Side Effects    : Modifies scanner, args and len
//
/----------------------------------------------------------------------*/
int parse_sort_list(Scanner_p in, SortTable_p sort_table, SortType **args, int *len)
{
   SortType sort;
   int arity;

   *len = 5;
   *args = SecureMalloc((*len) * sizeof(SortType));

   AcceptInpTok(in, OpenBracket);

   sort = SortParseTSTP(in, sort_table);
   (*args)[0] = sort;
   arity = 1;

   while(TestInpTok(in, Mult))
   {
      AcceptInpTok(in, Mult);
      sort = SortParseTSTP(in, sort_table);

      /* may have to resize args */
      if(arity == *len)
      {
         *len += 5;
         *args = SecureRealloc(*args, (*len) * sizeof(SortType));
      }
      (*args)[arity]= sort;
      arity++;
   }
   AcceptInpTok(in, CloseBracket);

   return arity;
}


/*---------------------------------------------------------------------*/
/*                    Exported Functions                               */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: TypeTableAlloc
// allocates and initialize a table that stores and shares types. It depends on,
// but does not own, a sort table.
//
//
// Global Variables:
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/
TypeTable_p TypeTableAlloc(SortTable_p sort_table)
{
    TypeTable_p res;

    res = TypeTableCellAlloc();
    res->sort_table = sort_table;
    res->size = 0;
    res->root = NULL;

    return res;
}


/*-----------------------------------------------------------------------
//
// Function: TypeTableFree
// free the content of the type table (all types) and the table itself
//
//
//
// Global Variables:
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/
void TypeTableFree(TypeTable_p junk)
{
    Type_p type;

    // free all types, removing them one by one
    while (junk->root)
    {
        type = TypeTreeExtract(&(junk->root), junk->root);
        TypeFree(type);
    }

    TypeTableCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: TypeNewConstant
//  Build a new constant type
//
//
// Global Variables: -
//
// Side Effects    :  Modifies the type table
//
/----------------------------------------------------------------------*/
Type_p TypeNewConstant(TypeTable_p table, SortType sort)
{
   Type_p type, res;

   type = TypeAlloc(sort, 0);
   res = TypeTreeInsert(&(table->root), type);
   if(!res)
   {
      return type;
   }
   TypeFree(type);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: TypeNewFunction()
//
//  Build a new function type.
//
// Global Variables: -
//
// Side Effects    :  Modifies the type table
//
/----------------------------------------------------------------------*/

Type_p TypeNewFunction(TypeTable_p table, SortType sort,
                       int arity, SortType *args)
{
   Type_p type, res;

   type = TypeAlloc(sort, arity);
   for(int i=0; i < arity; i++)
   {
      type->args[i] = args[i];
   }

   res = TypeTreeInsert(&(table->root), type);
   if(!res)
   {
      return type;
   }
   TypeFree(type);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: TypeCompare()
//
//   Implement total order on types.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int TypeCompare(Type_p t1, Type_p t2)
{
   int res;
   
   res = t1->domain_sort - t2->domain_sort;
   if(res)
   {
      return res;
   }
   
   res = t1->arity - t2->arity;
   if(res)
   {
      return res;
   }
   
   // same domain and arity, lexicographic comparison of arguments
   assert (t1->arity == t2->arity);
   for(int i = 0; !res && i < t1->arity; i++)
   {
      res = t1->args[i] - t2->args[i];
      if(res)
      {
         return res;
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: TypeCopyWithReturn()
//
//   Return a copy of this term, but with the given domain_sort.
//
// Global Variables: -
//
// Side Effects    : insert a type in the table
//
/----------------------------------------------------------------------*/

Type_p TypeCopyWithReturn(TypeTable_p table, Type_p source,
                          SortType new_domain)
{
   return TypeNewFunction(table, new_domain, source->arity, source->args);
}

/*-----------------------------------------------------------------------
//
// Function: TypePrintTSTP()
//
//   Prints the type in TSTP TFF syntax.
//
// Global Variables:
//
// Side Effects    : IO
//
/----------------------------------------------------------------------*/

void TypePrintTSTP(FILE *out, TypeTable_p table, Type_p type)
{
    int i;

    if (TypeIsConstant(type))
    {
        SortPrintTSTP(out, table->sort_table, type->domain_sort);
    }
    else
    {
        if (type->arity == 1)
        {
            SortPrintTSTP(out, table->sort_table, type->args[0]);
        }
        else
        {
            fputc('(', out);
            SortPrintTSTP(out, table->sort_table, type->args[0]);
            for (i = 1; i < type->arity; i++)
            {
                fputs("*", out);
                SortPrintTSTP(out, table->sort_table, type->args[i]);
            }
            fputc(')', out);
        }
        fputc('>', out);
        SortPrintTSTP(out, table->sort_table, type->domain_sort);
    }
}


/*-----------------------------------------------------------------------
//
// Function: TypeParseTSTP()
//
//   Parses a type in TSTP TFF format.
//
// Global Variables: -
//
// Side Effects    : reads from the scanner
//
/----------------------------------------------------------------------*/

Type_p TypeParseTSTP(Scanner_p in, TypeTable_p table)
{
   SortType left, right, *args = NULL;
   int arity, len;
   Type_p res;

   left = right = STNoSort;

   if (TestInpTok(in, OpenBracket))
   {
      /* Function type */
      arity = parse_sort_list(in, table->sort_table, &args, &len);
      AcceptInpTok(in, GreaterSign);
      right = SortParseTSTP(in, table->sort_table);
      assert(right!=STNoSort);

      res = TypeNewFunction(table, right, arity, args);
      FREE(args);
   }
   else
   {
      left = SortParseTSTP(in, table->sort_table);
      assert(left!=STNoSort);
      if (TestInpTok(in, GreaterSign))
      {
         /* Unary function type */
         AcceptInpTok(in, GreaterSign);
         right = SortParseTSTP(in, table->sort_table);
         args = SizeMalloc(sizeof(SortType));
         args[0] = left;
         res = TypeNewFunction(table, right, 1, args);
         SizeFree(args, sizeof(SortType));
      }
      else
      {
         /* constant type */
         res = TypeNewConstant(table, left);
      }
   }
   return res;
}


AVL_TRAVERSE_DEFINITION(Type, Type_p)
