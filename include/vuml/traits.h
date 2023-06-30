//
// Created by Homin Su on 2023/6/25.
//

#ifndef VUML_INCLUDE_VUML_TRAITS_H_
#define VUML_INCLUDE_VUML_TRAITS_H_

#include <type_traits>
#include <utility>

namespace vuml::traits {

template<typename T, typename = void>
struct is_iterable : ::std::false_type {};

template<typename T>
struct is_iterable<T, ::std::void_t<
    decltype(::std::begin(::std::declval<T &>())),
    decltype(::std::end(::std::declval<T &>()))
>> : std::true_type {
};

template<typename T>
constexpr bool is_iterable_v = is_iterable<T>::value;

template<typename T, typename = void>
struct is_iterator : ::std::false_type {};

template<typename T>
struct is_iterator<T, ::std::void_t<
    typename ::std::iterator_traits<T>::value_type,
    typename std::iterator_traits<T>::difference_type,
    typename std::iterator_traits<T>::pointer,
    typename std::iterator_traits<T>::reference,
    typename std::iterator_traits<T>::iterator_category
>> : std::true_type {
};

template<typename T>
constexpr bool is_iterator_v = is_iterator<T>::value;

template<typename Tuple>
struct tuple_args_len {};

template<typename ...Ts>
struct tuple_args_len<::std::tuple<Ts...>> {
  static constexpr ::std::size_t value = sizeof...(Ts);
};

template<typename Tuple>
constexpr auto tuple_args_len_v = tuple_args_len<Tuple>::value;

} // namespace vuml

#endif //VUML_INCLUDE_VUML_TRAITS_H_
