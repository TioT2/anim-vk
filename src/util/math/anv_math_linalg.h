/**
 * @brief       ANIM-VK Project
 * @file        src/util/anv_math_linalg.h
 * @description Math linear algebra implementation module
 * @last_update 31.12.2023
*/

#ifndef ANV_MATH_LINALG_H_
#define ANV_MATH_LINALG_H_

#include "util/meta/anv_meta_concepts.h"

#define ANV_MATH_LINALG_VEC2_DEFINE_OPERATOR(Op) \
  inline vec<component, 2>   operator Op   ( const vec<component, 2> &Rhs ) const { return vec<component, 2>(X Op Rhs.X, Y Op Rhs.Y); } \
  inline vec<component, 2> & operator Op##=( const vec<component, 2> &Rhs )       { X Op##= Rhs.X; Y Op##= Rhs.Y; return *this; }       \
  inline vec<component, 2>   operator Op   ( component Rhs ) const { return vec<component, 2>(X Op Rhs, Y Op Rhs); }                    \
  inline vec<component, 2> & operator Op##=( component Rhs )       { X Op##= Rhs; Y Op##= Rhs; return *this; }                          \

#define ANV_MATH_LINALG_VEC3_DEFINE_OPERATOR(Op) \
  inline vec<component, 3>   operator Op   ( const vec<component, 3> &Rhs ) const { return vec<component, 3>(X Op Rhs.X, Y Op Rhs.Y, Z Op Rhs.Z); } \
  inline vec<component, 3> & operator Op##=( const vec<component, 3> &Rhs )       { X Op##= Rhs.X; Y Op##= Rhs.Y; Z Op##= Rhs.Z; return *this; }    \
  inline vec<component, 3>   operator Op   ( component Rhs ) const { return vec<component, 3>(X Op Rhs, Y Op Rhs, Z Op Rhs); }                      \
  inline vec<component, 3> & operator Op##=( component Rhs )       { X Op##= Rhs; Y Op##= Rhs; Z Op##= Rhs; return *this; }                         \

#define ANV_MATH_LINALG_VEC4_DEFINE_OPERATOR(Op) \
  inline vec<component, 4>   operator Op   ( const vec<component, 4> &Rhs ) const { return vec<component, 4>(X Op Rhs.X, Y Op Rhs.Y, Z Op Rhs.Z, W Op Rhs.W); } \
  inline vec<component, 4> & operator Op##=( const vec<component, 4> &Rhs )       { X Op##= Rhs.X; Y Op##= Rhs.Y; Z Op##= Rhs.Z; W Op##= Rhs.W; return *this; } \
  inline vec<component, 4>   operator Op   ( component Rhs ) const { return vec<component, 4>(X Op Rhs, Y Op Rhs, Z Op Rhs, W Op Rhs); }                        \
  inline vec<component, 4> & operator Op##=( component Rhs )       { X Op##= Rhs; Y Op##= Rhs; Z Op##= Rhs; W Op##= Rhs; return *this; }                        \

namespace anv::math::linalg
{
  /**
   * @brief Get vector array index by it's component name
   * 
   * @tparam Symbol (CHAR) X, Y, Z or W
   * 
   * @return (INT) Index of component in array
  */
  template <CHAR Symbol>
    consteval INT GetVectorComponentIndex( VOID )
    {
      static_assert(Symbol == 'X' || Symbol == 'Y' || Symbol == 'Z' || Symbol == 'W', "Invalid swizzle component");

      if constexpr (Symbol == 'X') return 0;
      if constexpr (Symbol == 'Y') return 1;
      if constexpr (Symbol == 'Z') return 2;
      if constexpr (Symbol == 'W') return 3;
    } /* GetVectorSwizzleIndex */

  /**
   * @brief Vector basic class
   * 
   * @tparam component       (arithmetic_type) Vector component type
   * @tparam COMPONENT_COUNT (UINT)            Count of vector components Must be in [2, 4] range
  */
  template <arithmetic_type component, UINT COMPONENT_COUNT> requires (COMPONENT_COUNT > 1 && COMPONENT_COUNT < 5)
    struct vec
    {
      component Array[COMPONENT_COUNT]; ///< Array of vector components
    }; /* vec */

  /**
   * @brief Custom formatter for vectors
   * 
   * @tparam component       (arithmetic_type) Vector component type
   * @tparam COMPONENT_COUNT (UINT)            Vector component count
  */
  template <arithmetic_type component, UINT COMPONENT_COUNT>
    struct std::formatter<anv::math::linalg::vec<component, COMPONENT_COUNT>>
    {
      /**
       * @brief Custom format parameters parsing function
       * 
       * @param ParseContext (std::format_parse_context &) STL format parsing context
       * 
       * @return (auto) Parsing end iterator
      */
      constexpr auto parse( std::format_parse_context &ParseContext )
      {
        return ParseContext.begin();
      } /* parse */

      /**
       * @brief Formatting function
       * 
       * @param Object        (const anv::math::linalg::vec<component, COMPONENT_COUNT> &) Object to format by
       * @param FormatContext (std::format_context &)                                       STL format context
       * 
       * @return (auto) Formatted result
      */
      auto format( const anv::math::linalg::vec<component, COMPONENT_COUNT> &Object, std::format_context &FormatContext )
      {
        std::string Result;

        std::format_to(FormatContext.out(), "<");
        for (anv::UINT32 i = 0; i < COMPONENT_COUNT - 1; i++)
          Result += std::format_to(FormatContext.out(), "{}, ", Object.Array[i]);
        Result += std::format_to(FormatContext.out(), "{}", Object.Array[COMPONENT_COUNT - 1]);

        return std::format_to(FormatContext.out(), ">");
      } /* format */
    }; /* std::formatter */

