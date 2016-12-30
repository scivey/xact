#pragma once
#include <stdexcept>

namespace xact { namespace generalized_cas {

class ExceptionErrorPolicy {
 public:
  static bool onEmptyExecution();
};

}} // xact::generalized_cas
