/*-----------------------------------------------------------------------

File  : cle_annotations.h

Author: Stephan Schulz

Contents

  Functions and datatype for dealing with and administrating
  annotations.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Fri Jul 16 20:45:49 MET DST 1999
    New

-----------------------------------------------------------------------*/

#ifndef CLE_ANNOTATIONS

#define CLE_ANNOTATIONS


#include <clb_pdarrays.h>
#include <clb_numtrees.h>
#include <cio_basicparser.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/*

  An annotation is a NumTreeCell, where the key corresponds to the
  proof problem ("Sample number") and val1.p_val points to a DDArray
  which contains the meat. val2.i_val contains the number of
  annotations parsed or expected. val1.p_val[0] always is the number
  of original annotations merged into this annoation.

  Complete format of annotations for the prover:

  0: Number of example clauses combined in this annotation
  1: Proofs the fact occured in
  2: Average proof distance of the clause
  3: Number of used clauses simplified
  4: Number of unused clauses simplified
  5: Number of used clauses generated from this one
  6: Number of unused clauses generated
  7: Number of clauses subsumed by this one
 */

#define ANNOTATION_DEFAULT_SIZE 7
#define ANNOTATIONS_MERGE_ALL NULL

typedef NumTree_p Annotation_p;

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

Annotation_p AnnotationAlloc(void);
void         AnnotationFree(Annotation_p junk);
#define      AnnotationValues(anno) ((anno)->val1.p_val)
#define      AnnotationCount(anno)\
             DDArrayElement(((anno)->val1.p_val), 0)
#define      AnnotationLength(anno) ((anno)->val2.i_val)
void         AnnotationTreeFree(Annotation_p tree);
Annotation_p AnnotationParse(Scanner_p in, long expected);
long         AnnotationListParse(Scanner_p in, Annotation_p *tree,
             long expected);
void         AnnotationPrint(FILE* out, Annotation_p anno);
void         AnnotationListPrint(FILE* out, Annotation_p tree);
void         AnnotationCombine(Annotation_p res, Annotation_p new_anno);
long         AnnotationMerge(Annotation_p *tree, Annotation_p collect,
              PStack_p sources);
double       AnnotationEval(Annotation_p anno, double weights[]);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





