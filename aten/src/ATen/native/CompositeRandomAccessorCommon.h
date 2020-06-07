#pragma once

namespace at { namespace native {

// reference_holder generalized a reference `Type&`
// to a tuple of references `tuple<Types&...>`.
//
// It is designed to be used as a reference surrogate
// for the `CompositeRandomAccessor::reference` type trait
// which combines reference types of its template
// parameters.
template <typename Values, typename References>
class references_holder {
public:
  using values = Values;
  using references = References;

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

  references& data() {
    return refs;
  }

protected:
  references refs;
};

template <typename Values, typename References>
void swap(
  references_holder<Values, References> rh1,
  references_holder<Values, References> rh2
) {
  return std::swap(rh1.data(), rh2.data());
}

template<int N, typename Values, typename References>
auto get(references_holder<Values, References>& rh) -> decltype(std::get<N>(rh.data())) {
  return std::get<N>(rh.data());
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

template <typename KeyAccessor, typename ValueAccessor,
          template <typename...> class HeterogeneousContainer = std::tuple>
class CompositeRandomAccessor {
  using self_type = CompositeRandomAccessor<KeyAccessor, ValueAccessor>;

  using key_accessor_value_type =
    typename std::iterator_traits<KeyAccessor>::value_type;
  using value_accessor_value_type =
    typename std::iterator_traits<ValueAccessor>::value_type;
  using key_accessor_reference_type =
    typename std::iterator_traits<KeyAccessor>::reference;
  using value_accessor_reference_type =
    typename std::iterator_traits<ValueAccessor>::reference;

  using composite_value_type = HeterogeneousContainer<
    key_accessor_value_type,
    value_accessor_value_type>;
  using composite_reference = HeterogeneousContainer<
    key_accessor_reference_type,
    value_accessor_reference_type>;

public:
  using value_type = composite_value_type;
  using reference = references_holder<composite_value_type, composite_reference>;
  using pointer = typename std::iterator_traits<KeyAccessor>::pointer;
  using difference_type = typename std::iterator_traits<KeyAccessor>::difference_type;
  using iterator_category = std::random_access_iterator_tag;

  C10_HOST_DEVICE
  CompositeRandomAccessor(KeyAccessor keys, ValueAccessor values)
    : keys(keys), values(values)
  {}

  // Pointer-like operations {
  C10_HOST_DEVICE
  reference operator*() {
    return std::tie(*keys, *values);
  }

  C10_HOST_DEVICE
  auto* operator->() const {
    return keys.operator->();
  }

  C10_HOST_DEVICE
  reference operator[](difference_type idx) {
    return operator_brackets_proxy<self_type>(
      CompositeRandomAccessor(keys + idx, values + idx)
    );
  }
  // }

  // Prefix/postfix increment/decrement {
  C10_HOST_DEVICE
  CompositeRandomAccessor& operator++() {
    ++keys;
    ++values;
    return *this;
  }

  C10_HOST_DEVICE
  CompositeRandomAccessor operator++(int) {
    CompositeRandomAccessor copy(*this);
    ++*this;
    return copy;
  }

  C10_HOST_DEVICE
  CompositeRandomAccessor& operator--() {
    --keys;
    --values;
    return *this;
  }

  C10_HOST_DEVICE
  CompositeRandomAccessor operator--(int) {
    CompositeRandomAccessor copy(*this);
    --*this;
    return copy;
  }
  // }

  // Arithmetic operations {
  C10_HOST_DEVICE
  CompositeRandomAccessor& operator+=(difference_type offset) {
    keys += offset;
    values += offset;
    return *this;
  }

  C10_HOST_DEVICE
  CompositeRandomAccessor operator+(difference_type offset) const {
    return CompositeRandomAccessor(keys + offset, values + offset);
  }

  C10_HOST_DEVICE
  friend CompositeRandomAccessor operator+(
    difference_type offset,
    const CompositeRandomAccessor& accessor
  ) {
    return accessor + offset;
  }

  C10_HOST_DEVICE
  CompositeRandomAccessor& operator-=(difference_type offset) {
    keys -= offset;
    values -= offset;
    return *this;
  }

  C10_HOST_DEVICE
  CompositeRandomAccessor operator-(difference_type offset) const {
    return CompositeRandomAccessor(keys - offset, values - offset);
  }

  C10_HOST_DEVICE
  difference_type operator-(const CompositeRandomAccessor& other) const {
    return keys - other.keys;
  }
  // }

  // Comparison operators {
  C10_HOST_DEVICE
  bool operator==(const CompositeRandomAccessor& other) const {
    return keys == other.keys;
  }

  C10_HOST_DEVICE
  bool operator!=(const CompositeRandomAccessor& other) const {
    return keys != other.keys;
  }

  C10_HOST_DEVICE
  bool operator<(const CompositeRandomAccessor& other) const {
    return keys < other.keys;
  }

  C10_HOST_DEVICE
  bool operator<=(const CompositeRandomAccessor& other) const {
    return keys <= other.keys;
  }

  C10_HOST_DEVICE
  bool operator>(const CompositeRandomAccessor& other) const {
    return keys > other.keys;
  }

  C10_HOST_DEVICE
  bool operator>=(const CompositeRandomAccessor& other) const {
    return keys >= other.keys;
  }
  // }

protected:
  KeyAccessor keys;
  ValueAccessor values;
};

}} // namespace at::native
