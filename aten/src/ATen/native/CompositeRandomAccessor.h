#pragma once

namespace at { namespace native {

template <typename ...Ts>
struct references_holder {
  using references = std::tuple<Ts&...>;
  using values = std::tuple<Ts...>;

  references_holder(references refs)
    : refs{refs}
  {}

  operator references() {
    return refs;
  }

  operator values() {
    return refs;
  }

  references_holder& operator=(values vals) {
    refs = vals;
    return *this;
  }

  references refs;
};

template <typename ...Ts>
void swap(references_holder<Ts...> rh1, references_holder<Ts...> rh2) {
  return std::swap(rh1.refs, rh2.refs);
}

template<int N, typename ...Ts>
auto get(references_holder<Ts...>& rh) -> decltype(std::get<N>(rh.refs)) {
  return std::get<N>(rh.refs);
}

template <typename Accessor>
class operator_brackets_proxy {
public:
  using reference = typename std::iterator_traits<Accessor>::reference;
  using value_type = typename std::iterator_traits<Accessor>::value_type;

  operator_brackets_proxy(Accessor const& accessor)
    : accessor(accessor)
  {}

  operator reference() {
    return *accessor;
  }

  reference operator*() {
    return *accessor;
  }

  operator_brackets_proxy& operator=(value_type const& val) {
    *accessor = val;
    return *this;
  }

private:
  Accessor accessor;
};

template <typename KeyAccessor, typename ValueAccessor>
class IndexedRandomAccessor {
  using self_type = IndexedRandomAccessor<KeyAccessor, ValueAccessor>;

  using ValuePtrType = typename std::iterator_traits<KeyAccessor>::pointer;
  using IndexPtrType = typename std::iterator_traits<ValueAccessor>::pointer;
  using index_t = typename IndexTraits<KeyAccessor>::index_type;

  using value_accessor_value_type =
    typename std::iterator_traits<KeyAccessor>::value_type;
  using index_accessor_value_type =
    typename std::iterator_traits<ValueAccessor>::value_type;

public:
  using value_type = std::tuple<
    value_accessor_value_type,
    index_accessor_value_type>;
  using reference = references_holder<
    value_accessor_value_type,
    index_accessor_value_type>;
  using pointer = typename std::iterator_traits<KeyAccessor>::pointer;
  using difference_type = typename std::iterator_traits<KeyAccessor>::difference_type;
  using iterator_category = std::random_access_iterator_tag;

  IndexedRandomAccessor(KeyAccessor va, ValueAccessor ia)
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
    return operator_brackets_proxy<self_type>(
      IndexedRandomAccessor(va + idx, ia + idx)
    );
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
  KeyAccessor va;
  ValueAccessor ia;
};

}} // namespace at::native
