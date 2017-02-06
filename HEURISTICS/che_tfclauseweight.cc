/*-----------------------------------------------------------------------

File  : che_tfclauseweight.c

Contents
 
  Functions realising clause evaluation with clause weights based on TensorFlow.

-----------------------------------------------------------------------*/

extern "C" {
#include "che_tfclauseweight.h"
#include "che_clauseweight.h"
#include "che_hcb.h"
}  // extern "C"
#include "che_tfclauseweight_cc.h"

#include <stdio.h>
#include <unordered_map>
#include <unordered_set>
#include "heuristic.proto.h"

using deepmath::FastClause;

static bool using_tensorflow = false;

typedef std::unordered_map<int64, float> Evals;

// File descriptor for communication with the sandboxer
static const int comms_fd = 1023;

static void Send(const uint8* data, uint32 size) {
  while (size) {
    const auto sent = write(comms_fd, data, size);
    CHECK_GE(sent, 0);
    data += sent;
    size -= sent;
  }
}

static void Recv(uint8* data, uint32 size) {
  while (size) {
    const auto got = read(comms_fd, data, size);
    CHECK_GE(got, 0);
    data += got;
    size -= got;
  }
}

static void RpcCall(const deepmath::HeuristicRequest& request,
                    deepmath::HeuristicResponse* response) {
  // Tag for proto messages
  const uint32 proto_tag = 0x80000102;

  {
    // Send the request
    string str;
    CHECK(request.SerializeToString(&str));
    const uint32 size = str.size();
    Send(reinterpret_cast<const uint8*>(&proto_tag), sizeof(proto_tag));
    Send(reinterpret_cast<const uint8*>(&size), sizeof(size));
    Send(reinterpret_cast<const uint8*>(str.data()), size);
  }

  // Receive the reply
  uint32 tag, size;
  Recv(reinterpret_cast<uint8*>(&tag), sizeof(tag));
  CHECK_EQ(tag, proto_tag);
  Recv(reinterpret_cast<uint8*>(&size), sizeof(size));
  std::unique_ptr<uint8[]> value(new uint8[size]);
  Recv(reinterpret_cast<uint8*>(value.get()), size);
  CHECK(response->ParseFromArray(value.get(), size));
}

static Vocab& GetVocab() {
  static Vocab vocab;
  return vocab;
}

static Evals& GetEvals() {
  static Evals evals;
  return evals;
}

static char* SPrintIdent(long ident) {
  // This code is copied from ClauseTSTPPrint().
  // TODO(smloos): It would be better to use ClauseTSTPPrint directly.
  char identName[32];
  if (ident >= 0)
      snprintf(identName, sizeof(identName), "c_0_%ld ", ident);
  else
      snprintf(identName, sizeof(identName), "i_0_%ld ", ident-LONG_MIN);
  return strdup(identName);
}

static char* SPrintVar(FunCode var) {
  assert(var < 0);
  // This code is copied from VarPrint().
  // TODO(smloos): It would be better to use VarPrint directly.
  char id = 'X';
  if (var % 2) {
    id = 'Y';
  }
  char varName[64];
  snprintf(varName, sizeof(varName), "%c%ld", id, -((var - 1) / 2));
  return strdup(varName);
}

void TermVocab(TB_p bank, Term_p term,
               std::unordered_map<string, FunCode>* words) {
  assert(term);
  assert(bank->sig || TermIsVar(term));

  assert(
      !(SigSupportLists && TermPrintLists &&
        ((term->f_code == SIG_NIL_CODE) || (term->f_code == SIG_CONS_CODE))));

  if (TermIsVar(term)) {
    const char* varName = SPrintVar(term->f_code);
    words->insert(std::make_pair(varName, term->f_code));
    free((char*)varName);
  } else {
    words->insert(std::make_pair(SigFindName(bank->sig, term->f_code),
                                 term->f_code));
    if (!TermIsConst(term)) {
      assert(term->args);
      assert(term->arity >= 1);
      for (int i = 0; i < term->arity; i++) {
        TermVocab(bank, term->args[i], words);
      }
    }
  }
}

