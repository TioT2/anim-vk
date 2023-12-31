/**
 * @brief       ANIM-VK Project
 * @file        src/util/anv_math_extent.h
 * @description Math extent implementation module
 * @last_update 31.12.2023
*/

#ifndef ANV_MATH_EXTENT_H_
#define ANV_MATH_EXTENT_H_

#include "anv_math_linalg.h"

/**
 * @brief Math namespace
*/
namespace anv::math
{
  /**
   * @brief Extent structure
   * @tparam component Type of extent components
   * @tparam COMPONENT_COUNT Count of extent components
  */
  template <arithmetic_type component, UINT COMPONENT_COUNT> requires (COMPONENT_COUNT > 0)
    class extent
    {
    public:
      /* Extent componentns */
      component Arr[COMPONENT_COUNT];
    }; /* class extent */

  template <arithmetic_type component>
    class extent<component, 2>
    {
    public:
      union
      {
        component Arr[2]; // Extent componentns as array
        struct
        {
          component W, H; // Extent components
        }; /* struct */
      }; /* union */

      /**
       * @brief Extent default constructor
       * @param Vec Vector to create extent from
      */
      extent<component, 2>( VOID ) : W(component(0)), H(component(0))
      {

      } /* extent<component, 2>( const linalg::vec<component, 2> ) */

      /**
       * @brief Extent from vector constructor
       * @param Vec Vector to create extent from
      */
      extent<component, 2>( const linalg::vec<component, 2> &Vec ) : W(Vec.X), H(Vec.Y)
      {

      } /* extent<component, 2>( const linalg::vec<component, 2> ) */

      /**
       * @brief Extent from vector constructor
       * @param Vec Vector to create extent from
      */
      extent<component, 2>( const extent<component, 2> &Rhs ) : W(Rhs.W), H(Rhs.H)
      {

      } /* extent<component, 2>( const linalg::vec<component, 2> ) */

      /**
       * @brief Extent from components constructor
       * @param W Extent width
       * @param H Extent height
      */
      extent<component, 2>( component W, component H ) : W(W), H(H)
      {

      } /* extent<component, 2>( const linalg::vec<component, 2> ) */
    }; /* class extent<component, 2> */
} /* namespace anv::math */

#endif // !defined(ANV_MATH_EXTENT_H_)

/* file anv_math_extent.h */
