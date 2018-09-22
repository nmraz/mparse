#pragma once

#include "ast_ops/matching/expr.h"
#include "mparse/ast/ast_node.h"
#include "util/meta.h"
#include <type_traits>
#include <utility>

namespace ast_ops::matching {
namespace impl {

template <typename T, typename U>
struct is_same_capture
    : std::is_same<typename T::tag_type, typename U::tag_type> {
  static_assert(!is_same_capture::value ||
                    std::is_same_v<typename T::cap_type, typename U::cap_type>,
                "Captures with the same tag must capture the same type");
};

template <typename T, typename List>
struct is_in_caplist;

template <typename T, typename U, typename... Ts>
struct is_in_caplist<T, util::type_list<U, Ts...>>
    : std::disjunction<is_same_capture<T, U>,
                       is_in_caplist<T, util::type_list<Ts...>>> {};

template <typename T>
struct is_in_caplist<T, util::type_list<>> : std::false_type {};

template <typename T, typename List>
constexpr bool is_in_caplist_v = is_in_caplist<T, List>::value;


template <typename List, typename T>
using caplist_append_unique =
    std::conditional_t<is_in_caplist_v<T, List>, List,
                       util::type_list_append_t<List, T>>;

template <typename List, typename... Ts>
struct caplist_append_multi_unique {
  using type = List;
};

template <typename List, typename T, typename... Ts>
struct caplist_append_multi_unique<List, T, Ts...>
    : caplist_append_multi_unique<caplist_append_unique<List, T>, Ts...> {};

template <typename List>
struct unique_caplist;

template <typename... Ts>
struct unique_caplist<util::type_list<Ts...>>
    : caplist_append_multi_unique<util::type_list<>, Ts...> {};


template <typename Tag, typename Cap>
struct has_tag : std::is_same<Tag, typename Cap::tag_type> {};

template <typename Tag, typename List>
constexpr size_t count_caps_v =
    util::type_list_count<Tag, List, has_tag>::value;


template <typename... Caps>
class match_results_base {
public:
  template <typename T>
  int get(T) { // dummy
    static_assert(util::always_false<T>, "Expression not captured");
    return 0;
  }
};

template <typename Cap, typename... Caps>
class match_results_base<Cap, Caps...> : public match_results_base<Caps...> {
  using tag_type = typename Cap::tag_type;
  using cap_type = typename Cap::cap_type;

public:
  using match_results_base<Caps...>::get;

  cap_type& get(tag_type) & { return cap_; }

  const cap_type& get(tag_type) const& { return cap_; }

  cap_type&& get(tag_type) && { return std::move(cap_); }

  const cap_type&& get(tag_type) const&& { return std::move(cap_); }

private:
  cap_type cap_;
};

template <typename List>
struct get_match_results_base;

template <typename... Caps>
struct get_match_results_base<util::type_list<Caps...>> {
  using type = match_results_base<Caps...>;
};

template <typename List>
using get_match_results_base_t =
    typename get_match_results_base<typename unique_caplist<List>::type>::type;

} // namespace impl


template <typename Tag, typename Cap>
struct capture {
  using tag_type = Tag;
  using cap_type = Cap;
};


template <typename... Caps>
using caplist = util::type_list<Caps...>;

template <typename... Ls>
using caplist_cat = util::type_list_cat_t<Ls...>;

template <typename List, typename... Caps>
using caplist_append = util::type_list_append_t<List, Caps...>;


template <typename E>
struct matcher_traits;

template <typename E, typename = void>
struct get_captures {
  using type = caplist<>;
};

template <typename E>
struct get_captures<E, std::void_t<typename matcher_traits<E>::captures>> {
  using type = typename matcher_traits<E>::captures;
};

template <typename E>
using get_captures_t = typename get_captures<E>::type;


template <typename Caps>
class match_results : private impl::get_match_results_base_t<Caps> {
public:
  template <typename Tag>
  static constexpr size_t count_caps_with() {
    return impl::count_caps_v<Tag, Caps>;
  }

  template <typename Tag, typename E>
  friend decltype(auto) get_result(match_results<E>& match);

  template <typename Tag, typename E>
  friend decltype(auto) get_result(const match_results<E>& match);

  template <typename Tag, typename E>
  friend decltype(auto) get_result(match_results<E>&& match);

  template <typename Tag, typename E>
  friend decltype(auto) get_result(const match_results<E>&& match);
};

template <typename E>
using match_results_for = match_results<get_captures_t<E>>;


template <typename Tag, typename E>
decltype(auto) get_result(match_results<E>& match) {
  return match.get(Tag{});
}

template <typename Tag, typename E>
decltype(auto) get_result(const match_results<E>& match) {
  return match.get(Tag{});
}

template <typename Tag, typename E>
decltype(auto) get_result(match_results<E>&& match) {
  return std::move(match).get(Tag{});
}

template <typename Tag, typename E>
decltype(auto) get_result(const match_results<E>&& match) {
  return std::move(match).get(Tag{});
}


template <int N, typename Res>
decltype(auto) get_capture(Res&& results) {
  return get_result<capture_expr_tag<N>>(std::forward<Res>(results));
}

template <int N, typename Res>
decltype(auto) get_constant(Res&& results) {
  return get_result<constant_expr_tag<N>>(std::forward<Res>(results));
}

template <char C, typename Res>
decltype(auto) get_subexpr(Res&& results) {
  return get_result<subexpr_expr_tag<C>>(std::forward<Res>(results));
}

} // namespace ast_ops::matching