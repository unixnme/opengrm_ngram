
// Licensed under the Apache License, Version 2.0 (the 'License');
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an 'AS IS' BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Copyright 2005-2016 Brian Roark and Google, Inc.
// NGram model class for outputting a model or outputting perplexity of text.

#ifndef NGRAM_NGRAM_OUTPUT_H_
#define NGRAM_NGRAM_OUTPUT_H_

#include <ostream>
#include <string>

#include <fst/compose.h>
#include <ngram/ngram-context.h>
#include <ngram/ngram-mutable-model.h>
#include <ngram/util.h>

namespace ngram {

using fst::StdFst;
using fst::ComposeFst;
using fst::ComposeFstOptions;
using fst::CacheOptions;

using fst::MATCHER_REWRITE_NEVER;
using fst::PhiMatcher;

static const int kSpecialLabel = -2;

class NGramOutput : public NGramMutableModel<StdArc> {
 public:
  typedef StdArc::StateId StateId;
  typedef StdArc::Label Label;
  typedef StdArc::Weight Weight;

  // Construct an NGramModel object, consisting of the fst and some
  // information about the states under the assumption that the fst is a model
  explicit NGramOutput(StdMutableFst *infst, std::ostream &ostrm = std::cout,
                       Label backoff_label = 0, bool check_consistency = false,
                       const std::string &context_pattern = "",
                       bool include_all_suffixes = false)
      : NGramMutableModel<StdArc>(infst, backoff_label, kNormEps,
                                  !context_pattern.empty()),
        ostrm_(ostrm),
        include_all_suffixes_(include_all_suffixes),
        context_(context_pattern, HiOrder()) {
    if (!GetFst().InputSymbols()) {
      NGRAMERROR() << "NGramOutput: no symbol tables provided";
      NGramModel<StdArc>::SetError();
    }
  }

  enum ShowBackoff {
    EPSILON,  // Show backoff Weights as explicit epsilon transitions.
    INLINE,  // Show backoff Weights in a third column when present.
    NONE,
  };

  // Print the N-gram model: each n-gram is on a line with its weight
  void ShowNGramModel(ShowBackoff showeps, bool neglogs, bool intcnts,
                      bool ARPA) const;

  // Use n-gram model to calculate perplexity of input strings.
  bool PerplexityNGramModel(
      const std::vector<std::unique_ptr<fst::StdVectorFst>> &infsts,
      int32 v, bool phimatch, std::string *OOV_symbol, double OOV_class_size,
      double OOV_probability);

  // Extract random samples from model and output
  void SampleStringsFromModel(int64 samples, bool show_backoff) {
    DeBackoffNGramModel();                  // Convert from backoff
    if (Error()) return;
    RandNGramModel(samples, show_backoff);  // randgen from resulting model
  }

  typedef PhiMatcher<Matcher<Fst<StdArc> > > NGPhiMatcher;

  ComposeFst<StdArc> *FailLMCompose(const StdMutableFst &infst,
                                    Label special_label) const {
    ComposeFst<StdArc> *cfst = new ComposeFst<StdArc>(
        infst, GetFst(),
        ComposeFstOptions<StdArc, NGPhiMatcher>(
            CacheOptions(), new NGPhiMatcher(infst, MATCH_NONE, kNoLabel),
            new NGPhiMatcher(GetFst(), MATCH_INPUT, special_label, 1,
                             MATCHER_REWRITE_NEVER)));
    return cfst;
  }

  void FailLMCompose(const StdMutableFst &infst, StdMutableFst *ofst,
                     Label special_label) const {
    *ofst = ComposeFst<StdArc>(
        infst, GetFst(),
        ComposeFstOptions<StdArc, NGPhiMatcher>(
            CacheOptions(), new NGPhiMatcher(infst, MATCH_NONE, kNoLabel),
            new NGPhiMatcher(GetFst(), MATCH_INPUT, special_label, 1,
                             MATCHER_REWRITE_NEVER)));
  }

  // Switch backoff label to special label for phi matcher
  // assumed to be order preserving (as it is with <epsilon> and -2)
  void MakePhiMatcherLM(Label special_label);

  // Apply n-gram model to fst.  For now, assumes linear fst, accumulates stats
  double ApplyNGramToFst(const fst::StdVectorFst &input_fst,
                         const Fst<StdArc> &symbolfst, bool phimatch,
                         bool verbose, Label special_label, Label OOV_label,
                         double OOV_cost, double *logprob, int *words,
                         int *oovs, int *words_skipped);

