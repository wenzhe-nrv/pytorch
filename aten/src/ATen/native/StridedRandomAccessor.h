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
  using difference_type = index_t;
  using value_type = const T;
  using pointer = const typename PtrTraits<T>::PtrType;
  using reference = const value_type&;
  using iterator_category = std::random_access_iterator_tag;

  using PtrType = typename PtrTraits<T>::PtrType;

  // Constructors {
  C10_HOST_DEVICE
  ConstStridedRandomAccessor(PtrType ptr, difference_type stride)
    : ptr{ptr}, stride{stride}
  {}

  C10_HOST_DEVICE
  explicit ConstStridedRandomAccessor(PtrType ptr)
    : ptr{ptr}, stride{static_cast<difference_type>(1)}
  {}

  C10_HOST_DEVICE
  ConstStridedRandomAccessor()
    : ptr{nullptr}, stride{static_cast<difference_type>(1)}
  {}
  // }

  // Pointer-like operations {
  C10_HOST_DEVICE
  reference operator*() const {
    return *ptr;
  }

  C10_HOST_DEVICE
  const value_type* operator->() const {
    return reinterpret_cast<const value_type*>(ptr);
  }

  C10_HOST_DEVICE
  reference operator[](difference_type idx) const {
    return ptr[idx * stride];
  }
  // }

  // Prefix/postfix increment/decrement {
  C10_HOST_DEVICE
  ConstStridedRandomAccessor& operator++() {
    ptr += stride;
    return *this;
  }

  C10_HOST_DEVICE
  ConstStridedRandomAccessor operator++(int) {
    auto copy = *this;
    ++*this;
    return copy;
  }

  C10_HOST_DEVICE
  ConstStridedRandomAccessor& operator--() {
    ptr -= stride;
    return *this;
  }

  C10_HOST_DEVICE
  ConstStridedRandomAccessor operator--(int) {
    auto copy = *this;
    --*this;
    return copy;
  }
  // }

  // Arithmetic operations {
  C10_HOST_DEVICE
  ConstStridedRandomAccessor& operator+=(difference_type offset) {
    ptr += offset * stride;
    return *this;
  }

  C10_HOST_DEVICE
  ConstStridedRandomAccessor operator+(difference_type offset) const {
    return ConstStridedRandomAccessor(ptr + offset * stride, stride);
  }

  C10_HOST_DEVICE
  friend ConstStridedRandomAccessor operator+(
    difference_type offset,
    const ConstStridedRandomAccessor& accessor
  ) {
    return accessor + offset;
  }

  C10_HOST_DEVICE
  ConstStridedRandomAccessor& operator-=(difference_type offset) {
    ptr -= offset * stride;
    return *this;
  }

  C10_HOST_DEVICE
  ConstStridedRandomAccessor operator-(difference_type offset) const {
    return ConstStridedRandomAccessor(ptr - offset * stride, stride);
  }

  C10_HOST_DEVICE
  difference_type operator-(const ConstStridedRandomAccessor& other) const {
    return ptr - other.ptr;
  }
  // }
  
  // Comparison operators {
  C10_HOST_DEVICE
  bool operator==(const ConstStridedRandomAccessor& other) const {
    return (ptr == other.ptr) && (stride == other.stride);
  }

  C10_HOST_DEVICE
  bool operator!=(const ConstStridedRandomAccessor& other) const {
    return !(*this == other);
  }

  C10_HOST_DEVICE
  bool operator<(const ConstStridedRandomAccessor& other) const {
    return ptr < other.ptr;
  }

  C10_HOST_DEVICE
  bool operator<=(const ConstStridedRandomAccessor& other) const {
    return (*this < other) || (*this == other);
  }

  C10_HOST_DEVICE
  bool operator>(const ConstStridedRandomAccessor& other) const {
    return !(*this <= other);
  }

  C10_HOST_DEVICE
  bool operator>=(const ConstStridedRandomAccessor& other) const {
    return !(*this < other);
  }
  // }

protected:
  PtrType ptr;
  difference_type stride;
};

template <
  typename T,
  typename index_t = int64_t,
  template <typename U> class PtrTraits = DefaultPtrTraits
