#include <string>
#include <functional>
#include <thread>
#include <memory>
#include <vector>
#include <atomic>
#include <glog/logging.h>
#include "xact/TransactionExecutor.h"
#include "xact/multi/MultiTransaction.h"
#include "xact/detail/asm/util.h"
#include "xact/detail/macros.h"
#include "xact/LockableAtomicU64.h"
#include "xact/TransactionStatus.h"

using namespace std;
using namespace xact;
using namespace xact::detail;
using xact::multi::MultiTransaction;

using atom_t = xact_lockable_atomic_u64_t;

static atom_t* getPointer(LockableAtomicU64& atom) {
  return (atom_t*) LockableAtomicU64Inspector(atom).getPointer();
}
using gencas_exec_t = TransactionExecutor<DefaultTransactionRetryPolicy>;

using AU64 = xact::LockableAtomicU64;

void prof1() {
  for (size_t i = 0; i < 10000; i++) {
    AU64 x{0}, y{0};
    uint64_t xVal {0}, yVal {0};
    SmallVector<pair<atom_t*, uint64_t*>> args {
      {getPointer(x), &xVal},
      {getPointer(y), &yVal}
    };
    auto multiOp = MultiTransaction::load(args);
  }
}

void prof2() {
  AU64 x{0}, y{0};
  uint64_t xVal {0}, yVal {0};
  SmallVector<pair<atom_t*, uint64_t*>> args {
    {getPointer(x), &xVal},
    {getPointer(y), &yVal}
  };
  for (size_t i = 0; i < 10000; i++) {
    auto multiOp = MultiTransaction::load(args);
    multiOp.execute();
  }
}

int main() {
  prof1();
  prof2();
  LOG(INFO) << "done";
}

