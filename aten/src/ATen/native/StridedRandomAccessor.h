#pragma once

#include <c10/macros/Macros.h>
#include <c10/util/Exception.h>

namespace at { namespace native {

template <typename T>
struct IndexTraits {
  using index_type = typename T::index_type;
};

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
  using difference_type = ptrdiff_t;
  using value_type = const T;
  using pointer = const typename PtrTraits<T>::PtrType;
  using reference = const value_type&;
  using iterator_category = std::random_access_iterator_tag;

  using PtrType = typename PtrTraits<T>::PtrType;
  using index_type = index_t;

  // Constructors {
  C10_HOST_DEVICE
  ConstStridedRandomAccessor(PtrType ptr, index_t stride)
    : ptr{ptr}, stride{stride}
  {}

  C10_HOST_DEVICE
  explicit ConstStridedRandomAccessor(PtrType ptr)
    : ptr{ptr}, stride{static_cast<index_t>(1)}
  {}

  C10_HOST_DEVICE
  ConstStridedRandomAccessor()
    : ptr{nullptr}, stride{static_cast<index_t>(1)}
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
  reference operator[](index_t idx) const {
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
    ConstStridedRandomAccessor copy(*this);
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
    ConstStridedRandomAccessor copy(*this);
    --*this;
    return copy;
  }
  // }

  // Arithmetic operations {
  C10_HOST_DEVICE
  ConstStridedRandomAccessor& operator+=(index_t offset) {
    ptr += offset * stride;
    return *this;
  }

  C10_HOST_DEVICE
  ConstStridedRandomAccessor operator+(index_t offset) const {
    return ConstStridedRandomAccessor(ptr + offset * stride, stride);
  }

  C10_HOST_DEVICE
  friend ConstStridedRandomAccessor operator+(
    index_t offset,
    const ConstStridedRandomAccessor& accessor
  ) {
    return accessor + offset;
  }

  C10_HOST_DEVICE
  ConstStridedRandomAccessor& operator-=(index_t offset) {
    ptr -= offset * stride;
    return *this;
  }

  C10_HOST_DEVICE
  ConstStridedRandomAccessor operator-(index_t offset) const {
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
  using difference_type = ptrdiff_t;
  using value_type = T;
  using pointer = typename PtrTraits<T>::PtrType;
  using reference = value_type&;

  using BaseType = ConstStridedRandomAccessor<T, index_t, PtrTraits>;
  using PtrType = typename PtrTraits<T>::PtrType;

  // Constructors {
  C10_HOST_DEVICE
  StridedRandomAccessor(PtrType ptr, index_t stride)
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
  reference operator[](index_t idx) const {
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
    StridedRandomAccessor copy(*this);
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
    StridedRandomAccessor copy(*this);
    --*this;
    return copy;
  }
  // }

  // Arithmetic operations {
  C10_HOST_DEVICE
  StridedRandomAccessor& operator+=(index_t offset) {
    this->ptr += offset * this->stride;
    return *this;
  }

  C10_HOST_DEVICE
  StridedRandomAccessor operator+(index_t offset) const {
    return StridedRandomAccessor(this->ptr + offset * this->stride, this->stride);
  }

  C10_HOST_DEVICE
  friend StridedRandomAccessor operator+(
    index_t offset,
    const StridedRandomAccessor& accessor
  ) {
    return accessor + offset;
  }

  C10_HOST_DEVICE
  StridedRandomAccessor& operator-=(index_t offset) {
    this->ptr -= offset * this->stride;
    return *this;
  }

  C10_HOST_DEVICE
  StridedRandomAccessor operator-(index_t offset) const {
    return StridedRandomAccessor(this->ptr - offset * this->stride, this->stride);
  }

  C10_HOST_DEVICE
  difference_type operator-(const BaseType& other) const {
    return static_cast<const BaseType&>(*this) - other;
  }
  // }
};

template <typename ...Ts>
struct tuple_holder {
  tuple_holder(std::tuple<Ts...> data)
    : data{data}
  {}

  operator std::tuple<Ts...>() {
    return data;
  }

  std::tuple<Ts...>& as_tuple() {
    return data;
  }

