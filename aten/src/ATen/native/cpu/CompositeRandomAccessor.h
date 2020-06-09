#pragma once

#include <ATen/native/CompositeRandomAccessorCommon.h>

namespace at { namespace native {

template <typename KeyAccessor, typename ValueAccessor>
using CompositeRandomAccessorCPU =
  CompositeRandomAccessor<KeyAccessor, ValueAccessor, std::tuple>;

template <typename Values, typename References>
void swap(
  references_holder<Values, References> rh1,
  references_holder<Values, References> rh2
) {
  return std::swap(rh1.data(), rh2.data());
}

template <int N, typename Values, typename References>
auto get(references_holder<Values, References> rh) -> decltype(std::get<N>(rh.data())) {
  return std::get<N>(rh.data());
}

}} // namespace at::native
