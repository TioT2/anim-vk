/**
 * @brief       ANIM-VK Project
 * @file        src/anim/anv_anim.h
 * @description Animation system (engine) declaration module
 * @last_update 30.12.2023
*/

#ifndef ANV_ANIM_H_
#define ANV_ANIM_H_

#include "window/anv_window.h"
#include "render/anv_render.h"

/**
 * @brief Animation system context
*/
namespace anv::anim
{
  /* Context for all systems */
  class system_context
  {
  public:
    window::window *MainWindow;   // Main window pointer
    window::system *WindowSystem; // Windowing system pointer
    render::system *RenderSystem; // Rendering system pointer
  }; /* class system_context */

  /* Animation system class */
  class system
  {
    system_context Context; // Subsystem storage
  public:

    /* Constructor */
    system( VOID )
    {
      Context.WindowSystem = new window::system();
      Context.RenderSystem = new render::system();

      Context.MainWindow = Context.WindowSystem->Window()
        .SetTitle("anim-vk")
        .Build();

      Context.RenderSystem->Init(Context.MainWindow->GetRawHandle());
    } /* system */

    /**
     * @brief Context getting funciton
     * @returns Context reference
    */
    system_context & GetContext( VOID )
    {
      return Context;
    } /* GetContext */

    /**
     * @brief Subsystem deinitialization funcitons
    */
    VOID Close( VOID )
    {
      Context.RenderSystem->Close();
      Context.WindowSystem->Close();
    } /* WaitClose */

    /**
     * @brief System destructor
    */
    ~system( VOID )
    {
      delete Context.RenderSystem;
      delete Context.WindowSystem;
    } /* ~system */
  }; /* class system */
} /* namespace anv */

#endif // !defined(ANV_ANIM_H_)