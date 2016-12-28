#include "xact/fence.h"
#include "xact_asm/core.h"

namespace xact {

void mFence() {
  xact_mfence();
}

} // xact