>
class StridedRandomAccessor 
  : public ConstStridedRandomAccessor<T, index_t, PtrTraits> {
public:
  using difference_type = index_t;
  using value_type = T;
  using pointer = typename PtrTraits<T>::PtrType;
  using reference = value_type&;

  using BaseType = ConstStridedRandomAccessor<T, index_t, PtrTraits>;
  using PtrType = typename PtrTraits<T>::PtrType;

  // Constructors {
  C10_HOST_DEVICE
  StridedRandomAccessor(PtrType ptr, difference_type stride)
    : BaseType(ptr, stride)
  {}

  C10_HOST_DEVICE
  explicit StridedRandomAccessor(PtrType ptr)
    : BaseType(ptr)
  {}

  C10_HOST_DEVICE
  StridedRandomAccessor()
    : BaseType()
  {}
  // }

  // Pointer-like operations {
  C10_HOST_DEVICE
  reference operator*() const {
    return *this->ptr;
  }

  C10_HOST_DEVICE
  value_type* operator->() const {
    return reinterpret_cast<value_type*>(this->ptr);
  }

  C10_HOST_DEVICE
  reference operator[](difference_type idx) const {
    return this->ptr[idx * this->stride];
  }
  // }

  // Prefix/postfix increment/decrement {
  C10_HOST_DEVICE
  StridedRandomAccessor& operator++() {
    this->ptr += this->stride;
    return *this;
  }

  C10_HOST_DEVICE
  StridedRandomAccessor operator++(int) {
    auto copy = *this;
    ++*this;
    return copy;
  }

  C10_HOST_DEVICE
  StridedRandomAccessor& operator--() {
    this->ptr -= this->stride;
    return *this;
  }

  C10_HOST_DEVICE
  StridedRandomAccessor operator--(int) {
    auto copy = *this;
    --*this;
    return copy;
  }
  // }

  // Arithmetic operations {
  C10_HOST_DEVICE
  StridedRandomAccessor& operator+=(difference_type offset) {
    this->ptr += offset * this->stride;
    return *this;
  }

  C10_HOST_DEVICE
  StridedRandomAccessor operator+(difference_type offset) const {
    return StridedRandomAccessor(this->ptr + offset * this->stride, this->stride);
  }

  C10_HOST_DEVICE
  friend StridedRandomAccessor operator+(
    difference_type offset,
    const StridedRandomAccessor& accessor
  ) {
    return accessor + offset;
  }

  C10_HOST_DEVICE
  StridedRandomAccessor& operator-=(difference_type offset) {
    this->ptr -= offset * this->stride;
    return *this;
  }

  C10_HOST_DEVICE
  StridedRandomAccessor operator-(difference_type offset) const {
    return StridedRandomAccessor(this->ptr - offset * this->stride, this->stride);
  }

  C10_HOST_DEVICE
  difference_type operator-(const BaseType& other) const {
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
  using difference_type = index_t;
  using value_type = std::pair<T, index_t>;
  using pointer = typename PtrTraits<T>::PtrType;
  using reference = std::pair<T&, index_t&>;

  using ValueStridedAccessor = StridedRandomAccessor<T, index_t, PtrTraits>;
  using IndexStridedAccessor = StridedRandomAccessor<index_t, index_t, PtrTraits>;
  using ValuePtrType = typename ValueStridedAccessor::PtrType;
  using IndexPtrType = typename IndexStridedAccessor::PtrType;

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
  reference operator*() const {
    return reference(*getBase(), *isa);
  }

  reference operator[](difference_type idx) const {
    return reference(getBase()[idx], isa[idx]);
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

  // Arithmetic operations {
  IndexedStridedRandomAccessor& operator+=(difference_type offset) {
    getBase() += offset;
    isa += offset;
    return *this;
  }

  IndexedStridedRandomAccessor operator+(difference_type offset) const {
    return IndexedStridedRandomAccessor(getBase() + offset, isa + offset);
  }

  friend IndexedStridedRandomAccessor operator+(
    difference_type offset,
    const IndexedStridedRandomAccessor& accessor
  ) {
    return accessor + offset;
  }

  IndexedStridedRandomAccessor& operator-=(difference_type offset) {
    getBase() -= offset;
    isa -= offset;
    return *this;
  }

  IndexedStridedRandomAccessor operator-(difference_type offset) const {
    return IndexedStridedRandomAccessor(getBase() - offset, isa - offset);
  }

  difference_type operator-(const ValueStridedAccessor& other) const {
    return getBase() - other;
  }
  // }
  
protected:
  inline ValueStridedAccessor& getBase() {
    return static_cast<ValueStridedAccessor&>(*this);
  }

  inline const ValueStridedAccessor& getBase() const {
    return static_cast<const ValueStridedAccessor&>(*this);
  }

protected:
  IndexStridedAccessor isa;
};

}} // namespace at::native
