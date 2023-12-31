/**
 * @brief       ANIM-VK Project
 * @file        src/util/meta/anv_meta_concepts.h
 * @description Useful concept set
 * @last_update 30.12.2023
*/

#ifndef ANV_META_CONCEPTS_H_
#define ANV_META_CONCEPTS_H_

#include "anv_common.h"

/**
 * @brief Project namespace
*/
namespace anv
{
  /**
   * @brief Callable concept
   * @tparam callable_type Type to check
   * @tparam return_type Type, that this callable should return
   * @tparam ...argument_types Types of callable arguments
  */
  template <typename callable_type, typename return_type, typename ...argument_types>
    concept callable = std::is_invocable_v<callable_type, argument_types...> && requires(callable_type T, argument_types ...Arguments)
    {
      { T(std::forward<argument_types>(Arguments)...) } -> std::convertible_to<return_type>;
    }; /* concept callable */


  template <typename type>
    concept enumeration = std::is_enum_v<type>;

  template <typename type, typename ...in>
    concept contained_in = std::disjunction_v<std::is_same<type, in>...>;

  /* Meta used for std::variant handling */
  template<class... ts>
    struct overloaded : ts... { using ts::operator()...; };

  /**
   * @brief Concept of type is possible to do arithmetic operations with
  */
  template <typename type>
    concept arithmetic_type = std::is_arithmetic_v<type>;
} /* namespace anv::meta */

#endif // !defined(ANV_META_CONCEPTS_H_)