/*-----------------------------------------------------------------------

File  : skipList.c

Author: Denis Feuerstein

Contents

  Functions implementing and testing an int-based linked list.

  Copyright 2023 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes
...

-----------------------------------------------------------------------*/


#include "clb_skiplists.h"


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
//  Function: initSkiplist()
//
//  Initialize Skip List
//
//  Global Variables: -
//
//  Side Effects    : -
//
/----------------------------------------------------------------------*/

SkipList_p initSkiplist()
{
    //Set header element and the nextpointer, as well as the key to max-int
    SkipList_p root = (SkipList_p)malloc(sizeof(SkipList_p));
    node_p header = (node_p)malloc(sizeof(node_p));


    header->key = INT_MAX;
    header->next = (node_t **)malloc(sizeof(node_p) * (MAXLEVEL));
    root->header = header;

    int i;

    /*
    node_p sentinel = (node_p)malloc(sizeof(node_p));
    root->sentinel = sentinel;
    sentinel->key = INT_MIN;
    sentinel->next = (node_t **)malloc(sizeof(node_p) * (MAXLEVEL));
    */

    //set the next-pointer, with the adress to the head-element
    for (i = 0; i < MAXLEVEL; i++)
    {
        header->next[i] = root->header;
        /*header->next[i] = root->sentinel;
        sentinel->next[i] = NULL;*/
    }

    root->level = 1;

    return root;
}


/*-----------------------------------------------------------------------
//
//  Function: randomLevel()
//
//  Returns a random level ? Doesn't it get better?
//
//  Global Variables: -
//
//  Side Effects    : -
//
/----------------------------------------------------------------------*/

static int randomLevel()
{
    //randomlevel is not random -> change that
    int level = 1;

    while (rand() < RAND_MAX/2 && level < MAXLEVEL-1)
    {
        level++;
    }

    return level;
}


/*-----------------------------------------------------------------------
//
//  Function: SkipListInsert()
//
//  Insert a node into a fitting position of the skip list and
//  determine the level-height
//
//  Global Variables: -
//
//  Side Effects    : -
//
/----------------------------------------------------------------------*/

node_p SkipListInsert(SkipList_p root, node_p newnode) //, IntOrP value
{
    node_p update[MAXLEVEL] = {NULL};
    node_p currentNode = root->header;
    int i, level;


    for (i = root->level; i >= 1; i--)
    {

        //Search for the last key in level, that is smaller than the searchedKey
        while (currentNode->next[i]->key < newnode->key)
        {
            //save in current temp-node
            currentNode = currentNode->next[i];
        }

        //set update at position i with current temp
        update[i] = currentNode;

    }

    //Select the next item on Level 1, for which applies:
    //Searched key is smaller or equal to this key.
    currentNode = currentNode->next[1];

    //if equal, then change value
    if (newnode->key == currentNode->key)
    {
        currentNode->value = newnode->value;
        return newnode;
    }
    //if key is smaller than key in next item, insert new Node
    else
    {
        //set random Level
        level = randomLevel();

        //if level bigger than the current max-level in the list ...
        if (level > root->level)
        {
            //update the update-node at position of current max-level+1
            //and do this as long as there is a delta between the two
            for (i = root->level+1; i <= level; i++)
            {
                update[i] = root->header;
            }
            root->level = level;
        }

        //Initialize new node
        currentNode = (node_p)malloc(sizeof(node_t));
        currentNode->key = newnode->key;
        currentNode->value = newnode->value;


         //Only allocates enough memory for level -> instead allocate for the full level and insert null
         //Alternative:     head->next = (node_t **)malloc(sizeof(node_p) * (level + 1));
        currentNode->next = (node_t **)malloc(sizeof(node_p) * (MAXLEVEL));

        //iterate through all levels to insert the node
        for (i = 1; i <= MAXLEVEL-1; i++)
        {

             // if i <= level, which was determied by randomLevel-Function,
             // set with the next node from update on level i, otherwise, set to NULL
            if(i <= level) {
                currentNode->next[i] = update[i]->next[i];
                update[i]->next[i] = currentNode;
            } else {
                currentNode->next[i] = NULL;
            }
        }
    }

    return NULL; //return newnode;
}


/*-----------------------------------------------------------------------
//
//  Function: SkipListStore()
//
//  Insert a node associating key with val into the
//  skip list. Return false if an entry for this key exists, true
//  otherwise.
//
//  Global Variables: -
//
//  Side Effects    : -
//
/----------------------------------------------------------------------*/

bool SkipListStore(SkipList_p root, long key, IntOrP value) //, IntOrP val1, IntOrP val2
{
   node_p handle, newnode;

   handle = SkipListNodeAlloc();
   handle->key = key;
   handle->value = value;

   newnode = SkipListInsert(root, handle);

   if(newnode)
   {
        SkipListNodeFree(handle);
        return false;
   }
   return true;
}