  /**
   * @brief 2-component vector specialization
   * 
   * @tparam component (arithmetic_type) Vector component type
  */
  template <arithmetic_type component>
    struct vec<component, 2>
    {
      /// @private
      union
      {
        struct
        {
          component X, Y;
        };
        component Array[2];
      };

      /**
       * @brief Default constructor.
       * 
       * @warning Components are uninitialized for better performance (for example in std::vector)
      */
      vec( VOID )
      {

      } /* vec */

      /**
       * @brief Constructor for two components.
       * 
       * @param X (component) X vector component
       * @param Y (component) Y vector component
      */
      vec( component X, component Y ) : X(X), Y(Y)
      {

      } /* vec( component, component ) */

      /**
       * @brief Constructor with same components value.
       * 
       * Is explicit to avoid hard to find bugs.
       * 
       * @param V (component) Value of vector.
      */
      explicit vec( component V ) : X(V), Y(V)
      {

      } /* vec( component ) */

      /**
       * @brief Assignment operator
       * 
       * @tparam RHS_COMPONENT_COUNT (INT)                                         Count of components in other vector
       * @param  Rhs                 (const vec<component, RHS_COMPONENT_COUNT> &) Vector to assign
       * 
       * @return (vec &) Self reference.
      */
      template <INT RHS_COMPONENT_COUNT>
        vec & operator=( const vec<component, RHS_COMPONENT_COUNT> &Rhs )
        {
          if constexpr (RHS_COMPONENT_COUNT > 0) Array[0] = Rhs.Array[0];
          if constexpr (RHS_COMPONENT_COUNT > 1) Array[1] = Rhs.Array[1];

          return *this;
        } /* operator= */

      /**
       * @brief Squared vector length getting function
       * 
       * @return (component) Length of this vector squared
      */
      inline component Length2( VOID ) const
      {
        return X * X + Y * Y;
      } /* Length2 */

      /**
       * @brief Vector length getting function
       * 
       * @return (component) Length of vector
      */
      inline component Length( VOID ) const
      {
        return std::sqrt(X * X + Y * Y);
      } /* Length */

      /**
       * @brief Vector normalization function
       * 
       * @warning Zero-length vectors aren't handled for better performance
       * 
       * @return (vec &) Self reference
      */
      inline vec & Normalize( VOID )
      {
        component L = std::sqrt(X * X + Y * Y);

        X /= L;
        Y /= L;

        return *this;
      } /* Normalize */

      /**
       * @brief Normalized self getting fucntion
       * 
       * @warning Zero-length vector aren't handled for better performance
       * 
       * @return (vec) Normalized self
      */
      inline vec Normalized( VOID ) const
      {
        component L = std::sqrt(X * X + Y * Y);

        return vec(X / L, Y / L);
      } /* Normalize */

      /**
       * @brief Dot product operator
       * 
       * @param Rhs (const vec &) Right component
       * 
       * @return (component) Dot product of these vectors
      */
      inline component operator&( const vec &Rhs ) const
      {
        return X * Rhs.X + Y * Rhs.Y;
      } /* operator& */

      /**
       * @brief Vector cross product operator
       * 
       * @param Rhs (const vec &) Vector to get cross product with
       * 
       * @return (component) Cross product of these vectors
      */
      inline component operator%( const vec &Rhs ) const
      {
        return X * Rhs.Y - Y * Rhs.X;
      } /* operator% */

      // ANV_MATH_LINALG_VEC_DEFINE_SWIZZLED()
      // ANV_MATH_LINALG_VECN_DEFINE_SWIZZLE(2)
      ANV_MATH_LINALG_VEC2_DEFINE_OPERATOR(+)
      ANV_MATH_LINALG_VEC2_DEFINE_OPERATOR(-)
      ANV_MATH_LINALG_VEC2_DEFINE_OPERATOR(*)
      ANV_MATH_LINALG_VEC2_DEFINE_OPERATOR(/)
    }; /* vec2 */

  /**
   * @brief 3-component vector specialization
   * 
   * @tparam component (arithmetic_type) Vector component Type
  */
  template <arithmetic_type component>
    struct vec<component, 3>
    {
      /// @private
      union
      {
        struct
        {
          component X, Y, Z; // Vector components
        };
        component Array[3]; // Vector components as array
      };

      /**
       * @brief Vector default constructor
       * 
       * @warning Components are uninitialized for better performance
      */
      vec( VOID )
      {

      } /* vec( VOID ) */

      /**
       * @brief Vector from components constructor
       * 
       * @param X (component) X vector component
       * @param Y (component) Y vector component
       * @param Z (component) Z vector component
      */
      vec( component X, component Y, component Z ) : X(X), Y(Y), Z(Z)
      {

      } /* vec( component, component, component ) */

      /**
       * @brief Vector from one component constructor
       * 
       * Is explicit to avoid bugs with casting of numbers
       * 
       * @param V (component) Value to fill all vector components by
      */
      explicit vec( component V ) : X(V), Y(V), Z(V)
      {

      } /* vec( component ) */

      /**
       * @brief Assignment operator.
       * 
       * @tparam RHS_COMPONENT_COUNT (INT)                                         Count of rhs vector components.
       * @param  Rhs                 (const vec<component, RHS_COMPONENT_COUNT> &) Vector to assign.
       * 
       * @return (vec &) Self reference.
      */
      template <INT RHS_COMPONENT_COUNT>
        vec & operator=( const vec<component, RHS_COMPONENT_COUNT> &Rhs )
        {
          if constexpr (RHS_COMPONENT_COUNT > 0) Array[0] = Rhs.Array[0];
          if constexpr (RHS_COMPONENT_COUNT > 1) Array[1] = Rhs.Array[1];
          if constexpr (RHS_COMPONENT_COUNT > 2) Array[2] = Rhs.Array[2];

          return *this;
        } /* operator= */

