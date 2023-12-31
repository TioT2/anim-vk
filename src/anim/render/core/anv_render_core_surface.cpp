/**
 * @brief       ANIM-VK Project
 * @file        src/anim/render/core/anv_render_core_surface.cpp
 * @description Render core surface implementation module
 * @last_update 31.12.2023
*/
#include "anv.h"

namespace anv::render::core
{
  std::pair<vk::Result, vk::SurfaceKHR> CreateWin32Surface( VOID *Instance, VOID *Win32HWnd, VOID *Win32HInstance );
  std::vector<const CHAR *> GetWin32Extensions( VOID );

  VOID system::InitSurface( window::raw_handle &Window )
  {
    switch (Window.Kind)
    {
    case window::raw_handle::kind::eWin32:
      Surface = CreateWin32Surface((VkInstance)Instance, Window.Win32.HWnd, Window.Win32.HInstance).second;
      break;

    default:
      throw std::runtime_error("Platform unsupported");
    }
  } /* InitSurface */

  std::vector<const CHAR *> system::GetRequiredSurfaceExtensions( window::raw_handle &Window )
  {
    switch (Window.Kind)
    {
    case window::raw_handle::kind::eWin32:
      return GetWin32Extensions();

    default:
      throw std::runtime_error("Platform unsupported");
    }
  } /* GetRequiredSurfaceExtensions */
} /* namespace anv::render::core */

#include <windows.h>
#include <vulkan/vulkan_win32.h>

#undef VOID

/**
 * @brief Render core namespace
*/
namespace anv::render::core
{
  /**
   * @brief Win32 surface create function
   * @param Instance Vulkan instance to create interface with
   * @param Win32HWnd Win32 HWND
   * @param Win32HInstance Win32 HINSTANCE
   * @return Surface and it's creation result
  */
  std::pair<vk::Result, vk::SurfaceKHR> CreateWin32Surface( VOID *Instance, VOID *Win32HWnd, VOID *Win32HInstance )
  {
    VkWin32SurfaceCreateInfoKHR SurfaceCreateInfo
    {
      .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
      .hinstance = reinterpret_cast<HINSTANCE>(Win32HInstance),
      .hwnd = reinterpret_cast<HWND>(Win32HWnd),
    };

    VkSurfaceKHR RawSurface;
    return {
      (vk::Result)vkCreateWin32SurfaceKHR(reinterpret_cast<VkInstance>(Instance), &SurfaceCreateInfo, nullptr, &RawSurface),
      (vk::SurfaceKHR)RawSurface
    };
  } /* CreateWin32Surface */

  std::vector<const CHAR *> GetWin32Extensions( VOID )
  {
    return { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME };
  } /* GetWin32Extensions */
} /* namespace anv::render::core */

/* file anv_render_core_surface.cpp */