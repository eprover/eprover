/*-----------------------------------------------------------------------

  File  : cle_tsm.c

  Author: Stephan Schulz

  Contents


  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: Fri Aug  6 23:17:27 GMT 1999

  -----------------------------------------------------------------------*/

#include "cle_tsm.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

char* TSMTypeNames[] =
{
   "NoType",
   "Flat",
   "Recursive",
   "Recurrent",
   "RecLocal",
   NULL
};

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

#ifdef NEVER_DEFINED

/*-----------------------------------------------------------------------
//
// Function: dist_combi_entropy()
//
//   Compute the entropy of a class/partition distribution.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static double dist_combi_entropy(PDArray_p distribution, long
                                 maxindex, double limit)
{
   long sum = 0, pcount, ncount, i;
   long *cache = SizeMalloc(2*(maxindex+1)*sizeof(long));
   double res = 0.0, relfreq;
   FlatAnnoTerm_p handle;

   for(i=0; i<=maxindex; i++)
   {
      pcount = 0;
      ncount = 0;
      for(handle = PDArrayElementP(distribution,i); handle; handle =
             handle->next)
      {
         if(TSMEvalNormalize(handle->eval, limit) == -1)
         {
            ncount += handle->sources;
         }
         else
         {
            pcount += handle->sources;
         }
      }
      cache[i]            = ncount;
      cache[i+maxindex+1] = pcount;
      sum += (ncount+pcount);
   }
   for(i=0; i<=(2*maxindex+1); i++)
   {
      if(cache[i])
      {
         relfreq = (double)cache[i]/(double)sum;
         res -= relfreq*log2(relfreq);
      }
   }
   SizeFree(cache, 2*(maxindex+1)*sizeof(long));
   return res;
}

#endif /* NEVER_DEFINED */


/*-----------------------------------------------------------------------
//
// Function: distribution_entropy()
//
//   Compute the entropy of a distribution.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static double distribution_entropy(PDArray_p distribution, long
                                   maxindex)
{
   long sum = 0, count, i;
   long *cache = SizeMalloc((maxindex+1)*sizeof(long));
   double res = 0.0, relfreq;
   FlatAnnoTerm_p handle;

   for(i=0; i<=maxindex; i++)
   {
      count = 0;
      for(handle = PDArrayElementP(distribution,i); handle; handle =
             handle->next)
      {
         count += handle->sources;
      }
      cache[i] = count;
      sum += count;
   }
   for(i=0; i<=maxindex; i++)
   {
      if(cache[i])
      {
         relfreq = (double)cache[i]/(double)sum;
         res -= relfreq*log2(relfreq);
      }
   }
   SizeFree(cache, (maxindex+1)*sizeof(long));
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: evaluate_index()
//
//   Given an index and a set, return the relative information gain
//   from this index.
//
// Global Variables: -
//
// Side Effects    : Expensive, memory operations
//
/----------------------------------------------------------------------*/

static double evaluate_index(FlatAnnoSet_p set, TSMIndex_p index,
                             PDArray_p cache, double limit)
{
   PDArray_p  partition;
   double     entropy, remainder, relgain = 0.0;
   long       parts, maxindex;

   partition = PDArrayAlloc(1000,2000);

   maxindex = TSMPartitionSet(partition, index, set, cache);

   entropy = TSMFlatAnnoSetEntropy(set, limit);
   remainder = TSMRemainderEntropy(partition, &parts, limit,
                                   maxindex);

   if(parts != 1)
   {
      /* relgain = (entropy-remainder)/(double)parts; */
      relgain = (entropy-remainder)/
         (distribution_entropy(partition,
                               maxindex)-(entropy-remainder));
      /* relgain = (entropy-remainder)/dist_combi_entropy(partition,
         maxindex, limit);  */
   }
   PDArrayFree(partition);

   return relgain;
}


/*-----------------------------------------------------------------------
//
// Function: evaluate_index_desc()
//
//   Given an index description, return the relative information gain
//   from this index.
//
// Global Variables: -
//
// Side Effects    : Expensive, memory operations
//
/----------------------------------------------------------------------*/

