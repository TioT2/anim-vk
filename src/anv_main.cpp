#include "anv.h"

namespace main
{
  using namespace anv::common_types;

  VOID Main( VOID )
  {
    anv::anim::system Anim;
    auto &AnimContext = Anim.GetContext();

    while (AnimContext.MainWindow->IsOpen() && !AnimContext.MainWindow->IsKeyPressed(SDLK_ESCAPE))
      ;

    Anim.Close();
  } /* End of 'main' function */
}

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

INT WINAPI WinMain( 
    _In_ HINSTANCE HInstance,
    _In_opt_ HINSTANCE HPrevInstance,
    _In_ LPSTR CommandLineArgs,
    _In_ int ShowCommand )
{
  AllocConsole();

  std::freopen("conout$", "w", stdout);
  std::freopen("conin$", "r", stdin);
  std::freopen("conout$", "w", stderr);

  main::Main();

  std::system("pause");
  FreeConsole();

  return 0;
}
