#ifndef ANV_WINDOW_H_
#define ANV_WINDOW_H_

#include "anv_common.h"

#include "util/meta/anv_meta_builder.h"
#include "util/math/anv_math.h"

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

namespace anv::window
{
  /* Raw window handle class */
  union raw_handle
  {
    /* Window kind enumeration */
    enum class kind
    {
      eWin32,
      eUnknown,
    }; /* enum class kind */

    /* Win32 window handle */
    struct win32
    {
      kind Kind;
      VOID *HWnd;
      VOID *HInstance;
    }; /* struct win32 */

    struct unknown
    {
      kind Kind;
    }; /* struct unknown */

    kind Kind; // Window kind
    win32 Win32;
    unknown Unknown;
  }; /* class raw_handle */

  using window_id = UINT32;

  /* SDL Window */
  class window
  {
    friend class system;

    system &System;
    const window_id Id;

    SDL_Window *Window = nullptr;
    std::string Title = "anim-vk";

    BOOL Opened = FALSE;
    BOOL KeyPressedStates[SDL_NUM_SCANCODES] {FALSE};

    /**
     * @brief Window constructor
     * @param System system reference
     * @param Id Window identifier
    */
    window( system &System, window_id Id ) : System(System), Id(Id)
    {
    } /* window */
  public:

    /**
     * @brief Window builder declaration structure
    */
    ANV_BUILDER_HEAD(window, system)
      ANV_BUILDER_FIELD(std::string_view, Title) {"ANV Window"}; // Window title
      ANV_BUILDER_FIELD(extent2, Extent) {800, 600};             // Window extent
    ANV_BUILDER_END;

    /**
     * @brief Window identifier getting funciton
    */
    window_id GetID( VOID );

    /**
     * @brief Window 
     * @param  
     * @return 
    */
    BOOL IsOpen( VOID );

    BOOL IsKeyPressed( SDL_Keycode Keycode );

    /**
     * @brief Window title setting function
     * @param NewTitle New window title
    */
    VOID SetTitle( std::string_view NewTitle );

    std::string_view GetTitle( VOID );

    raw_handle GetRawHandle( VOID );
  }; /* class window */

  class system
  {
  private:
    friend class window;

    std::thread SdlThread;

    std::mutex SdlTaskListMutex;
    std::deque<std::function<VOID(VOID)>> SdlThreadTaskList;

    std::atomic_bool RunSdlThread;

    std::mutex WindowPoolMutex;
    std::map<window_id, window *> WindowPool;
    BOOL IsClosed = FALSE;

    VOID SdlThreadMain( VOID );

    VOID AddSdlThreadTask( std::function<VOID(VOID)> &&Func );
  public:
    /**
     * @brief System constructor
    */
    system( VOID );

    /**
     * @brief Window builder getting function
     * @return Window builder
    */
    window::builder Window( VOID );

    /**
     * @brief Window building function
     * @param Builder Builder reference
     * @return Created window
    */
    window * Build( window::builder &Builder );

    /* System close funciton */
    VOID Close( VOID );

    /**
     * @brief System destructor
    */
    ~system( VOID );
  }; /* class sytem */
} /* namespace anv_common */

#endif // !defined(ANV_WINDOW_H_)