void ClauseVocab(Clause_p clause,
                 std::unordered_map<string, FunCode>* words) {
  Eqn_p eq = clause->literals;
  while (eq) {
    // Skip over $false is in a disjunction.
    if (!EqnIsPropFalse(eq)) {
      TermVocab(eq->bank, eq->lterm, words);
      if (EqnIsEquLit(eq)) {
        TermVocab(eq->bank, eq->rterm, words);
      }
    }
    eq = eq->next;
  }
}

void MakeFastTerm(Term_p term, const Vocab& vocab,
                  FastClause::Term* fast_term) {
  assert(term);
  assert(
      !(SigSupportLists && TermPrintLists &&
        ((term->f_code == SIG_NIL_CODE) || (term->f_code == SIG_CONS_CODE))));

  fast_term->set_id(vocab.id(term->f_code));
  if (!TermIsVar(term) && !TermIsConst(term)) {
    assert(term->args);
    assert(term->arity >= 1);
    for (int i = 0; i < term->arity; i++) {
      MakeFastTerm(term->args[i], vocab, fast_term->add_args());
    }
  }
}

void MakeFastClause(Clause_p clause, const Vocab& vocab,
                    FastClause* fast_clause) {
  Eqn_p eq = clause->literals;
  while (eq) {
    // Skip over $false in a disjunction.
    if (!EqnIsPropFalse(eq)) {
      auto* fast_equation = fast_clause->add_equations();
      fast_equation->set_negated(EqnIsNegative(eq));
      MakeFastTerm(eq->lterm, vocab, fast_equation->mutable_left());
      if (EqnIsEquLit(eq)) {
        MakeFastTerm(eq->rterm, vocab,
                     fast_equation->mutable_right());
      }
    }
    eq = eq->next;
  }
}

void VocabInit(PQueue_p clauses) {
  // Collect terms from clauses
  std::unordered_map<string, FunCode> words;
  for (int i = PQueueTailIndex(clauses); i != -1;
       i = PQueueIncIndex(clauses, i)) {
    Clause_p clause = (Clause_p)PQueueElementP(clauses, i);
    ClauseVocab(clause, &words);
  }

  // Add extra variables (variables that may be created during the proof)
  for (int i = 1; i <= 50; i++) {
    std::ostringstream s;
    s << "X" << i;
    words.insert(std::make_pair(s.str(), x_variable_f_code(i)));
  }

  // Request vocab
  deepmath::HeuristicRequest request;
  for (const auto& word : words) {
    request.add_vocabulary(word.first);
  }
  deepmath::HeuristicResponse response;
  RpcCall(request, &response);

  // Save vocab
  CHECK_EQ(request.vocabulary_size(), response.vocab_ids_size());
  CHECK_EQ(response.logits_size(), 0);
  Vocab& vocab = GetVocab();
  for (int i = 0; i < request.vocabulary_size(); i++) {
    const auto it = words.find(request.vocabulary(i));
    CHECK(it != words.end());
    vocab.SetId(it->second, response.vocab_ids(i));
  }

  // TODO(smloos): This is a hack to get large variables into the vocab.
  // For all variables "Xn" where n > 50, embed as "X50".
  const int x50_id = vocab.id(x_variable_f_code(50));
  for (int i = 51; i < 5000; i++) {
    vocab.SetId(x_variable_f_code(i), x50_id);
  }
}

void NegatedConjecturesInit(PQueue_p clauses) {
  deepmath::HeuristicRequest request;
  deepmath::HeuristicResponse response;

  // Build negated conjectures request (using saved vocabulary)
  for (int i = PQueueTailIndex(clauses); i != -1;
       i = PQueueIncIndex(clauses, i)) {
    Clause_p clause = (Clause_p)PQueueElementP(clauses, i);
    if (ClauseQueryTPTPType(clause) == CPTypeNegConjecture) {
      MakeFastClause(clause, GetVocab(), request.add_negated_conjecture());
    }
  }
  RpcCall(request, &response);
}

