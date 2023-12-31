/**
 * @brief       ANIM-VK Project
 * @file        src/anim/render/core/anv_render_core_material.cpp
 * @description Render core material implementation module
 * @last_update 30.12.2023
*/

#include "anv.h"

/**
 * @brief Render core namespace
*/
namespace anv::render::core
{
  /**
   * @brief Material building function
   * @param Builder Builder to build material in
   * @return Created material
  */
  material * pipeline::Build( material::builder &Builder )
  {
    material *Result = new material(*this);

    // Sizes for all kinds of attachments
    std::vector<vk::DescriptorPoolSize> PoolSizes;
    PoolSizes.reserve(ShaderBindingTypes.size());

    // Calculate pool sizes
    for (auto BindingType : ShaderBindingTypes)
      PoolSizes.push_back(vk::DescriptorPoolSize()
        .setType(TranslateShaderBindingType(BindingType))
        .setDescriptorCount(1)
      );

    // Create pool for exactly one set
    vk::DescriptorPoolCreateInfo DescriptorPoolCreateInfo;
    DescriptorPoolCreateInfo
      .setPoolSizes(PoolSizes)
      .setMaxSets(1)
      ;
    Result->DescriptorPool = System.Device.createDescriptorPool(DescriptorPoolCreateInfo);

    vk::DescriptorSetAllocateInfo DescriptorSetAllocateInfo;
    DescriptorSetAllocateInfo
      .setDescriptorPool(Result->DescriptorPool)
      .setDescriptorSetCount(1)
      .setSetLayouts(DescriptorSetLayout)
      ;
    Result->DescriptorSet = System.Device.allocateDescriptorSets(DescriptorSetAllocateInfo)[0];

    std::vector<vk::WriteDescriptorSet> DescriptorWrites;
    std::vector<vk::DescriptorBufferInfo> BufferInfos;
    std::vector<vk::DescriptorImageInfo> ImageInfos;
    DescriptorWrites.reserve(Builder.AttachedResources.size());

    // Write descriptor sets

    for (UINT32 bi = 0; bi < ShaderBindingTypes.size(); bi++)
    {
      auto BindingType = ShaderBindingTypes[bi];

      DescriptorWrites.push_back(vk::WriteDescriptorSet()
        .setDstBinding(bi)
        .setDescriptorType(TranslateShaderBindingType(BindingType))
      );

      switch (BindingType)
      {
      case pipeline::shader_binding_type::eSampledImage:
      {
        auto Sampler = Builder.AttachedResources[bi].Sampler;
        auto ImageView = Builder.AttachedResources[bi].ImageView;

        ImageInfos.push_back(vk::DescriptorImageInfo()
          .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
          .setImageView(ImageView->ImageView)
          .setSampler(Sampler->Sampler)
        );

        DescriptorWrites.back().setImageInfo(ImageInfos.back());

        Sampler->Grab();
        ImageView->Grab();
        Result->AttachedResources.push_back(Sampler);
        Result->AttachedResources.push_back(ImageView);

        break;
      }
      case pipeline::shader_binding_type::eSampler:
      {
        sampler *Sampler = static_cast<sampler *>(Builder.AttachedResources[bi].Resource);

        ImageInfos.push_back(vk::DescriptorImageInfo()
          .setSampler(Sampler->Sampler)
        );

        DescriptorWrites.back().setImageInfo(ImageInfos.back());

        break;
      }
      case pipeline::shader_binding_type::eStorageImage:
      {
        image::view *ImageView = static_cast<image::view *>(Builder.AttachedResources[bi].Resource);

        ImageInfos.push_back(vk::DescriptorImageInfo()
          .setImageLayout(vk::ImageLayout::eGeneral)
          .setImageView(ImageView->ImageView)
        );

        DescriptorWrites.back().setImageInfo(ImageInfos.back());

        break;
      }

      case pipeline::shader_binding_type::eStorageBuffer :
      case pipeline::shader_binding_type::eUniformBuffer :
      {
        buffer::view *Buffer = static_cast<buffer::view *>(Builder.AttachedResources[bi].Resource);

        BufferInfos.push_back(vk::DescriptorBufferInfo()
          .setBuffer(Buffer->View)
          .setOffset(0)
          .setRange(Buffer->Size)
        );

        DescriptorWrites.back().setBufferInfo(BufferInfos.back());

        break;
      }
      }
    }

    System.Device.updateDescriptorSets(DescriptorWrites, {});

    // Grab attached resources
    Result->AttachedResources = {Builder.AttachedResources.begin(), Builder.AttachedResources.end()};
    for (auto &Resource : Result->AttachedResources)
      Resource.Grab();

    System.ResourcePool.Add(Result);

    return Result;
  } /* Build */

  /**
   * @brief Material constructor
   * @param Pipeline Pipeline pointer
  */
  material::material( pipeline &Pipeline ) : Pipeline(Pipeline)
  {
    Pipeline.Grab();
  } /* Material */

  /**
   * @brief Resource destroy callback
  */
  VOID material::OnDestroy( VOID )
  {
    // Release attached resources
    for (auto &Resource : AttachedResources)
      Resource.Release();

    // Destroy descriptor pool
    Pipeline.System.Device.destroyDescriptorPool(DescriptorPool);

    Pipeline.Release();

    delete this;
  } /* OnDestroy */
} /* namespace anv::render::core */