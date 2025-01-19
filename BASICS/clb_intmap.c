/*-----------------------------------------------------------------------

File  : clb_intmap.c

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Functions implementing the multi-representation N_0->void* mapping
  data type.

  Copyright 2004 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Mon Dec 27 17:34:48 CET 2004
    New

-----------------------------------------------------------------------*/

#include <clb_intmap.h>



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
// Function: switch_to_array()
//
//   Return true if representation should switch to array (because of
//   high density)
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static bool switch_to_array(long old_min, long old_max, long new_key, long entries)
{
   long max_key = MAX(old_max, new_key);
   long min_key = MIN(old_min, new_key);

   if((entries * MIN_TREE_DENSITY) > (max_key-min_key))
   {
      return true;
   }
   return false;
}


/*-----------------------------------------------------------------------
//
// Function: switch_to_tree()
//
//   Return true if representation should switch to tree (because of
//   low density)
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static bool switch_to_arraytree(long old_min, long old_max, long new_key, long entries)
{
    long max_key = MAX(old_max, new_key);
    long min_key = MIN(old_min, new_key);

    if ((entries * MAX_TREE_DENSITY) < (max_key - min_key))
    {
        return true;
    }
    return false;
}


/*-----------------------------------------------------------------------
//
// Function: add_new_tree_node()
//
//   Add a *new* key node to a IntMap in tree form and return its
//   address. Assertion fail, if key is not new. Increases element
//   count!
//
// Global Variables: -
//
// Side Effects    : Changes tree
//
/----------------------------------------------------------------------*/

ArrayTree_p add_new_arraytree_node(IntMap_p map, long key, void *val)
{
    ArrayTree_p handle;
    assert(map->type == IMArrayTree);

    handle = malloc(sizeof(ArrayTreeNode));
    handle->key = key;
    handle->array = PDRangeArrAlloc(key, MAX_ARRAYTREE_NODE_SIZE);
    PDRangeArrAssignP(handle->array, key, val);
    handle->left = NULL;
    handle->right = NULL;

    ArrayTreeInsert(&(map->values.arrayTree), key, val);

    map->entry_no++;

    return handle;
}


/*-----------------------------------------------------------------------
//
// Function: array_to_tree()
//
//   Convert a IntMap in array form to an equivalent one in tree
//   form.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void array_to_arraytree(IntMap_p map)
{
    PDRangeArr_p tmp_arr;
    IntOrP tmp_val;
    long i;
    ArrayTree_p new_tree = NULL;

    assert(map->type == IMArray);

    tmp_arr = map->values.array;
    map->values.arrayTree = NULL;
    map->type = IMArrayTree;
    map->entry_no = 0;

    for (i = PDRangeArrLowKey(tmp_arr); i <= map->max_key; i++)
    {
        tmp_val.p_val = PDRangeArrElementP(tmp_arr, i);
        if (tmp_val.p_val)
        {
            ArrayTreeInsert(&new_tree, i, tmp_val.p_val);
            map->entry_no++;
        }
    }
    map->values.arrayTree = new_tree;
    PDRangeArrFree(tmp_arr);
}


/*-----------------------------------------------------------------------
//
// Function: tree_to_array()
//
//   Convert a IntMap in tree form to an equivalent one in array
//   form.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void arraytree_to_array(IntMap_p map)
{
    PDRangeArr_p tmp_arr;
    long i;
    PStack_p tree_iterator;
    ArrayTree_p handle;

    assert(map->type == IMArrayTree);

    map->entry_no = 0;
    tmp_arr = PDRangeArrAlloc(map->min_key, IM_ARRAY_SIZE);
    tree_iterator = ArrayTreeTraverseInit(map->values.arrayTree);

    while ((handle = ArrayTreeTraverseNext(tree_iterator)))
    {
        for (i = PDRangeArrLowKey(handle->array); i < PDRangeArrLimitKey(handle->array); i++)
        {
            void *val = PDRangeArrElementP(handle->array, i);
            if (val)
            {
                PDRangeArrAssignP(tmp_arr, i, val);
                map->entry_no++;
            }
        }
    }

    ArrayTreeTraverseExit(tree_iterator);
    ArrayTreeFree(map->values.arrayTree);
    map->values.array = tmp_arr;
    map->type = IMArray;
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: IntMapAlloc()
//
//   Allocate an empty int mapper.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

IntMap_p IntMapAlloc(void)
{
   IntMap_p handle = IntMapCellAlloc();

   handle->type = IMEmpty;

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: IntMapFree()
//
//   Free an int mapper (does _not_ free pointed-to elements).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void IntMapFree(IntMap_p map)
{
    assert(map);

    switch (map->type)
    {
    case IMEmpty:
    case IMSingle:
        break;

    case IMArray:
        PDRangeArrFree(map->values.array);
        break;

    case IMArrayTree:
        ArrayTreeFree(map->values.arrayTree);
        break;

    default:
        assert(false && "Unknown IntMap type.");
    }

    IntMapCellFree(map);
}


/*-----------------------------------------------------------------------
//
// Function: IntMapGetVal()
//
//   Given a key, return the associated value or NULL, if no suitable
//   key/value pair exists.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void* IntMapGetVal(IntMap_p map, long key)
{
    void *res = NULL;

    if (!map)
    {
        return NULL;
    }
    switch (map->type)
    {
    case IMEmpty:
        break;

    case IMSingle:
        if (map->max_key == key)
        {
            res = map->values.value;
        }
        break;

    case IMArray:
        if (key <= map->max_key)
        {
            res = PDRangeArrElementP(map->values.array, key);
        }
        break;

    case IMArrayTree:
        if (key <= map->max_key)
        {
            ArrayTree_p node = ArrayTreeFind(&(map->values.arrayTree), key);
            if (node && PDRangeArrIndexIsCovered(node->array, key))
            {
                res = PDRangeArrElementP(node->array, key);
            }
        }
        break;

    default:
        assert(false && "Unknown IntMap type.");
    }
    return res;
}


/*-----------------------------------------------------------------------
//
// Function: IntMapGetRef()
//
//   Get a reference to the address of the value of a key/value
//   pair. Note that this always creates the key value pair (with
//   empty value) if it does not exist yet.
//
// Global Variables: -
//
// Side Effects    : May reorganize the map.
//
/----------------------------------------------------------------------*/


