
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
// Calculates perplexity of an input FST archive using the given model.

#include <fstream>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include <fst/flags.h>
#include <fst/extensions/far/far.h>
#include <ngram/ngram-output.h>

DECLARE_bool(use_phimatcher);
DECLARE_string(OOV_symbol);
DECLARE_double(OOV_class_size);
DECLARE_double(OOV_probability);
DECLARE_string(context_pattern);

int ngramperplexity_main(int argc, char **argv) {
  std::string usage = "Apply n-gram model to input FST archive.\n\n  Usage: ";
  usage += argv[0];
  usage += " [--options] ngram.fst [in.far [out.txt]]\n";
  std::set_new_handler(FailedNewHandler);
  SET_FLAGS(usage.c_str(), &argc, &argv, true);

  if (argc < 2 || argc > 4) {
    ShowUsage();
    return 1;
  }

  fst::FstReadOptions opts;
  std::string in1_name = strcmp(argv[1], "-") != 0 ? argv[1] : "";
  std::string in2_name =
      (argc > 2 && (strcmp(argv[2], "-") != 0)) ? argv[2] : "";
  std::string out_name =
      (argc > 3 && (strcmp(argv[3], "-") != 0)) ? argv[3] : "";

  std::unique_ptr<fst::StdMutableFst> fst(
      fst::StdMutableFst::Read(in1_name, true));
  if (!fst) return 1;

  std::ofstream ofstrm;
  if (argc > 3 && (strcmp(argv[3], "-") != 0)) {
    ofstrm.open(argv[3]);
    if (!ofstrm) {
      LOG(ERROR) << argv[0] << ": Open failed, file = " << argv[3];
      return 1;
    }
  }
  std::ostream &ostrm = ofstrm.is_open() ? ofstrm : std::cout;

  ngram::NGramOutput ngram(fst.get(), ostrm, 0, false, FLAGS_context_pattern);

  if (in2_name.empty()) {
    if (in1_name.empty()) {
      LOG(ERROR) << argv[0] << ": Can't use standard i/o for both inputs.";
      return 1;
    }
  }
  std::unique_ptr<fst::FarReader<fst::StdArc>> far_reader(
      fst::FarReader<fst::StdArc>::Open(in2_name));
  if (!far_reader) {
    LOG(ERROR) << "unable to open fst archive " << in2_name;
    return 1;
  }

  std::vector<std::unique_ptr<fst::StdVectorFst>> infsts;
  while (!far_reader->Done()) {
    infsts.push_back(std::unique_ptr<fst::StdVectorFst>(
        new fst::StdVectorFst(*far_reader->GetFst())));
    far_reader->Next();
  }

  return !ngram.PerplexityNGramModel(
      infsts, FLAGS_v, FLAGS_use_phimatcher, &FLAGS_OOV_symbol,
      FLAGS_OOV_class_size, FLAGS_OOV_probability);
}
