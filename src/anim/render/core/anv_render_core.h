/**
 * @brief       ANIM-VK Project
 * @file        src/anim/render/core/anv_render_core.h
 * @description Render core declaration module
 * @last_update 31.12.2023
*/

#ifndef ANV_RENDER_CORE_H_
#define ANV_RENDER_CORE_H_

#include "../../window/anv_window.h"

#include "util/meta/anv_meta_literals.h"
#include "util/meta/anv_meta_concepts.h"
#include "util/meta/anv_meta_flags.h"

#include "util/meta/anv_meta_builder.h"

#include "util/resource/anv_resource_rc.h"
#include "util/math/anv_math.h"

#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>

/**
 * @brief Project namespace
*/
namespace anv::render::core
{
  class system;

  /* Format enumeration */
  struct format
  {
    /* Component type */
    enum class type : BYTE
    {
      eU8,      // UINT8
      eU16,     // UINT16
      eU32,     // UINT32

      eU8Norm,  // Normalized UINT8
      eU16Norm, // Normalized UINT16

      eU8Srgb,  // Srgb UINT8

      eI8,      // INT8
      eI16,     // INT16
      eI32,     // INT32

      eI8Norm,  // Normalized INT8
      eI16Norm, // Normalized INT16
      eI32Norm, // Normalized INT32

      eF16,     // FLOAT16
      eF32,     // FLOAT32

      _eCount,  // Helper
    }; /* enum type */

    type Type = type::eU8Srgb; // Format component type
    UINT8 Count = 4;           // Format component count
  }; /* struct format */

  /**
   * @brief Format translate function
   * @param Format ANV Format to translate
   * @return Vulkan format
  */
  vk::Format TranslateFormat( format Format );

  /* Render pass indexing structure */
  enum class render_pass
  {
    eMarker,   // Non-shaded subpass
    eGeometry, // Main geometry pass
    eOverlay,  // UI and other overlay pass
  }; /* enum render_pass */

  /**
   * @brief Primitive topology
  */
  enum class topology
  {
    eTriangleList,  // List of triangles  0 1 2 3 4 5 -> (0 1 2) (3 4 5)
    eTriangleStrip, // Strip of triangles 0 1 2 3 4 5 -> (0 1 2) (1 2 3) (2 3 4) (3 4 5)
    eTriangleFan,   // Fan of triangles   0 1 2 3 4 5 -> (0 1 2) (0 2 3) (0 3 4) (0 4 5)

    eLineList,      // List of lines      0 1 2 3 -> (0 1) (2 3)
    eLineStrip,     // Strip of lines     0 1 2 3 -> (0 1) (1 2) (2 3)

    ePointList,     // List of points     0 1 -> (0) (1)
  }; /* enum topology */

  /**
   * @brief Culling mode enumeration
  */
  enum class cull_mode
  {
    eFront = 0x1, // Cull front face
    eBack  = 0x2, // Cull back face

    ANV_FLAG_BITS_SIGN
  }; /* enum cull_mode */

  // Cull mode flags
  using cull_mode_flags = flags<cull_mode>;

  /**
   * @brief Polygon mode enumeration
  */
  enum class polygon_mode
  {
    eFill,  // Fill polygon
    eLine,  // Represent polygon as line set
    ePoint, // Represent polygon as it's central point
  }; /* enum polygon_modes */

  /* Image representation class */
  class image : public rc::resource
  {
  public:
    enum class usage
    {
      eSampled = 0x1, // Image, used as rendering attachment
      eStorage = 0x2, // Image, used as multidimensional buffer

      ANV_FLAG_BITS_SIGN
    }; /* enum usage */

    using usage_flags = flags<usage>; // Image usage flags

    /**
     * @brief Image builder structure
    */
    ANV_BUILDER_HEAD(image, system)
      ANV_BUILDER_FIELD(usage_flags, Usage) {0}; // Image usage
      ANV_BUILDER_FIELD(extent2, Extent) {0, 0}; // Image extent
      ANV_BUILDER_FIELD(UINT, MipLevels) = 1;    // MipMap level count
      ANV_BUILDER_FIELD(format, Format);         // Image format
    ANV_BUILDER_END; /* struct builder */