static double evaluate_index_desc(TSMAdmin_p admin, FlatAnnoSet_p set,
                                  long depth, IndexType indextype,
                                  double limit)
{
   TSMIndex_p index;
   double     relgain;

   index = TSMIndexAlloc(indextype, depth, admin->index_bank,
                         admin->subst);
   relgain = evaluate_index(set, index, NULL,limit);
   TSMIndexFree(index);

   if(OutputLevel)
   {
      printf(COMCHAR" Index type = %2d, depth = %2ld, relative gain = %f\n",
             indextype, depth, relgain);
   }
   return relgain;
}


/*-----------------------------------------------------------------------
//
// Function: evaluate_top_index()
//
//   Evlauate all termtop index functions described by indextype. If
//   one of them beats to_beat, set *best_type to the value of the
//   best index function and return its relative information gain.
//
// Global Variables: -
//
// Side Effects    : Via evaluate_index_desc()
//
/----------------------------------------------------------------------*/

static double evaluate_top_index(TSMAdmin_p admin, FlatAnnoSet_p set,
                                 long depth, IndexType indextype,
                                 IndexType *best_type, double to_beat,
                                 double limit)
{
   double relgain;

   if(indextype & IndexTop)
   {
      relgain = evaluate_index_desc(admin, set, depth, IndexTop, limit);
      if(relgain > to_beat)
      {
         to_beat    = relgain;
         *best_type = IndexTop;
      }
   }
   if(indextype & IndexAltTop)
   {
      relgain = evaluate_index_desc(admin, set, depth, IndexAltTop, limit);
      if(relgain > to_beat)
      {
         to_beat    = relgain;
         *best_type = IndexAltTop;
      }
   }
   if(indextype & IndexCSTop)
   {
      relgain = evaluate_index_desc(admin, set, depth, IndexCSTop, limit);
      if(relgain > to_beat)
      {
         to_beat    = relgain;
         *best_type = IndexCSTop;
      }
   }
   if(indextype & IndexESTop)
   {
      relgain = evaluate_index_desc(admin, set, depth, IndexESTop, limit);
      if(relgain > to_beat)
      {
         to_beat    = relgain;
         *best_type = IndexESTop;
      }
   }
   return to_beat;
}



/*-----------------------------------------------------------------------
//
// Function: compute_list_entropy()
//
//   Return the entropy of list (and the length in *count).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static double compute_list_entropy(FlatAnnoTerm_p list, long *count,
                                   double limit)
{

   long           pos=0, neg=0;
   double         res, freq;

   while(list)
   {
      if(TSMEvalNormalize(list->eval, limit) == -1)
      {
         neg += list->sources;
      }
      else
      {
         pos += list->sources;
      }
      list = list->next;
   }
   *count = pos+neg;

   if((pos == 0) || (neg ==0)) /* We are perfectly classified ->
                                  Entropy is 0 */
   {
      res = 0.0;
   }
   else
   {
      freq = (double)pos/(double)(pos + neg);
      res = freq*-log2(freq);
      freq = (double)neg/(double)(pos + neg);
      res -= freq*log2(freq);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: tsm_rec_eval()
//
//   Recursivly evaluate a term with a tsm. Return the weighted sum of
//   all found evaluations in *res and the weight directly.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static double tsm_rec_eval(TSMAdmin_p admin, double *res,  TSM_p tsm,
                           Term_p term, PatternSubst_p subst)
{
   long  key;
   TSA_p tsa;
   double eval_weight = 0;

   key = TSMIndexFind(tsm->index, term, subst);

   if((key!=-1) && (tsa = PDArrayElementP(tsm->tsas, key)))
   {
      int i;

      assert(tsa->arity == term->arity);
      eval_weight =  tsa->eval_weight;
      *res += tsa->eval_weight * tsa->eval;

      if(admin->tsmtype != TSMTypeFlat)
      {
         for(i=0; i<tsa->arity; i++)
         {
            eval_weight+= tsm_rec_eval(admin, res, tsa->arg_tsms[i],
                                       term->args[i], subst );
         }
      }
   }
   else
   {
      eval_weight = admin->unmapped_weight;
      *res += admin->unmapped_eval * admin->unmapped_weight;
   }
   return eval_weight;
}


