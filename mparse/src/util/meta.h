#pragma once

#include <cstddef>
#include <type_traits>

namespace util {

template <typename T>
constexpr bool always_false = false; // dependent false

template <typename T>
using identity = T;


template <typename... Ts>
struct type_list {
  using seq = std::index_sequence_for<Ts...>;
  static constexpr std::size_t size = sizeof...(Ts);
};

template <typename... Ls>
struct type_list_cat;

template <>
struct type_list_cat<> {
  using type = type_list<>;
};

template <typename... Ts, typename... Us>
struct type_list_cat<type_list<Ts...>, type_list<Us...>> {
  using type = type_list<Ts..., Us...>;
};

template <typename... Ts, typename... Rest>
struct type_list_cat<type_list<Ts...>, Rest...> {
  using type =
      typename type_list_cat<type_list<Ts...>,
                             typename type_list_cat<Rest...>::type>::type;
};

template <typename... Ls>
using type_list_cat_t = typename type_list_cat<Ls...>::type;

template <typename List, typename... Ts>
using type_list_append_t = type_list_cat_t<List, type_list<Ts...>>;

template <typename T, typename List, template <typename, typename> typename Cmp>
struct type_list_count;

template <typename T,
          typename... Ts,
          template <typename, typename> typename Cmp>
struct type_list_count<T, type_list<Ts...>, Cmp>
    : std::integral_constant<std::size_t, (0 + ... + Cmp<T, Ts>::value)> {};

template <typename T, typename List>
constexpr std::size_t type_list_count_v =
    type_list_count<T, List, std::is_same>::value;

} // namespace util