  // Adds a phi loop (rho) at unigram state for OOVs
  // OOV_class_size (N) and OOV_probability (p) determine weight of loop: p/N
  // Rest of unigrams renormalized accordingly, by 1-p
  void RenormUnigramForOOV(Label special_label, Label OOV_label,
                           double OOV_class_size, double OOV_probability);

  // Checks to see if a state or ngram is in context
  bool InContext(StateId st) const;
  bool InContext(const std::vector<Label> &ngram) const;

 private:
  // Convert to a new log base for printing (ARPA)
  double ShowLogNewBase(double neglogcost, double base) const {
    return -neglogcost / log(base);
  }

  // Print the header portion of the ARPA model format
  void ShowARPAHeader() const;

  // Print n-grams leaving a particular state for the ARPA model format
  void ShowARPANGrams(StdArc::StateId st, const std::string &str,
                      int order) const;

  // Print the N-gram model in ARPA format
  void ShowARPAModel() const;

  // Print n-grams leaving a particular state, standard output format
  void ShowNGrams(StdArc::StateId st, const std::string &str,
                  ShowBackoff showeps, bool neglogs, bool intcnts) const;

  void ShowStringFst(const Fst<StdArc> &infst) const;

  void RelabelAndSetSymbols(StdMutableFst *infst, const Fst<StdArc> &symbolfst);

  void ShowPhiPerplexity(const ComposeFst<StdArc> &cfst, bool verbose,
                         int special_label, Label OOV_label, double *logprob,
                         int *words, int *oovs, int *words_skipped) const;

  void ShowNonPhiPerplexity(const Fst<StdArc> &infst, bool verbose,
                            double OOV_cost, Label OOV_label, double *logprob,
                            int *words, int *oovs, int *words_skipped) const;

  void FindNextStateInModel(StateId *mst, Label label, double OOV_cost,
                            Label OOV_label, double *neglogprob, int *word_cnt,
                            int *oov_cnt, int *words_skipped,
                            std::string *history, bool verbose,
                            std::vector<Label> *ngram) const;

  // add symbol to n-gram history string
  void AppendWordToNGramHistory(std::string *str,
                                const std::string &symbol) const {
    if (!str->empty()) *str += ' ';
    *str += symbol;
  }

  // Calculate and show (if verbose) </s> n-gram, and accumulate stats
  void ApplyFinalCost(StateId mst, std::string history, int word_cnt,
                      int oov_cnt, int skipped, double neglogprob,
                      double *logprob, int *words, int *oovs,
                      int *words_skipped, bool verbose,
                      const std::vector<Label> &ngram) const;

  // Header for verbose n-gram entries
  void ShowNGramProbHeader() const {
    ostrm_ << "                                                ";
    ostrm_ << "ngram  -logprob\n";
    ostrm_ << "        N-gram probability                      ";
    ostrm_ << "found  (base10)\n";
  }

  // Show the verbose n-gram entries with history order and neglogprob
  void ShowNGramProb(std::string symbol, std::string history, bool oov,
                     int order, double ngram_cost) const;

  // Show summary perplexity numbers, similar to summary given by SRILM
  void ShowPerplexity(size_t sentences, int word_cnt, int oov_cnt,
                      int words_skipped, double logprob) const {
    ostrm_ << sentences << " sentences, ";
    ostrm_ << word_cnt << " words, ";
    ostrm_ << oov_cnt << " OOVs\n";
    if (words_skipped > 0) {
      ostrm_ << "NOTE: " << words_skipped << " OOVs with no probability"
             << " were skipped in perplexity calculation\n";
      word_cnt -= words_skipped;
    }
    ostrm_ << "logprob(base 10)= " << logprob;
    ostrm_ << ";  perplexity = ";
    ostrm_ << pow(10, -logprob / (word_cnt + sentences)) << "\n\n";
  }

  // Calculate prob of </s> and add to accum'd prob, and update total prob
  double SetInitRandProb(StateId hi_state, StateId st, double *r) const;

  // Show symbol during random string generation
  StateId ShowRandSymbol(Label lbl, bool *first_printed, bool show_backoff,
                         StateId st) const;

  // Find random symbol and show if necessary
  StateId GetAndShowSymbol(StateId st, double p, double r, StateId *hi_state,
                           bool *first_printed, bool show_backoff) const;

  // Produce and output random samples from model using rand/srand
  void RandNGramModel(int64 samples, bool show_backoff) const;

  // Checks parameterization of perplexity calculation and sets OOV_label
  bool GetOOVLabel(double *OOV_probability, std::string *OOV_symbol,
                   StdArc::Label *OOV_label);

 private:
  std::ostream &ostrm_;
  bool include_all_suffixes_;
  NGramContext context_;
};

}  // namespace ngram

#endif  // NGRAM_NGRAM_OUTPUT_H_
