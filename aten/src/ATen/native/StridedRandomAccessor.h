#pragma once

#include <c10/macros/Macros.h>
#include <c10/util/Exception.h>

namespace at { namespace native {

template <typename T>
struct DefaultPtrTraits {
  using PtrType = T*;
};

#if (defined(_WIN32) || defined(_WIN64))
#define RESTRICT __restrict
#else
#define RESTRICT __restrict__
#endif

template <typename T>
struct RestrictPtrTraits {
  using PtrType = T* RESTRICT;
};

template <
  typename T,
  template <typename U> class PtrTraits = DefaultPtrTraits,
  typename index_t = int64_t
>
class ConstStridedRandomAccessor {
public:
  using PtrType = const typename PtrTraits<T>::PtrType;
  using RefType = const T&;

  // Constructors {
  C10_HOST_DEVICE ConstStridedRandomAccessor(PtrType ptr, index_t stride)
    : ptr{ptr}, stride{stride}
  {}

  C10_HOST_DEVICE explicit ConstStridedRandomAccessor(PtrType ptr)
    : ptr{ptr}, stride{static_cast<index_t>(1)}
  {}

  C10_HOST_DEVICE ConstStridedRandomAccessor()
    : ptr{nullptr}, stride{static_cast<index_t>(1)}
  {}
  // }

  // Pointer-like operations {
  C10_HOST_DEVICE RefType operator*() const {
    return *ptr;
  }

  C10_HOST_DEVICE const T* operator->() const {
    return reinterpret_cast<const T*>(ptr);
  }

  C10_HOST_DEVICE RefType operator[](index_t idx) const {
    return ptr[idx * stride];
  }
  // }

  // Prefix/postfix increment/decrement {
  C10_HOST_DEVICE ConstStridedRandomAccessor& operator++() {
    ptr += stride;
    return *this;
  }

  C10_HOST_DEVICE ConstStridedRandomAccessor operator++(int) {
    PtrType copy_ptr = ptr;
    ++*this;
    return ConstStridedRandomAccessor(copy_ptr, stride);
  }

  C10_HOST_DEVICE ConstStridedRandomAccessor& operator--() {
    ptr -= stride;
    return *this;
  }

  C10_HOST_DEVICE ConstStridedRandomAccessor operator--(int) {
    PtrType copy_ptr = ptr;
    --*this;
    return ConstStridedRandomAccessor(copy_ptr, stride);
  }
  // }

  // Arithmetic operations {
  C10_HOST_DEVICE ConstStridedRandomAccessor& operator+=(index_t offset) {
    ptr += offset * stride;
    return *this;
  }

  C10_HOST_DEVICE ConstStridedRandomAccessor operator+(index_t offset) const {
    return ConstStridedRandomAccessor(ptr + offset * stride, stride);
  }

  friend C10_HOST_DEVICE ConstStridedRandomAccessor operator+(
    index_t offset,
    const ConstStridedRandomAccessor& accessor
  ) {
    return accessor + offset;
  }

  C10_HOST_DEVICE ConstStridedRandomAccessor& operator-=(index_t offset) {
    ptr -= offset * stride;
    return *this;
  }

  C10_HOST_DEVICE ConstStridedRandomAccessor operator-(index_t offset) const {
    return ConstStridedRandomAccessor(ptr - offset * stride, stride);
  }

  C10_HOST_DEVICE index_t operator-(const ConstStridedRandomAccessor& other) const {
    return ptr - other.ptr;
  }
  // }
  
  // Comparison operators {
  bool operator==(const ConstStridedRandomAccessor& other) const {
    return (ptr == other.ptr) && (stride == other.stride);
  }

  bool operator!=(const ConstStridedRandomAccessor& other) const {
    return !(*this == other);
  }

  bool operator<(const ConstStridedRandomAccessor& other) const {
    return ptr < other.ptr;
  }

  bool operator<=(const ConstStridedRandomAccessor& other) const {
    return (*this < other) || (*this == other);
  }

  bool operator>(const ConstStridedRandomAccessor& other) const {
    return !(*this <= other);
  }

  bool operator>=(const ConstStridedRandomAccessor& other) const {
    return !(*this < other);
  }
  // }

protected:
  PtrType ptr;
  index_t stride;
};

}} // namespace at::native
