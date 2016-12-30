#include "xact/generalized_cas/Operation.h"
#include "xact/generalized_cas/Precondition.h"
#include "xact/generalized_cas/GeneralizedCAS.h"
#include "xact_asm/core.h"
#include <glog/logging.h>
namespace xact { namespace generalized_cas {


GeneralizedCASCore& GeneralizedCAS::core() {
  return core_;
}

const GeneralizedCASCore& GeneralizedCAS::core() const {
  return core_;
}

bool GeneralizedCAS::execute() {
  auto result = xact_generalized_cas_op(
    core_.preconditions, core_.nPreconditions,
    core_.operations, core_.nOperations
  );
  LOG(INFO) << "result: " << ((uint64_t)result);
  return result == 0;
}

}} // xact::generalized_cas