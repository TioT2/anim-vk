#ifndef ANV_RENDER_H_
#define ANV_RENDER_H_

#include "core/anv_render_core.h"

namespace anv::render
{
  /* Rendering system */
  class system
  {
    BOOL IsClosed = FALSE;
    core::system *Core = nullptr; // Render core
  public:

    system( VOID )
    {

    } /* End of 'system' function */

    /**
     * @brief Initialization function
     * @param RawWindowHandle Raw window handle
     * @return 
    */
    VOID Init( window::raw_handle RawWindowHandle )
    {
      Core = new core::system(RawWindowHandle);
    } /* End of 'Init' funciton */

    VOID Close( VOID )
    {
      delete Core;
      IsClosed = TRUE;
    } /* Close */

    ~system( VOID )
    {
      if (!IsClosed)
        Close();
    } /* ~system */
  }; /* class system */
} /* namespace anv::render */

#endif // !defined(ANV_RENDER_H_)