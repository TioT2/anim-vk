#ifndef ANV_META_WEAK_ENUM_H_
#define ANV_META_WEAK_ENUM_H_

#include "anv_common.h"

// Signales that this is flag bits
#define ANV_FLAG_BITS_SIGN _eANVFlagBits

namespace anv
{
  // Enumeration that may have flags
  template <typename enum_type>
    concept flag_bits = std::is_enum_v<enum_type> && requires() { { enum_type::ANV_FLAG_BITS_SIGN }; };

  /**
   * @brief Flags structure
   * @tparam flag_bits Flag bits enumeration
  */
  template <flag_bits bits>
    struct flags
    {
      // Unsigned integral type with same with flag_bits size
      using mask_type = std::underlying_type<bits>::type;

      mask_type Bits; // Flags actual value

      constexpr flags( VOID ) noexcept : Bits(0)
      {

      } /* flags */

      constexpr flags( bits B ) noexcept : Bits(static_cast<mask_type>(B))
      {

      } /* flag_bits */

      constexpr flags( flags<bits> const & F ) noexcept = default;

      constexpr explicit flags( mask_type M ) noexcept : Bits(M)
      {

      }

      constexpr std::strong_ordering operator<=>( flags Rhs ) const noexcept
      {
        return Bits <=> Rhs.Bits;
      } /* operator<=> */

      constexpr flags operator&( flags F ) const noexcept
      {
        return flags(Bits & F.Bits);
      }

      constexpr flags operator|( flags F ) const noexcept
      {
        return flags(Bits | F.Bits);
      } /* operator| */

      constexpr operator BOOL( VOID ) const noexcept
      {
        return Bits != 0;
      } /* operator BOOL */
    }; /* struct flags */

  template <flag_bits bits>
    constexpr flags<bits> operator|( bits Lhs, bits Rhs ) noexcept
    {
      return flags(Lhs) | Rhs;
    }

  template <flag_bits bits>
    constexpr flags<bits> operator&( bits Lhs, bits Rhs ) noexcept
    {
      return flags(Lhs) & Rhs;
    }
} /* namespace anv */

#endif // !defined(ANV_META_WEAK_ENUM_H_)