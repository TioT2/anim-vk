#include "anv.h"

namespace anv::render::core
{
  /**
   * @brief Vulkan debug callback function
   * @param MessageSeverity Message severity
   * @param MessageTypes Message type
   * @param CallbackData Message data (with string)
   * @param UserData User data (this ptr, in this case)
   * @return vk::False (required by specification)
  */
  static VkBool32 VKAPI_PTR DebugCallback_VK(
    VkDebugUtilsMessageSeverityFlagBitsEXT           MessageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT                  MessageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT*      CallbackData,
    void*                                            UserData )
  {
    std::string Fmt = std::format(
      "\n"
      "SEVERITY: {}\n"
      "TYPE: {}\n"
      "MESSAGE: {}\n"
      "\n",
      vk::to_string(vk::DebugUtilsMessageSeverityFlagsEXT(MessageSeverity)),
      vk::to_string(vk::DebugUtilsMessageSeverityFlagsEXT(MessageSeverity)),
      CallbackData->pMessage
    );
    std::printf(Fmt.data());

    return vk::False;
  } /* static DebugCallback */

  system::system( window::raw_handle &Window )
  {
    EnabledInstanceExtensions = GetRequiredSurfaceExtensions(Window);
    EnabledInstanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    EnabledDeviceExtensions =
    {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
    EnabledInstanceLayers =
    {
      "VK_LAYER_KHRONOS_validation",
      "VK_LAYER_RENDERDOC_Capture",
    };

    vk::ApplicationInfo AppInfo;
    AppInfo
      .setApiVersion(vk::ApiVersion13)
      .setPApplicationName("ConsoleApplication")
      .setPEngineName("ConsoleApplication::render::system")
      ;

    auto DebugMessengerCreateInfo = vk::DebugUtilsMessengerCreateInfoEXT()
      .setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eError   |
                          vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
      .setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral              |
                      vk::DebugUtilsMessageTypeFlagBitsEXT::eDeviceAddressBinding |
                      vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance          |
                      vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation)
      .setPfnUserCallback(DebugCallback_VK)
      .setPUserData(this)
      ;

    Instance = vk::createInstance(vk::InstanceCreateInfo()
      .setPApplicationInfo(&AppInfo)
      .setPEnabledLayerNames(EnabledInstanceLayers)
      .setPEnabledExtensionNames(EnabledInstanceExtensions)
      .setPNext(&DebugMessengerCreateInfo)
    );

    DynamicFunctions.vkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(Instance.getProcAddr("vkCreateDebugUtilsMessengerEXT"));
    DynamicFunctions.vkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(Instance.getProcAddr("vkDestroyDebugUtilsMessengerEXT"));

    VkDebugUtilsMessengerEXT CDebugMessenger;
    DynamicFunctions.vkCreateDebugUtilsMessengerEXT(
      Instance,
      &(const VkDebugUtilsMessengerCreateInfoEXT &)DebugMessengerCreateInfo,
      nullptr,
      &CDebugMessenger
    );
    DebugMessenger = CDebugMessenger;

    InitSurface(Window);

    vk::PhysicalDeviceFeatures
      RequiredFeatures,
      DeviceFeatures;

    auto PhysicalDevices = Instance.enumeratePhysicalDevices();
    for (auto &Device : PhysicalDevices)
    {
      auto Props = Device.getProperties();
      auto Features = Device.getFeatures();
      
      if (Features >= RequiredFeatures && Props.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
        PhysicalDevice = Device;
    }
    if (!PhysicalDevice)
      PhysicalDevice = PhysicalDevices[0];
    DeviceFeatures = PhysicalDevice.getFeatures();

    auto QueueFamilyProperties = PhysicalDevice.getQueueFamilyProperties();
    for (UINT i = 0; i < QueueFamilyProperties.size(); i++)
    {
      if (GraphicsQueueFamilyIndex == INVALID_QUEUE_FAMILY_INDEX && (QueueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics) == vk::QueueFlagBits::eGraphics)
        GraphicsQueueFamilyIndex = i;

      if (PresentQueueFamilyIndex == INVALID_QUEUE_FAMILY_INDEX && PhysicalDevice.getSurfaceSupportKHR(i, Surface))
        PresentQueueFamilyIndex = i;
    }

    FLOAT QueuePriorities[] {1.0F, 0.5F, 0.25F};
    std::vector<vk::DeviceQueueCreateInfo> QueueCreateInfos
    {
      vk::DeviceQueueCreateInfo()
        .setQueueFamilyIndex(GraphicsQueueFamilyIndex)
        .setPQueuePriorities(QueuePriorities)
        .setQueueCount(2)
    };
    if (GraphicsQueueFamilyIndex == PresentQueueFamilyIndex)
      QueueCreateInfos[0].queueCount++;
    else
      QueueCreateInfos.push_back(vk::DeviceQueueCreateInfo()
        .setQueueFamilyIndex(PresentQueueFamilyIndex)
        .setPQueuePriorities(QueuePriorities)
        .setQueueCount(1)
      );

    Device = PhysicalDevice.createDevice(vk::DeviceCreateInfo()
      .setPEnabledFeatures(&DeviceFeatures)
      .setPEnabledExtensionNames(EnabledDeviceExtensions)
      .setQueueCreateInfos(QueueCreateInfos)
    );
    GraphicsQueue = Device.getQueue(GraphicsQueueFamilyIndex, 0);
    ComputeQueue = Device.getQueue(GraphicsQueueFamilyIndex, 1);

    if (GraphicsQueueFamilyIndex == PresentQueueFamilyIndex)
      PresentQueue = Device.getQueue(GraphicsQueueFamilyIndex, 2);
    else
      PresentQueue = Device.getQueue(PresentQueueFamilyIndex, 0);

    /* Create memory allocator */
    VmaAllocatorCreateInfo AllocatorCreateInfo
    {
      .physicalDevice = PhysicalDevice,
      .device = Device,
      .instance = Instance,
      .vulkanApiVersion = vk::ApiVersion13,
    };

    if (auto Result = vmaCreateAllocator(&AllocatorCreateInfo, &Allocator); Result != VK_SUCCESS)
      vk::detail::throwResultException(vk::Result(Result), "vmaCreateAllocator");


    auto SurfaceCapabilities = PhysicalDevice.getSurfaceCapabilitiesKHR(Surface);
    SwapchainImageExtent = SurfaceCapabilities.currentExtent;

    Swapchain = Device.createSwapchainKHR(vk::SwapchainCreateInfoKHR()
      .setSurface(Surface)
      .setMinImageCount(SurfaceCapabilities.maxImageCount == UINT32_MAX ? SurfaceCapabilities.minImageCount + 1 : SurfaceCapabilities.maxImageCount)
      .setImageArrayLayers(1)
      .setImageFormat([this]() -> vk::Format
        {
          auto SurfaceFormats = PhysicalDevice.getSurfaceFormatsKHR(Surface);
          SwapchainColorSpace = SurfaceFormats[0].colorSpace;
          SwapchainImageFormat = SurfaceFormats[0].format;
          for (auto Format : SurfaceFormats)
            if (Format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear && Format.format == vk::Format::eB8G8R8A8Srgb)
            {
              SwapchainColorSpace = Format.colorSpace;
              SwapchainImageFormat = Format.format;
            }

          return SwapchainImageFormat;
        }())
      .setImageColorSpace(SwapchainColorSpace)
      .setImageExtent(SwapchainImageExtent)
      .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
      .setImageSharingMode(vk::SharingMode::eExclusive)
      .setPresentMode([this]( VOID ) -> vk::PresentModeKHR
        {
          auto SurfacePresentModes = PhysicalDevice.getSurfacePresentModesKHR(Surface);
          vk::PresentModeKHR PresentMode = SurfacePresentModes[0];
          for (auto Mode : SurfacePresentModes)
            if (Mode == vk::PresentModeKHR::eMailbox)
              PresentMode = Mode;
          return PresentMode;
        }())
      .setPreTransform(SurfaceCapabilities.currentTransform)
    );

    RenderCommandPool = Device.createCommandPool(vk::CommandPoolCreateInfo()
      .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
      .setQueueFamilyIndex(GraphicsQueueFamilyIndex)
    );

    /* Initailize FirstTriangle-level render pass */

    constexpr UINT32 MarkerSubpass = 0;   //      -> Output, Depth
    constexpr UINT32 GeometrySubpass = 1; //      -> GBuf, Depth
    constexpr UINT32 ShadingSubpass = 2;  // GBuf -> Output
    constexpr UINT32 OverlaySubpass = 3;  //      -> Output

    DepthAttachmentFormat = vk::Format::eD32Sfloat;
    OutputAttachmentFormat = SwapchainImageFormat;

    vk::AttachmentDescription AttachmentDescriptions[6]
    {
      /* Position, ObjectID */ vk::AttachmentDescription()
        .setLoadOp(vk::AttachmentLoadOp::eDontCare)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setFormat(PositionObjectIDAttachmentFormat)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal),
      /* NormalXY */ vk::AttachmentDescription()
        .setLoadOp(vk::AttachmentLoadOp::eDontCare)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setFormat(NormalAttachmentFormat)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal),
      /* BaseColorXYZ, AO */ vk::AttachmentDescription()
        .setLoadOp(vk::AttachmentLoadOp::eDontCare)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setFormat(BaseColorAmbientOcclusionAttachmentFormat)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal),
      /* Metallic, Roughness, Instance */ vk::AttachmentDescription()
        .setLoadOp(vk::AttachmentLoadOp::eDontCare)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setFormat(MetallicRoughnessInstanceAttachmentFormat)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal),
      /* Depth */ vk::AttachmentDescription()
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setFormat(DepthAttachmentFormat)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal),
      /* Output */ vk::AttachmentDescription()
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setFormat(OutputAttachmentFormat)
        .setFinalLayout(vk::ImageLayout::ePresentSrcKHR),
    }; /* AttachmentDescriptions */

    vk::AttachmentReference OutputColorAttachmentReference {OutputAttachmentIndex, vk::ImageLayout::eColorAttachmentOptimal};
    vk::AttachmentReference DepthAttachmentReference {DepthAttachmentIndex, vk::ImageLayout::eDepthStencilAttachmentOptimal};

    vk::AttachmentReference GeometryColorAttachmentReferences[]
    {
      {PositionObjectIDAttachmentIndex, vk::ImageLayout::eColorAttachmentOptimal},
      {NormalAttachmentIndex, vk::ImageLayout::eColorAttachmentOptimal},
      {BaseColorAmbientOcclusionAttachmentIndex, vk::ImageLayout::eColorAttachmentOptimal},
      {MetallicRoughnessInstanceAttachmentIndex, vk::ImageLayout::eColorAttachmentOptimal},
    };

    vk::AttachmentReference ShadingInputAttachmentReferences[]
    {
      {PositionObjectIDAttachmentIndex, vk::ImageLayout::eShaderReadOnlyOptimal},
      {NormalAttachmentIndex, vk::ImageLayout::eShaderReadOnlyOptimal},
      {BaseColorAmbientOcclusionAttachmentIndex, vk::ImageLayout::eShaderReadOnlyOptimal},
      {MetallicRoughnessInstanceAttachmentIndex, vk::ImageLayout::eShaderReadOnlyOptimal},
    };

    vk::SubpassDescription Subpasses[]
    {
      /* Marker */ vk::SubpassDescription()
        .setColorAttachments(OutputColorAttachmentReference)
        .setPDepthStencilAttachment(&DepthAttachmentReference),
      /* Geometry */ vk::SubpassDescription()
        .setColorAttachments(GeometryColorAttachmentReferences)
        .setPDepthStencilAttachment(&DepthAttachmentReference),
      /* Shading */ vk::SubpassDescription()
        .setInputAttachments(ShadingInputAttachmentReferences)
        .setColorAttachments(OutputColorAttachmentReference),
      /* Overlay */ vk::SubpassDescription()
        .setColorAttachments(OutputColorAttachmentReference),
    };

    vk::SubpassDependency SubpassDependencies[]
    {
      /* Marker -> Geometry */ vk::SubpassDependency()
        .setSrcSubpass(MarkerSubpass)
        .setDstSubpass(GeometrySubpass)
        .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite)
        .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentRead)
        .setSrcStageMask(vk::PipelineStageFlagBits::eAllGraphics)
        .setDstStageMask(vk::PipelineStageFlagBits::eAllGraphics),
      /* Geometry -> Shading */ vk::SubpassDependency()
        .setSrcSubpass(GeometrySubpass)
        .setDstSubpass(ShadingSubpass)
        .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentRead)
        .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead)
        .setSrcStageMask(vk::PipelineStageFlagBits::eAllGraphics)
        .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eFragmentShader),
      /* Shading -> Overlay */ vk::SubpassDependency()
        .setSrcSubpass(ShadingSubpass)
        .setDstSubpass(OverlaySubpass)
        .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead)
        .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
        .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
        .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput),
    };

    OutputRenderPass = Device.createRenderPass(vk::RenderPassCreateInfo()
      .setAttachments(AttachmentDescriptions)
      .setSubpasses(Subpasses)
      .setDependencies(SubpassDependencies)
    );


    /* Setup attachment images */
    struct
    {
      attachment_image &Result;
      vk::Format Format;
      vk::ImageUsageFlagBits Usage;
    } AttachmentBuildDescriptions[]
    {
      {PositionObjectID,          PositionObjectIDAttachmentFormat         , vk::ImageUsageFlagBits::eColorAttachment       },
      {Normal,                    NormalAttachmentFormat                   , vk::ImageUsageFlagBits::eColorAttachment       },
      {BaseColorAmbientOcclusion, BaseColorAmbientOcclusionAttachmentFormat, vk::ImageUsageFlagBits::eColorAttachment       },
      {MetallicRoughnessInstance, MetallicRoughnessInstanceAttachmentFormat, vk::ImageUsageFlagBits::eColorAttachment       },
      {Depth,                     DepthAttachmentFormat                    , vk::ImageUsageFlagBits::eDepthStencilAttachment},
    };

    for (auto [Result, Format, Usage] : AttachmentBuildDescriptions)
    {
      vk::ImageCreateInfo ImageCreateInfo;
      ImageCreateInfo
        .setExtent(vk::Extent3D(SwapchainImageExtent, 1))
        .setFormat(Format)
        .setUsage(Usage | vk::ImageUsageFlagBits::eInputAttachment)
        .setSharingMode(vk::SharingMode::eExclusive)
        .setImageType(vk::ImageType::e2D)
        .setTiling(vk::ImageTiling::eOptimal)
        .setMipLevels(1)
        .setArrayLayers(1)
        ;

      VmaAllocationCreateInfo AllocationCreateInfo
      {
        .usage = VMA_MEMORY_USAGE_GPU_ONLY,
        .requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      };

      VkImage CImage;
      VmaAllocation Allocation;
      auto ImageCreateResult = vmaCreateImage(Allocator, &(VkImageCreateInfo &)ImageCreateInfo, &AllocationCreateInfo, &CImage, &Allocation, nullptr);

      Result.Image = CImage;
      Result.Allocation = Allocation;

      vk::ImageViewCreateInfo ImageViewCreateInfo;
      ImageViewCreateInfo
        .setFormat(Format)
        .setImage(Result.Image)
        .setViewType(vk::ImageViewType::e2D)
        .setSubresourceRange(vk::ImageSubresourceRange()
          .setLevelCount(1)
          .setLayerCount(1)
          .setAspectMask(Usage == vk::ImageUsageFlagBits::eDepthStencilAttachment
            ? vk::ImageAspectFlagBits::eDepth
            : vk::ImageAspectFlagBits::eColor)
        )
        ;
      Result.View = Device.createImageView(ImageViewCreateInfo);
    }


    auto SwapchainImages = Device.getSwapchainImagesKHR(Swapchain);
    Frames.resize(SwapchainImages.size());

    // Allocate main command buffer
    MainCommandBuffer = Device.allocateCommandBuffers(vk::CommandBufferAllocateInfo()
      .setCommandPool(RenderCommandPool)
      .setCommandBufferCount(1)
    )[0];

    // Allocate subpass command buffers
    std::vector<vk::CommandBuffer> SubpassCommandBuffers = Device.allocateCommandBuffers(vk::CommandBufferAllocateInfo()
      .setCommandPool(RenderCommandPool)
      .setCommandBufferCount(3)
      .setLevel(vk::CommandBufferLevel::eSecondary)
    );
    MarkerCommandBuffer = SubpassCommandBuffers[0];
    GeometryCommandBuffer = SubpassCommandBuffers[1];
    OverlayCommandBuffer = SubpassCommandBuffers[2];

    SwapchainOutputSemaphore = Device.createSemaphore(vk::SemaphoreCreateInfo());
    RenderFinishedFence = Device.createFence(vk::FenceCreateInfo()
      .setFlags(vk::FenceCreateFlagBits::eSignaled)
    );
    ImageAckquiredSemaphore = Device.createSemaphore(vk::SemaphoreCreateInfo());

    for (UINT32 i = 0; i < Frames.size(); i++)
    {
      auto &Frame = Frames[i];

      Frame.SwapchainImage = SwapchainImages[i];
      Frame.SwapchainImageView = Device.createImageView(vk::ImageViewCreateInfo()
        .setFormat(SwapchainImageFormat)
        .setImage(Frame.SwapchainImage)
        .setViewType(vk::ImageViewType::e2D)
        .setSubresourceRange(vk::ImageSubresourceRange()
          .setAspectMask(vk::ImageAspectFlagBits::eColor)
          .setLayerCount(1)
          .setLevelCount(1)
        )
      );
      vk::ImageView ImageViews[]
      {
        PositionObjectID.View,
        Normal.View,
        BaseColorAmbientOcclusion.View,
        MetallicRoughnessInstance.View,
        Depth.View,
        Frame.SwapchainImageView,
      };

      Frame.Framebuffer = Device.createFramebuffer(vk::FramebufferCreateInfo()
        .setAttachments(ImageViews)
        .setHeight(SwapchainImageExtent.height)
        .setWidth(SwapchainImageExtent.width)
        .setRenderPass(OutputRenderPass)
        .setLayers(1)
      );
    }

    // Start rendering
    DoRender = TRUE;
    RenderThread = std::thread([this]( VOID ) { StartRendering(); });
  } /* kernel */

  system::~system( VOID )
  {
    DoRender = FALSE;
    DoRender.notify_one();
    RenderThread.join();

    Device.waitIdle();

    PrimitivePool.Clear();
    ResourcePool.Clear();

    /* Destroy frame contexts */
    for (auto &Frame : Frames)
    {
      Device.destroyFramebuffer(Frame.Framebuffer);
      Device.destroyImageView(Frame.SwapchainImageView);
    }

    // Destory target
    attachment_image * AttachmentImages[]
    {
      &PositionObjectID,
      &Normal,
      &BaseColorAmbientOcclusion,
      &MetallicRoughnessInstance,
      &Depth,
    };
    for (auto &Img : AttachmentImages)
    {
      Device.destroyImageView(Img->View);
      vmaDestroyImage(Allocator, Img->Image, Img->Allocation);
    }

    Device.destroySemaphore(ImageAckquiredSemaphore);
    Device.destroyFence(RenderFinishedFence);
    Device.destroySemaphore(SwapchainOutputSemaphore);

    Device.destroyCommandPool(RenderCommandPool);

    Device.destroyRenderPass(OutputRenderPass);

    vmaDestroyAllocator(Allocator);

    Device.destroySwapchainKHR(Swapchain);
    Device.destroy();
    Instance.destroySurfaceKHR(Surface);
    DynamicFunctions.vkDestroyDebugUtilsMessengerEXT(Instance, DebugMessenger, nullptr);
    Instance.destroy();
  } /* ~system */

  /**
    * @brief Frame rendering function, working in another thread
  */
  VOID system::StartRendering( VOID )
  {
    while (DoRender)
    {
      auto WaitResult = Device.waitForFences(RenderFinishedFence, vk::True, UINT64_MAX);

      // GC pass
      if (GlobalFrameIndex % 1000 == 0)
      {
        Device.waitIdle();

        PrimitivePool.CollectGarbage();
        ResourcePool.CollectGarbage();
      }

      vk::Result Result;
      UINT32 Index;
      try
      {
        auto Pair = Device.acquireNextImageKHR(Swapchain, UINT64_MAX, ImageAckquiredSemaphore);
        Result = Pair.result;
        Index = Pair.value;
      }
      catch (vk::OutOfDateKHRError &)
      {
        DoRender = FALSE;
        return;
      }
      auto Frame = Frames[Index];

      Device.resetFences(RenderFinishedFence);

      // Fill up marker, geometry and overlay command buffers
      MarkerCommandBuffer.reset();
      GeometryCommandBuffer.reset();
      OverlayCommandBuffer.reset();

      MarkerCommandBuffer.begin(vk::CommandBufferBeginInfo());
      GeometryCommandBuffer.begin(vk::CommandBufferBeginInfo());
      OverlayCommandBuffer.begin(vk::CommandBufferBeginInfo());

      vk::CommandBuffer *CommandBuffers[]
      {
        &MarkerCommandBuffer,
        &GeometryCommandBuffer,
        &OverlayCommandBuffer,
      };

      // Write pass command buffers
      for (primitive *Primitive : PrimitivePool)
        if (!Primitive->Instances.empty())
        {


          switch (Primitive->Pipeline.RenderPass)
          {
          case render_pass::eMarker:
            break;
          case render_pass::eGeometry:
            break;
          case render_pass::eOverlay:
            break;
          }
          if (Primitive->IndexBuffer != nullptr)
            ;
        }

      MarkerCommandBuffer.end();
      GeometryCommandBuffer.end();
      OverlayCommandBuffer.end();

      MainCommandBuffer.reset();

      MainCommandBuffer.begin(vk::CommandBufferBeginInfo());

      /* Bake lighting depthmaps */
      FLOAT Time = std::chrono::duration_cast<std::chrono::duration<FLOAT>>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

      // Shadow passes somewhere here

      // Fill secondary command buff

      vk::ClearValue OutputClearValues[]
      {
        /* PositionObjectID          */ {{INFINITY, INFINITY, INFINITY, INFINITY}},
        /* Normal                    */ {{0.00F, 0.01F, 0.00F, 0.00F}},
        /* BaseColorAmbientOcclusion */ {{0.30F, 0.47F, 0.80F, 1.0F}},
        /* MetallicRoughnessInstance */ {{0.5F, 0.5F, 0.0F, 1.0F}},
        /* Depth                     */ {1.0F},
        /* Output                    */ {{0.30F, 0.47F, 0.80F, 1.00F}},
      };

      // Marker pass
      MainCommandBuffer.beginRenderPass(vk::RenderPassBeginInfo()
        .setFramebuffer(Frame.Framebuffer)
        .setRenderPass(OutputRenderPass)
        .setClearValues(OutputClearValues)
        .setRenderArea(vk::Rect2D({0, 0}, SwapchainImageExtent)),
        vk::SubpassContents::eSecondaryCommandBuffers
      );
      MainCommandBuffer.executeCommands(MarkerCommandBuffer);

      // Geometry pass
      MainCommandBuffer.nextSubpass(vk::SubpassContents::eSecondaryCommandBuffers);
      MainCommandBuffer.executeCommands(GeometryCommandBuffer);

      // Shading pass
      MainCommandBuffer.nextSubpass(vk::SubpassContents::eInline);

      /* Do lighting and so on */

      // Overlay pass
      MainCommandBuffer.nextSubpass(vk::SubpassContents::eSecondaryCommandBuffers);
      MainCommandBuffer.executeCommands(OverlayCommandBuffer);


      MainCommandBuffer.endRenderPass();
      MainCommandBuffer.end();

      vk::PipelineStageFlags WaitStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
      GraphicsQueue.submit(vk::SubmitInfo()
        .setCommandBuffers(MainCommandBuffer)
        .setWaitSemaphores(ImageAckquiredSemaphore)
        .setWaitDstStageMask(WaitStageMask)
        .setSignalSemaphores(SwapchainOutputSemaphore),
        RenderFinishedFence
      );

      try
      {
        auto PresentResult = GraphicsQueue.presentKHR(vk::PresentInfoKHR()
          .setWaitSemaphores(SwapchainOutputSemaphore)
          .setSwapchains(Swapchain)
          .setImageIndices(Index)
        );
      }
      catch (vk::OutOfDateKHRError &)
      {
        DoRender = FALSE;
        return;
      }

      GlobalFrameIndex++;
    }
  } /* StartRendering */

  /**
   * @brief Render pass subpass index getting function
   * @param Pass Render pass to get subpass index of
   * @return Subpass index
  */
  UINT32 system::GetRenderPassSubpassIndex( render_pass Pass )
  {
    return std::array{0, 1, 3}[(UINT32)Pass];
  } /* GetRenderPassSubpassIndex */
} /* namespace anv::render::core */
