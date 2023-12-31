/**
 * @brief       ANIM-VK Project
 * @file        src/anim/render/core/anv_render_core_material.cpp
 * @description Render core material implementation module
 * @last_update 30.12.2023
*/

#include "anv.h"

/**
 * @brief Renderer core namespace
*/
namespace anv::render::core
{
  /**
   * @brief Primitive building function
   * @param Builder Builder to build primitive in
   * @return Created primitive
  */
  primitive * pipeline::Build( primitive::builder &Builder )
  {
    primitive *Result = new primitive(*this);

    Result->IndexBuffer = Builder.IndexBufferView;
    Result->VertexBuffers = {Builder.VertexBufferViews.begin(), Builder.VertexBufferViews.end()};
    Result->Material = Builder.Material;
    Result->IndexBuffer->Grab();
    for (buffer::view *VertexBuffer : Result->VertexBuffers)
      VertexBuffer->Grab();
    Result->Material->Grab();

    System.PrimitivePool.Add(Result);

    return Result;
  } /* Build */

  /**
   * @brief Primitive constructor
   * @param Pipeline Pipeline to use as basis for this primitive
  */
  primitive::primitive( pipeline &Pipeline ) : Pipeline(Pipeline)
  {
    Pipeline.Grab();
  } /* primitive */

  /**
   * @brief Resource destroy callback
  */
  VOID primitive::OnDestroy( VOID )
  {
    if (IndexBuffer != nullptr)
      IndexBuffer->Release();
    for (buffer::view *VertexBuffer : VertexBuffers)
      VertexBuffer->Release();
    Material->Release();

    Pipeline.Release();

    delete this;
  } /* OnDestroy */

  /**
   * @brief Material getting function
   * @return Material, used for this primitive
  */
  material * primitive::GetMaterial( VOID ) const
  {
    return Material;
  } /* GetMaterial */

  /**
   * @brief Material setting function
   * @param NewMaterial New material for this primitive
  */
  VOID primitive::SetMaterial( material *NewMaterial )
  {
    Material = NewMaterial;
  } /* SetMaterial */

  /**
   * @brief Instance create function
  */
  primitive::instance * primitive::Instance( const mat4x4 &Transform )
  {
    instance *Result = new instance(*this, (UINT32)Instances.size());

    Instances.push_back(Result);
    Transforms.push_back(Transform);

    Result->Grab();

    return Result;
  } /* Instance */

  /**
   * @brief Instance destroy callback
  */
  VOID primitive::OnInstanceDestroy( instance *Instance )
  {
    Instances.erase(Instances.begin() + Instance->Index);
    Transforms.erase(Transforms.begin() + Instance->Index);

    // Update indices
    for (UINT32 Index = Instance->Index, N = (UINT32)Instances.size(); Index < N; Index++)
      Instances[Index]->Index = Index;
  } /* OnInstanceDestroy */

  /****
   * Primitive instance implementation
  ***/

  /**
   * @brief Transform getting function
   * @return Current transformation matrix
  */
  mat4x4 primitive::instance::GetTransform( VOID )
  {
    return Primitive.Transforms[Index];
  } /* GetTransform */

  /**
   * @brief Transform setting function
   * @param NewTrasnform New transformation matrix
  */
  VOID primitive::instance::SetTransform( const mat4x4 &NewTransform )
  {
    Primitive.Transforms[Index] = NewTransform;
  } /* SetTrasnform */

  /**
   * @brief Resource destroy callback
  */
  VOID primitive::instance::OnDestroy( VOID )
  {
    Primitive.OnInstanceDestroy(this);

    delete this;
  } /* OnDestroy */
} /* namespace anv::render::core */