      /**
       * @brief Vector length squared getting function
       * 
       * @return (component) Vector length squared
      */
      inline component Length2( VOID ) const
      {
        return X * X + Y * Y + Z * Z;
      } /* Length2 */

      /**
       * @brief Vector length getting function
       * 
       * @return (component) Vector length
      */
      inline component Length( VOID ) const
      {
        return std::sqrt(X * X + Y * Y + Z * Z);
      } /* Length */

      /**
       * @brief Vector normalization function
       * 
       * @warning Zero-length vectors aren't handled for better performance
       * 
       * @return (vec &) Self reference
      */
      inline vec & Normalize( VOID )
      {
        component L = 1.0F / std::sqrt(X * X + Y * Y + Z * Z);

        X *= L;
        Y *= L;
        Z *= L;

        return *this;
      } /* Normalize */

      /**
       * @brief Vector normalization function
       * 
       * @warning Zero-length vectors aren't handled for better performance
       * 
       * @return (vec) This vector normalized
      */
      inline vec Normalized( VOID ) const
      {
        component L = std::sqrt(X * X + Y * Y + Z * Z);

        return vec(X / L, Y / L, Z / L);
      } /* Normalize */

      /**
       * @brief Negate operator
       * 
       * @return (veg) This vector negated
      */
      inline vec operator-( VOID ) const
      {
        return vec(-X, -Y, -Z);
      } /* operator- */

      /**
       * @brief Dot product operator
       * 
       * @param Rhs (const vec &) Vector to get dot product with
       * 
       * @return (component) Dot product of this and Rhs
      */
      inline component operator&( const vec &Rhs ) const
      {
        return X * Rhs.X + Y * Rhs.Y + Z * Rhs.Z;
      } /* operator& */

      /**
       * @brief Cross product operator
       * 
       * @param Rhs (const vec &) Vector to get cross product with
       * 
       * @return (vec) Cross product of these vectors
      */
      inline vec operator%( const vec &Rhs ) const
      {
        return vec
        (
          Y * Rhs.Z - Z * Rhs.Y,
          Z * Rhs.X - X * Rhs.Z,
          X * Rhs.Y - Y * Rhs.X
        );
      } /* operator% */

      /**
       * @brief Cross product with assignment operator
       * 
       * @param Rhs (const vec &) Vector to get cross product with
       * 
       * @return (vec &) Self reference
      */
      inline vec & operator%=( const vec &Rhs )
      {
        X = Y * Rhs.Z - Z * Rhs.Y;
        Y = Z * Rhs.X - X * Rhs.Z;
        Z = X * Rhs.Y - Y * Rhs.X;

        return *this;
      } /* operator%= */

      // ANV_MATH_LINALG_VEC_DEFINE_SWIZZLED()
      // ANV_MATH_LINALG_VECN_DEFINE_SWIZZLE(3)
      ANV_MATH_LINALG_VEC3_DEFINE_OPERATOR(+)
      ANV_MATH_LINALG_VEC3_DEFINE_OPERATOR(-)
      ANV_MATH_LINALG_VEC3_DEFINE_OPERATOR(*)
      ANV_MATH_LINALG_VEC3_DEFINE_OPERATOR(/)
    }; /* vec<component, 3> */

  /**
   * @brief 4-component vector specialization
   * 
   * @tparam component (arithmetic_type) Vector component type
  */
  template <arithmetic_type component>
    struct vec<component, 4>
    {
      /// @private
      union
      {
        struct
        {
          component X, Y, Z, W; // Vector components
        };
        component Array[4]; // Vector components as array
      };

      /**
       * @brief Assignment operator
       * 
       * @tparam RHS_COMPONENT_COUNT (INT)                                         Count of RHS components
       * @param  Rhs                 (const vec<component, RHS_COMPONENT_COUNT> &) Vector to assign
       * 
       * @return (vec &) Self reference
      */
      template <INT RHS_COMPONENT_COUNT>
        vec & operator=( const vec<component, RHS_COMPONENT_COUNT> &Rhs )
        {
          if constexpr (RHS_COMPONENT_COUNT > 0) Array[0] = Rhs.Array[0];
          if constexpr (RHS_COMPONENT_COUNT > 1) Array[1] = Rhs.Array[1];
          if constexpr (RHS_COMPONENT_COUNT > 2) Array[2] = Rhs.Array[2];
          if constexpr (RHS_COMPONENT_COUNT > 3) Array[3] = Rhs.Array[3];

          return *this;
        } /* vec */

      /**
       * @brief Default construcor
       * 
       * @warning Components are uninitialized for better performance
      */
      vec( VOID )
      {

      } /* vec */

      /**
       * @brief Constructor by all components
       * 
       * @param X (component) X vector component
       * @param Y (component) Y vector component
       * @param Z (component) Z vector component
       * @param W (component) W vector component
      */
      vec( component X, component Y, component Z, component W ) : X(X), Y(Y), Z(Z), W(W)
      {
      } /* vec( component, component, component, component ) */

      /**
       * @brief Constructor by only one value
       * 
       * @param V (component) Value to fill all components by
      */
      explicit vec( component V ) : X(V), Y(V), Z(V), W(V)
      {

      } /* vec( component ) */

