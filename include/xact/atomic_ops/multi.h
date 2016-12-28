#pragma once
#include <cstdint>
#include <array>
#include <limits>
#include "xact/util/util.h"

namespace xact { namespace atomic_ops {

bool casU64M(uint64_t **atomTargets, const uint64_t *expecteds,
    const uint64_t *desireds, uint8_t n);
bool storeU64M(uint64_t **atomTargets, const uint64_t *sources, uint8_t n);
bool loadU64M(uint64_t **atomTargets, uint64_t *dests, uint8_t n);
bool addU64M(uint64_t **atomTargets, uint64_t amount, uint8_t n);
bool fetchAddU64M(uint64_t **atomTargets, uint64_t *results, uint64_t amount, uint8_t n);
bool subU64M(uint64_t **atomTargets, uint64_t amount, uint8_t n);
bool fetchSubU64M(uint64_t **atomTargets, uint64_t *results, uint64_t amount, uint8_t n);


template<size_t N>
bool addU64M(std::array<uint64_t*, N>& atomTargets, uint64_t addBy) {
  return addU64M(atomTargets.data(), addBy, util::safeCastSizeT<N, uint8_t>());
}

template<size_t N>
bool fetchAddU64M(std::array<uint64_t*, N>& atomTargets,
      std::array<uint64_t, N>& results, uint64_t addBy) {
  return fetchAddU64M(atomTargets.data(), results.data(), addBy, util::safeCastSizeT<N, uint8_t>());
}

template<size_t N>
bool subU64M(std::array<uint64_t*, N>& atomTargets, uint64_t subBy) {
  return subU64M(atomTargets.data(), subBy, util::safeCastSizeT<N, uint8_t>());
}

template<size_t N>
bool fetchSubU64M(std::array<uint64_t*, N>& atomTargets,
      std::array<uint64_t, N>& results, uint64_t subBy) {
  return fetchSubU64M(atomTargets.data(), results.data(), subBy, util::safeCastSizeT<N, uint8_t>());
}

template<size_t N>
bool casU64M(std::array<uint64_t*, N>& atomTargets,
    const std::array<uint64_t, N>& expecteds,
    const std::array<uint64_t, N>& desireds) {
  return casU64M(atomTargets.data(), expecteds.data(),
    desireds.data(), util::safeCastSizeT<N, uint8_t>()
  );
}

template<size_t N>
bool storeU64M(std::array<uint64_t*, N>& atomTargets, const std::array<uint64_t, N>& sources) {
  return storeU64M(atomTargets.data(), sources.data(), util::safeCastSizeT<N, uint8_t>());
}

template<size_t N>
bool loadU64M(std::array<uint64_t*, N>& atomTargets, std::array<uint64_t, N>& dests) {
  return loadU64M(atomTargets.data(), dests.data(), util::safeCastSizeT<N, uint8_t>());
}


}} // xact::atomic_ops
