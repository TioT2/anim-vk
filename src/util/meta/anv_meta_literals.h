#ifndef ANV_META_LITERALS_H_
#define ANV_META_LITERALS_H_

#include "anv_common.h"

namespace anv
{
  template <std::integral number>
    constexpr number ExtractStringCharacter( const CHAR *Str, SIZE_T StrLength, INT Index )
    {
      return (number)Str[Index] << (Index << 3);
    } /* End of 'ExtractStringCharacter' function */

  constexpr BYTE operator""_uint8( const CHAR *Str, SIZE_T StrLength )
  {
    return Str[0]; // ExtractStringCharacter<BYTE>(Str, StrLength, 0);
  } /* End of 'operator""_byte' function */

  constexpr WORD operator""_uint16( const CHAR *Str, SIZE_T StrLength )
  {
    return ExtractStringCharacter<WORD>(Str, StrLength, 0) |
           ExtractStringCharacter<WORD>(Str, StrLength, 1);
  } /* End of 'operator""_word' function */

  constexpr DWORD operator""_uint32( const CHAR *Str, SIZE_T StrLength )
  {
    return ExtractStringCharacter<DWORD>(Str, StrLength, 0) |
           ExtractStringCharacter<DWORD>(Str, StrLength, 1) |
           ExtractStringCharacter<DWORD>(Str, StrLength, 2) |
           ExtractStringCharacter<DWORD>(Str, StrLength, 3);
  } /* End of 'operator""_dword' function */

  constexpr QWORD operator""_uint64( const CHAR *Str, std::size_t StrLength )
  {
    return ExtractStringCharacter<QWORD>(Str, StrLength, 0) |
           ExtractStringCharacter<QWORD>(Str, StrLength, 1) |
           ExtractStringCharacter<QWORD>(Str, StrLength, 2) |
           ExtractStringCharacter<QWORD>(Str, StrLength, 3) |
           ExtractStringCharacter<QWORD>(Str, StrLength, 4) |
           ExtractStringCharacter<QWORD>(Str, StrLength, 5) |
           ExtractStringCharacter<QWORD>(Str, StrLength, 6) |
           ExtractStringCharacter<QWORD>(Str, StrLength, 7);
  } /* End of 'operator""_qword' function */
}

#endif // !defined(ANV_META_LITERALS_H_)