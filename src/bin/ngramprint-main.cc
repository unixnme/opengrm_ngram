
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
// Prints a given n-gram model to various kinds of textual formats.

#include <fstream>
#include <memory>
#include <ostream>
#include <string>

#include <ngram/ngram-output.h>

DECLARE_bool(ARPA);
DECLARE_bool(backoff);
DECLARE_bool(backoff_inline);
DECLARE_bool(negativelogs);
DECLARE_bool(integers);
DECLARE_int64(backoff_label);
DECLARE_bool(check_consistency);
DECLARE_string(context_pattern);
DECLARE_bool(include_all_suffixes);
DECLARE_string(symbols);

int ngramprint_main(int argc, char **argv) {
  std::string usage = "Print n-gram counts and models.\n\n  Usage: ";
  usage += argv[0];
  usage += " [--options] [in.fst [out.txt]]\n";
  std::set_new_handler(FailedNewHandler);
  SET_FLAGS(usage.c_str(), &argc, &argv, true);

  if (argc > 3) {
    ShowUsage();
    return 1;
  }

  std::string in_name =
      (argc > 1 && (strcmp(argv[1], "-") != 0)) ? argv[1] : "";
  std::string out_name =
      (argc > 2 && (strcmp(argv[2], "-") != 0)) ? argv[2] : "stdout";

  std::unique_ptr<fst::StdMutableFst> fst(
      fst::StdMutableFst::Read(in_name, true));
  if (!fst) return 1;

  if (!FLAGS_symbols.empty()) {
    std::unique_ptr<fst::SymbolTable> syms(
        fst::SymbolTable::ReadText(FLAGS_symbols));
    if (!syms) return 1;
    fst->SetInputSymbols(syms.get());
    fst->SetOutputSymbols(syms.get());
  }

  std::ofstream ofstrm;
  if (argc > 2 && (strcmp(argv[2], "-") != 0)) {
    ofstrm.open(argv[2]);
    if (!ofstrm) {
      LOG(ERROR) << argv[0] << ": Open failed, file = " << argv[2];
      return 1;
    }
  }
  std::ostream &ostrm = ofstrm.is_open() ? ofstrm : std::cout;

  ngram::NGramOutput ngram(fst.get(), ostrm, FLAGS_backoff_label,
                           FLAGS_check_consistency, FLAGS_context_pattern,
                           FLAGS_include_all_suffixes);

  // Parse --backoff and --backoff_inline flags, where --backoff takes precedent
  ngram::NGramOutput::ShowBackoff show_backoff =
      ngram::NGramOutput::ShowBackoff::NONE;
  if (FLAGS_backoff) {
    show_backoff = ngram::NGramOutput::ShowBackoff::EPSILON;
    if (FLAGS_backoff_inline)
      show_backoff = ngram::NGramOutput::ShowBackoff::INLINE;
  }

  ngram.ShowNGramModel(show_backoff, FLAGS_negativelogs, FLAGS_integers,
                       FLAGS_ARPA);
  return 0;
}
