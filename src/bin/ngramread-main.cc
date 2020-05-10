
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
// Reads textual model representations and produces n-gram model FST.

#include <fstream>
#include <iostream>
#include <string>

#include <ngram/ngram-input.h>

DECLARE_bool(ARPA);
DECLARE_bool(renormalize_arpa);
DECLARE_string(symbols);
DECLARE_string(epsilon_symbol);
DECLARE_string(OOV_symbol);
DECLARE_string(start_symbol);  // defined in ngram-output.cc
DECLARE_string(end_symbol);    // defined in ngram-output.cc

int ngramread_main(int argc, char **argv) {
  std::string usage = "Transform text formats to FST.\n\n  Usage: ";
  usage += argv[0];
  usage += " [--options] [in.txt [out.fst]]\n";
  std::set_new_handler(FailedNewHandler);
  SET_FLAGS(usage.c_str(), &argc, &argv, true);

  if (argc > 3) {
    ShowUsage();
    return 1;
  }

  std::ifstream ifstrm;
  if (argc > 1 && (strcmp(argv[1], "-") != 0)) {
    ifstrm.open(argv[1]);
    if (!ifstrm) {
      LOG(ERROR) << argv[0] << ": Open failed: " << argv[1];
      return 1;
    }
  }
  std::istream &istrm = ifstrm.is_open() ? ifstrm : std::cin;

  std::ofstream ofstrm;
  if (argc > 2 && (strcmp(argv[2], "-") != 0)) {
    ofstrm.open(argv[2]);
    if (!ofstrm) {
      LOG(ERROR) << argv[0] << ": Open failed: " << argv[2];
      return 1;
    }
  }
  std::ostream &ostrm = ofstrm.is_open() ? ofstrm : std::cout;

  ngram::NGramInput input(istrm, ostrm, FLAGS_symbols, FLAGS_epsilon_symbol,
                          FLAGS_OOV_symbol, FLAGS_start_symbol,
                          FLAGS_end_symbol);
  return !input.ReadInput(FLAGS_ARPA, /*symbols=*/false, /*output=*/true,
                          FLAGS_renormalize_arpa);
}