/*-----------------------------------------------------------------------
//
// Function: TensorFlowInit()
//
//   Given the initial clauses, create a vocabulary and initialize
//   TensorFlow model with negated_conjecture cnfs.
//
// Global Variables: -
//
// Side Effects    : Creates vocab, initializes TensorFlow model via RPC calls.
//
/----------------------------------------------------------------------*/


extern "C" void TensorFlowInit(PQueue_p clauses)
{
  if (using_tensorflow)
  {
    VocabInit(clauses);
    NegatedConjecturesInit(clauses);
  }
}

/*-----------------------------------------------------------------------
//
// Function: TensorFlowWeightInit()
//
//   Return an initialized WFCB for TensorFlowWeight evaluation.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

extern "C" WFCB_p TensorFlowWeightInit(ClausePrioFun prio_fun)
{
   return WFCBAlloc(TensorFlowWeightCompute, prio_fun, TrivialWeightExit, NULL);
}

/*-----------------------------------------------------------------------
//
// Function: TensorFlowWeightParse()
//
//   Parse a funweight-weight function based on weight given to clause
//   from TensorFlow model.
//
// Global Variables: -
//
// Side Effects    : Via ConjectureFunWeightInit, I/O.
//
/----------------------------------------------------------------------*/

extern "C" WFCB_p TensorFlowWeightParse(Scanner_p in, OCB_p ocb,
                                        ProofState_p state)
{
   ClausePrioFun prio_fun;

   AcceptInpTok(in, kOpenBracket);
   prio_fun = ParsePrioFun(in);
   AcceptInpTok(in, kCloseBracket);

   using_tensorflow = true;

   return TensorFlowWeightInit(prio_fun);
}

/*-----------------------------------------------------------------------
//
// Function: TensorFlowWeightBatchCompute()
//
//   Request and store evaluations for a set of pending clauses
//
/----------------------------------------------------------------------*/

void BatchInit(PQueue_p clauses) {
  auto& evals = GetEvals();
  evals.clear();

  deepmath::HeuristicRequest request;
  deepmath::HeuristicResponse response;

  // Build and send request with all pending clauses
  for (int i = PQueueTailIndex(clauses); i != -1;
       i = PQueueIncIndex(clauses, i)) {
    Clause_p clause = (Clause_p)PQueueElementP(clauses, i);
    MakeFastClause(clause, GetVocab(), request.add_clauses());
  }
  RpcCall(request, &response);

  // Save evaluation scores to be returned one-at-a-time from compute function.
  int logit_index = 0;
  for (int i = PQueueTailIndex(clauses); i != -1;
       i = PQueueIncIndex(clauses, i)) {
    Clause_p clause = (Clause_p)PQueueElementP(clauses, i);
    assert(clause->ident);
    // Negate the logit to get a lower-is-better score.
    evals.emplace(clause->ident, -response.logits().Get(logit_index));
    logit_index++;
  }
}

extern "C" void TensorFlowWeightBatchCompute(PQueue_p clauses)
{
  if (using_tensorflow && !TFLimitReached)
  {
   BatchInit(clauses);
  }
}

/*-----------------------------------------------------------------------
//
// Function: TensorFlowWeightCompute()
//
//   Retrieve a pre-computed evaluation for a clause.
//
// Global Variables: <The serialized negated_conjectures>
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

extern "C" double TensorFlowWeightCompute(void* data, Clause_p clause)
{
  if (TFLimitReached)
  {
    // These values are no longer being checked.
    return INFINITY;
  }

  auto it = GetEvals().find(clause->ident);
  QCHECK(it != GetEvals().end())
      << "TensorFlow weight not pre-computed for clause '"
      << SPrintIdent(clause->ident) << "'";
  return it->second;
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