  std::tuple<Ts...> data;
};

template <typename ...Ts>
void swap(tuple_holder<Ts...> th1, tuple_holder<Ts...> th2) {
  return std::swap(th1.data, th2.data);
}

template<int N, typename ...Ts>
auto get(tuple_holder<Ts...> th)->decltype(std::get<N>(th.data)){
  return std::get<N>(th.data);
}

template <typename Accessor>
class reference_proxy {
public:
  using reference = typename std::iterator_traits<Accessor>::reference;
  using value_type = typename std::iterator_traits<Accessor>::value_type;

  reference_proxy(Accessor const& accessor)
    : accessor(accessor)
  {}

  operator reference() {
    return *accessor;
  }

  reference operator*() {
    return *accessor;
  }

  reference_proxy& operator=(value_type const& val) {
    *accessor = val;
    return *this;
  }

private:
  Accessor accessor;
};

template <typename ValueAccessor, typename IndexAccessor>
class IndexedRandomAccessor {
public:
  using value_type = std::tuple<
    typename std::iterator_traits<ValueAccessor>::value_type,
    typename std::iterator_traits<IndexAccessor>::value_type>;
  using reference = std::tuple<
    typename std::iterator_traits<ValueAccessor>::value_type&,
    typename std::iterator_traits<IndexAccessor>::value_type&>;
  using pointer = typename std::iterator_traits<ValueAccessor>::pointer;
  using difference_type = typename std::iterator_traits<ValueAccessor>::difference_type;
  using iterator_category = std::random_access_iterator_tag;

  using self_type = IndexedRandomAccessor<ValueAccessor, IndexAccessor>;
  using ref_proxy = reference_proxy<self_type>;

  using ValuePtrType = typename std::iterator_traits<ValueAccessor>::pointer;
  using IndexPtrType = typename std::iterator_traits<IndexAccessor>::pointer;
  using index_t = typename IndexTraits<ValueAccessor>::index_type;

  IndexedRandomAccessor(ValueAccessor va, IndexAccessor ia)
    : va(va), ia(ia)
  {}

  IndexedRandomAccessor(
    ValuePtrType vptr, index_t vstride,
    IndexPtrType iptr, index_t istride)
    : va(vptr, vstride), ia(iptr, istride)
  {}

  // Pointer-like operations {
  reference operator*() {
    return std::tie(*va, *ia);
  }

  auto* operator->() const {
    return va.operator->();
  }

  reference operator[](index_t idx) {
    return ref_proxy(IndexedRandomAccessor(va + idx, ia + idx));
  }
  // }

  // Prefix/postfix increment/decrement {
  IndexedRandomAccessor& operator++() {
    ++va;
    ++ia;
    return *this;
  }

  IndexedRandomAccessor operator++(int) {
    IndexedRandomAccessor copy(*this);
    ++*this;
    return copy;
  }

  IndexedRandomAccessor& operator--() {
    --va;
    --ia;
    return *this;
  }

  IndexedRandomAccessor operator--(int) {
    IndexedRandomAccessor copy(*this);
    --*this;
    return copy;
  }
  // }

  // Arithmetic operations {
  IndexedRandomAccessor& operator+=(index_t offset) {
    va += offset;
    ia += offset;
    return *this;
  }

  IndexedRandomAccessor operator+(index_t offset) const {
    return IndexedRandomAccessor(va + offset, ia + offset);
  }

  friend IndexedRandomAccessor operator+(
    index_t offset,
    const IndexedRandomAccessor& accessor
  ) {
    return accessor + offset;
  }

  IndexedRandomAccessor& operator-=(index_t offset) {
    va -= offset;
    ia -= offset;
    return *this;
  }

  IndexedRandomAccessor operator-(index_t offset) const {
    return IndexedRandomAccessor(va - offset, ia - offset);
  }

  difference_type operator-(const IndexedRandomAccessor& other) const {
    return va - other.va;
  }
  // }

  // Comparison operators {
  bool operator==(const IndexedRandomAccessor& other) const {
    return va == other.va;
  }

  bool operator!=(const IndexedRandomAccessor& other) const {
    return va != other.va;
  }

  bool operator<(const IndexedRandomAccessor& other) const {
    return va < other.va;
  }

  bool operator<=(const IndexedRandomAccessor& other) const {
    return va <= other.va;
  }

  bool operator>(const IndexedRandomAccessor& other) const {
    return va > other.va;
  }

  bool operator>=(const IndexedRandomAccessor& other) const {
    return va >= other.va;
  }
  // }

protected:
  ValueAccessor va;
  IndexAccessor ia;
};

}} // namespace at::native
