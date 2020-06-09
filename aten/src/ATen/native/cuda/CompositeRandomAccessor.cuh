#pragma once

#include <ATen/native/CompositeRandomAccessorCommon.h>

namespace at { namespace native {

template <typename KeyAccessor, typename ValueAccessor>
using CompositeRandomAccessorCUDA =
  CompositeRandomAccessor<KeyAccessor, ValueAccessor, thrust::tuple>;

template <typename Values, typename References>
void swap(
  references_holder<Values, References> rh1,
  references_holder<Values, References> rh2
) {
  return thrust::swap(rh1, rh2);
}

template <int N, typename Values, typename References>
auto get(references_holder<Values, References> rh) -> decltype(thrust::get<N>(rh.data())) {
  return thrust::get<N>(rh.data());
}

}} // namespace at::native