      /**
       * @brief Length of vector squared getting function
       * 
       * @return (component) Vector length squared
      */
      inline component Length2( VOID ) const
      {
        return X * X + Y * Y + Z * Z + W * W;
      } /* Length2 */

      /**
       * @brief Length of vector getting function
       * 
       * @return (component) Vector length
      */
      inline component Length( VOID ) const
      {
        return std::sqrt(X * X + Y * Y + Z * Z + W * W);
      } /* Length */

      /**
       * @brief Vector normalization function
       * 
       * @warning Zero-length vectors aren't handled for better performance
       * 
       * @return (vec &) Self reference
      */
      inline vec & Normalize( VOID )
      {
        component L = std::sqrt(X * X + Y * Y + Z * Z + W * W);

        X /= L;
        Y /= L;
        Z /= L;
        W /= L;

        return *this;
      } /* Normalize */

      /**
       * @brief Normalized vector getting function
       * 
       * @warning Zero-length vectors aren't handled for better performance
       * 
       * @return (vec) Normalized self vector
      */
      inline vec Normalized( VOID )
      {
        component L = std::sqrt(X * X + Y * Y + Z * Z + W * W);

        return vec(X / L, Y / L, Z / L, W / L);
      } /* Normalize */

      /**
       * @brief Dot product operator
       * 
       * @param Rhs (const vec &) Vector to get dot product with
       * 
       * @return (component) Dot product
      */
      inline component operator&( const vec &Rhs ) const
      {
        return X * Rhs.X + Y * Rhs.Y + Z * Rhs.Z + W * Rhs.W;
      } /* operator& */

      // ANV_MATH_LINALG_VEC_DEFINE_SWIZZLED()
      // ANV_MATH_LINALG_VECN_DEFINE_SWIZZLE(4)
      ANV_MATH_LINALG_VEC4_DEFINE_OPERATOR(+)
      ANV_MATH_LINALG_VEC4_DEFINE_OPERATOR(-)
      ANV_MATH_LINALG_VEC4_DEFINE_OPERATOR(*)
      ANV_MATH_LINALG_VEC4_DEFINE_OPERATOR(/)
    }; /* vec<component, 4> */

  /**
   * @brief Matrix basic class
   * 
   * @tparam component    (arithmetic_type) Matrix component type
   * @tparam ROW_COUNT    (INT)             Count of matrix rows
   * @tparam COLUMN_COUNT (INT)             Count of matrix columns
  */
  template <arithmetic_type component, INT ROW_COUNT, INT COLUMN_COUNT>
    struct mat
    {
      /** @brief Type of matrix row. Represented by COLUMN_COUNT vector. */
      using row = vec<component, COLUMN_COUNT>;

      /// @private
      union
      {
        component Data[ROW_COUNT][COLUMN_COUNT];
        component Array[ROW_COUNT * COLUMN_COUNT];
        vec<component, COLUMN_COUNT> Rows[ROW_COUNT];
      };
    }; /* struct mat */

  /**
   * @brief Matrix formatter.
   * 
   * @tparam component    (arithmetic_type) Matrix component type
   * @tparam ROW_COUNT    (INT)             Count of matrix rows
   * @tparam COLUMN_COUNT (INT)             Count of matrix columns
  */
  template <arithmetic_type component, INT ROW_COUNT, INT COLUMN_COUNT>
    struct std::formatter<anv::math::linalg::mat<component, ROW_COUNT, COLUMN_COUNT>>
    {
      /**
       * @brief Argument parsing function
       * 
       * @param ParseContext (std::format_parse_context &) STL parsing context
       * 
       * @return (auto) STL stuff
      */
      constexpr auto parse( std::format_parse_context &ParseContext )
      {
        return ParseContext.begin();
      } /* parse */

      /**
       * @brief Formatting function
       * 
       * @param Object        (const anv::math::linalg::mat<component, ROW_COUNT, COLUMN_COUNT> &) Object to format
       * @param FormatContext (std::format_context &)                                               Context to format in
       * 
       * @return (auto) Formatted result
      */
      auto format( const anv::math::linalg::mat<component, ROW_COUNT, COLUMN_COUNT> &Object, std::format_context &FormatContext )
      {
        std::string Result;

        for (anv::UINT32 i = 0; i < ROW_COUNT; i++)
          Result += std::format("{}\n", Object.Rows[i]);

        return std::format_to(FormatContext.out(), "{}", Result);
      } /* format */
    }; /* std::formatter */

  /**
   * @brief 3x3 matrix specialization
   * 
   * @tparam component Matrix component type
  */
  template <arithmetic_type component>
    struct mat<component, 3, 3>
    {
      /** * @brief Row type */
      using row = vec<component, 3>;

      /// @private
      union
      {
        component Data[3][3];
        component Array[9];
        vec<component, 3> Rows[3];
      };

      /**
       * @brief 3x3 matrix determinant getting function
       * 
       * @param D00 (component) [0, 0] matrix component
       * @param D01 (component) [0, 1] matrix component
       * @param D02 (component) [0, 2] matrix component
       * @param D10 (component) [1, 0] matrix component
       * @param D11 (component) [1, 1] matrix component
       * @param D12 (component) [1, 2] matrix component
       * @param D20 (component) [2, 0] matrix component
       * @param D21 (component) [2, 1] matrix component
       * @param D22 (component) [2, 2] matrix component
       * 
       * @return (component) Determinant of D00..D22 matrix
      */
      static component Determ
      (
        component D00, component D01, component D02,
        component D10, component D11, component D12,
        component D20, component D21, component D22
      )
      {
      return D00 * D11 * D22 + D01 * D12 * D20 + D02 * D10 * D21 -
             D00 * D12 * D21 - D01 * D10 * D22 - D02 * D11 * D20;
      } /* Determ */
    }; /* struct mat<component, 3, 3> */