    /**
     * @brief Extent getting funciton
     * @return Image current extent
    */
    extent2 GetExtent( VOID ) const
    {
      return Extent;
    } /* GetExtent */

    /**
     * @brief Image view representation class
    */
    class view : public rc::resource
    {
    public:
      /**
       * @brief Image component swizzle values
      */
      enum class component_swizzle
      {
        eIdentity, // Use component as-is
        eR,        // Use R component instead
        eG,        // Use G component instead
        eB,        // Use B component instead
        eA,        // Use A component instead
      }; /* component_swizzle */

      /**
       * @brief Image view builder representation structure
      */
      ANV_BUILDER_HEAD(view, image)
        ANV_BUILDER_FIELD(format, Format);                                             // View format
        ANV_BUILDER_FIELD(component_swizzle, SwizzleR) = component_swizzle::eIdentity; // R color component destination swizzle
        ANV_BUILDER_FIELD(component_swizzle, SwizzleG) = component_swizzle::eIdentity; // G color component destination swizzle
        ANV_BUILDER_FIELD(component_swizzle, SwizzleB) = component_swizzle::eIdentity; // B color component destination swizzle
        ANV_BUILDER_FIELD(component_swizzle, SwizzleA) = component_swizzle::eIdentity; // A color component destination swizzle
        ANV_BUILDER_FIELD(UINT32, BaseMipLevel) = 0;                                   // Subimage base mip level
        ANV_BUILDER_FIELD(UINT32, MipLevelCount) = 1;                                  // Subimage mip level count

        /**
         * @brief Image view builder constructor
         * @param Image Parent image reference
         * @param Format subimage format
        */
        builder( image &Image, format Format ) : System(Image), Format(Format)
        {

        } /* builder */
      ANV_BUILDER_END;

    private:
      friend class material;
      friend class pipeline;

      friend class image;

      image &Image;            // Owner image
      vk::ImageView ImageView; // Image view

      /**
       * @brief Image view constructor
       * @param Image Base image reference
      */
      view( image &Image ) : Image(Image)
      {

      } /* view */

      /**
       * @brief Resource destroy callback
      */
      VOID OnDestroy( VOID ) override;
    }; /* class view */

    /**
     * @brief Image view create function
    */
    view::builder View( VOID )
    {
      return view::builder(*this, Format);
    } /* View */

  private:
    /**
     * @brief View building function
    */
    view * Build( view::builder &Builder );

    friend class system;
    friend class view;

    system &System;

    /**
     * @brief Image constructor
     * @param System System pointer
    */
    image( system *System ) : System(*System)
    {

    } /* image */

    usage_flags Usage;        // Usage
    extent2 Extent;           // Image extent
    format Format;            // Format
    vk::Format FormatVK;      // Format
    vk::Image Image;          // Vulkan image
    VmaAllocation Allocation; // Image memory

    /**
     * @brief Resource destroy callback
    */
    VOID OnDestroy( VOID ) override;
  }; /* class image */

  /* Texture sampler create function */
  class sampler : public rc::resource
  {
  public:
    /* Addressing mode */
    enum class address_mode
    {
      eRepeat,         // Repeat image
      eMirroredRepeat, // Mirrored repeat image
      eClampToBorder,  // Clamp image to border (some another color otherwise)
      eClampToEdge     // Clamp image to edge   (image edge color otherwise)
    }; /* enum address_mode */

    /* Texture filter */
    enum class filter
    {
      eNearest, // Nearest, sharp edges
      eLinear,  // Linear, not sharp
    }; /* enum filter */

    /**
     * @brief Sampler builder decleration.
    */
    ANV_BUILDER_HEAD(sampler, system)
      ANV_BUILDER_FIELD(address_mode, AddressModeU) = address_mode::eRepeat;
      ANV_BUILDER_FIELD(address_mode, AddressModeV) = address_mode::eRepeat;
      ANV_BUILDER_FIELD(filter, MinFilter) = filter::eLinear;
      ANV_BUILDER_FIELD(filter, MagFilter) = filter::eLinear;
    ANV_BUILDER_END;

