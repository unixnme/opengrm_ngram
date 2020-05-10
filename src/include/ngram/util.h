
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
#ifndef NGRAM_UTIL_H_
#define NGRAM_UTIL_H_

#include <fst/flags.h>
#include <fst/log.h>

// UTILITY FOR ERROR HANDLING

DECLARE_bool(ngram_error_fatal);

#define NGRAMERROR() \
  (FLAGS_ngram_error_fatal ? LOG(FATAL) : LOG(ERROR))

#endif  // NGRAM_UTIL_H_
