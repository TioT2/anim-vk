#include "anv.h"

namespace anv::window
{
  VOID system::SdlThreadMain( VOID )
  {
    std::this_thread::sleep_for(std::chrono::duration<FLOAT>(1));
    while (RunSdlThread)
    {
      SdlTaskListMutex.lock();
      if (!SdlThreadTaskList.empty())
      {
        for (auto &&Task : SdlThreadTaskList)
          Task();
        SdlThreadTaskList.clear();
      }
      SdlTaskListMutex.unlock();

      SDL_Event Event;

      while (SDL_PollEvent(&Event))
      {
        switch (Event.type)
        {
        case SDL_QUIT:
          // Finish thread
          RunSdlThread = FALSE;
          RunSdlThread.notify_all();
          break;

        case SDL_WINDOWEVENT:
        {
          WindowPoolMutex.lock();

          if (auto WindowIter = WindowPool.find(Event.window.windowID); WindowIter != WindowPool.end())
          {
            auto [WindowID, Window] = *WindowIter;

            switch (Event.window.event)
            {
            case SDL_WINDOWEVENT_CLOSE:
              Window->Opened = FALSE;
              break;
            }
          }

          WindowPoolMutex.unlock();
          break;
        }

        case SDL_KEYUP:
        case SDL_KEYDOWN:
        {
          WindowPoolMutex.lock();

          if (auto WindowIter = WindowPool.find(Event.window.windowID); WindowIter != WindowPool.end())
            WindowIter->second->KeyPressedStates[Event.key.keysym.scancode] = (Event.type == SDL_KEYDOWN);

          WindowPoolMutex.unlock();
          break;
        }
        }
      }
    }


    for (auto &[Id, Window] : WindowPool)
      SDL_DestroyWindow(Window->Window);
    SDL_Quit();
  } /* SdlThreadMain */


  VOID system::AddSdlThreadTask( std::function<VOID(VOID)> &&Func )
  {
    SdlTaskListMutex.lock();
    SdlThreadTaskList.push_back(Func);
    SdlTaskListMutex.unlock();
  } /* AddSdlThreadTask */

  system::system( VOID )
  {
    RunSdlThread = TRUE;

    AddSdlThreadTask([]()
      {
        SDL_Init(SDL_INIT_VIDEO);
      });
    SdlThread = std::thread([this](){ SdlThreadMain(); });
  } /* system */


  /* Window builder implementation */
  ANV_BUILDER_IMPL(window);

  /**
   * @brief Window builder getting function
   * @return Window builder
  */
  window::builder system::Window( VOID )
  {
    return window::builder(*this);
  } /* Window */

  /**
   * @brief Window building function
   * @param Builder Builder reference
   * @return Created window
  */
  window * system::Build( window::builder &Builder )
  {
    window *Result = nullptr;
    std::atomic_bool BuildCompleted = FALSE;

    AddSdlThreadTask([&]( VOID )
      {
        SDL_Window *SdlWindow = SDL_CreateWindow(Builder.Title.data(), 30, 30, Builder.Extent.W, Builder.Extent.H, SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);
        if (SdlWindow == nullptr)
          std::printf(SDL_GetError());
        window_id WindowID = SDL_GetWindowID(SdlWindow);
        window *Window = new window(*this, WindowID);

        Window->Opened = TRUE;

        WindowPool.emplace(WindowID, Window);

        Window->Window = SdlWindow;

        Result = Window;

        BuildCompleted = TRUE;
        BuildCompleted.notify_all();
      });
    BuildCompleted.wait(FALSE);

    return Result;
  } /* Build */

  VOID system::Close( VOID )
  {
    if (IsClosed)
      return;

    IsClosed = TRUE;
    if (RunSdlThread == TRUE)
    {
      RunSdlThread = FALSE;
      RunSdlThread.notify_all();
    }
    if (SdlThread.joinable())
      SdlThread.join();

    for (auto &[Id, Window] : WindowPool)
      delete Window;
    WindowPool.clear();
  }

  system::~system( VOID )
  {
    Close();
  } /* ~system */

  /**
   * @brief Window identifier getting funciton
  */
  window_id window::GetID( VOID )
  {
    return Id;
  } /* GetID */

  BOOL window::IsOpen( VOID )
  {
    return Opened;
  } /* End of 'IsOpen' function */

  BOOL window::IsKeyPressed( SDL_Keycode Keycode )
  {
    return KeyPressedStates[SDL_GetScancodeFromKey(Keycode)];
  } /* End of 'GetKeyState' function */

  /**
   * @brief Window title setting function
   * @param NewTitle New window title
  */
  VOID window::SetTitle( std::string_view NewTitle )
  {
    Title = NewTitle;
    System.AddSdlThreadTask([this]() { SDL_SetWindowTitle(Window, Title.data()); });
  } /* SetTitle */

  std::string_view window::GetTitle( VOID )
  {
    return Title;
  } /* End of 'GetTitle' function */
} /* namespace anv::window */

#include <SDL2/SDL_syswm.h>

#undef VOID
#undef TRUE
#undef FALSE

namespace anv::window
{
  raw_handle window::GetRawHandle( VOID )
  {
    raw_handle Handle;
    std::atomic_bool FinishFlag = FALSE;

    System.AddSdlThreadTask([this, &Handle, &FinishFlag]()
      {
        SDL_SysWMinfo WMInfo;
        SDL_VERSION(&WMInfo.version);
        SDL_GetWindowWMInfo(Window, &WMInfo);
        if (WMInfo.subsystem != SDL_SYSWM_WINDOWS)
          Handle = raw_handle
          {
            .Unknown = raw_handle::unknown { .Kind = raw_handle::kind::eUnknown, }
          };
        else
          Handle = raw_handle
          {
            .Win32 = raw_handle::win32
            {
              .Kind = raw_handle::kind::eWin32,
              .HWnd = WMInfo.info.win.window,
              .HInstance = WMInfo.info.win.hinstance,
            }
          };
        FinishFlag = TRUE;
        FinishFlag.notify_all();
      });
    FinishFlag.wait(FALSE);

    return Handle;
  } /* End of 'GetRawHandle' function */

} /* namespace anv::window */