  private:
    friend class pipeline;
    friend class material;
    friend class system;

    /**
     * @brief Sampler constructor
     * @param System Owner system pointer
    */
    sampler( system *System ) : System(*System)
    {

    } /* sampler */

    system &System;      // System reference
    vk::Sampler Sampler; // Vulkan sampler reference

    /**
     * @brief Resource destroy callback
    */
    VOID OnDestroy( VOID ) override;
  }; /* class sampler */

  /* Buffer representation class */
  class buffer : public rc::resource
  {
  public:
    /* buffer usage flags */
    enum class usage : UINT32
    {
      eVertex  = 0x1, // Vertex
      eIndex   = 0x2, // Index
      eUniform = 0x4, // Uniform
      eStorage = 0x8, // Storage

      ANV_FLAG_BITS_SIGN,
    }; /* enum usage */

    /* Buffer usage  */
    using usage_flags = flags<usage>;

    /**
     * @brief Buffer builder structure
    */
    ANV_BUILDER_HEAD(buffer, system)
      ANV_BUILDER_FIELD(SIZE_T, Size) = 0;                    // Buffer size
      ANV_BUILDER_FIELD(usage_flags, Usage) = usage_flags(0); // Buffer usage
    ANV_BUILDER_END;

    /* Buffer view representation class */
    class view : public rc::resource
    {
    public:
      /**
       * @brief View builder structure
      */
      ANV_BUILDER_HEAD(view, buffer)
        ANV_BUILDER_FIELD(SIZE_T, Offset) = 0;                  // Buffer offset
        ANV_BUILDER_FIELD(SIZE_T, Size) = 0;                    // Buffer size
        ANV_BUILDER_FIELD(usage_flags, Usage) = usage_flags(0); // Buffer usage
      ANV_BUILDER_END;

    private:
      friend class system;
      friend class buffer;
      friend class material;
      friend class pipeline;

      /**
       * @brief View create function
       * @param Buffer Parent buffer reference
       * @param Offset Offset from parent buffer memory
       * @param Size Size of view
      */
      view( buffer *Buffer, SIZE_T Offset, SIZE_T Size );

      buffer &Buffer;      // Parent buffer reference
      SIZE_T Offset, Size; // Offset and size of view
      vk::Buffer View;     // Buffer view itself

      /**
       * @brief Resource destroy callback
      */
      VOID OnDestroy( VOID ) override;
    }; /* class view */

  private:
    /**
     * @brief View building function
     * @param Builder Builder reference
     * @return Created view pointer
    */
    view * Build( view::builder &Builder );
  public:

    /**
     * @brief View builder getting function
     * @return View builder
    */
    view::builder View( VOID )
    {
      return view::builder(*this);
    } /* View */

  private:
    friend class buffer_view;
    friend class system;

    system &System;
    SIZE_T Size;
    usage_flags Usage;
    vk::BufferUsageFlags UsageFlags; // Vulkan buffer usage flags
    VmaAllocation Memory;            // Vulkan memory allocation
    vk::Buffer Buffer;               // Vulkan buffer

    /**
     * @brief Buffer constructor
     * @param System System pointer
    */
    buffer( system *System );

    /**
     * @brief Resource destroy callback
    */
    VOID OnDestroy( VOID ) override;
  }; /* class buffer */

  /**
   * @brief Pipeline forward declaration
  */
  class pipeline;

  class material;
  class primitive;

  /**
   * @brief Primitive representation class
  */
  class primitive : public rc::resource
  {
  public:
    /**
     * @brief Primitive builder representation structure
    */
    ANV_BUILDER_HEAD(primitive, pipeline)
      ANV_BUILDER_FIELD(std::span<buffer::view *>, VertexBufferViews); // Buffer views
      ANV_BUILDER_FIELD(buffer::view *, IndexBufferView) = nullptr;    // Index buffer pointer
      ANV_BUILDER_FIELD(material *, Material) = nullptr;               // Material pointer
    ANV_BUILDER_END;

    /**
     * @brief Material getting function
     * @return Material, used for this primitive
    */
    material * GetMaterial( VOID ) const;

