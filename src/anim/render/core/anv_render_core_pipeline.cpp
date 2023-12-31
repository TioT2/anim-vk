#include "anv.h"

namespace anv::render::core
{
  /**
   * @brief Pipeline builder implementation
  */
  ANV_BUILDER_IMPL(pipeline)

  vk::DescriptorType TranslateShaderBindingType( pipeline::shader_binding_type Type )
  {
    return std::array
    {
      vk::DescriptorType::eSampledImage,
      vk::DescriptorType::eSampler,
      vk::DescriptorType::eStorageImage,
      vk::DescriptorType::eStorageBuffer,
      vk::DescriptorType::eUniformBuffer,
    }[(UINT32)Type];
  } /* TranslateShaderBindingType */

  vk::PrimitiveTopology TranslateTopology( topology Topology )
  {
    return std::array
    {
      vk::PrimitiveTopology::eTriangleList,
      vk::PrimitiveTopology::eTriangleStrip,
      vk::PrimitiveTopology::eTriangleFan,
      vk::PrimitiveTopology::eLineList,
      vk::PrimitiveTopology::eLineStrip,
      vk::PrimitiveTopology::ePointList,
    }[(UINT32)Topology];
  } /* TranslateTopology */

  vk::CullModeFlags TranslateCullMode( cull_mode_flags CullModeFlags )
  {
    return std::array
    {
      vk::CullModeFlagBits::eNone,
      vk::CullModeFlagBits::eFront,
      vk::CullModeFlagBits::eBack,
      vk::CullModeFlagBits::eFrontAndBack,
    }[(UINT32)CullModeFlags];
  } /* TranslateCullMode */

  vk::PolygonMode TranslatePolygonMode( polygon_mode PolygonMode )
  {
    return std::array
    {
      vk::PolygonMode::eFill,
      vk::PolygonMode::eLine,
      vk::PolygonMode::ePoint,
    }[(UINT32)PolygonMode];
  }