/*-----------------------------------------------------------------------
//
// Function: tsm_rec_eval_no_weight()
//
//   Recursivly evaluate a term with a tsm. Return the sum of
//   all found evaluations in *res and the number of matched nodes
//   directly.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static double tsm_rec_eval_no_weight(TSMAdmin_p admin, double *res,
                                     TSM_p tsm, Term_p term, PatternSubst_p subst)
{
   long  key;
   TSA_p tsa;
   double eval_weight = 1;
   int i;

   key = TSMIndexFind(tsm->index, term, subst);

   if((key!=-1) && (tsa = PDArrayElementP(tsm->tsas, key)))
   {

      assert(tsa->arity == term->arity);
      *res += tsa->eval;

      if(admin->tsmtype != TSMTypeFlat)
      {
         for(i=0; i<term->arity; i++)
         {
            eval_weight+= tsm_rec_eval(admin, res, tsa->arg_tsms[i],
                                       term->args[i], subst );
         }
      }
   }
   else
   {
      if(admin->tsmtype == TSMTypeRecursive)
      {
         eval_weight = TermWeight(term,1,1);
      }
      *res += eval_weight*admin->unmapped_eval;
      if(admin->tsmtype == TSMTypeRecurrent)
      {
         for(i=0; i<term->arity; i++)
         {
            eval_weight+= tsm_rec_eval(admin, res, tsm, term->args[i],
                                       subst );
         }
      }
   }
   return eval_weight;
}



/*-----------------------------------------------------------------------
//
// Function: tsmbasealloc()
//
//   Return a tsm with admin and index set.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static TSM_p tsmbasealloc(TSMAdmin_p admin, IndexType type, long depth)
{
   TSM_p     tsm = TSMCellAlloc();

   tsm->admin = admin;
   tsm->tsas  = NULL;
   tsm->index = TSMIndexAlloc(type, depth, admin->index_bank,
                              admin->subst);
   tsm->maxindex = -1;
   return tsm;
}


/*-----------------------------------------------------------------------
//
// Function: tsmcomplete()
//
//   Complete a base tsm cell.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

static void tsmcomplete(TSMAdmin_p admin, TSM_p tsm, FlatAnnoSet_p set)
{
   long      i;
   PDArray_p partition = PDArrayAlloc(1000,2000);
   FlatAnnoTerm_p part;

   tsm->maxindex = TSMPartitionSet(partition, tsm->index, set, NULL);

   tsm->tsas = PDArrayAlloc(tsm->maxindex+2, 2000); /* For the case -1
                                                     */

   for(i=0; i<=tsm->maxindex; i++)
   {
      part = PDArrayElementP(partition, i);
      if(part)
      {
         PDArrayAssignP(tsm->tsas, i, TSACreate(admin, part));
      }
   }
   PDArrayFree(partition);
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: TSMRemainderEntropy()
//
//   Compute the remainder entropy of the pos/neg distinction (defined
//   by the terms evaluation) under the assumption of the
//   partition. *parts is set to the number of non-empty partitions.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

double TSMRemainderEntropy(PDArray_p partition, long *parts, double
                           limit, long max_index)
{
   double res = 0, lres;
   long   count, global_count = 0, i;

   *parts = 0;
   for(i=0; i<=max_index; i++)
   {
      lres = compute_list_entropy(PDArrayElementP(partition, i),
                                  &count, limit);
      if(count)
      {
         (*parts)++;
         res          += count*lres;
         global_count += count;
      }
   }
   return res/(double)global_count;
}