    /**
     * @brief Material setting function
     * @param NewMaterial New material for this primitive
    */
    VOID SetMaterial( material *NewMaterial );

    /**
     * @brief Primitive instance representation structure
    */
    class instance : public resource
    {
    public:

      /**
       * @brief Transform getting function
       * @return Current transformation matrix
      */
      mat4x4 GetTransform( VOID );

      /**
       * @brief Transform setting function
       * @param NewTransform New transformation matrix
      */
      VOID SetTransform( const mat4x4 &NewTransform );

    private:
      friend class primitive;

      /**
       * @brief Instance constructor
       * @param Primitive Primitive reference
       * @param Index Index in matrix pool
      */
      instance( primitive &Primitive, UINT32 Index ) : Primitive(Primitive), Index(Index)
      {

      } /* Instance */

      primitive &Primitive;     // Primitive
      UINT32 Index = 0;         // Matrix index

      /**
       * @brief Resource destroy callback
      */
      VOID OnDestroy( VOID ) override;
    }; /* instance */

    /**
     * @brief Instance create function
    */
    instance * Instance( const mat4x4 &Transform = mat4x4::Identity() );

  public:
    pipeline &Pipeline;                         // Primitive parent pipeline, must be matched with pipeline one.

    material *Material = nullptr;               // Material pointer
    buffer::view *IndexBuffer = nullptr;        // Index buffer list
    std::vector<buffer::view *> VertexBuffers;  // Vertex buffer list

    std::vector<instance *> Instances; // Instance vector
    std::vector<mat4x4> Transforms;    // Instance trasnformation matrix

    /**
     * @brief Instance destroy callback
    */
    VOID OnInstanceDestroy( instance *Instance );

    /**
     * @brief Primitive constructor
     * @param Pipeline Pipeline to use as basis for this primitive
    */
    primitive( pipeline &Pipeline );

    /**
     * @brief Resource destroy callback
    */
    VOID OnDestroy( VOID ) override;
  }; /* class primitive */

  /**
   * @brief Material representation class
  */
  class material : public rc::resource
  {
  public:
    // using attached_resource = std::variant<std::pair<sampler *, image::view *>, sampler *, image::view *, buffer::view *>;

    /**
     * @brief Attached resource wrapper
    */
    struct attached_resource
    {
      BOOL IsTexture = FALSE;
      union
      {
        resource *Resource;
        struct
        {
          image::view *ImageView;
          sampler *Sampler;
        };
      };

      attached_resource( resource *Resource ) : IsTexture(FALSE), Resource(Resource)
      {

      } /* attached_resource */

      attached_resource( image::view *ImageView, sampler *Sampler ) : IsTexture(TRUE), ImageView(ImageView), Sampler(Sampler)
      {

      } /* attached_resource */

      VOID Release( VOID )
      {
        if (IsTexture)
        {
          Sampler->Release();
          ImageView->Release();
        }
        else
          Resource->Release();
      } /* Release */

      VOID Grab( VOID )
      {
        if (IsTexture)
        {
          Sampler->Grab();
          ImageView->Grab();
        }
        else
          Resource->Grab();
      } /* Grab */
    }; /* attr */

    /**
     * @brief Mateiral builder class
    */
    ANV_BUILDER_HEAD(material, pipeline)
      ANV_BUILDER_FIELD(std::span<attached_resource>, AttachedResources); // Initially attached resources
    ANV_BUILDER_END;

    // /**
    //  * @brief Material binding updating function
    //  * @param BindingIndex Index of attachment to update
    //  * @param Resource Pointer to resource to update
    // */
    // VOID UpdateBinding( UINT32 BindingIndex, attached_resource Resource );

  public:
    friend class pipeline;

    pipeline &Pipeline;                               // Parent pipeline
    vk::DescriptorPool DescriptorPool;                // Really, TMP Solution)))
    vk::DescriptorSet DescriptorSet;                  // Descriptor set appended to this material

    std::vector<attached_resource> AttachedResources; // List of resources attached

    /**
     * @brief Material constructor
     * @param Pipeline Pipeline pointer
    */
    material( pipeline &Pipeline );