/*-----------------------------------------------------------------------
//
// Function: SkipListNodeAllocEmpty()
//
//   Allocate a empty, initialized SkipListNode. Pointer to children
//   are NULL, int values are 0 (and pointer values in ANSI-World
//   undefined, in practice NULL on 32 bit machines)(This comment is
//   superfluous!). The balance field is (correctly) set to 0.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

node_p SkipListNodeAllocEmpty()
{
   node_p handle = SkipListNodeAlloc();

   //handle->value = 0;
   handle->next = NULL;

   return handle;
}


/*-----------------------------------------------------------------------
//
//  Function: SkipListFind()
//
//  Searches the node by key and returns the node
//
//  Global Variables: -
//
//  Side Effects    : -
//
/----------------------------------------------------------------------*/

node_p SkipListFind(SkipList_p root, long key)
{
    node_p currentNode = root->header;
    int i;

    //search through all the levels until you find the last position
    //for which applies searchedKey smaller than key of next node
    for (i = root->level; i >= 1; i--)
    {
        while (currentNode->next[i]->key < key)
        {
            currentNode = currentNode->next[i];
        }
    }

    //if the following node holds the key, return the node,
    //else return NULL
    if (currentNode->next[1]->key == key)
    {
        return currentNode->next[1];
    }
    else
    {
        return NULL;
    }

    return NULL;

}


/*-----------------------------------------------------------------------
//
//  Function: freeNode()
//
//  Frees the memory of node
//
//  Global Variables: -
//
//  Side Effects    : -
//
/----------------------------------------------------------------------*/

static void freeNode(node_p node)
{
    if (node)
    {
        free(node->next);
        free(node);
    }
}


/*-----------------------------------------------------------------------
//
//  Function: SkipListDeleteNode()
//
//  Deletes a node from the list
//
//  Global Variables: -
//
//  Side Effects    : -
//
/----------------------------------------------------------------------*/

int SkipListDeleteNode(SkipList_p root, long key)
{
    // init update and temp-nodes
    node_p update[MAXLEVEL];
    node_p currentNode = root->header;
    int i;

    //iterate through all levels until you find the last position
    //for which applies searchedKey smaller than key of the next node
    for (i = root->level; i >= 1; i--) // >1
    {
        while (currentNode->next[i]->key < key)
        {
            currentNode = currentNode->next[i];
        }
        update[i] = currentNode;
    }

    //set to next node
    currentNode = currentNode->next[1];

    //if next node equals key, delete it, and return 1, else return 0 for failure
    if (currentNode->key == key)
    {
        for (i = 1; i <= root->level; i++)
        {
            if (update[i]->next[i] != currentNode)
            {
                break;
            }
            update[i]->next[i] = currentNode->next[i];//->next[i]
        }
        SkipListNodeFree(currentNode);//freeNode(head);


        //what in the hell???
        while (root->level > 1 && root->header->next[root->level] == root->header)
        {
            root->level--;
        }
        return 0;
    }
    return 1;
}


/*-----------------------------------------------------------------------
//
//  Function: SkipListFree()
//
//  Deletes the entire list
//
//  Global Variables: -
//
//  Side Effects    : -
//
/----------------------------------------------------------------------*/

void SkipListFree(SkipList_p root) {
    node_p currentNode = root->header;

    while (currentNode && currentNode->next[1] != root->header) {
        node_p tempNode = currentNode;
        currentNode = currentNode->next[1];
        SkipListNodeFree(tempNode);//freeNode(tempNode);
    }
    //free(root);
}


/*-----------------------------------------------------------------------
//
// Function: SkipListExtractEntry()
//
//   Find the entry where skipList->key==key, remove extract it from the
//   skip list, and return the pointer to the removed element. Return
//   NULL if no matching element exists.
//
// Global Variables: -
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

node_p SkipListExtractEntry(SkipList_p root, long key)
{
    // init update and temp-nodes
    node_p update[MAXLEVEL];
    node_p currentNode = root->header;
    int i;

    if (!(root))
    {
      return NULL;
    }

    //iterate through all levels until you find the last position
    //for which applies searchedKey smaller than key of the next node
    for (i = root->level; i >= 1; i--) // >1
    {
        while (currentNode->next[i]->key < key)
        {
            currentNode = currentNode->next[i];
        }
        update[i] = currentNode;
    }

    //set to next node
    currentNode = currentNode->next[1];

    //if next node equals key, extract it, and return it
    if (currentNode->key == key)
    {
        for (i = 1; i <= root->level; i++)
        {
            if (update[i]->next[i] != currentNode)
            {
                break;
            }
            update[i]->next[i] = currentNode->next[i];//->next[i]
        }

        //Resetting the level if necessary
        while (root->level > 1 && root->header->next[root->level] == root->header)
        {
            root->level--;
        }
        return currentNode;
    }
    return NULL;
}


/*-----------------------------------------------------------------------
//
// Function: SkipListExtractRoot()
//
//   Extract the node at the root of the skip list and return it (or
//   NULL if the tree is empty).
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

node_p SkipListExtractRoot(SkipList_p root)
{
   if(root)
   {
      return SkipListExtractEntry(root, root->header->next[1]->key);
   }
   return NULL;
}


/*-----------------------------------------------------------------------
//
// Function: SkipListDeleteEntry()
//
//   Delete the entry with key key from the skip list.
//
// Global Variables: -
//
// Side Effects    : By NumTreeExtract(), memory operations
//
/----------------------------------------------------------------------*/

