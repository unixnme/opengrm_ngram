
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
// Applies smoothed marginalization constraints to given model.

#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <ngram/ngram-marginalize.h>
#include <ngram/util.h>

DECLARE_int64(backoff_label);
DECLARE_int32(max_bo_updates);
DECLARE_double(norm_eps);
DECLARE_bool(check_consistency);

int ngrammarginalize_main(int argc, char **argv) {
  std::string usage =
      "Marginalize n-gram model from input model file.\n\n  Usage: ";
  usage += argv[0];
  usage += " [--options] [in.fst [out.fst]]\n";
  std::set_new_handler(FailedNewHandler);
  SET_FLAGS(usage.c_str(), &argc, &argv, true);

  if (argc > 3) {
    ShowUsage();
    return 1;
  }

  std::string in_name =
      (argc > 1 && (strcmp(argv[1], "-") != 0)) ? argv[1] : "";
  std::string out_name = argc > 2 ? argv[2] : "";

  std::unique_ptr<fst::StdVectorFst> fst(
      fst::StdVectorFst::Read(in_name));
  if (!fst) return 1;

  ngram::NGramMarginal ngramarg(fst.get(), FLAGS_backoff_label, FLAGS_norm_eps,
                                FLAGS_max_bo_updates, FLAGS_check_consistency);

  ngramarg.MarginalizeNGramModel();
  if (ngramarg.Error()) return 1;
  ngramarg.GetFst().Write(out_name);
  return 0;
}
