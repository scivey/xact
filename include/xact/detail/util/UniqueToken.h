#pragma once

/*
  [copied over from score]
  UniqueToken is a standin for a move-only resource like
  std::unique_ptr.

  It's used where a class has a transferable (but non-copyable)
  responsibility or right to perform some action, and that action
  doesn't involve another move-only resource.
  See e.g. ScopeGuard, TimeLogger.
*/

namespace xact { namespace detail { namespace util {

class UniqueToken {
 protected:
  bool value_ {false};
  UniqueToken(const UniqueToken&) = delete;
  UniqueToken& operator=(const UniqueToken&) = delete;
  UniqueToken(bool value);
 public:
  UniqueToken();
  void mark() noexcept;
  void clear() noexcept;
  UniqueToken(UniqueToken&& other);
  UniqueToken& operator=(UniqueToken&& other);
  bool good() const noexcept;
  operator bool() const noexcept;
};

}}} // xact::detail::util