void** IntMapGetRef(IntMap_p map, long key)
{
    void **res = NULL;
    ArrayTree_p node;

    assert(map);

    switch (map->type)
    {
    case IMEmpty:
        map->type = IMSingle;
        map->max_key = key;
        map->min_key = key;
        map->values.value = NULL;
        res = &(map->values.value);
        map->entry_no = 1;
        break;

    case IMSingle:
        if (key == map->max_key)
        {
            res = &(map->values.value);
        }
        else if (switch_to_arraytree(map->min_key, map->max_key, key, 2))
        {
            map->type = IMArrayTree;
            map->values.arrayTree = NULL;
            add_new_arraytree_node(map, map->max_key, map->values.value);
            ArrayTreeInsert(&(map->values.arrayTree), key, NULL);
            res = IntMapGetRef(map, key);
        }
        break;

    case IMArray:
        if (((key > map->max_key) || (key < map->min_key)) &&
            switch_to_arraytree(map->min_key, map->max_key, key, map->entry_no + 1))
        {
            array_to_arraytree(map);
            res = IntMapGetRef(map, key);
        }
        else
        {
            res = &(PDRangeArrElementP(map->values.array, key));
            if (!(*res))
            {
                map->entry_no++;
            }
        }
        break;

    case IMArrayTree:
        node = ArrayTreeFind(&(map->values.arrayTree), key);
        if (node && PDRangeArrIndexIsCovered(node->array, key))
        {
            res = &(PDRangeArrElementP(node->array, key));
        }
        else
        {
            add_new_arraytree_node(map, key, NULL);
            res = IntMapGetRef(map, key);
        }
        break;

    default:
        assert(false && "Unknown IntMap type.");
    }
    return res;
}


/*-----------------------------------------------------------------------
//
// Function: IntMapAssign()
//
//   Add key/value pair to map, overriding any previous association.
//
// Global Variables: -
//
// Side Effects    : Changes map, may trigger reorganization
//
/----------------------------------------------------------------------*/

void IntMapAssign(IntMap_p map, long key, void* value)
{
   void** ref;

   assert(map);

   ref  = IntMapGetRef(map, key);
   *ref = value;
}


