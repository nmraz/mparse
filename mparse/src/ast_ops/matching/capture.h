#pragma once

#include "mparse/ast/ast_node.h"
#include <type_traits>

namespace ast_ops::matching {

template<typename Tag, typename Node>
struct capture {
  static_assert(std::is_base_of_v<mparse::ast_node, Node>, "Capture types must be derived from ast_node");

  using tag_type = Tag;
  using node_type = Node;
};

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


template<typename T, typename List, template<typename, typename> typename Pred>
struct is_in;

template<typename T, typename U, typename... Ts, template<typename, typename> typename Pred>
struct is_in<T, type_list<U, Ts...>, Pred> :
  std::disjunction<
    Pred<T, U>,
    is_in<T, type_list<Ts...>, Pred>
  >
{};

template<typename T>
struct is_in<T, type_list<>> : std::false_type {};

template<typename T, typename List, template<typename, typename> typename Pred>
constexpr bool is_in_v = is_in<T, List, Pred>::value;



}  // namespace impl

}  // namespace ast_ops::matching