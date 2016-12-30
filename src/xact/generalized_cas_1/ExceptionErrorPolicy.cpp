#include "xact/generalized_cas_1/ExceptionErrorPolicy.h"

namespace xact { namespace generalized_cas_1 {

bool ExceptionErrorPolicy::onEmptyExecution() {
  throw std::runtime_error {"Attempted to execute an empty GeneralizedCAS."};
}

}} // xact::generalized_cas_1
