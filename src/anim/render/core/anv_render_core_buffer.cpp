#include "anv.h"

namespace anv::render::core
{
  ANV_BUILDER_IMPL(buffer)
  ANV_BUILDER_IMPL(buffer::view)

  /**
   * @brief Buffer usage translate function
   * @param Usage ANV Buffer usage
   * @return VK Buffer usage
  */
  static vk::BufferUsageFlags TranslateBufferUsage( buffer::usage_flags Usage )
  {
    return vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc
      | ((Usage & (buffer::usage_flags)buffer::usage::eIndex  ) ? vk::BufferUsageFlagBits::eIndexBuffer   : vk::BufferUsageFlagBits())
      | ((Usage & (buffer::usage_flags)buffer::usage::eVertex ) ? vk::BufferUsageFlagBits::eVertexBuffer  : vk::BufferUsageFlagBits())
      | ((Usage & (buffer::usage_flags)buffer::usage::eStorage) ? vk::BufferUsageFlagBits::eStorageBuffer : vk::BufferUsageFlagBits())
      | ((Usage & (buffer::usage_flags)buffer::usage::eUniform) ? vk::BufferUsageFlagBits::eUniformBuffer : vk::BufferUsageFlagBits())
      ;
  } /* End of 'TranslateBufferUsage' function */

  buffer * system::Build( buffer::builder &Builder )
  {
    auto Usage_VK = TranslateBufferUsage(Builder.Usage);
    vk::BufferCreateInfo BufferCreateInfo;
    BufferCreateInfo
      .setSharingMode(vk::SharingMode::eExclusive)
      .setSize(Builder.Size)
      .setUsage(Usage_VK)
      ;

    VmaAllocationCreateInfo AllocationCreateInfo
    {
      .usage = VMA_MEMORY_USAGE_GPU_ONLY,
      .requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    };

    VkBuffer Buffer;
    VmaAllocation Allocation;
    if (auto Result = vmaCreateBuffer(Allocator, &(const VkBufferCreateInfo &)BufferCreateInfo, &AllocationCreateInfo, &Buffer, &Allocation, nullptr); Result == VK_SUCCESS)
    {
      buffer *NewBuffer = new buffer(this);

      NewBuffer->Buffer = Buffer;
      NewBuffer->Memory = Allocation;
      NewBuffer->Usage = Builder.Usage;
      NewBuffer->UsageFlags = Usage_VK;
      NewBuffer->Size = Builder.Size;

      ResourcePool.Add(NewBuffer);

      return NewBuffer;
    }
    return nullptr;
  } /* Build */

  /**
   * @brief View building function
   * @param Builder Builder reference
   * @return 
  */
  buffer::view * buffer::Build( view::builder &Builder )
  {
    if ((Usage & Builder.Usage) != Builder.Usage)
      return nullptr;

    vk::BufferCreateInfo BufferCreateInfo;
    BufferCreateInfo
      .setSharingMode(vk::SharingMode::eExclusive)
      .setSize(Builder.Size)
      .setUsage(TranslateBufferUsage(Builder.Usage))
      ;

    vk::Buffer Buffer;
    view *View = new view(this, Builder.Offset, Builder.Size);
    vmaCreateAliasingBuffer2(System.Allocator, Memory, Builder.Offset, &(const VkBufferCreateInfo &)BufferCreateInfo, &(VkBuffer &)View->View);

    View->Grab();

    System.ResourcePool.Add(View);

    return View;
  } /* Build */

  /**
   * @brief Buffer constructor
   * @param System System pointer
  */
  buffer::buffer( system *System ) : System(*System)
  {

  } /* buffer */

  /**
   * @brief Resource destroy callback
  */
  VOID buffer::OnDestroy( VOID )
  {
    vmaDestroyBuffer(System.Allocator, Buffer, Memory);

    delete this;
  } /* OnDestroy */

  /**
    * @brief View create function
    * @param Buffer Parent buffer reference
    * @param Offset Offset from parent buffer memory
    * @param Size Size of view
  */
  buffer::view::view( buffer *Buffer, SIZE_T Offset, SIZE_T Size ) : Buffer(*Buffer), Offset(Offset), Size(Size)
  {
    Buffer->Grab();
  } /* view */


  /**
   * @brief Resource destroy callback
  */
  VOID buffer::view::OnDestroy( VOID )
  {
    Buffer.System.Device.destroyBuffer(View);
    Buffer.Release();

    // yay
    delete this;
  } /* OnDestroy */
} /* namespace anv::render::core */