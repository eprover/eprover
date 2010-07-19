/*-----------------------------------------------------------------------

File  : cte_simplesorts.c

Author: Stephan Schulz (schulz@eprover.org)

Contents
 
  Implementation of the simple sort table infrastructure.

  Copyright 2007 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Mon Sep 17 21:43:28 EDT 2007
    New

-----------------------------------------------------------------------*/

#include "cte_simplesorts.h"



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
// Function: default_sort_table_init()
//
//   Add the default types in proper order.
//
// Global Variables: -
//
// Side Effects    : Indirectly via SortTableInsert()
//
/----------------------------------------------------------------------*/
void default_sort_table_init(SortTable_p table)
{
   SortType res;

   res = SortTableInsert(table, "$no_type");
   assert(res == STNoSort);
   res = SortTableInsert(table, "$oType");
   assert(res == STBool);
   res = SortTableInsert(table, "$iType");
   assert(res == STIndividuals);
   res = SortTableInsert(table, "$int");
   assert(res == STInteger);
   res = SortTableInsert(table, "$real");   
   assert(res == STReal);
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: SortTableAlloc()
//
//   Allocate an empty but initialized sort table.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

SortTable_p SortTableAlloc(void)
{
   SortTable_p table = SortTableCellAlloc();

   table->sort_index   = NULL;
   table->back_index   = PStackAlloc();

   return table;
}

/*-----------------------------------------------------------------------
//
// Function: SortTableFree()
//
//   Free a SortTable.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

void SortTableFree(SortTable_p junk)
{
   assert(junk);

   PStackFree(junk->back_index);
   StrTreeFree(junk->sort_index);
   SortTableCellFree(junk);
}

/*-----------------------------------------------------------------------
//
// Function: SortTableInsert()
//
//   Add an entry (i.e. sort name) to the table (if unknown) and
//   retrieve its encoding. 
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

SortType SortTableInsert(SortTable_p table, char* sort_name)
{
   StrTree_p cell = StrTreeFind(&(table->sort_index), sort_name);
   
   if(!cell)
   {
      IntOrP sort_val;

      sort_val.i_val = PStackGetSP(table->back_index);
      cell = StrTreeStore(&(table->sort_index), sort_name, sort_val, sort_val);
      assert(cell);
      PStackPushP(table->back_index, cell->key);
   }
   return cell->val1.i_val;
}


/*-----------------------------------------------------------------------
//
// Function: DefaultSortTableAlloc()
//
//   Allocate a sort table and insert the system-defined sorts in the
//   proper order for their reserved names to work.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

SortTable_p DefaultSortTableAlloc(void)
{
   SortTable_p table = SortTableAlloc();
   default_sort_table_init(table);

   return table;
}


/*-----------------------------------------------------------------------
//
// Function: SortTableGetRep()
//
//   Given a sort, return a pointer to its external representation.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
char* SortTableGetRep(SortTable_p table, SortType sort)
{
   return PStackElementP(table->back_index, sort);
}


/*-----------------------------------------------------------------------
//
// Function: SortTablePrint()
//
//   Print a sort table (mainly for debugging)
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/
void SortTablePrint(FILE* out, SortTable_p table)
{
   PStackPointer i;
   StrTree_p     cell;
   PStack_p      stack;

   fprintf(out, "Sort table in order of sort creation:\n");
   fprintf(out, "=====================================\n");
   for(i=0; i<PStackGetSP(table->back_index); i++)
   {
      fprintf(out, "Type %4u: %s\n", i, SortTableGetRep(table, i));
   }
   fprintf(out, "\nSort table in alphabetic order:\n");
   fprintf(out, "=====================================\n");
   

   stack = StrTreeTraverseInit(table->sort_index);
   while((cell=StrTreeTraverseNext(stack)))
   {
      fprintf(out, "Type %4ld: %s\n", cell->val1.i_val, cell->key);
   }
   PStackFree(stack);
   fprintf(out, "\n");
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


