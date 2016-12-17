#include <glog/logging.h>

int main() {
  google::InstallFailureSignalHandler();
  LOG(INFO) << "scratch";
}