    /**
     * @brief Resource destroy callback
    */
    VOID OnDestroy( VOID ) override;
  }; /* class material */

  /* Material class representation structure */
  class pipeline : public rc::resource
  {
  public:
    /**
     * @brief Shader binding type
    */
    enum class shader_binding_type
    {
      eSampledImage,  // Image with sampler
      eSampler,       // Sampler
      eStorageImage,  // Just image
      eStorageBuffer, // SSBO
      eUniformBuffer, // UBO
    }; /* enum shader_binding_type */

    /**
     * @brief Vertex input rate
    */
    enum class vertex_input_rate : BYTE
    {
      eVertex,
      eInstance,
    }; /* enum vertex_rate */

    /**
     * @brief Vertex attribute layout
    */
    struct vertex_attribute_layout
    {
      format Format;        // Vertex attachment format
      BYTE Offset = 0;      // Offset in vertex buffer
      BYTE BufferIndex = 0; // Vertex buffer index
    }; /* struct vertex_layout */

    /**
     * @brief Vertex buffer layout
    */
    struct vertex_buffer_layout
    {
      UINT16 Stride;                                       // Stride of vertices
      vertex_input_rate Rate = vertex_input_rate::eVertex; // Vertex rate
    }; /* struct vertex_buffer_layout */

    /**
     * @brief Pipeline builder
    */
    ANV_BUILDER_HEAD(pipeline, system)
      ANV_BUILDER_FIELD(render_pass,                              RenderPass) = render_pass::eGeometry;      // Render pass for this primitive to render in
      ANV_BUILDER_FIELD(std::span<const UINT32>,                  VertexSPV);                                // SPIRV Of vertex shader
      ANV_BUILDER_FIELD(std::span<const UINT32>,                  FragmentSPV);                              // SPIRV Of index shader
      ANV_BUILDER_FIELD(std::span<shader_binding_type>,           ShaderBindingTypes);                       // Info about shader bindings
      ANV_BUILDER_FIELD(topology,                                 PrimitiveTopology) = topology::ePointList; // Topology of primitives created for this pipeline
      ANV_BUILDER_FIELD(cull_mode_flags,                          CullMode);                                 // Cull mode
      ANV_BUILDER_FIELD(polygon_mode,                             PolygonMode) = polygon_mode::eFill;        // Polygon mode
      ANV_BUILDER_FIELD(std::span<const vertex_attribute_layout>, VertexAttributeLayouts);                   // Vertex attribute layout
      ANV_BUILDER_FIELD(std::span<const vertex_buffer_layout>,    VertexBufferLayouts);                      // Vertex buffer layout
    ANV_BUILDER_END;

    /**
     * @brief Material builder getting function
     * @return Material builder
    */
    material::builder Material( VOID )
    {
      return material::builder(*this);
    } /* Material */

    /**
     * @brief Material building function
     * @param Builder Builder to build material in
     * @return Created material
    */
    material * Build( material::builder &Builder );

    /**
     * @brief Primitive building function
     * @param Builder Builder to build primitive in
     * @return Created primitive
    */
    primitive * Build( primitive::builder &Builder );

  private:
    friend class primitive::builder;
    friend class material::builder;

    friend class system;
    friend class primitive;
    friend class material;

    /**
     * @brief Pipeline constructor
     * @param System Owner system reference
    */
    pipeline( system &System ) : System(System)
    {

    } /* pipeline */

    system &System;                                  // System reference
    vk::PipelineLayout PipelineLayout;               // Layout of pipelines
    vk::DescriptorSetLayout DescriptorSetLayout;     // Layout of descriptor sets
    vk::Pipeline Pipeline;                           // Pipeline
    render_pass RenderPass;                          // Render pass index
    std::vector<shader_binding_type> ShaderBindingTypes; // Shader binding descriptions

    /**
     * @brief Resource destroy callback
    */
    VOID OnDestroy( VOID ) override;
  }; /* class pipeline */

  /**
   * @brief Shader binding type translation function
   * @param Type Shader binidng type to translate to Vulkan descriptor type
   * @return Vulkan descriptor type
  */
  vk::DescriptorType TranslateShaderBindingType( pipeline::shader_binding_type Type );

