#pragma once

namespace at { namespace native {

// reference_holder generalized a reference `Type&`
// to a tuple of references `tuple<Types&...>`.
//
// It is designed to be used as a reference surrogate
// for the `CompositeRandomAccessor::reference` type trait
// which combines reference types of its template
// parameters.
template <typename ...Ts>
class references_holder {
public:
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

  references& as_tuple() {
    return refs;
  }

  friend void swap(references_holder rh1, references_holder rh2) {
    return std::swap(rh1.refs, rh2.refs);
  }

protected:
  references refs;
};

template<int N, typename ...Ts>
auto get(references_holder<Ts...>& rh) -> decltype(std::get<N>(rh.as_tuple())) {
  return std::get<N>(rh.as_tuple());
}

template <typename Accessor>
class operator_brackets_proxy {
  using reference = typename std::iterator_traits<Accessor>::reference;
  using value_type = typename std::iterator_traits<Accessor>::value_type;

public:
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
class CompositeRandomAccessor {
  using self_type = CompositeRandomAccessor<KeyAccessor, ValueAccessor>;

  using index_t = typename IndexTraits<KeyAccessor>::index_type;

  using key_accessor_value_type =
    typename std::iterator_traits<KeyAccessor>::value_type;
  using value_accessor_value_type =
    typename std::iterator_traits<ValueAccessor>::value_type;

public:
  using value_type = std::tuple<
    key_accessor_value_type,
    value_accessor_value_type>;
  using reference = references_holder<
    key_accessor_value_type,
    value_accessor_value_type>;
  using pointer = typename std::iterator_traits<KeyAccessor>::pointer;
  using difference_type = typename std::iterator_traits<KeyAccessor>::difference_type;
  using iterator_category = std::random_access_iterator_tag;

  CompositeRandomAccessor(KeyAccessor keys, ValueAccessor values)
    : keys(keys), values(values)
  {}

  // Pointer-like operations {
  reference operator*() {
    return std::tie(*keys, *values);
  }

  auto* operator->() const {
    return keys.operator->();
  }

  reference operator[](index_t idx) {
    return operator_brackets_proxy<self_type>(
      CompositeRandomAccessor(keys + idx, values + idx)
    );
  }
  // }

  // Prefix/postfix increment/decrement {
  CompositeRandomAccessor& operator++() {
    ++keys;
    ++values;
    return *this;
  }

  CompositeRandomAccessor operator++(int) {
    CompositeRandomAccessor copy(*this);
    ++*this;
    return copy;
  }

  CompositeRandomAccessor& operator--() {
    --keys;
    --values;
    return *this;
  }

  CompositeRandomAccessor operator--(int) {
    CompositeRandomAccessor copy(*this);
    --*this;
    return copy;
  }
  // }

  // Arithmetic operations {
  CompositeRandomAccessor& operator+=(index_t offset) {
    keys += offset;
    values += offset;
    return *this;
  }

  CompositeRandomAccessor operator+(index_t offset) const {
    return CompositeRandomAccessor(keys + offset, values + offset);
  }

  friend CompositeRandomAccessor operator+(
    index_t offset,
    const CompositeRandomAccessor& accessor
  ) {
    return accessor + offset;
  }

  CompositeRandomAccessor& operator-=(index_t offset) {
    keys -= offset;
    values -= offset;
    return *this;
  }

  CompositeRandomAccessor operator-(index_t offset) const {
    return CompositeRandomAccessor(keys - offset, values - offset);
  }

  difference_type operator-(const CompositeRandomAccessor& other) const {
    return keys - other.keys;
  }
  // }

  // Comparison operators {
  bool operator==(const CompositeRandomAccessor& other) const {
    return keys == other.keys;
  }

  bool operator!=(const CompositeRandomAccessor& other) const {
    return keys != other.keys;
  }

  bool operator<(const CompositeRandomAccessor& other) const {
    return keys < other.keys;
  }

  bool operator<=(const CompositeRandomAccessor& other) const {
    return keys <= other.keys;
  }

  bool operator>(const CompositeRandomAccessor& other) const {
    return keys > other.keys;
  }

  bool operator>=(const CompositeRandomAccessor& other) const {
    return keys >= other.keys;
  }
  // }

protected:
  KeyAccessor keys;
  ValueAccessor values;
};

}} // namespace at::native