/*-----------------------------------------------------------------------
//
// Function: IntMapDelKey()
//
//   Delete a key/value association. If there was one, return the
//   value, otherwise return NULL.
//
//   **Currently, arrays never shrink. This might be worth
//   **changing (unlikely, though).
//
// Global Variables: -
//
// Side Effects    : May reorganize map
//
/----------------------------------------------------------------------*/

void* IntMapDelKey(IntMap_p map, long key)
{
    void* res = NULL;
    ArrayTree_p node;

    assert(map);

    switch (map->type)
    {
    case IMEmpty:
        res = NULL;
        break;

    case IMSingle:
        if (key == map->max_key)
        {
            res = map->values.value;
            map->type = IMEmpty;
            map->entry_no = 0;
        }
        break;

    case IMArray:
        if (key > map->max_key)
        {
            res = NULL;
        }
        else if ((res = PDRangeArrElementP(map->values.array, key)))
        {
            PDRangeArrAssignP(map->values.array, key, NULL);
            map->entry_no--;
            if (switch_to_arraytree(map->min_key, map->max_key, map->max_key, map->entry_no))
            {
                array_to_arraytree(map);
            }
        }
        break;

    case IMArrayTree:
        node = ArrayTreeFind(&(map->values.arrayTree), key);
        if (node && PDRangeArrIndexIsCovered(node->array, key))
        {
            res = PDRangeArrElementP(node->array, key);
            PDRangeArrAssignP(node->array, key, NULL);
            map->entry_no--;

            /* Prüfen, ob der Baum in ein Array zurückkonvertiert werden soll */
            if (switch_to_array(map->min_key, map->max_key, map->max_key, map->entry_no))
            {
                arraytree_to_array(map);
            }
        }
        break;

    default:
        assert(false && "Unknown IntMap type.");
        break;
    }

    return res;
}


/*-----------------------------------------------------------------------
//
// Function: IntMapIterAlloc()
//
//   Allocate an iterator object iterating over key range lower_key to
//   upper_key (both inclusive) in map. This is only valid as long as
//   no new key is introduced or old key is deleted.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

IntMapIter_p IntMapIterAlloc(IntMap_p map, long lower_key, long upper_key)
{
    IntMapIter_p handle = IntMapIterCellAlloc();

    handle->map = map;
    if (map)
    {
        handle->lower_key = MAX(lower_key, map->min_key);
        handle->upper_key = MIN(upper_key, map->max_key);

        switch (map->type)
        {
        case IMEmpty:
            break;

        case IMSingle:
            handle->admin_data.seen = true;
            if ((map->max_key >= lower_key) && (map->max_key <= upper_key))
            {
                handle->admin_data.seen = false;
            }
            break;

        case IMArray:
            handle->admin_data.current = lower_key;
            break;

        case IMArrayTree:
            handle->admin_data.tree_iter =
                ArrayTreeLimitedTraverseInit(map->values.arrayTree, lower_key);
            break;

        default:
            assert(false && "Unknown IntMap type.");
            break;
        }
    }
    return handle;
}


/*-----------------------------------------------------------------------
//
// Function: IntMapIterFree()
//
//   Free an IntMapIterator.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void IntMapIterFree(IntMapIter_p junk)
{
    assert(junk);

    if (junk->map)
    {
        switch (junk->map->type)
        {
        case IMEmpty:
        case IMSingle:
        case IMArray:
            break;

        case IMArrayTree:
            PStackFree(junk->admin_data.tree_iter);
            break;

        default:
            assert(false && "Unknown IntMap type.");
            break;
        }
    }
    IntMapIterCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: IntMapDebugPrint()
//
//   Print an intmap datatype as a list of key:value pairs.
//
// Global Variables: -
//
// Side Effects    : Output, memory operations
//
/----------------------------------------------------------------------*/

void IntMapDebugPrint(FILE* out, IntMap_p map)
{
   IntMapIter_p iter = IntMapIterAlloc(map, LONG_MIN, LONG_MAX);
   void* val;
   long  key = 0;

   fprintf(out, "# ==== IntMapType %d Size = %ld\n", map->type, IntMapStorage(map));
   for(val=IntMapIterNext(iter, &key); val; val=IntMapIterNext(iter, &key))
   {
      fprintf(out, "# %5ld : %p\n", key, val);
   }
   fprintf(out, "# ==== IntMap End\n");

   IntMapIterFree(iter);
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
