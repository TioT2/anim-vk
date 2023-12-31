/**
 * @brief       ANIM-VK Project
 * @file        src/util/anv_math_camera.h
 * @description Math support implementation module
 * @last_update 31.12.2023
*/

#ifndef ANV_MATH_CAMERA_H_
#define ANV_MATH_CAMERA_H_

#include "anv_math.h"

/**
 * @brief Math utilities namespace
*/
namespace anv::math::util
{
  class camera
  {
  public:
    /**
      * @brief Projection mode representation enumeration
    */
    enum class projection_mode
    {
      eOrthographic, // Orthographic projection
      ePerspective   // Perspective projection
    }; /* enum projection */

    /* Camera near and far repesentation structure */
    struct projection_parameters
    {
      projection_mode Mode; // Projection mode
      FLOAT
        NearPlaneDistance,  // Camera plane distance
        FarPlaneDistance;   // Camera plane distance
      FLOAT
        UnitWidth,          // Width of projection rectangle
        UnitHeight;         // Height of projection rectangle
    }; /* struct projection_parameters */

    /**
      * @brief Projection parameters getting function
      * @return Actual projection parameters
    */
    projection_parameters GetProjectionParameters( VOID ) const
    {
      return Projection;
    } /* GetProjectionParameters */

    /**
      * @brief Projection parameters setting function
      * @param Projection Projection parameters
    */
    VOID SetProjectionParameters( projection_parameters NewProjection )
    {
      Projection = NewProjection;
      UpdateProjection();
    } /* SetProjectionParameters */

    /**
      * @brief Projection matrices
    */
    struct projection_matrices
    {
      mat4x4<FLOAT> View;       // View matrix
      mat4x4<FLOAT> Projection; // Projection matrix
    }; /* projection_matrices */

    /**
      * @brief Projection matrices getting function
      * @return 
    */
    projection_matrices GetProjectionMatrices( VOID ) const
    {
      return ProjectionMatrices;
    } /* GetProjectionMatrices */

    /**
      * @brief Camera location info
    */
    struct camera_location_info
    {
      vec3<FLOAT> ViewDirection;  // View direction
      vec3<FLOAT> RightDirection; // Right direction
      vec3<FLOAT> UpDirection;    // Up direction

      vec3<FLOAT> CameraLocation;      // Camera location
      vec3<FLOAT> PointOfViewLocation; // Point of view location
    }; /* camera_location_info */

    /**
      * @brief Camera location setting function
      * @param Location New camera location
      * @param PointOfView New camera point of view
      * @param ApproximateUp New camera approximate up direction
    */
    VOID SetCameraLocation( vec3<FLOAT> Location, vec3<FLOAT> PointOfView, vec3<FLOAT> ApproximateUp = vec3<FLOAT>(0, 1, 0) )
    {

    } /* SetCameraLocation */

    /**
      * @brief Camera location getting function
      * @return Info about camera location
    */
    camera_location_info GetCameraLocationInfo( VOID ) const
    {
      return CameraLocationInfo;
    } /* GetCameraLocation */

    /**
      * @brief Camera target frame extent setting function
      * @param NewFrameExtent New frame extent
    */
    VOID SetFrameExtent( extent2<INT> NewFrameExtent )
    {
      FrameExtent = NewFrameExtent;
      UpdateProjection();
    } /* SetFrameExtent */

    /**
      * @brief Camera target frame extent getting function
      * @return Current camera frame extent
    */
    extent2<INT> GetFrameExtent( VOID ) const
    {
      return FrameExtent;
    } /* GetFrameExtent */

  private:
    /**
     * @brief Projection-related parameters update function
    */
    VOID UpdateProjection( VOID )
    {
      FLOAT
        Wp = Projection.UnitWidth,
        Hp = Projection.UnitHeight
        ;

      if (FrameExtent.W > FrameExtent.H)
      {
        Wp *= (FLOAT)FrameExtent.W / FrameExtent.H;
        Hp *= (FLOAT)FrameExtent.W / FrameExtent.H;
      }
      else
      {
        Wp *= (FLOAT)FrameExtent.H / FrameExtent.W;
        Hp *= (FLOAT)FrameExtent.H / FrameExtent.W;
      }

      switch (Projection.Mode)
      {
      case projection_mode::eOrthographic:
        ProjectionMatrices.Projection = mat4x4<FLOAT>::OrthographicProjection(-Wp / 2.0F, Wp / 2.0F, -Hp / 2.0F, Hp / 2.0F, Projection.NearPlaneDistance, Projection.FarPlaneDistance);
        break;
      case projection_mode::ePerspective:
        ProjectionMatrices.Projection = mat4x4<FLOAT>::FrustumProjection(-Wp / 2.0F, Wp / 2.0F, -Hp / 2.0F, Hp / 2.0F, Projection.NearPlaneDistance, Projection.FarPlaneDistance);
        break;
      }
    } /* UpdateProjection */

    /**
     * @brief View-related parameters update function
    */
    VOID UpdateView( VOID )
    {

    } /* UpdateCamera */

    extent2<INT> FrameExtent;                // Extent of current frame
    projection_parameters Projection;            // Projection parameters
    camera_location_info CameraLocationInfo; // Camera location info
    projection_matrices ProjectionMatrices;  // Projection matrices
  }; /* class camera */
} /* namespace anv::math::util */

#endif // !defined(ANV_MATH_CAMERA_H_)

/* file anv_math_camera.h */