  /**
   * @brief 4x4 matrix specialization
   * 
   * @tparam component Matrix component
  */
  template <arithmetic_type component>
    struct mat<component, 4, 4>
    {
      /** @brief Matrix row type */
      using row = vec<component, 4>;

      /// @private
      union
      {
        component Data[4][4];
        component Array[16];
        row Rows[4];
      };

      /**
       * @brief Matrix default constructor
       * 
       * @warning Components are uninitialized for better performance
      */
      mat( VOID )
      {

      } /* mat */

      /**
       * @brief Constructor by ALL components
       * 
       * @param D00 (component) [0, 0] matrix component
       * @param D10 (component) [1, 0] matrix component
       * @param D20 (component) [2, 0] matrix component
       * @param D30 (component) [3, 0] matrix component
       * @param D01 (component) [0, 1] matrix component
       * @param D11 (component) [1, 1] matrix component
       * @param D21 (component) [2, 1] matrix component
       * @param D31 (component) [3, 1] matrix component
       * @param D02 (component) [0, 2] matrix component
       * @param D12 (component) [1, 2] matrix component
       * @param D22 (component) [2, 2] matrix component
       * @param D32 (component) [3, 2] matrix component
       * @param D03 (component) [0, 3] matrix component
       * @param D13 (component) [1, 3] matrix component
       * @param D23 (component) [2, 3] matrix component
       * @param D33 (component) [3, 3] matrix component
      */
      mat
      (
        component D00, component D10, component D20, component D30,
        component D01, component D11, component D21, component D31,
        component D02, component D12, component D22, component D32,
        component D03, component D13, component D23, component D33
      ) : Array
      {
        D00, D10, D20, D30,
        D01, D11, D21, D31,
        D02, D12, D22, D32,
        D03, D13, D23, D33
      }
      {

      } /* mat */

      /**
       * @brief Constructor by row vectors
       * 
       * @param R0 (row) 1st matrix row
       * @param R1 (row) 2nd matrix row
       * @param R2 (row) 3rd matrix row
       * @param R3 (row) 4th matrix row
      */
      mat( const row &R0, const row &R1, const row &R2, const row &R3 ) :
        Rows { R0, R1, R2, R3 }
      {

      } /* mat */

      /**
       * @brief Transposed matrix getting function
       * 
       * @return (mat) Transposed self
      */
      mat Transposed( VOID ) const
      {
        return mat
        {
          Data[0][0], Data[1][0], Data[2][0], Data[3][0],
          Data[0][1], Data[1][1], Data[2][1], Data[3][1],
          Data[0][2], Data[1][2], Data[2][2], Data[3][2],
          Data[0][3], Data[1][3], Data[2][3], Data[3][3],
        };
      } /* Transposed */

      /**
       * @brief Transposition function
       * 
       * @return (mat &) Self reference
      */
      mat & Transpose( VOID )
      {
        return *this = Transposed();
      } /* Transpose */

      /**
       * @brief Matrix multiplication operator
       * 
       * @param Rhs (const mat &) Matrix to multiply reference.
       * 
       * @return (mat) Multiplied matrix
      */
      mat operator*( const mat &Rhs ) const
      {
        return mat
        {
          Data[0][0] * Rhs.Data[0][0] + Data[0][1] * Rhs.Data[1][0] + Data[0][2] * Rhs.Data[2][0] + Data[0][3] * Rhs.Data[3][0],
          Data[0][0] * Rhs.Data[0][1] + Data[0][1] * Rhs.Data[1][1] + Data[0][2] * Rhs.Data[2][1] + Data[0][3] * Rhs.Data[3][1],
          Data[0][0] * Rhs.Data[0][2] + Data[0][1] * Rhs.Data[1][2] + Data[0][2] * Rhs.Data[2][2] + Data[0][3] * Rhs.Data[3][2],
          Data[0][0] * Rhs.Data[0][3] + Data[0][1] * Rhs.Data[1][3] + Data[0][2] * Rhs.Data[2][3] + Data[0][3] * Rhs.Data[3][3],

          Data[1][0] * Rhs.Data[0][0] + Data[1][1] * Rhs.Data[1][0] + Data[1][2] * Rhs.Data[2][0] + Data[1][3] * Rhs.Data[3][0],
          Data[1][0] * Rhs.Data[0][1] + Data[1][1] * Rhs.Data[1][1] + Data[1][2] * Rhs.Data[2][1] + Data[1][3] * Rhs.Data[3][1],
          Data[1][0] * Rhs.Data[0][2] + Data[1][1] * Rhs.Data[1][2] + Data[1][2] * Rhs.Data[2][2] + Data[1][3] * Rhs.Data[3][2],
          Data[1][0] * Rhs.Data[0][3] + Data[1][1] * Rhs.Data[1][3] + Data[1][2] * Rhs.Data[2][3] + Data[1][3] * Rhs.Data[3][3],

          Data[2][0] * Rhs.Data[0][0] + Data[2][1] * Rhs.Data[1][0] + Data[2][2] * Rhs.Data[2][0] + Data[2][3] * Rhs.Data[3][0],
          Data[2][0] * Rhs.Data[0][1] + Data[2][1] * Rhs.Data[1][1] + Data[2][2] * Rhs.Data[2][1] + Data[2][3] * Rhs.Data[3][1],
          Data[2][0] * Rhs.Data[0][2] + Data[2][1] * Rhs.Data[1][2] + Data[2][2] * Rhs.Data[2][2] + Data[2][3] * Rhs.Data[3][2],
          Data[2][0] * Rhs.Data[0][3] + Data[2][1] * Rhs.Data[1][3] + Data[2][2] * Rhs.Data[2][3] + Data[2][3] * Rhs.Data[3][3],

          Data[3][0] * Rhs.Data[0][0] + Data[3][1] * Rhs.Data[1][0] + Data[3][2] * Rhs.Data[2][0] + Data[3][3] * Rhs.Data[3][0],
          Data[3][0] * Rhs.Data[0][1] + Data[3][1] * Rhs.Data[1][1] + Data[3][2] * Rhs.Data[2][1] + Data[3][3] * Rhs.Data[3][1],
          Data[3][0] * Rhs.Data[0][2] + Data[3][1] * Rhs.Data[1][2] + Data[3][2] * Rhs.Data[2][2] + Data[3][3] * Rhs.Data[3][2],
          Data[3][0] * Rhs.Data[0][3] + Data[3][1] * Rhs.Data[1][3] + Data[3][2] * Rhs.Data[2][3] + Data[3][3] * Rhs.Data[3][3]
        };
      } /* operator*( const mat & ) */

