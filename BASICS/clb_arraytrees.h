/*-----------------------------------------------------------------------
  File    : clb_arraytrees.h
  Author  : Anpassung von Stephan Schulz' NumTree
  Purpose : Definition eines Array-basierten Splay-Baums (ArrayTree),
            bei dem jeder Knoten ein Array für mehrere Schlüssel-Wert-Paare enthält.
            Optimiert für schnellen Zugriff auf den zuletzt verwendeten Schlüssel
            und speichereffizient durch den Datentyp uint8_t.
-----------------------------------------------------------------------*/

#ifndef CLB_ARRAYTREES_H
#define CLB_ARRAYTREES_H

#include <clb_dstrings.h>
#include <clb_avlgeneric.h>

/* Maximal erlaubte Größe des Arrays pro Knoten */
#define MAX_NODE_ARRAY_SIZE 8

/* Struktur zur Darstellung eines Schlüssel-Wert-Paares */
typedef struct
{
    long key;    // Der Schlüssel
    IntOrP val1;
    IntOrP val2;
} ArrayEntry;

/* Array-basierter Knoten im Baum */
typedef struct arraytree_node
{
    ArrayEntry entries[MAX_NODE_ARRAY_SIZE]; // Array für Schlüssel-Wert-Paare
    uint8_t entry_count;                     // Anzahl der belegten Einträge (max. 255)
    uint8_t last_access_index;               // Index des zuletzt verwendeten Eintrags (max. 255)
    struct arraytree_node* lson;             // Zeiger auf den linken Kindknoten
    struct arraytree_node* rson;             // Zeiger auf den rechten Kindknoten
} ArrayTreeNode, *ArrayTree_p;


/* Allocation macros for ArrayTree nodes */
#define ArrayTreeNodeAlloc() (ArrayTree_p)SizeMalloc(sizeof(ArrayTreeNode))
#define ArrayTreeNodeFree(junk) SizeFree(junk, sizeof(ArrayTreeNode))

#ifdef CONSTANT_MEM_ESTIMATE
#define ARRAYTREECELL_MEM (8 + (MAX_NODE_ARRAY_SIZE * sizeof(ArrayEntry)))
#else
#define ARRAYTREECELL_MEM MEMSIZE(ArrayTreeNode)
#endif

/* Core functions for ArrayTree */
ArrayTree_p ArrayTreeNodeAllocEmpty(void);
void        ArrayTreeFree(ArrayTree_p root);
ArrayTree_p ArrayTreeInsert(ArrayTree_p *root, ArrayTree_p newnode);
bool        ArrayTreeStore(ArrayTree_p *root, long key, IntOrP val1, IntOrP val2);
ArrayTree_p ArrayTreeFind(ArrayTree_p *root, long key);
bool        ArrayTreeDeleteEntry(ArrayTree_p *root, long key);
long        ArrayTreeDebugPrint(FILE* out, ArrayTree_p tree, bool keys_only);
ArrayTree_p ArrayTreeExtractEntry(ArrayTree_p* root, long key);
ArrayTree_p ArrayTreeExtractRoot(ArrayTree_p* root);
long        ArrayTreeNodes(ArrayTree_p root);
ArrayTree_p ArrayTreeMaxNode(ArrayTree_p root);
#define     ArrayTreeMaxKey(tree) ((tree && ArrayTreeMaxNode(tree)) ? \
                                  ArrayTreeMaxNode(tree)->entries[tree->entry_count - 1].key : 0)

/* Traversal functions for ArrayTree */
PStack_p    ArrayTreeLimitedTraverseInit(ArrayTree_p root, long limit);

AVL_TRAVERSE_DECLARATION(ArrayTree, ArrayTree_p)
#define ArrayTreeTraverseExit(stack) PStackFree(stack)

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/