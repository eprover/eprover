/*-----------------------------------------------------------------------

File  : clb_properties.h

Author: Stephan Schulz

Contents

  Macros for dealing with 1 bit properties of objects (well,
  structs). It requires the object to be dealt with to have a field
  named "properties" that is of some integer or enumeration type. This
  is pretty ugly, but I did not want to spend to much time on it.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Fri Sep 18 14:27:52 MET DST 1998
    New

-----------------------------------------------------------------------*/

#ifndef CLB_PROPERTIES

#define CLB_PROPERTIES



/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


#define SetProp(obj, prop) ((obj)->properties = (obj)->properties | (prop))
#define DelProp(obj, prop) ((obj)->properties = (obj)->properties & ~(prop))
#define FlipProp(obj, prop) ((obj)->properties = (obj)->properties ^ (prop))
#define AssignProp(obj, sel, prop) DelProp((obj),(sel));SetProp((obj),(sel)&(prop))

/* Are _all_ properties in prop set in obj? */
#define QueryProp(obj, prop) (((obj)->properties & (prop)) == (prop))

/* Are any properties in prop set in obj? */
#define IsAnyPropSet(obj, prop) ((obj)->properties & (prop))

/* Return the properties of object...yes, this is the same code as
   above, but implements a different concept */
#define GiveProps(obj,prop) ((obj)->properties & (prop))

/* Are two property sets equivalent? */

#define PropsAreEquiv(obj1, obj2, props)\
        (((obj1->properties)&(props))==((obj2->properties)&(props)))

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





