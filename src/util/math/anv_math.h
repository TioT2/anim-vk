/**
 * @brief       ANIM-VK Project
 * @file        src/util/anv_math.h
 * @description Math support implementation module
 * @last_update 31.12.2023
*/

#ifndef ANV_MATH_H_
#define ANV_MATH_H_

#include "anv_math_linalg.h"
#include "anv_math_extent.h"

// #define ANV_MATH_LINALG_VEC_SWIZZLE_ELEM(INDEX) Array[GetVectorComponentIndex<SwizzleString[INDEX]>()]
// 
// #define ANV_MATH_LINALG_VEC_DEFINE_SWIZZLED() \
//   template <constexpr_string<2> SwizzleString> vec<component, 2> Swizzled( VOID ) const { return vec<component, 2>(ANV_MATH_LINALG_VEC_SWIZZLE_ELEM(0), ANV_MATH_LINALG_VEC_SWIZZLE_ELEM(1)); } \
//   template <constexpr_string<3> SwizzleString> vec<component, 3> Swizzled( VOID ) const { return vec<component, 3>(ANV_MATH_LINALG_VEC_SWIZZLE_ELEM(0), ANV_MATH_LINALG_VEC_SWIZZLE_ELEM(1), ANV_MATH_LINALG_VEC_SWIZZLE_ELEM(2)); } \
//   template <constexpr_string<4> SwizzleString> vec<component, 4> Swizzled( VOID ) const { return vec<component, 4>(ANV_MATH_LINALG_VEC_SWIZZLE_ELEM(0), ANV_MATH_LINALG_VEC_SWIZZLE_ELEM(1), ANV_MATH_LINALG_VEC_SWIZZLE_ELEM(2), ANV_MATH_LINALG_VEC_SWIZZLE_ELEM(3)); }
// 
// #define ANV_MATH_LINALG_VECN_DEFINE_SWIZZLE(N) \
//   template <constexpr_string<N> SwizzleString> vec<component, N> Swizzle( VOID ) { return Swizzled<SwizzleString>(); }

/**
 * @brief Global namespace
*/
namespace anv
{
  /**
   * @brief All complicated math(linear algebra, noise, etc.) namespace
  */
  namespace math
  {
    /**
     * @brief Very fast sign getting function
     * @param Number (const FLOAT) Number
     * @return (FLOAT) Number sign. Only 1.0f or -1.0f values are possible
    */
    inline FLOAT FastSign( const FLOAT Number )
    {
      union float_dword
      {
        FLOAT Float;
        DWORD Dword;
      };
    
      /* We're cutting sign bit of number and attaching it to floating point one here */
      return float_dword { .Dword = (reinterpret_cast<const DWORD &>(Number) & 0x80000000) | 0x3F800000 }.Float;
    } /* FastSign */
    
    /**
     * @brief Legendary fast inverse square root
     * @param Number (const FLOAT) Number to get fast inverse square root of
     * @return (FLOAT) 1.0f / sqrt(Number), but faster
    */
    inline FLOAT FastInverseSqrt( const FLOAT Number )
    {
      union float_dword
      {
        FLOAT Float;
        DWORD Dword;
      };
    
      float_dword IY { .Float = Number };
      const FLOAT x2 = Number * 0.5F;
    
      IY.Dword = 0x5F3759DF - (IY.Dword >> 1);                    // Evil bit hack
      IY.Float  = IY.Float * (1.5F - (x2 * IY.Float * IY.Float)); // 1st iteration
      IY.Float  = IY.Float * (1.5F - (x2 * IY.Float * IY.Float)); // 2nd iteration, this can be removed
    
      return IY.Float;
    } /* FastInverseSqrt */
    
    /**
     * @brief Very fast module getting function
     * @param Number (const FLOAT) Number
     * @return (FLOAT) Number module.
    */
    inline FLOAT FastAbs( const FLOAT Number )
    {
      union float_dword
      {
        FLOAT Float;
        DWORD Dword;
      };
    
      /* We're cutting sign bit of number and attaching it to floating point one here */
      return float_dword { .Dword = (reinterpret_cast<const DWORD &>(Number) & 0x7FFF'FFFF) }.Float;
    } /* FastAbs */

    /**
     * @brief 3D Extent structure
     * @tparam component Extent component type
    */
    template <arithmetic_type component>
      using extent2 = extent<component, 2>;

    /**
     * @brief 3D Extent structure
     * @tparam component Extent component type
    */
    template <arithmetic_type component>
      using extent3 = extent<component, 3>;

    /**
     * @brief 2-component vector math namespace alias
     * 
     * @tparam component (arithmetic_type) Vector component type
    */
    template <arithmetic_type component>
      using vec2 = linalg::vec<component, 2>;

    /**
     * @brief 3-component vector math namespace alias
     * 
     * @tparam component (arithmetic_type) Vector component type
    */
    template <arithmetic_type component>
      using vec3 = linalg::vec<component, 3>;

    /**
     * @brief 4-component vector math namespace alias
     * 
     * @tparam component (arithmetic_type) Vector component type
    */
    template <arithmetic_type component>
      using vec4 = linalg::vec<component, 4>;

    /**
     * @brief 4x4 matrix math namespace alias
     * 
     * @tparam component (arithmetic_type) Matrix component type
    */
    template <arithmetic_type component> using mat4x4 = linalg::mat<component, 4, 4>;

    /**
     * @brief Namespace with math constants
    */
    namespace consts
    {
      /** @brief PI constant with floating point double precision */
      inline constexpr DOUBLE PI = 3.14159265358979323846264338327950;

      /** @brief E  constant with floating point double precision */
      inline constexpr DOUBLE E  = 2.71828182845904523536028747135266;
    } /* namespace consts */
  } /* namespace math */

  using ivec2 = math::vec2<INT>;
  using ivec3 = math::vec3<INT>;
  using ivec4 = math::vec4<INT>;
  using imat4x4 = math::mat4x4<INT>;
  using iextent2 = math::extent2<INT>;

  using dvec2 = math::vec2<DOUBLE>;
  using dvec3 = math::vec3<DOUBLE>;
  using dvec4 = math::vec4<DOUBLE>;
  using dmat4x4 = math::mat4x4<DOUBLE>;

  using fvec2 = math::vec2<FLOAT>;
  using fvec3 = math::vec3<FLOAT>;
  using fvec4 = math::vec4<FLOAT>;
  using fmat4x4 = math::mat4x4<FLOAT>;

  using extent2 = iextent2;

  using vec2 = fvec2;
  using vec3 = fvec3;
  using vec4 = fvec4;
  using mat4x4 = fmat4x4;
} /* namespace anv */

#endif // !defined(ANV_MATH_H_)