  /***
   * Kernel implementation
  ***/

  /* Renderer core system */
  class system
  {
  private:
    struct
    {
      PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT;   // Debug messenger create function
      PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT; // Debug messenger destroy function
    } DynamicFunctions; // Dynamically loaded functions

    // Extensions and validation layers
    std::vector<const CHAR *>
      EnabledInstanceExtensions, // Enabled instance extensions
      EnabledDeviceExtensions,   // Enabled device extensions
      EnabledInstanceLayers;     // Enabled instance layers

    vk::Instance Instance;                     // Instance
    vk::PhysicalDevice PhysicalDevice;         // Physical device
    vk::Device Device;                         // Logical device
    vk::DebugUtilsMessengerEXT DebugMessenger; // Debug messenger

    VmaAllocator Allocator; // Memory allocator


    vk::SurfaceKHR Surface; // Rendering surface
    vk::Queue
      GraphicsQueue, // Graphcis queue
      ComputeQueue,  // Compute queue
      PresentQueue;  // Present queue

    constexpr static UINT32 INVALID_QUEUE_FAMILY_INDEX = ~0U; // Invalid queue family index, used in initialization process
    UINT32
      GraphicsQueueFamilyIndex = INVALID_QUEUE_FAMILY_INDEX, // Graphcis queue index
      ComputeQueueFamilyIndex = INVALID_QUEUE_FAMILY_INDEX,  // Compute queue index
      PresentQueueFamilyIndex = INVALID_QUEUE_FAMILY_INDEX;  // Present queue index

    vk::RenderPass OutputRenderPass; // Output render pass

    vk::SwapchainKHR Swapchain;            // Swapchain
    vk::Format SwapchainImageFormat;       // Swapchain image format
    vk::ColorSpaceKHR SwapchainColorSpace; // Swapchain image color space
    vk::Extent2D SwapchainImageExtent;     // Swapchain image extent (e.g. Dst extent)

    std::atomic_int32_t GlobalFrameIndex; // Global frame indexs

    /**
     * @brief Structure, that describes all data that is used in this frame only
    */
    struct frame_context
    {
      // Context of every frame
      vk::Image SwapchainImage;         // Swaphcain image
      vk::ImageView SwapchainImageView; // Swapchain image view
      vk::Framebuffer Framebuffer;      // FBO
    }; /* struct frame_context */

    vk::CommandPool RenderCommandPool;  // Command pool

    vk::CommandBuffer MainCommandBuffer;   // Frame-dependent command buffer ID
    // Render pass command buffers
    vk::CommandBuffer
      MarkerCommandBuffer,   // Marker render subpass command buffer
      GeometryCommandBuffer, // Geometry render subpass command buffer
      OverlayCommandBuffer;  // Overlay render subpass command buffer

    vk::Semaphore SwapchainOutputSemaphore; // Swaphcain out semaphore
    vk::Semaphore ImageAckquiredSemaphore;  // Semaphore that shows that image is ackquired from swapchain
    vk::Fence RenderFinishedFence;          // Fence that shows that frame rendering is finished

    // Pipeline info

    constexpr static UINT32 PositionObjectIDAttachmentIndex = 0;          // <- F32x4
    constexpr static UINT32 NormalAttachmentIndex = 1;                    // <- U16x2
    constexpr static UINT32 BaseColorAmbientOcclusionAttachmentIndex = 2; // <- U8x4
    constexpr static UINT32 MetallicRoughnessInstanceAttachmentIndex = 3; // <- U8x2, U16
    constexpr static UINT32 DepthAttachmentIndex = 4;                     // <- D32
    constexpr static UINT32 OutputAttachmentIndex = 5;                    // <- U8x4

    constexpr static vk::Format PositionObjectIDAttachmentFormat = vk::Format::eR32G32B32A32Sfloat;     // Attachment format
    constexpr static vk::Format NormalAttachmentFormat = vk::Format::eR16G16Snorm;                      // Attachment format
    constexpr static vk::Format BaseColorAmbientOcclusionAttachmentFormat = vk::Format::eR8G8B8A8Unorm; // Attachment format
    constexpr static vk::Format MetallicRoughnessInstanceAttachmentFormat = vk::Format::eR8G8B8A8Unorm; // Attachment format
    vk::Format DepthAttachmentFormat;                                                                   // (Defined in runtime)
    vk::Format OutputAttachmentFormat;                                                                  // (Defined by hardware)

