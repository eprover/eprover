/*-----------------------------------------------------------------------

File  : skipList.h

Author: Denis Feuerstein

Contents

  Definitions and functions for an int-based linked list.

  Copyright 2023 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes
...

-----------------------------------------------------------------------*/


/*
 * TODO:
 * Zufallszahl bei den Funktionen noch nicht so gut
 * Anzahl der Elemente der Liste
 * Einmaligkeit und Reihenfolge der Werte (als Menge/Set)
 */


#ifndef CLB_SKIPLISTS

#define CLB_SKIPLISTS



#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <limits.h>
#include <stdbool.h>
#include <clb_avlgeneric.h>

#define MAXLEVEL 4
//why not higher than 5??

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/*#define      InitOutput() GlobalOut=stdout;GlobalOutFD=STDOUT_FILENO
void         OpenGlobalOut(char* outname);
FILE*        OutOpen(char* name);
void         OutClose(FILE* file);*/


//skiplist structure
typedef struct node_t {
    long                key;
    IntOrP 				value;
    struct node_t       **next;
} node_t, *node_p;


typedef struct skiplist {
    int level;
    struct node_t *header;
} SkipList_t, *SkipList_p;



/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


#ifdef CONSTANT_MEM_ESTIMATE
#define SKIPLISTNODE_MEM 24
#else
#define SKIPLISTNODE_MEM MEMSIZE(node_t)
#endif

#define SkipListNodeAlloc() (node_t*)SizeMalloc(sizeof(node_t))
#define SkipListNodeFree(junk)        SizeFree(junk, sizeof(node_t))

//////////////////////
//Initialize functions
SkipList_p initSkiplist();
static int randomLevel();
bool SkipListStore(SkipList_p root, long key, IntOrP value);
node_p SkipListInsert(SkipList_p list, node_p newnode);
node_p SkipListNodeAllocEmpty();


////////////////
//Find function
node_p SkipListFind(SkipList_p list, long key);


////////////////
//Delete functions
static void freeNode(node_p node);
int SkipListDeleteNode(SkipList_p list, long key);
void SkipListFree(SkipList_p list);
node_p SkipListExtractEntry(SkipList_p root, long key);
node_p SkipListExtractRoot(SkipList_p root);
bool SkipListDeleteEntry(SkipList_p root, long key);


//////////////////////
// Analyze functions
long SkipListDebugPrint(FILE* out, SkipList_p list, bool keys_only);
static int print_all_levels(SkipList_p list);
static int print_only_keys(SkipList_p list);
long SkipListNodes(SkipList_p list);
int SkipListGetLevel();
node_p SkipListMinNode(SkipList_p list);
node_p SkipListMaxNode(SkipList_p list);
#define   SkipListMinKey(list) (SkipListMinNode(list)->key)
#define   SkipListMaxKey(list) (SkipListMaxNode(list)->key)

//////////////////////
//Test Functions
/*
void testSkipListInsert(SkipList_p list, long key);
void testSkipListFree(SkipList_p list);
void testSkipListDeleteNode(SkipList_p list);
void testSkipListFind(SkipList_p list);
*/

#endif



/*
Weil nicht alle Seiteneffekte bekannt sind
besser das Vorgehen diese Funktionen nachzubilden
und dann einzufügen.
Da nicht alle Seiteneffekte des bisherigen Datentyps bekannt sind,
um unvorhergesehenen Problemen zuvor zu kommen.
*/

/*
NumTreeTraverseInit -> Gehe Datenstruktur bis zum gegebenen Limit durch und
sollte Schlüssel kleiner als limit sein, dann gehe rechten Teilbaum durch,
Andernfalls, ist der Schlüssel entweder gleich dem aktuellen Schlüssel oder größer
Nur für letztere beide Fälle wird ein Elemenet im Stack angelegt
*/


/*
// KOMPILIERUNG
gcc skipList.c -o skipList
./skipList

// INTERNE FUNKTIONEN
--Initializierung der Liste
--Einfügen
(--Löschen der Liste)
--Löschen eines Nodes
--Suchen
(--Iteration (asc)?)
--Zählen der Knoten
--Darstellung der Liste

// TESTFUNKTIONEN

*/

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/

