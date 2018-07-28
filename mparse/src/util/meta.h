#pragma once

#include <cstddef>
#include <type_traits>

namespace util {

template<typename T>
using identity = T;


template<typename... Ts>
struct type_list {
  using seq = std::index_sequence_for<Ts...>;
  static constexpr std::size_t size = sizeof...(Ts);
};

template<typename First, typename Second>
struct type_list_cat;

template<typename... Ts, typename... Us>
struct type_list_cat<type_list<Ts...>, type_list<Us...>> {
  using type = type_list<Ts..., Us...>;
};

template<typename First, typename Second>
using type_list_cat_t = typename type_list_cat<First, Second>::type;

template<typename List, typename... Ts>
using type_list_append_t = type_list_cat_t<List, type_list<Ts...>>;

}  // namespace util