#pragma once

#include "mparse/ast/ast_node.h"
#include <type_traits>
#include <utility>

namespace ast_ops::matching {
namespace impl {

template<typename...>
struct type_list {};

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


template<typename T, typename U>
struct is_same_capture : std::is_same<typename T::tag_type, typename U::tag_type> {
  static_assert(!is_same_capture::value || std::is_same_v<typename T::cap_type, typename U::cap_type>,
    "Captures with the same tag must capture the same type");
};

template<typename T, typename List>
struct is_in_caplist;

template<typename T, typename U, typename... Ts>
struct is_in_caplist<T, type_list<U, Ts...>> :
  std::disjunction<
    is_same_capture<T, U>,
    is_in_caplist<T, type_list<Ts...>>
  >
{};

template<typename T>
struct is_in_caplist<T, type_list<>> : std::false_type {};

template<typename T, typename List>
constexpr bool is_in_caplist_v = is_in_caplist<T, List>::value;


template<typename List, typename T>
using caplist_append_unique = std::conditional_t<
  is_in_caplist_v<T, List>,
  List,
  type_list_append_t<List, T>
>;

template<typename List, typename... Ts>
struct caplist_append_multi_unique {
  using type = List;
};

template<typename List, typename T, typename... Ts>
struct caplist_append_multi_unique<List, T, Ts...>
  : caplist_append_multi_unique<caplist_append_unique<List, T>, Ts..> {};

template<typename List>
struct unique_caplist;

template<typename... Ts>
struct unique_caplist<type_list<Ts...>> : caplist_append_multi_unique<type_list<>, Ts...> {};


template<typename... Caps>
class match_results_base {
public:
  void get() {}  // dummy
};

template<typename Cap, typename... Caps>
class match_results_base<Cap, Caps...> : match_results_base<Caps...> {
  using tag_type = typename Cap::tag_type;
  using cap_type = typename Cap::cap_type;

public:
  using match_results_base<Caps...>::get;

  cap_type& get(tag_type) & {
    return cap;
  }

  const cap_type& get(tag_type) const & {
    return cap;
  }

  cap_type&& get(tag_type) && {
    return std::move(cap_);
  }

  const cap_type&& get(tag_type) const && {
    return std::move(cap_);
  }

private:
  cap_type cap_;
};

template<typename List>
struct get_match_results_base;

template<typename... Caps>
struct get_match_results_base<type_list<Caps...>> {
  using type = match_results_base<Caps...>;
};

template<typename List>
using get_match_results_base_t = typename get_match_results_base<typename unique_caplist<List>::type>::type;

}  // namespace impl


template<typename Tag, typename Cap>
struct capture {
  using tag_type = Tag;
  using cap_type = Cap;
};

}  // namespace ast_ops::matching