    constexpr static UINT32 MarkerSubpassIndex = 0;   //      -> Output, Depth
    constexpr static UINT32 GeometrySubpassIndex = 1; //      -> GBuf, Depth
    constexpr static UINT32 ShadingSubpassIndex = 2;  // GBuf -> Output
    constexpr static UINT32 OverlaySubpassIndex = 3;  //      -> Output

    /**
     * @brief Render pass subpass index getting function
     * @param Pass Render pass to get subpass index of
     * @return Subpass index
    */
    UINT32 GetRenderPassSubpassIndex( render_pass Pass );

    /**
     * @brief Attachment image representation structure
    */
    struct attachment_image
    {
      vk::Image Image;          // Image
      vk::ImageView View;       // ImageView
      VmaAllocation Allocation; // Image allocation
    }; /* struct attachment_image */

    attachment_image
      PositionObjectID,          // Attachment image
      Normal,                    // Attachment image
      BaseColorAmbientOcclusion, // Attachment image
      MetallicRoughnessInstance, // Attachment image
      Depth;                     // Attachment image

    std::vector<frame_context> Frames; // Frame contexts

    /**
     * @brief Surface initialization function
    */
    VOID InitSurface( window::raw_handle &Window );

    /**
     * @brief Required surface extensions getting function, platform-dependent
     * @return Required surface extensions
    */
    std::vector<const CHAR *> GetRequiredSurfaceExtensions( window::raw_handle &Window );

    std::atomic_bool DoRender; // Rendering flag
    std::thread RenderThread;  // Rendering thread

    /**
     * Resource management
    */

    rc::pool<rc::resource> ResourcePool; // Pool of renderer resources
    rc::pool<primitive>   PrimitivePool; // Pool of primitives only

    friend class pipeline::builder;
    friend class buffer::builder;
    friend class sampler::builder;
    friend class image::builder;

    friend class pipeline;
    friend class buffer;
    friend class sampler;
    friend class image;
    friend class material;

    /**
     * @brief Rendering starting function
    */
    VOID StartRendering( VOID );

    /**
     * @brief Pipeline building function
     * @param Builder Builder reference
     * @return Pipeline pointer
    */
    pipeline * Build( pipeline::builder &Builder );

    /**
     * @brief Buffer building function
     * @param Builder Builder reference
     * @return Buffer pointer
    */
    buffer * Build( buffer::builder &Builder );

    /**
     * @brief Sampler building function
     * @param Builder Builder reference
     * @return Sampler pointer
    */
    sampler * Build( sampler::builder &Builder );

    /**
     * @brief Image building function
     * @param Builder Builder reference
     * @return Image pointer
    */
    image * Build( image::builder &Builder );
  public:

    /**
     * @brief Buffer builder getting function
     * @return New buffer builder
    */
    buffer::builder Buffer( VOID )
    {
      return buffer::builder(*this);
    } /* Buffer */

    /**
     * @brief Sampler builder getting function
     * @return New sampler builder
    */
    sampler::builder Sampler( VOID )
    {
      return sampler::builder(*this);
    } /* Sampler */

    /**
     * @brief Image builder getting function
     * @return Image builder
    */
    image::builder Image( VOID )
    {
      return image::builder(*this);
    } /* Image */

    /**
     * @brief Pipeline builder getting function
     * @return Pipeline builder
    */
    pipeline::builder Pipeline( VOID )
    {
      return pipeline::builder(*this);
    } /* Pipeline */

    /**
     * @brief System constructor
     * @param Window Window for system to render in
    */
    system( window::raw_handle &Window );

    /**
     * @brief System destructor
     * @param  
    */
    ~system( VOID );
  }; /* class system */
} /* namespace anv::render::core */

#endif // !defined(ANV_RENDER_CORE_H_)

/* file anv_render_core.h */