/*-----------------------------------------------------------------------
//
// Function: TSMFlatAnnoSetEntropy()
//
//   Compute the entropy of a flat annotation set under the assumption
//   that terms with eval >limit are class one, all other terms are
//   class2.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double TSMFlatAnnoSetEntropy(FlatAnnoSet_p set, double limit)
{
   PStack_p  stack;
   long      pos=0, neg=0;
   double    res, freq;
   NumTree_p handle;
   FlatAnnoTerm_p term;

   stack = NumTreeTraverseInit(set->set);

   while((handle = NumTreeTraverseNext(stack)))
   {
      term = handle->val1.p_val;
      if(TSMEvalNormalize(term->eval, limit) == -1)
      {
         neg += term->sources;
      }
      else
      {
         pos += term->sources;
      }
   }
   NumTreeTraverseExit(stack);

   if((pos == 0) || (neg ==0)) /* We are perfectly classified ->
                                  Entropy is 0 */
   {
      res = 0.0;
   }
   else
   {
      freq = (double)pos/(double)(pos + neg);
      res = freq*-log2(freq);
      freq = (double)neg/(double)(pos + neg);
      res += freq*-log2(freq);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: TSMPartitionSet()
//
//   Generates a partition by assigning each FlatAnnoterm from set to an
//   element of index(set). Returns largest index. If cache is != 0,
//   use it as a cache (Du!) ;-)
//
// Global Variables: -
//
// Side Effects    : Changes next pointers in terms from set.
//
/----------------------------------------------------------------------*/

long TSMPartitionSet(PDArray_p partition, TSMIndex_p index,
                     FlatAnnoSet_p set, PDArray_p cache)
{
   long           res=-1;
   PStack_p       stack;
   NumTree_p      handle;
   FlatAnnoTerm_p current;
   long           key;

   stack = NumTreeTraverseInit(set->set);
   while((handle = NumTreeTraverseNext(stack)))
   {
      current = handle->val1.p_val;
      if(cache)
      {
         key = PDArrayElementInt(cache, current->term->entry_no);
         if(key)
         {
            key--;
         }
         else
         {
            key = TSMIndexInsert(index,  current->term);
            PDArrayAssignInt(cache, current->term->entry_no, key+1);
         }
      }
      else
      {
         key = TSMIndexInsert(index,  current->term);
      }
      res = MAX(res, key);
      current->next = PDArrayElementP(partition, key);
      PDArrayAssignP(partition, key, current);
   }
   NumTreeTraverseExit(stack);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: TSMFindOptimalIndex()
//
//   Find the optimal index (i.e. the one with the largest relative
//   information gain) among those specified. If *depth != 0, try only
//   at that depth.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

IndexType TSMFindOptimalIndex(TSMAdmin_p admin, FlatAnnoSet_p set,
                              long *depth, IndexType indextype, double
                              limit)
{
   long i;
   double     best = -1.0;
   IndexType  best_index = IndexNoIndex;
   long       best_depth = *depth;
   double     relgain;
   IndexType  tmp = indextype;

   while(!(tmp & 1))
   {
      tmp = tmp >> 1;
   }

   if(indextype & IndexArity)
   {
      if(tmp!=1)
      {
         relgain = evaluate_index_desc(admin, set, 0, IndexArity, limit);
         if(relgain > best)
         {
            best       = relgain;
            best_index = IndexArity;
            best_depth = 0;
         }
      }
      else
      {
         best_index = indextype;
      }
   }
   if(indextype & IndexSymbol)
   {
      if(tmp!=1)
      {
         relgain = evaluate_index_desc(admin, set, 0, IndexSymbol, limit);
         if(relgain > best)
         {
            best       = relgain;
            best_index = IndexSymbol;
            best_depth  = 0;
         }
      }
      else
      {
         best_index = indextype;
      }
   }
   if(indextype & IndexIdentity)
   {
      if(tmp!=1)
      {
         relgain = evaluate_index_desc(admin, set, 0, IndexIdentity, limit);
         if(relgain > best)
         {
            best       = relgain;
            best_index = IndexIdentity;
            best_depth = 0;
         }
      }
      else
      {
         best_index = indextype;
      }
   }
   if(*depth != IndexDynamicDepth)
   {
      if(tmp != 1)
      {
         relgain = evaluate_top_index(admin, set, *depth, indextype,
                                      &best_index, best, limit);
         if(relgain > best)
         {
            best = relgain;
            best_depth = *depth;
         }
      }
      else
      {
         best_index = indextype;
      }
   }
   else
   {
      for(i=1; i<= TSM_MAX_TERMTOP; i++)
      {
         relgain = evaluate_top_index(admin, set, i, indextype,
                                      &best_index, best, limit);
         if(relgain > best)
         {
            best_depth = i;
            best = relgain;
         }
      }
   }
   *depth = best_depth;
   if(OutputLevel)
   {
      fprintf(GlobalOut, COMCHAR" Selected: %2d  depth: %2ld\n", best_index,
              best_depth);
   }
   return best_index;
}

/*-----------------------------------------------------------------------
//
// Function: TSMInsertSubtermSet()
//
//   Given a list of FlatAnnoTerms(), insert new, non-reference-carrying
//   FlatAnnoterms corresponding to the subterms at position select into
//   set. Return number of elements in new set.
//
// Global Variables: -
//
// Side Effects    : Changes set.
//
/----------------------------------------------------------------------*/

long TSMCreateSubtermSet(FlatAnnoSet_p set, FlatAnnoTerm_p list, int sel)
{
   Term_p         term;
   long           count = 0;
   FlatAnnoTerm_p new_aterm;

   while(list)
   {
      term = list->term;
      assert(term->arity > sel);
      new_aterm = FlatAnnoTermAlloc(term->args[sel],
                                    list->eval, list->eval_weight,
                                    list->sources);
      FlatAnnoSetAddTerm(set, new_aterm);
      list = list->next;
      count++;
   }
   return count;
}


/*-----------------------------------------------------------------------
//
// Function: TSMAdminAlloc()
//
//   Return an initialized TSMAdminCell suitable for building an TSM
//   with.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

TSMAdmin_p TSMAdminAlloc(Sig_p sig, TSMType type)
{
   TSMAdmin_p handle = TSMAdminCellAlloc();

   handle->index_bank      = TBAlloc(sig);
   handle->tsmtype         = type;
   handle->index_type      = IndexNoIndex;
   handle->index_depth     = 0;
   handle->limit           = 0;
   handle->eval_limit      = 0;
   handle->unmapped_eval   = 0;
   handle->unmapped_weight = 0;
   handle->tsm             = NULL;
   handle->subst           = NULL;
   handle->local_limit     = true;
   handle->emptytsm        = tsmbasealloc(handle, IndexEmpty, 0);
   if(handle->tsmtype == TSMTypeRecurrentLocal)
   {
      handle->tsmstack = PStackAlloc();
      handle->cachestack = PStackAlloc();
   }
   else
   {
      handle->tsmstack = NULL;
   }
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: TSMAdminFree()
//
//   Free a TSMAdmin data structure
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void TSMAdminFree(TSMAdmin_p junk)
{
   assert(junk);

   if(junk->tsmstack)
   {
      assert(junk->tsmtype == TSMTypeRecurrentLocal);
      while(!PStackEmpty(junk->tsmstack))
      {
         TSMFree(PStackPopP(junk->tsmstack));
      }
      PStackFree(junk->tsmstack);
      assert(junk->cachestack);
      while(!PStackEmpty(junk->cachestack))
      {
         PDArrayFree(PStackPopP(junk->cachestack));
      }
      PStackFree(junk->cachestack);
   }
   else if(junk->tsm)
   {
      TSMFree(junk->tsm);
   }
   if(junk->emptytsm)
   {
      TSMFree(junk->emptytsm);
   }
   junk->index_bank->sig = NULL;
   TBFree(junk->index_bank);

   /* subst is external! */
   TSMAdminCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: TSMAdminBuildTSM()
//
//   Given a set of flatly annotated terms, build a TSM.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void TSMAdminBuildTSM(TSMAdmin_p admin, FlatAnnoSet_p set, IndexType
                      type, int depth, PatternSubst_p subst)
{
   FlatAnnoSet_p flatset = FlatAnnoSetAlloc();
   TSM_p tsm;
   int i;
   PStackPointer sp;
   double relgain, bestgain = -1;

   assert(admin);
   assert(set);
   assert(type!=IndexNoIndex);

   admin->index_type = type;
   admin->index_depth = depth;
   admin->subst  = subst;
   admin->limit = FlatAnnoSetEvalAverage(set);
   admin->eval_limit = admin->limit; /* May be changed later! */

   switch(admin->tsmtype)
   {
   case TSMTypeRecursive:
         TSMCreate(admin, set);
         break;
   case TSMTypeFlat:
         TSMCreate(admin, set);
         break;
   case TSMTypeRecurrent:
         FlatAnnoSetFlatten(flatset, set);
         TSMCreate(admin, flatset);
         break;
   case TSMTypeRecurrentLocal:
         tsm = tsmbasealloc(admin, IndexArity, 0);
         PStackPushP(admin->tsmstack, tsm);
         PStackPushP(admin->cachestack, PDArrayAlloc(10,50));
         tsm = tsmbasealloc(admin, IndexSymbol, 0);
         PStackPushP(admin->tsmstack, tsm);
         PStackPushP(admin->cachestack, PDArrayAlloc(10,50));
         for(i=1; i<= TSM_MAX_TERMTOP; i++)
         {
            tsm = tsmbasealloc(admin, IndexTop, i);
            PStackPushP(admin->tsmstack, tsm);
            PStackPushP(admin->cachestack, PDArrayAlloc(20*i*i,30*i*i));
            tsm = tsmbasealloc(admin, IndexAltTop, i);
            PStackPushP(admin->tsmstack, tsm);
            PStackPushP(admin->cachestack, PDArrayAlloc(20*i*i,30*i*i));
            tsm = tsmbasealloc(admin, IndexCSTop, i);
            PStackPushP(admin->tsmstack, tsm);
            PStackPushP(admin->cachestack, PDArrayAlloc(20*i*i,30*i*i));
            tsm = tsmbasealloc(admin, IndexESTop, i);
            PStackPushP(admin->tsmstack, tsm);
            PStackPushP(admin->cachestack, PDArrayAlloc(20*i*i,30*i*i));
         }
         FlatAnnoSetFlatten(flatset, set);
         admin->tsm = NULL;
         for(sp = 0; sp < PStackGetSP(admin->tsmstack); sp++)
         {
            tsm = PStackElementP(admin->tsmstack,sp);
            tsmcomplete(admin, tsm, flatset);
            relgain =
               evaluate_index(flatset, tsm->index,
                              PStackElementP(admin->cachestack,sp),
                              admin->limit);
            if(relgain > bestgain)
            {
               bestgain = relgain;
               admin->tsm = tsm;
            }
         }
         assert(admin->tsm);
         break;
   default:
         assert(false && "Illegal TSMtype in TSMAdminBuildTSM()");
         break;
   }
   FlatAnnoSetFree(flatset);
   /* TSMIndexPrint(stdout, admin->tsm->index,0); */
   /* TSMPrintRek(stdout, admin, admin->tsm, 0); */
   /* TSMPrintFlat(stdout, admin->tsm); */
}


/*-----------------------------------------------------------------------
//
// Function: TSMCreate()
//
//   Create a TSM according to the specification in admin.
//
// Global Variables: -
//
// Side Effects    : Memory operations, if admin->tsm == NULL, sets it.
//
/----------------------------------------------------------------------*/

TSM_p TSMCreate(TSMAdmin_p admin, FlatAnnoSet_p set)
{
   TSM_p     tsm;
   IndexType indextype;
   long      depth;
   double    limit = admin->limit;

   depth = admin->index_depth;

   if(admin->local_limit)
   {
      limit = FlatAnnoSetEvalWeightedAverage(set);
      /* printf(COMCHAR" Limit: %f\n", limit); */
   }
   indextype = TSMFindOptimalIndex(admin, set, &depth,
                                   admin->index_type, limit);
   /* printf("Selected: Index type = %d, depth = %ld\n", indextype,
      depth);  */
   /* printf(COMCHAR" TSMCreate: %ld patterns\n", NumTreeNodes(set->set));
      FlatAnnoSetPrint(stdout, set, admin->index_bank->sig);*/

   tsm = tsmbasealloc(admin, indextype, depth);

   if(!admin->tsm)
   {
      admin->tsm = tsm;
   }
   tsmcomplete(admin, tsm, set);
   return tsm;
}


/*-----------------------------------------------------------------------
//
// Function: TSMFree()
//
//   Free a TSM.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void TSMFree(TSM_p tsm)
{
   long i;
   TSA_p tsa;

   assert(tsm);

   if(tsm->tsas)
   {
      for(i=0; i<=tsm->maxindex; i++)
      {
         tsa = PDArrayElementP(tsm->tsas, i);
         if(tsa)
         {
            TSAFree(tsa);
         }
      }
      PDArrayFree(tsm->tsas);
   }
   TSMIndexFree(tsm->index);
   TSMCellFree(tsm);
}


/*-----------------------------------------------------------------------
//
// Function: TSACreate()
//
//   Create a TSA from a list of flatly annotated terms.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

TSA_p TSACreate(TSMAdmin_p admin, FlatAnnoTerm_p list)
{
   int i;
   double eval, eval_weight;
   FlatAnnoTerm_p handle;
   TSA_p tsa = TSACellAlloc();
   FlatAnnoSet_p subset;
   double relgain, bestgain;
   PStackPointer sp;
   TSM_p tsm;

   assert(list);
   assert(admin);

   tsa->admin = admin;
   tsa->arity   = list->term->arity;
   eval        = 0.0;
   eval_weight = 0.0;
   for(handle = list; handle; handle = handle->next)
   {
      assert(handle->term->arity == tsa->arity);
      eval        += handle->eval_weight*handle->eval;
      eval_weight += handle->eval_weight;
   }
   tsa->eval        = eval/eval_weight;
   tsa->eval_weight = eval_weight;

   if(tsa->arity)
   {
      tsa->arg_tsms = SizeMalloc(tsa->arity * sizeof(TSM_p));
      for(i=0; i<tsa->arity; i++)
      {
         switch(admin->tsmtype)
         {
         case TSMTypeFlat:
               tsa->arg_tsms[i] = admin->emptytsm;
               break;
         case TSMTypeRecursive:
               subset = FlatAnnoSetAlloc();
               TSMCreateSubtermSet(subset, list, i);
               tsa->arg_tsms[i] = TSMCreate(admin, subset);
               FlatAnnoSetFree(subset);
               break;
         case TSMTypeRecurrent:
               tsa->arg_tsms[i] = admin->tsm;
               break;
         case TSMTypeRecurrentLocal:
               bestgain = -1.0;
               subset = FlatAnnoSetAlloc();
               TSMCreateSubtermSet(subset, list, i);
               for(sp = 0; sp < PStackGetSP(admin->tsmstack); sp++)
               {
                  tsm = PStackElementP(admin->tsmstack,sp);
                  relgain =
                     evaluate_index(subset, tsm->index,
                                    PStackElementP(admin->cachestack,sp),
                                    admin->limit);
                  if(relgain > bestgain)
                  {
                     bestgain = relgain;
                     tsa->arg_tsms[i] = tsm;
                  }
               }
               FlatAnnoSetFree(subset);
               break;
         default:
               assert(false && "Unknown type in TSACreate!");
               break;
         }
      }
   }
   else
   {
      tsa->arg_tsms = NULL;
   }
   return tsa;
}


/*-----------------------------------------------------------------------
//
// Function: TSAFree()
//
//   Free a TSA.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void TSAFree(TSA_p tsa)
{
   int i;

   if(tsa->arg_tsms)
   {
      assert(tsa->arity);
      if(tsa->admin->tsmtype == TSMTypeRecursive)
      {
         for(i=0; i<tsa->arity; i++)
         {
            TSMFree(tsa->arg_tsms[i]);
         }
      }
      SizeFree(tsa->arg_tsms, tsa->arity * sizeof(TSM_p));
   }
   TSACellFree(tsa);
}


/*-----------------------------------------------------------------------
//
// Function: TSMEvalTerm()
//
//   Return an evaluation of term (as the weighted average evaluation
//   of all TSM nodes selected by term)
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double TSMEvalTerm(TSMAdmin_p admin, Term_p term, PatternSubst_p subst)
{
   double  res = 0.0;
   double  eval_weight;

   eval_weight = tsm_rec_eval_no_weight(admin, &res, admin->tsm, term, subst);
   if(eval_weight == 0.0)
   {
      return admin->limit;
   }
   return res/eval_weight;
}


/*-----------------------------------------------------------------------
//
// Function: TSMComputeClassificationLimit()
//
//   Evaluate all terms and return the (avgevalpos+avgevalneg)/2.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double TSMComputeClassificationLimit(TSMAdmin_p admin,  FlatAnnoSet_p
                                     set)
{
   PStack_p       setstack;
   double         poseval = 0.0, negeval = 0.0, eval = 0;
   long           pos=0, neg=0;
   NumTree_p      handle;
   FlatAnnoTerm_p fterm;

   setstack = NumTreeTraverseInit(set->set);

   while((handle = NumTreeTraverseNext(setstack)))
   {
      fterm = handle->val1.p_val;
      eval = TSMEvalTerm(admin, fterm->term, admin->subst);
      if(fterm->eval < admin->limit)
      {
         poseval+=eval*fterm->sources;
         pos+=fterm->sources;
      }
      else
      {
         negeval+=eval*fterm->sources;
         neg+=fterm->sources;
      }
   }
   NumTreeTraverseExit(setstack);

   if(!pos && !neg)
   {
      return 0.0;
   }
   if(!pos)
   {
      return negeval / (double)neg;
   }
   if(!neg)
   {
      return poseval / (double)pos;
   }
   negeval = negeval / (double)neg;
   poseval = poseval / (double)pos;
   eval = (poseval+negeval)/2.0;
   return eval;
}


/*-----------------------------------------------------------------------
//
// Function: TSMComputeAverageEval
//
//   Evaluate all terms and return average evaluation.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double TSMComputeAverageEval(TSMAdmin_p admin,  FlatAnnoSet_p set)
{
   PStack_p       setstack;
   double         eval = 0.0;
   long           count = 0;
   NumTree_p      handle;
   FlatAnnoTerm_p fterm;

   if(!set->set)
   {
      return 0.0;
   }

   setstack = NumTreeTraverseInit(set->set);

   while((handle = NumTreeTraverseNext(setstack)))
   {
      fterm = handle->val1.p_val;
      eval += TSMEvalTerm(admin, fterm->term, admin->subst)*fterm->sources;
      count+=fterm->sources;
   }
   NumTreeTraverseExit(setstack);

   return eval/(double)count;
}


/*-----------------------------------------------------------------------
//
// Function: TSMPrintFlat()
//
//   Print the tsm's tsa-distribution.
//
// Global Variables:
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void TSMPrintFlat(FILE* out, TSM_p tsm)
{
   long i;
   TSA_p tsa;

   for(i=0; i<=tsm->maxindex;i++)
   {
      tsa = PDArrayElementP(tsm->tsas, i);
      if(tsa)
      {
         fprintf(out, COMCHAR" %3ld: Weight = %6.3f EvalWeight = %6.3f\n",
                 i, tsa->eval, tsa->eval_weight);
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: TSMPrintRek()
//
//   Print a complete TSM
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void TSMPrintRek(FILE* out, TSMAdmin_p admin, TSM_p tsm, int depth)
{
   long i, j;
   TSA_p tsa;
   char pattern[70];

   sprintf(pattern,
           COMCHAR" %%%ds%%4ld: Weight = %%7.5f EvalWeight = %%7.5f\n",
           3*depth);
   TSMIndexPrint(stdout, tsm->index, depth);
   for(i=0; i<=tsm->maxindex;i++)
   {
      tsa = PDArrayElementP(tsm->tsas, i);
      if(tsa)
      {
         fprintf(out, pattern,"",
                 i, tsa->eval, tsa->eval_weight);
         for(j=0; j<tsa->arity; j++)
         {
            TSMPrintRek(out, admin, tsa->arg_tsms[j], depth+1);
         }
      }
   }
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