bool SkipListDeleteEntry(SkipList_p root, long key)
{
   node_p cell;

   cell = SkipListExtractEntry(root, key);
   if(cell)
   {
      SkipListNodeFree(cell);
      return true;
   }
   return false;
}


/*-----------------------------------------------------------------------
//
//  Function: SkipListMinNode()
//
//  Returns the node of the skip list with the smallest key
//  aka: the first node of the list
//
//  Global Variables: -
//
//  Side Effects    : -
//
/----------------------------------------------------------------------*/

node_p SkipListMinNode(SkipList_p root) {
    return root->header->next[1];
}

/*-----------------------------------------------------------------------
//
//  Function: SkipListMaxNode()
//
//  Returns the level-size of the skip list
//
//  Global Variables: -
//
//  Side Effects    : -
//
/----------------------------------------------------------------------*/

node_p SkipListMaxNode(SkipList_p root)
{
    node_p currentNode = root->header;
    int i;

    /*
     * iterate through all the levels until you find the last node
     * of each level.
     */
    for (i = root->level; i >= 1; i--)
    {
        while (currentNode->next[i] != root->header && currentNode->next[i] != NULL)
        {
            currentNode = currentNode->next[i];
        }
    }

    return currentNode;

}


/*-----------------------------------------------------------------------
//
//  Function: SkipListGetLevel()
//
//  Returns the level-size of the skip list
//
//  Global Variables: -
//
//  Side Effects    : -
//
/----------------------------------------------------------------------*/

int SkipListGetLevel() {
    return MAXLEVEL;
}


/*-----------------------------------------------------------------------
//
//  Function: SkipListNodes()
//
//  Counts the nodes inside of the list and returns the amount as long
//
//  Global Variables: -
//
//  Side Effects    : -
//
/----------------------------------------------------------------------*/

long SkipListNodes(SkipList_p root){
    long i = 0;
    //temp-variable for root-adress
    node_p currentNode = root->header;

    while (currentNode && currentNode->next[1] != root->header) { //? Eins fehlt
        ++i;
        currentNode = currentNode->next[1];
    }

    return i;

}


/*-----------------------------------------------------------------------
//
//  Function: print_all_levels()
//
//  Display the entire list
//
//  Global Variables: -
//
//  Side Effects    : -
//
/----------------------------------------------------------------------*/

//static int print_all_levels(FILE* out, skiplist *list)
static int print_all_levels(SkipList_p root)
{
    node_p currentNode = root->header;
    int i = 0;
    int size = 0;


    printf("List: %p\n", root);

    // Printe table header
    printf("Key\tCurrent\t");
    for (i = 1; i <= root->level; i++)
    {
        printf("\t\tLevel_%d\t", i);
    }
    printf("\n");

    // Print Nodes
    while (currentNode && currentNode->next[1] != root->header)
    {
        if(currentNode != root->header)
        {
            printf("%ld->", currentNode->key);
        }
        else
        {
            printf("X->");
        }

        printf("\t%p\t", currentNode);

        for (i = 1; i <= root->level; i++)
        {
            printf("\t%p\t", currentNode->next[i]);
        }

        size++;
        printf("\n");
        currentNode = currentNode->next[1];
    }

    //Print the last node
    printf("%ld->", currentNode->key);
    printf("\t%p\t", currentNode);
    for (i = 1; i <= root->level; i++)
    {
        printf("\t%p\t", currentNode->next[i]);
    }

    return size;

}

/*-----------------------------------------------------------------------
//
//  Function: print_only_keys()
//
//  Display all the keys of list
//
//  Global Variables: -
//
//  Side Effects    : -
//
/----------------------------------------------------------------------*/


