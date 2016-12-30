#pragma once
#include <stdexcept>

namespace xact { namespace generalized_cas_1 {

class ExceptionErrorPolicy {
 public:
  static bool onEmptyExecution();
};

}} // xact::generalized_cas_1
