/*-----------------------------------------------------------------------

File  : che_tfclauseweight_cc.h

Header file for c++ functions called from unit tests.

-----------------------------------------------------------------------*/

#ifndef CHE_TFCLAUSEWEIGHT_CC

#define CHE_TFCLAUSEWEIGHT_CC


extern "C" {
#include <che_wfcb.h>
}

// Memorize some constants that depend on macros we're about to remove
static const auto kOpenBracket = OpenBracket;
static const auto kCloseBracket = CloseBracket;

// Remove some horrendous preprocessor defines.  As a consequence, no eprover
// header can be included after this header.
#undef Name
#undef String

#include <unordered_map>
#include <unordered_set>
#include <heuristic.proto.h>

// Map from f_codes to tensorflow model vocab ids.
class Vocab {
 public:
  Vocab() {}

  int id(FunCode f_code) const {
    const auto& table = f_code < 0 ? neg_to_id_ : pos_to_id_;
    const FunCode n = f_code < 0 ? ~f_code : f_code;
    QCHECK(n < table.size() && table[n] > 0) << "Unknown f_code " << f_code;
    return table[n];
  }

  void SetId(FunCode f_code, int id) {
    auto& table = f_code < 0 ? neg_to_id_ : pos_to_id_;
    const FunCode n = f_code < 0 ? ~f_code : f_code;
    if (n >= table.size()) {
      table.resize(n + 1, -1);  // Use -1's for sentinels
    }
    QCHECK_EQ(table[n], -1) << "Can't set f_code " << f_code << " to " << id
        << " (already set to " << table[n] << ")";
    table[n] = id;
  }

 private:
  // Use separate maps for negative and positive f_codes (vars and functions).
  // Negative f_codes are complemented to get vector indices.
  std::vector<int> neg_to_id_;
  std::vector<int> pos_to_id_;

  DISALLOW_COPY_AND_ASSIGN(Vocab);
};

// X<x> to corresponding f_code
static inline FunCode x_variable_f_code(int x) {
  return -2 * x;
}

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

void ClauseVocab(Clause_p clause, std::unordered_map<string, FunCode>* words);

void MakeFastClause(Clause_p clause, const Vocab& vocab,
                    deepmath::FastClause* fast_clause);

#endif  // CHE_TFCLAUSEWEIGHT_CC

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