      /**
       * @brief Multiplication with assignment operator.
       * 
       * @param Rhs (const mat &) Matrix to multiply by.
       * 
       * @return (mat &) Self reference.
      */
      mat & operator*=( const mat &Rhs )
      {
        return *this = *this * Rhs;
      } /* operator *= */

      /**
       * @brief Determinant getting operator
       * 
       * @return (component) Determinant of this matrix
      */
      component operator!( VOID ) const
      {
        return
          +Data[0][0] * mat<component, 3, 3>::Determ(Data[1][1], Data[1][2], Data[1][3],
                                                     Data[2][1], Data[2][2], Data[2][3],
                                                     Data[3][1], Data[3][2], Data[3][3]) +

          -Data[0][1] * mat<component, 3, 3>::Determ(Data[1][0], Data[1][2], Data[1][3],
                                                     Data[2][0], Data[2][2], Data[2][3],
                                                     Data[3][0], Data[3][2], Data[3][3]) +

          +Data[0][2] * mat<component, 3, 3>::Determ(Data[1][0], Data[1][1], Data[1][3],
                                                     Data[2][0], Data[2][1], Data[2][3],
                                                     Data[3][0], Data[3][1], Data[3][3]) +

          -Data[0][3] * mat<component, 3, 3>::Determ(Data[1][0], Data[1][1], Data[1][2],
                                                     Data[2][0], Data[2][1], Data[2][2],
                                                     Data[3][0], Data[3][1], Data[3][2]);
      } /* operator! */

      /**
       * @brief Inversed matrix getting function
       * 
       * @return (mat) Inversed self matrix. Identity if determinant is 0.
      */
      mat Inversed( VOID ) const
      {
        component Det = !*this;

        if (Det == 0)
          return Identity();

        return mat
        {
          +mat<component, 3, 3>::Determ(Data[1][1], Data[1][2], Data[1][3],
                                        Data[2][1], Data[2][2], Data[2][3],
                                        Data[3][1], Data[3][2], Data[3][3]) / Det,
          -mat<component, 3, 3>::Determ(Data[0][1], Data[0][2], Data[0][3],
                                        Data[2][1], Data[2][2], Data[2][3],
                                        Data[3][1], Data[3][2], Data[3][3]) / Det,
          +mat<component, 3, 3>::Determ(Data[0][1], Data[0][2], Data[0][3],
                                        Data[1][1], Data[1][2], Data[1][3],
                                        Data[3][1], Data[3][2], Data[3][3]) / Det,
          -mat<component, 3, 3>::Determ(Data[0][1], Data[0][2], Data[0][3],
                                        Data[1][1], Data[1][2], Data[1][3],
                                        Data[2][1], Data[2][2], Data[2][3]) / Det,

          -mat<component, 3, 3>::Determ(Data[1][0], Data[1][2], Data[1][3],
                                        Data[2][0], Data[2][2], Data[2][3],
                                        Data[3][0], Data[3][2], Data[3][3]) / Det,
          +mat<component, 3, 3>::Determ(Data[0][0], Data[0][2], Data[0][3],
                                        Data[2][0], Data[2][2], Data[2][3],
                                        Data[3][0], Data[3][2], Data[3][3]) / Det,
          -mat<component, 3, 3>::Determ(Data[0][0], Data[0][2], Data[0][3],
                                        Data[3][0], Data[3][2], Data[3][3],
                                        Data[1][0], Data[1][2], Data[1][3]) / Det,
          +mat<component, 3, 3>::Determ(Data[0][0], Data[0][2], Data[0][3],
                                        Data[1][0], Data[1][2], Data[1][3],
                                        Data[2][0], Data[2][2], Data[2][3]) / Det,

          +mat<component, 3, 3>::Determ(Data[1][0], Data[1][1], Data[1][3],
                                        Data[2][0], Data[2][1], Data[2][3],
                                        Data[3][0], Data[3][1], Data[3][3]) / Det,
          -mat<component, 3, 3>::Determ(Data[0][0], Data[0][1], Data[0][3],
                                        Data[2][0], Data[2][1], Data[2][3],
                                        Data[3][0], Data[3][1], Data[3][3]) / Det,
          +mat<component, 3, 3>::Determ(Data[0][0], Data[0][1], Data[0][3],
                                        Data[1][0], Data[1][1], Data[1][3],
                                        Data[3][0], Data[3][1], Data[3][3]) / Det,
          -mat<component, 3, 3>::Determ(Data[0][0], Data[0][1], Data[0][3],
                                        Data[1][0], Data[1][1], Data[1][3],
                                        Data[2][0], Data[2][1], Data[2][3]) / Det,

          -mat<component, 3, 3>::Determ(Data[1][0], Data[1][1], Data[1][2],
                                        Data[2][0], Data[2][1], Data[2][2],
                                        Data[3][0], Data[3][1], Data[3][2]) / Det,
          +mat<component, 3, 3>::Determ(Data[0][0], Data[0][1], Data[0][2],
                                        Data[2][0], Data[2][1], Data[2][2],
                                        Data[3][0], Data[3][1], Data[3][2]) / Det,
          -mat<component, 3, 3>::Determ(Data[0][0], Data[0][1], Data[0][2],
                                        Data[1][0], Data[1][1], Data[1][2],
                                        Data[3][0], Data[3][1], Data[3][2]) / Det,
          +mat<component, 3, 3>::Determ(Data[0][0], Data[0][1], Data[0][2],
                                        Data[1][0], Data[1][1], Data[1][2],
                                        Data[2][0], Data[2][1], Data[2][2]) / Det,
        };
      } /* Inversed */

