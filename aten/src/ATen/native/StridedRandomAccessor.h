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
  typename index_t = int64_t,
  template <typename U> class PtrTraits = DefaultPtrTraits
>
class ConstStridedRandomAccessor {
public:
  using PtrType = typename PtrTraits<T>::PtrType;
  using RawPtrType = const T*;
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

  C10_HOST_DEVICE RawPtrType operator->() const {
    return reinterpret_cast<RawPtrType>(ptr);
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
  C10_HOST_DEVICE bool operator==(const ConstStridedRandomAccessor& other) const {
    return (ptr == other.ptr) && (stride == other.stride);
  }

  C10_HOST_DEVICE bool operator!=(const ConstStridedRandomAccessor& other) const {
    return !(*this == other);
  }

  C10_HOST_DEVICE bool operator<(const ConstStridedRandomAccessor& other) const {
    return ptr < other.ptr;
  }

  C10_HOST_DEVICE bool operator<=(const ConstStridedRandomAccessor& other) const {
    return (*this < other) || (*this == other);
  }

  C10_HOST_DEVICE bool operator>(const ConstStridedRandomAccessor& other) const {
    return !(*this <= other);
  }

  C10_HOST_DEVICE bool operator>=(const ConstStridedRandomAccessor& other) const {
    return !(*this < other);
  }
  // }

protected:
  PtrType ptr;
  index_t stride;
};

template <
  typename T,
  typename index_t = int64_t,
  template <typename U> class PtrTraits = DefaultPtrTraits
>
class StridedRandomAccessor 
  : public ConstStridedRandomAccessor<T, index_t, PtrTraits> {
public:
  using BaseType = ConstStridedRandomAccessor<T, index_t, PtrTraits>;
  using PtrType = typename PtrTraits<T>::PtrType;
  using RawPtrType = T*;
  using RefType = T&;

  // Constructors {
  C10_HOST_DEVICE StridedRandomAccessor(PtrType ptr, index_t stride)
    : BaseType(ptr, stride)
  {}

  C10_HOST_DEVICE explicit StridedRandomAccessor(PtrType ptr)
    : BaseType(ptr)
  {}

  C10_HOST_DEVICE StridedRandomAccessor()
    : BaseType()
  {}
  // }

  // Pointer-like operations {
  C10_HOST_DEVICE RefType operator*() const {
    return *this->ptr;
  }

  C10_HOST_DEVICE RawPtrType operator->() const {
    return reinterpret_cast<RawPtrType>(this->ptr);
  }

  C10_HOST_DEVICE RefType operator[](index_t idx) const {
    return this->ptr[idx * this->stride];
  }
  // }

  // Prefix/postfix increment/decrement {
  C10_HOST_DEVICE StridedRandomAccessor& operator++() {
    this->ptr += this->stride;
    return *this;
  }

  C10_HOST_DEVICE StridedRandomAccessor operator++(int) {
    PtrType copy_ptr = this->ptr;
    ++*this;
    return StridedRandomAccessor(copy_ptr, this->stride);
  }

  C10_HOST_DEVICE StridedRandomAccessor& operator--() {
    this->ptr -= this->stride;
    return *this;
  }

  C10_HOST_DEVICE StridedRandomAccessor operator--(int) {
    PtrType copy_ptr = this->ptr;
    --*this;
    return StridedRandomAccessor(copy_ptr, this->stride);
  }
  // }

  // Arithmetic operations {
  C10_HOST_DEVICE StridedRandomAccessor& operator+=(index_t offset) {
    this->ptr += offset * this->stride;
    return *this;
  }

  C10_HOST_DEVICE StridedRandomAccessor operator+(index_t offset) const {
    return StridedRandomAccessor(this->ptr + offset * this->stride, this->stride);
  }

  friend C10_HOST_DEVICE StridedRandomAccessor operator+(
    index_t offset,
    const StridedRandomAccessor& accessor
  ) {
    return accessor + offset;
  }

  C10_HOST_DEVICE StridedRandomAccessor& operator-=(index_t offset) {
    this->ptr -= offset * this->stride;
    return *this;
  }

  C10_HOST_DEVICE StridedRandomAccessor operator-(index_t offset) const {
    return StridedRandomAccessor(this->ptr - offset * this->stride, this->stride);
  }

  C10_HOST_DEVICE index_t operator-(const BaseType& other) const {
    return static_cast<const BaseType&>(*this) - other;
  }
  // }
};

// IndexedStridedRandomAccessor stores two accessors:
// one for values, the other for indicies.
// It is designed to be used with sorting-like operations,
// hence the accessors should not be constant.
// For CPU only.
template <
  typename T,
  typename index_t = int64_t,
  template <typename U> class PtrTraits = DefaultPtrTraits
>
class IndexedStridedRandomAccessor
  : public StridedRandomAccessor<T, index_t, PtrTraits> {
public:
  using ValueStridedAccessor = StridedRandomAccessor<T, index_t, PtrTraits>;
  using IndexStridedAccessor = StridedRandomAccessor<index_t, index_t, PtrTraits>;
  using ValuePtrType = typename ValueStridedAccessor::PtrType;
  using IndexPtrType = typename IndexStridedAccessor::PtrType;
  using RefType = std::pair<T&, index_t&>;

  // Constructors {
  IndexedStridedRandomAccessor(
    const ValueStridedAccessor& vsa, const IndexStridedAccessor& isa
  ) : ValueStridedAccessor(vsa), isa{isa}
  {}

  IndexedStridedRandomAccessor()
    : ValueStridedAccessor()
  {}
  // }

  // Pointer-like operations {
  RefType operator*() const {
    return RefType(*getBase(), *isa);
  }

  RefType operator[](index_t idx) const {
    return RefType(getBase()[idx], isa[idx]);
  }
  // }

  // Prefix/postfix increment/decrement {
  IndexedStridedRandomAccessor& operator++() {
    ++getBase();
    ++isa;
    return *this;
  }

  IndexedStridedRandomAccessor operator++(int) {
    auto copy = IndexedStridedRandomAccessor(*this);
    ++*this;
    return copy;
  }

  IndexedStridedRandomAccessor& operator--() {
    --getBase();
    --isa;
    return *this;
  }

  IndexedStridedRandomAccessor operator--(int) {
    auto copy = IndexedStridedRandomAccessor(*this);
    --*this;
    return copy;
  }
  // }
  
private:
  inline ValueStridedAccessor& getBase() {
    return static_cast<ValueStridedAccessor&>(*this);
  }

protected:
  IndexStridedAccessor isa;
};

}} // namespace at::native
