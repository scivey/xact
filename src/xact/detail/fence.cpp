#include "xact/detail/fence.h"
#include "xact/detail/asm/util.h"

namespace xact { namespace detail {

void mFence() {
  xact_mfence();
}

}} // xact::detail