      /**
       * @brief Transformation of vector, like if this matrix is 3x3
       * 
       * @param V (const vec<component, 3> &) Vector to transform
       * 
       * @return (vec<component, 3>) Transformed vector
      */
      vec<component, 3> Transform3x3( const vec<component, 3> &V ) const
      {
        return vec<component, 3>
        {
          V.X * Data[0][0] + V.Y * Data[1][0] + V.Z * Data[2][0],
          V.X * Data[0][1] + V.Y * Data[1][1] + V.Z * Data[2][1],
          V.X * Data[0][2] + V.Y * Data[1][2] + V.Z * Data[2][2],
        };
      } /* Transform3x3 */

      /**
       * @brief Transformation of vector, like if this matrix is 4x3
       * 
       * @param V (const vec<component, 3> &) Vector to transform
       * 
       * @return (vec<component, 3>) Transformed vector
      */
      vec<component, 3> Transform4x3( const vec<component, 3> &V ) const
      {
        return vec<component, 3>
        {
          V.X * Data[0][0] + V.Y * Data[1][0] + V.Z * Data[2][0] + Data[3][0],
          V.X * Data[0][1] + V.Y * Data[1][1] + V.Z * Data[2][1] + Data[3][1],
          V.X * Data[0][2] + V.Y * Data[1][2] + V.Z * Data[2][2] + Data[3][2],
        };
      } /* Transform4x3 */

      /**
       * @brief Transformation of vector
       * 
       * @param V (const vec<component, 3> &) Vector to transform
       * 
       * @return (vec<component, 3>) Transformed vector
      */
      vec<component, 3> Transform4x4( const vec<component, 3> &V ) const
      {
        component W = V.X * Data[0][3] + V.Y * Data[1][3] + V.Z * Data[2][3] + Data[3][3];

        return vec<component, 3>
        {
          (V.X * Data[0][0] + V.Y * Data[1][0] + V.Z * Data[2][0] + Data[3][0]) / W,
          (V.X * Data[0][1] + V.Y * Data[1][1] + V.Z * Data[2][1] + Data[3][1]) / W,
          (V.X * Data[0][2] + V.Y * Data[1][2] + V.Z * Data[2][2] + Data[3][2]) / W,
        };
      } /* Transform4x4 */

      /**
       * @brief On-vector multiplication operator
       * 
       * Simply calls Transform4x4 for V vector
       * 
       * @param V (const vec<component, 3> &) Vector mutliply on
       * 
       * @return (vec<component, 3>) Multiplication result
      */
      vec<component, 3> operator*( const vec<component, 3> &V ) const
      {
        return Transform4x4(V);
      } /* operaor*( const vec3<component> & ) */

      /**
       * @brief Identity matrix building function
       * 
       * @return (mat) Identity matrix
      */
      static mat Identity( VOID )
      {
        return mat
        {
          1, 0, 0, 0,
          0, 1, 0, 0,
          0, 0, 1, 0,
          0, 0, 0, 1,
        };
      } /* Identity */

      /**
       * @brief Rotate by axis matrix building function
       * 
       * @param Angle (component)           Angle to rotate by
       * @param Axis  (vec<component, 3> &) Axis to rotate along
       * 
       * @return (mat) Matrix, that rotates given vector to 'Angle' angle along 'Axis' axis
      */
      static mat Rotate( const component Angle, const vec<component, 3> &Axis )
      {
        vec<component, 3> V = Axis.Normalized();
        component S = std::sin(Angle), C = std::cos(Angle);

        return mat
        {
          V.X * V.X * (1 - C) + C,         V.X * V.Y * (1 - C) - V.Z * S,   V.X * V.Z * (1 - C) + V.Y * S,   0,
          V.Y * V.X * (1 - C) + V.Z * S,   V.Y * V.Y * (1 - C) + C,         V.Y * V.Z * (1 - C) - V.X * S,   0,
          V.Z * V.X * (1 - C) - V.Y * S,   V.Z * V.Y * (1 - C) + V.X * S,   V.Z * V.Z * (1 - C) + C,         0,
          0,                               0,                               0,                               1,
        };
      } /* Rotate */

      /**
       * @brief Rotation along X axis matrix building function
       * 
       * @param Angle (component) Angle to rotate by
       * 
       * @return (mat) Rotation matrix
      */
      static mat RotateX( component Angle )
      {
        component S = std::sin(Angle), C = std::cos(Angle);

        return mat
        {
          1,  0, 0, 0,
          0,  C, S, 0,
          0, -S, C, 0,
          0,  0, 0, 1
        };
      } /* RotateX */

