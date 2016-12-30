#include "xact/generalized_cas/ExceptionErrorPolicy.h"

namespace xact { namespace generalized_cas {

bool ExceptionErrorPolicy::onEmptyExecution() {
  throw std::runtime_error {"Attempted to execute an empty GeneralizedCAS."};
}

}} // xact::generalized_cas