//static int print_only_keys(FILE* out, skiplist *root)
static int print_only_keys(SkipList_p root)
{
    node_p currentNode = root->header;
    int i = 0;
    int size = 0;


    printf("List: %p\n", root);

    // Printe table header
    printf("Key\n");

    // Print Keys
    while (currentNode && currentNode->next[1] != root->header)
    {
        if(currentNode != root->header)
        {
            printf("%ld", currentNode->key);
        }
        else
        {
            printf("X->");
        }

        size++;
        printf("\n");
        currentNode = currentNode->next[1];
    }

    //Print the last node
    printf("%ld", currentNode->key);

    return size;

}


/*-----------------------------------------------------------------------
//
//  Function: skiplist_print()
//
//  Display the entire list, with size
//
//  Global Variables: -
//
//  Side Effects    : -
//
/----------------------------------------------------------------------*/

static long skiplist_print(FILE* out, SkipList_p root, bool keys_only)
{
   int i, size;

   if(!root)
   {
      fprintf(out, "\n");
      size = 0;
   }
   else
   {
      if(keys_only)
      {
         size = print_only_keys(root);
      }
      else
      {
         size = print_all_levels(root);
      }

   }

   return size;
}


/*-----------------------------------------------------------------------
//
//  Function: SkipListDebugPrint()
//
//  Display the entire list, with size
//
//  Global Variables: -
//
//  Side Effects    : -
//
/----------------------------------------------------------------------*/

long SkipListDebugPrint(FILE* out, SkipList_p root, bool keys_only)
{
   long size;
   int level;

   size = skiplist_print(out, root, keys_only);
   level = SkipListGetLevel();
   print_all_levels(root);

   fprintf(out, "Skip list size: %ld\n", size);
   fprintf(out, "Skip list levels: %d\n", level);

   return size;
}



/*---------------------------------------------------------------------*/
/*                         Test Functions                              */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
//  Function: testSkipListInsert()
//
//  Test the time to append a node to the end of the list
//
//  Global Variables: -
//
//  Side Effects    : -
//
/----------------------------------------------------------------------*/

/*
void testSkipListInsert(SkipList_p root, long key) {
    int i;
    int arr[20] = {0,1,2,3,4,5,6,7,8,9, 10,11, 12, 13, 14, 15,16, 17, 18, 19};
    //node_p latestNode = list;
    clock_t begin, end;
    begin = clock();

    for(i = 0; i < key; i++) {
        //SkipListStore(root, arr[i], NULL);
    }

    end = clock();

    printf("\nTime to append a node:\n");
    printf("Start \t | \t End \t | \t Dif \n");
    printf("%ld \t | \t %ld \t | \t %ld\n", begin, end, end-begin);

}
*/


/*-----------------------------------------------------------------------
//
//  Function: testSkipListFree()
//
//  Test the time to delete the entire list
//
//  Global Variables: -
//
//  Side Effects    : -
//
/----------------------------------------------------------------------*/

/*
void testSkipListFree(SkipList_p root) {
    clock_t begin, end;
    begin = clock();

    SkipListFree(root);

    end = clock();

    printf("\nTime to delete a list:\n");
    printf("Start \t | \t End \t | \t Dif \n");
    printf("%ld \t | \t %ld \t | \t %ld\n", begin, end, end-begin);

}
*/


/*-----------------------------------------------------------------------
//
//  Function: testSkipListDeleteNode()
//
//  Test the time to delete a node from the list
//
//  Global Variables: -
//
//  Side Effects    : -
//
/----------------------------------------------------------------------*/

/*
void testSkipListDeleteNode(SkipList_p root) {
    int r;
    clock_t begin, end;
    time_t t;

    srand((unsigned) time(&t));
    //r = (rand()+3)%SkipListNodes(list);
    r = 14;

    begin = clock();
    printf("\nDelete key: %d\n", r);
    SkipListDeleteNode(root, r);

    end = clock();

    printf("Time to delete a node:\n");
    printf("Start \t | \t End \t | \t Dif \n");
    printf("%ld \t | \t %ld \t | \t %ld\n", begin, end, end-begin);

}
*/


/*-----------------------------------------------------------------------
//
//  Function: testSkipListFind()
//
//  Test the time to search a node in the list
//
//  Global Variables: -
//
//  Side Effects    : -
//
/----------------------------------------------------------------------*/

/*
void testSkipListFind(SkipList_p root) {
    long r;
    clock_t begin, end;
    time_t t;

    srand((unsigned) time(&t));
    //r = rand()%SkipListNodes(list);
    r = 17;

    begin = clock();

    printf("\nSearched key: \t %ld \n", r);
    SkipListFind(root, r);

    end = clock();

    printf("Time to search a node:\n");
    printf("Start \t | \t End \t | \t Dif \n");
    printf("%ld \t | \t %ld \t | \t %ld\n", begin, end, end-begin);

}
*/



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