      /**
       * @brief Rotation along Y axis matrix building function
       * 
       * @param Angle (component) Angle to rotate by
       * 
       * @return (mat) Rotation matrix
      */
      static mat RotateY( component Angle )
      {
        component S = std::sin(Angle), C = std::cos(Angle);

        return mat
        {
          C, 0, -S, 0,
          0, 1,  0, 0,
          S, 0,  C, 0,
          0, 0,  0, 1
        };
      } /* RotateY */

      /**
       * @brief Rotation along Z axis matrix building function
       * 
       * @param Angle (component) Angle to rotate by
       * 
       * @return (mat) Rotation matrix
      */
      static mat RotateZ( component Angle )
      {
        component S = std::sin(Angle), C = std::cos(Angle);

        return mat
        {
           C, S, 0, 0,
          -S, C, 0, 0,
           0, 0, 1, 0,
           0, 0, 0, 1
        };
      } /* RotateZ */

      /**
       * @brief Scale matrix building function
       * 
       * @param X (component) X scale coordinate
       * @param Y (component) Y scale coordinate
       * @param Z (component) Z scale coordinate
       * 
       * @return Scale matrix
      */
      static mat Scale( component X, component Y, component Z )
      {
        return mat
        {
          X, 0, 0, 0,
          0, Y, 0, 0,
          0, 0, Z, 0,
          0, 0, 0, 1
        };
      } /* Scale( component, component, component ) */

      /**
       * @brief Scale matrix building function
       *
       * @param ScaleVector (const vec<component, 3> &) Vector to scale by
       * 
       * @return (mat) Scale matrix
      */
      static mat Scale( const vec<component, 3> &ScaleVector )
      {
        return Scale(ScaleVector.X, ScaleVector.Y, ScaleVector.Z);
      } /* Scale( const vec<component, 3> & ) */

      /**
       * @brief Translate matrix building function
       * 
       * @param X (component) X translate coordinate
       * @param Y (component) Y translate coordinate
       * @param Z (component) Z translate coordinate
       * 
       * @return Translate matrix
      */
      static mat Translate( component X, component Y, component Z )
      {
        return mat
        {
          1, 0, 0, 0,
          0, 1, 0, 0,
          0, 0, 1, 0,
          X, Y, Z, 1,
        };
      } /* Translate( component, component, component ) */

      /**
       * @brief Translate matrix building function
       *
       * @param TranslateVector (const vec<component, 3> &) Vector to scale by
       * 
       * @return (mat) Translate matrix
      */
      static mat Translate( const vec<component, 3> &TranslateVector )
      {
        return Translate(TranslateVector.X, TranslateVector.Y, TranslateVector.Z);
      } /* Translate( const vec<component, 3> & ) */

      /**
       * @brief From camera space to NDC projection matrix building function
       * 
       * @param L (component) View space Left border
       * @param R (component) View space Right border
       * @param B (component) View space Bottom border
       * @param T (component) View space Top border
       * @param N (component) View space Near plane
       * @param F (component) View space Far plane
       * 
       * @return (mat) Orthographic projection matrix
      */
      static mat OrthographicProjection( component L, component R, component B, component T, component N, component F )
      {
        return mat
        {
          2 / (R - L),        0,                  0,                  0,
          0,                  -2 / (T - B),       0,                  0,
          0,                  0,                  -2 / (F - N),       0,
          -(R + L) / (R - L), -(T + B) / (T - B), -(F + N) / (F - N), 1
        };
      } /* OrthographicProjection */

      /**
       * @brief From camera space to NDC projection matrix building function
       * 
       * @param L (component) Frustum near rectangle Left coordinate
       * @param R (component) Frustum near rectangle Right coordinate
       * @param B (component) Frustum near rectangle Bottom coordinate
       * @param T (component) Frustum near rectangle Top coordinate
       * @param N (component) Frustum Near plane
       * @param F (component) Frustum Far plane
       * 
       * @return (mat) Frustum projection matrix
      */
      static mat FrustumProjection( component L, component R, component B, component T, component N, component F )
      {
        return mat
        {
          2 * N / (R - L),   0,                 0,                    0,
          0,                 2 * N / (T - B),   0,                    0,
          (R + L) / (R - L), (T + B) / (T - B), -(F + N) / (F - N),  -1,
          0,                 0,                 -2 * N * F / (F - N), 0
        };
      } /* FrustumProjection */

      /**
       * @brief From World space to camera space transformation matrix building function
       * 
       * @param Location (const vec<component, 3> &) Location of camera
       * @param At       (const vec<component, 3> &) Point that camera is looking to now
       * @param ApproxUp (const vec<component, 3> &) Approximately up direction (will be recalculated later, used for right direction finding)
       * 
       * @return (mat) Camera view matrix
      */
      static mat View( const vec<component, 3> &Location, const vec<component, 3> &At, const vec<component, 3> &ApproxUp )
      {
        const vec<component, 3>
          Direction = (At - Location).Normalize(),
          Right     = (Direction % ApproxUp).Normalize(),
          Up        = (Right % Direction).Normalize();

        return mat
        {
          Right.X,             Up.X,             -Direction.X,           0,
          Right.Y,             Up.Y,             -Direction.Y,           0,
          Right.Z,             Up.Z,             -Direction.Z,           0,
          -(Location & Right), -(Location & Up), (Location & Direction), 1
        };
      } /* View */
    }; /* class mat */
} /* namespace anv::math::linalg */

#endif // !defined(ANV_MATH_LINALG_H_)

/* file anv_math_linalg.h */