  /**
   * @brief Pipeline building function
   * @param Builder Builder reference
   * @return Pipeline pointer
  */
  pipeline * system::Build( pipeline::builder &Builder )
  {
    pipeline *Result = new pipeline(*this);

    // Copy shader bindings to result
    Result->ShaderBindingTypes = {Builder.ShaderBindingTypes.begin(), Builder.ShaderBindingTypes.end()};

    std::vector<vk::DescriptorSetLayoutBinding> Bindings;
    Bindings.reserve(Builder.ShaderBindingTypes.size());
    UINT32 BindingIndex = 0;
    for (auto BindingType : Builder.ShaderBindingTypes)
      Bindings.push_back(vk::DescriptorSetLayoutBinding()
        .setBinding(BindingIndex++)
        .setDescriptorCount(1)
        .setDescriptorType(TranslateShaderBindingType(BindingType))
        .setStageFlags(vk::ShaderStageFlagBits::eAllGraphics)
      );

    vk::DescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo;
    DescriptorSetLayoutCreateInfo
      .setBindings(Bindings)
      ;

    Result->DescriptorSetLayout = Device.createDescriptorSetLayout(DescriptorSetLayoutCreateInfo);

    vk::PipelineLayoutCreateInfo PipelineLayoutCreateInfo;
    PipelineLayoutCreateInfo
      .setSetLayouts(Result->DescriptorSetLayout)
      ;

    std::vector<vk::PipelineColorBlendAttachmentState> ColorBlendAttachmentStates;

    switch (Builder.RenderPass)
    {
    case render_pass::eMarker:
      ColorBlendAttachmentStates =
      {
        vk::PipelineColorBlendAttachmentState().setBlendEnable(FALSE)
          .setColorWriteMask(vk::ColorComponentFlagBits::eR |
                             vk::ColorComponentFlagBits::eG |
                             vk::ColorComponentFlagBits::eB |
                             vk::ColorComponentFlagBits::eA),
      };
      break;

    case render_pass::eGeometry:
      ColorBlendAttachmentStates =
      {
        vk::PipelineColorBlendAttachmentState().setBlendEnable(vk::False)
          .setColorWriteMask(vk::ColorComponentFlagBits::eR |
                             vk::ColorComponentFlagBits::eG |
                             vk::ColorComponentFlagBits::eB |
                             vk::ColorComponentFlagBits::eA),
        vk::PipelineColorBlendAttachmentState().setBlendEnable(vk::False)
          .setColorWriteMask(vk::ColorComponentFlagBits::eR |
                             vk::ColorComponentFlagBits::eG |
                             vk::ColorComponentFlagBits::eB |
                             vk::ColorComponentFlagBits::eA),
        vk::PipelineColorBlendAttachmentState().setBlendEnable(vk::False)
          .setColorWriteMask(vk::ColorComponentFlagBits::eR |
                             vk::ColorComponentFlagBits::eG |
                             vk::ColorComponentFlagBits::eB |
                             vk::ColorComponentFlagBits::eA),
        vk::PipelineColorBlendAttachmentState().setBlendEnable(vk::False)
          .setColorWriteMask(vk::ColorComponentFlagBits::eR |
                             vk::ColorComponentFlagBits::eG |
                             vk::ColorComponentFlagBits::eB |
                             vk::ColorComponentFlagBits::eA),
      };
      break;

    case render_pass::eOverlay:
      ColorBlendAttachmentStates =
      {
        vk::PipelineColorBlendAttachmentState().setBlendEnable(vk::False)
          .setColorWriteMask(vk::ColorComponentFlagBits::eR |
                             vk::ColorComponentFlagBits::eG |
                             vk::ColorComponentFlagBits::eB |
                             vk::ColorComponentFlagBits::eA),
      };
      break;
    }

    vk::PipelineColorBlendStateCreateInfo ColorBlendState = vk::PipelineColorBlendStateCreateInfo()
      .setAttachments(ColorBlendAttachmentStates)
      ;

    vk::DynamicState DynamicStates[]
    {
      vk::DynamicState::eViewport,
      vk::DynamicState::eScissor,
    };
    vk::PipelineDynamicStateCreateInfo DynamicState;
    DynamicState
      .setDynamicStates(DynamicStates)
      ;

    vk::PipelineInputAssemblyStateCreateInfo InputAssemblyState;
    InputAssemblyState
      .setPrimitiveRestartEnable(vk::False)
      .setTopology(TranslateTopology(Builder.PrimitiveTopology))
      ;

    vk::PipelineRasterizationStateCreateInfo RasterizationState;
    RasterizationState
      .setCullMode(TranslateCullMode(Builder.CullMode))
      .setPolygonMode(TranslatePolygonMode(Builder.PolygonMode))
      ;


    /* Translate vertex attribute layouts */
    std::vector<vk::VertexInputBindingDescription> VertexBindingDescriptions;
    VertexBindingDescriptions.reserve(Builder.VertexBufferLayouts.size());
    for (UINT32 i = 0; i < Builder.VertexBufferLayouts.size(); i++)
    {
      const auto &BufferLayout = Builder.VertexBufferLayouts[i];
      VertexBindingDescriptions.push_back(vk::VertexInputBindingDescription()
        .setBinding(i)
        .setStride(BufferLayout.Stride)
        .setInputRate(std::array
          {
            vk::VertexInputRate::eVertex,
            vk::VertexInputRate::eInstance,
          }[(UINT32)BufferLayout.Rate])
      );
    }

    /* Translate vertex buffer layouts */
    std::vector<vk::VertexInputAttributeDescription> VertexInputAttributeDescriptions;
    VertexInputAttributeDescriptions.reserve(Builder.VertexAttributeLayouts.size());
    for (UINT32 i = 0; i < Builder.VertexAttributeLayouts.size(); i++)
    {
      const auto &AttributeLayout = Builder.VertexAttributeLayouts[i];

      VertexInputAttributeDescriptions.push_back(vk::VertexInputAttributeDescription()
        .setLocation(i)
        .setBinding(AttributeLayout.BufferIndex)
        .setOffset(AttributeLayout.Offset)
        .setFormat(TranslateFormat(AttributeLayout.Format))
      );
    }

    vk::PipelineVertexInputStateCreateInfo VertexInputState;
    VertexInputState
      .setVertexAttributeDescriptions(VertexInputAttributeDescriptions)
      .setVertexBindingDescriptions(VertexBindingDescriptions)
      ;

    vk::PipelineMultisampleStateCreateInfo MultisampleState;
    MultisampleState.setRasterizationSamples(vk::SampleCountFlagBits::e1);

    vk::PipelineViewportStateCreateInfo ViewportState;
    ViewportState
      .setViewportCount(1)
      .setScissorCount(1)
      ;

    vk::ShaderModule VertexModule = Device.createShaderModule(vk::ShaderModuleCreateInfo().setCode(Builder.VertexSPV));
    vk::ShaderModule FragmentModule = Device.createShaderModule(vk::ShaderModuleCreateInfo().setCode(Builder.FragmentSPV));

    vk::PipelineShaderStageCreateInfo ShaderStageCreateInfos[]
    {
      vk::PipelineShaderStageCreateInfo()
        .setModule(VertexModule)
        .setPName("vs_main")
        .setStage(vk::ShaderStageFlagBits::eVertex),
      vk::PipelineShaderStageCreateInfo()
        .setModule(FragmentModule)
        .setPName("fs_main")
        .setStage(vk::ShaderStageFlagBits::eFragment),
    };

    vk::PipelineDepthStencilStateCreateInfo *PDepthStencilState = nullptr;
    vk::PipelineDepthStencilStateCreateInfo DepthStencilState;

    switch(Builder.RenderPass)
    {
    case render_pass::eMarker   :
    case render_pass::eGeometry :
      PDepthStencilState = &DepthStencilState;
      DepthStencilState
        .setDepthCompareOp(vk::CompareOp::eLessOrEqual)
        .setDepthTestEnable(vk::True)
        .setDepthWriteEnable(vk::True)
        ;
      break;
    }

    Result->PipelineLayout = Device.createPipelineLayout(PipelineLayoutCreateInfo);
    vk::GraphicsPipelineCreateInfo PipelineCreateInfo;
    PipelineCreateInfo
      .setLayout(Result->PipelineLayout)
      .setPColorBlendState(&ColorBlendState)
      .setPDynamicState(&DynamicState)
      .setPInputAssemblyState(&InputAssemblyState)
      .setPRasterizationState(&RasterizationState)
      .setPVertexInputState(&VertexInputState)
      .setPMultisampleState(&MultisampleState)
      .setPViewportState(&ViewportState)
      .setPDepthStencilState(PDepthStencilState)
      .setStages(ShaderStageCreateInfos)
      .setRenderPass(OutputRenderPass)
      .setSubpass(GetRenderPassSubpassIndex(Builder.RenderPass))
      ;

    Device.destroyShaderModule(VertexModule);
    Device.destroyShaderModule(FragmentModule);

    vk::Result PipelineCreateResult;
    std::tie(PipelineCreateResult, Result->Pipeline) = Device.createGraphicsPipeline(nullptr, PipelineCreateInfo);

    if (PipelineCreateResult != vk::Result::eSuccess)
    {
      Device.destroyPipelineLayout(Result->PipelineLayout);
      Device.destroyDescriptorSetLayout(Result->DescriptorSetLayout);

      delete Result;
      return nullptr;
    }

    Result->Grab();
    ResourcePool.Add(Result);
    return Result;
  } /* Build */


  /**
   * @brief Resource destroy callback
  */
  VOID pipeline::OnDestroy( VOID )
  {
    System.Device.destroyPipeline(Pipeline);
    System.Device.destroyPipelineLayout(PipelineLayout);
    System.Device.destroyDescriptorSetLayout(DescriptorSetLayout);

    delete this;
  } /* OnDestroy */
} /* namespace anv::render::core */