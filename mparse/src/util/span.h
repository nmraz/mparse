#pragma once

#include <algorithm>
#include <iterator>
#include <type_traits>

namespace util {

template<typename T>
class span;

namespace impl {

template<typename T>
constexpr bool is_span_impl = false;
template<typename T>
constexpr bool is_span_impl<span<T>> = true;
template<typename T>
constexpr bool is_span = is_span_impl<std::decay_t<T>>;

template<typename From, typename To>
constexpr bool is_safe_array_conv = std::is_convertible_v<From(&)[], To(&)[]>;


template<typename Cont, typename = void>
constexpr bool has_integral_size = false;

template<typename Cont>
constexpr bool has_integral_size<
  Cont,
  std::void_t<
    decltype(std::declval<Cont&>().size())
  >
> = std::is_integral_v<decltype(std::declval<Cont&>().size())>;


template<typename Data, typename T>
constexpr bool is_convertible_data = std::is_pointer_v<Data>
  && is_safe_array_conv<std::remove_pointer_t<Data>, T>;

template<typename Cont, typename T, typename = void>
constexpr bool has_convertible_data = false;

template<typename Cont, typename T>
constexpr bool has_convertible_data<
  Cont,
  T,
  std::void_t<
    decltype(std::declval<Cont&>().data())
  >
> = is_convertible_data<
  decltype(std::declval<Cont&>().data()),
  T
>;


template<typename Cont, typename T>
constexpr bool is_compatible_container = !is_span<Cont> && has_convertible_data<Cont, T>
  && has_integral_size<Cont>;

}  // namespace impl


template<typename T>
class span {
public:
  using index_type = std::ptrdiff_t;
  using element_type = T;
  using pointer = T*;
  using reference = T&;
  using iterator = T*;
  using const_iterator = const T*;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  constexpr span() : data_(nullptr), size_(0) {}
  constexpr span(std::nullptr_t) : span() {}
  constexpr span(pointer ptr, index_type size) : data_(ptr), size_(size) {}
  constexpr span(pointer begin, pointer end) : span(begin, end - begin) {}

  constexpr span(const span&) = default;
  constexpr span(span&&) = default;

  template<
    typename Cont,
    typename = std::enable_if_t<impl::is_compatible_container<Cont, T>>
  > constexpr span(Cont& cont) : span(cont.data(), cont.size()) {}

  template<
    typename U,
    std::size_t N,
    typename = std::enable_if_t<impl::is_safe_array_conv<U, T>>
  > constexpr span(U(&array)[N]) : span(array, N) {}

  template<typename U, typename = std::enable_if_t<impl::is_safe_array_conv<U, T>>>
  constexpr span(const span<U>& rhs) : span(rhs.data(), rhs.size()) {}

  template<typename U, typename = std::enable_if_t<impl::is_safe_array_conv<U, T>>>
  constexpr span(span<U>&& rhs) : span(rhs.data(), rhs.size()) {}

  constexpr void swap(span& other);

  constexpr reference operator[](index_type idx) const { return data_[idx]; }
  constexpr pointer data() const { return data_; }

  constexpr index_type size() const { return size_; }
  constexpr index_type length() const { return size_; }
  constexpr bool empty() const { return size_ == 0; }

  constexpr span first(index_type count);
  constexpr span last(index_type count);
  constexpr span subspan(index_type offset, index_type count);

  constexpr iterator begin() const { return data_; }
  constexpr const_iterator cbegin() const { return data_; }

  constexpr iterator end() const { return data_ + size_; }
  constexpr const_iterator cend() const { return data_ + size_; }

  constexpr reverse_iterator rbegin() const { return reverse_iterator(end()); }
  constexpr const_reverse_iterator crbegin() const {
    return const_reverse_iterator(cend());
  }

  constexpr reverse_iterator rend() const { return reverse_iterator(begin()); }
  constexpr const_reverse_iterator crend() const {
    return const_reverse_iterator(cbegin());
  }

private:
  pointer data_;
  index_type size_;
};


template<typename T>
constexpr void span<T>::swap(span& other) {
  using std::swap;
  swap(data_, other.data_);
  swap(size_, other.size_);
}

template<typename T>
constexpr inline void swap(span<T>& lhs, span<T>& rhs) {
  lhs.swap(rhs);
}


template<typename T>
constexpr span<T> span<T>::first(index_type count) {
  return { data_, count };
}

template<typename T>
constexpr span<T> span<T>::last(index_type count) {
  return { data_ + size_ - count, count };
}

template<typename T>
constexpr span<T> span<T>::subspan(index_type offset, index_type count) {
  return { data_ + offset, count };
}


template<typename T>
constexpr bool operator==(const span<T>& lhs, const span<T>& rhs) {
  return std::equal(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend());
}

template<typename T>
constexpr bool operator!=(const span<T>& lhs, const span<T>& rhs) {
  return !(lhs == rhs);
}

template<typename T>
constexpr bool operator<(const span<T>& lhs, const span<T>& rhs) {
  return std::lexicographical_compare(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend());
}

template<typename T>
constexpr bool operator>(const span<T>& lhs, const span<T>& rhs) {
  return rhs < lhs;
}

template<typename T>
constexpr bool operator<=(const span<T>& lhs, const span<T>& rhs) {
  return !(lhs > rhs);
}

template<typename T>
constexpr bool operator>=(const span<T>& lhs, const span<T>& rhs) {
  return !(lhs < rhs);
}


template<typename T>
constexpr inline span<T> make_span(T* data, std::ptrdiff_t size) {
  return { data, size };
}

template<typename T>
constexpr inline span<T> make_span(T* begin, T* end) {
  return { begin, end };
}

template<typename T, std::size_t N>
constexpr inline span<T> make_span(T(&array)[N]) {
  return { array };
}

template<
  typename Cont,
  typename T = typename Cont::value_type,
  typename = std::enable_if_t<impl::is_compatible_container<Cont, T>>
> constexpr inline span<T> make_span(Cont& cont) {
  return { cont };
}

template<
  typename Cont,
  typename T = const typename Cont::value_type,
  typename = std::enable_if_t<impl::is_compatible_container<const Cont, T>>
> constexpr inline span<T> make_span(const Cont& cont) {
  return { cont };
}

template<typename Cont, typename = std::enable_if_t<!impl::is_span<Cont>>>
constexpr void make_span(const Cont&&) = delete;  // temporary

template<typename T>
constexpr inline span<T> make_span(const span<T>& sp) {
  return sp;
}

}  // namespace util