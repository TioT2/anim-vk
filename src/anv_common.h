#ifndef ANV_COMMON_H_
#define ANV_COMMON_H_

/* Debug memory allocation support */ 
#ifndef NDEBUG
#  define _CRTDBG_MAP_ALLOC
#  include <crtdbg.h> 
#  define SetDbgMemHooks() \
     _CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_ALWAYS_DF | \
     _CRTDBG_ALLOC_MEM_DF | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))

namespace anv
{
  /* class for debug memory allocation support */
  class __mem_dummy
  {
  public: 
    /* Class constructor */
    __mem_dummy( void )
    {
      //_CrtSetBreakAlloc(281); -- use for memory leak check
      SetDbgMemHooks(); 
    } /* __mem_dummy */
  }; /* class __mem_dummy */

  static __mem_dummy __MEM_DUMMY; // Memory allocation initialization dummy constructor
} /* namespace anv */

#endif /* _DEBUG */

#ifndef NDEBUG
#  ifdef _CRTDBG_MAP_ALLOC 
#    define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#  endif /* _CRTDBG_MAP_ALLOC */ 
#endif /* _DEBUG */

// Containers
#include <vector>
#include <map>
#include <variant>
#include <deque>
#include <future>
#include <functional>
#include <variant>
#include <span>

// IO
#include <fstream>
#include <chrono>
#include <filesystem>

namespace anv
{
  namespace common_types
  {
    /* Integer types */
    using INT8  = char;      // 1-byte signed int
    using INT16 = short;     // 2-byte signed int
    using INT32 = int;       // 4-byte signed int
    using INT64 = long long; // 8-byte signed int

    /* Unsigned integer types */
    using UINT8  = unsigned char;       // 1-byte unsigned int
    using UINT16 = unsigned short;      // 2-byte unsigned int
    using UINT32 = unsigned int;        // 4-byte unsigned int
    using UINT64 = unsigned long long;  // 8-byte unsigned int

    /* Special type aliases */
    using BYTE  = UINT8;  // UINT8  alias
    using WORD  = UINT16; // UINT16 alias
    using DWORD = UINT32; // UINT32 alias
    using QWORD = UINT64; // UINT64 alias

    /* Platform-dependent integer type aliases */
    using SIZE_T = size_t;       // Value, that can safely be converted into pointer
    using SSIZE_T = ptrdiff_t;   // Value, that can safely be converted into pointer difference
    using INT    = int;          // Platform-preferred integer type
    using UINT   = unsigned int; // Platform-preferred unsigned integer type

    /* Default floating point aliases */
    using FLOAT = float;         // 4-byte float
    using DOUBLE = double;       // 8-byte float
    using LDOUBLE = long double; // float with high precision, actually is double analog

    /* Floating point types */
    using FLOAT32 = FLOAT;  // 4-byte float
    using FLOAT64 = DOUBLE; // 8-byte float
    // using FLOAT80 = LDOUBLE; // hidden)))

    /* Short floating point type aliases */
    using FLT = FLOAT;
    using DBL = DOUBLE;
    using LDBL = LDOUBLE;

    /* Special types */
    using CHAR = char;     // ASCII Character type
    using WCHAR = wchar_t; // UTF-16 Character type
    using VOID = void;     // VOID Type

    /* Boolean type and basic constants */
    using BOOL = bool;            // Boolean type
    constexpr BOOL TRUE = true;   // True constant
    constexpr BOOL FALSE = false; // False constant

    /* Global debug mode flag */
    constexpr BOOL IS_DEBUG =
  #ifndef NDEBUG
      TRUE
  #else
      FALSE
  #endif // !defined(NDEBUG)
      ;
  } /* namespace common_types */

  // Use common types globally in project
  using namespace common_types;
} /* namespace anv */

#endif // !defined(ANV_COMMON_H_)