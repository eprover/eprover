/*-----------------------------------------------------------------------

  File  : cle_tsm.h

  Author: Stephan Schulz

  Contents

  Finally, the term space map!

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Fri Aug  6 16:47:31 MET DST 1999

  -----------------------------------------------------------------------*/

#ifndef CLE_TSM

#define CLE_TSM

#include <cle_indexfunctions.h>
#include <cle_flatannoterms.h>
#include <clb_ddarrays.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef enum
{
   TSMTypeNoType,
   TSMTypeFlat,
   TSMTypeRecursive,
   TSMTypeRecurrent,
   TSMTypeRecurrentLocal
}TSMType;

typedef struct tsmadmincell
{
   TSMType        tsmtype;
   TB_p           index_bank;      /* Where all the index terms go */
   IndexType      index_type;
   int            index_depth;
   double         limit;
   bool           local_limit;
   double         eval_limit; /* Set a posteriory for classification */
   double         unmapped_eval;
   double         unmapped_weight;
   struct tsmcell *tsm;
   struct tsmcell *emptytsm;
   PStack_p       tsmstack;
   PStack_p       cachestack;
   PatternSubst_p subst;
}TSMAdminCell, *TSMAdmin_p;


typedef struct tsmcell
{
   TSMAdmin_p admin;
   TSMIndex_p index;
   long       maxindex;
   PDArray_p  tsas;
}TSMCell, *TSM_p;

typedef struct tsacell
{
   TSMAdmin_p admin;
   double eval_weight;
   double eval;
   int    arity;
   TSM_p  *arg_tsms;
}TSACell, *TSA_p;


#define TSM_MAX_TERMTOP 5

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

extern char* TSMTypeNames[];

#define       GetTSMType(name) StringIndex((name), TSMTypeNames)

#define       TSMEvalNormalize(eval, limit) (((eval) < (limit))?-1:1)

double        TSMRemainderEntropy(PDArray_p partition, long *parts,
                                  double limit, long max_index);
double        TSMFlatAnnoSetEntropy(FlatAnnoSet_p set, double limit);
long          TSMPartitionSet(PDArray_p partition, TSMIndex_p index,
                              FlatAnnoSet_p set, PDArray_p cache);

IndexType     TSMFindOptimalIndex(TSMAdmin_p admin, FlatAnnoSet_p set,
                                  long *depth, IndexType indextype,
                                  double limit);

long          TSMCreateSubtermSet(FlatAnnoSet_p set, FlatAnnoTerm_p
                                  list, int sel);
double        TSMFindPartLimit(FlatAnnoSet_p set, double part);

#define TSMAdminCellAlloc() (TSMAdminCell*)SizeMalloc(sizeof(TSMAdminCell))
#define TSMAdminCellFree(junk) SizeFree(junk, sizeof(TSMAdminCell))

TSMAdmin_p TSMAdminAlloc(Sig_p sig, TSMType type);
void       TSMAdminFree(TSMAdmin_p junk);

#define TSMCellAlloc() (TSMCell*)SizeMalloc(sizeof(TSMCell))
#define TSMCellFree(junk) SizeFree(junk, sizeof(TSMCell))

void TSMAdminBuildTSM(TSMAdmin_p admin, FlatAnnoSet_p set, IndexType
                      type, int depth, PatternSubst_p subst);
TSM_p   TSMCreate(TSMAdmin_p admin, FlatAnnoSet_p set);
void    TSMFree(TSM_p tsm);

#define TSACellAlloc() (TSACell*)SizeMalloc(sizeof(TSACell))
#define TSACellFree(junk) SizeFree(junk, sizeof(TSACell))

TSA_p   TSACreate(TSMAdmin_p admin, FlatAnnoTerm_p list);
void    TSAFree(TSA_p tsa);

double  TSMEvalTerm(TSMAdmin_p admin, Term_p term, PatternSubst_p
                    subst);
double  TSMComputeClassificationLimit(TSMAdmin_p admin,  FlatAnnoSet_p
                                      set);
double TSMComputeAverageEval(TSMAdmin_p admin,  FlatAnnoSet_p set);

void    TSMPrintFlat(FILE* out, TSM_p tsm);
void    TSMPrintRek(FILE* out, TSMAdmin_p admin, TSM_p tsm, int depth);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
