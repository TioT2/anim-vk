#include "anv.h"

/**
 * @brief Render core namespace
*/
namespace anv::render::core
{
  vk::ImageUsageFlags TranslateImageUsage( image::usage_flags Usage )
  {
    return vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc
      | ((Usage & (image::usage_flags)image::usage::eSampled ) ? vk::ImageUsageFlagBits::eSampled : vk::ImageUsageFlagBits())
      | ((Usage & (image::usage_flags)image::usage::eStorage ) ? vk::ImageUsageFlagBits::eStorage : vk::ImageUsageFlagBits())
      ;
  } /* TranslateImageUsage */

  vk::Format TranslateFormat( format Format )
  {
    static vk::Format FormatLUTTable[(SIZE_T)format::type::_eCount][4]
    {
      /* eU8      */ {vk::Format::eR8Uint, vk::Format::eR8G8Uint, vk::Format::eR8G8B8Uint, vk::Format::eR8G8B8A8Uint},
      /* eU16     */ {vk::Format::eR8Uint, vk::Format::eR8G8Uint, vk::Format::eR8G8B8Uint, vk::Format::eR8G8B8A8Uint},
      /* eU32     */ {vk::Format::eR8Uint, vk::Format::eR8G8Uint, vk::Format::eR8G8B8Uint, vk::Format::eR8G8B8A8Uint},
      /* eU8Norm  */ {vk::Format::eR8Unorm, vk::Format::eR8G8Unorm, vk::Format::eR8G8B8Unorm, vk::Format::eR8G8B8A8Unorm},
      /* eU16Norm */ {vk::Format::eR8Unorm, vk::Format::eR8G8Unorm, vk::Format::eR8G8B8Unorm, vk::Format::eR8G8B8A8Unorm},
      /* eU8Srgb  */ {vk::Format::eR8Srgb, vk::Format::eR8G8Srgb, vk::Format::eR8G8B8Srgb, vk::Format::eR8G8B8A8Srgb},
      /* eI8      */ {vk::Format::eR8Sint, vk::Format::eR8G8Sint, vk::Format::eR8G8B8Sint, vk::Format::eR8G8B8A8Sint},
      /* eI16     */ {vk::Format::eR8Sint, vk::Format::eR8G8Sint, vk::Format::eR8G8B8Sint, vk::Format::eR8G8B8A8Sint},
      /* eI32     */ {vk::Format::eR8Sint, vk::Format::eR8G8Sint, vk::Format::eR8G8B8Sint, vk::Format::eR8G8B8A8Sint},
      /* eI8Norm  */ {vk::Format::eR8Snorm, vk::Format::eR8G8Snorm, vk::Format::eR8G8B8Snorm, vk::Format::eR8G8B8A8Snorm},
      /* eI16Norm */ {vk::Format::eR8Snorm, vk::Format::eR8G8Snorm, vk::Format::eR8G8B8Snorm, vk::Format::eR8G8B8A8Snorm},
      /* eI32Norm */ {vk::Format::eR8Snorm, vk::Format::eR8G8Snorm, vk::Format::eR8G8B8Snorm, vk::Format::eR8G8B8A8Snorm},
      /* eF16     */ {vk::Format::eR32Sfloat, vk::Format::eR32G32Sfloat, vk::Format::eR32G32B32Sfloat, vk::Format::eR32G32B32A32Sfloat},
      /* eF32     */ {vk::Format::eR32Sfloat, vk::Format::eR32G32Sfloat, vk::Format::eR32G32B32Sfloat, vk::Format::eR32G32B32A32Sfloat},
    };

    if (Format.Count > 4 || Format.Count < 1)
      return vk::Format::eUndefined;
    return FormatLUTTable[(SIZE_T)Format.Type][Format.Count - 1];
  } /* TranslateFormat */

  /**
   * @brief Image building function
   * @param Builder Builder reference
   * @return Image pointer
  */
  image * system::Build( image::builder &Builder )
  {
    vk::ImageUsageFlags UsageVK = TranslateImageUsage(Builder.Usage);
    vk::Format FormatVK = TranslateFormat(Builder.Format);
    vk::ImageCreateInfo ImageCreateInfo;
    ImageCreateInfo
      .setMipLevels(Builder.MipLevels)
      .setFormat(FormatVK)
      .setExtent({(UINT32)Builder.Extent.W, (UINT32)Builder.Extent.H, 1})
      .setSharingMode(vk::SharingMode::eExclusive)
      ;

    image *Image = new image(this);
    Image->Image = Device.createImage(ImageCreateInfo);
    Image->Extent = Builder.Extent;
    Image->Format = Builder.Format;
    Image->Usage = Builder.Usage;
    Image->FormatVK = FormatVK;

    Image->Grab();

    ResourcePool.Add(Image);

    return Image;
  } /* Build */

  /**
   * @brief Resource destroy callback
  */
  VOID image::OnDestroy( VOID )
  {
    System.Device.destroyImage(Image);
    delete this;
  } /* OnDestroy */

  /***
   * Image view implementation
   ***/

  /**
   * @brief Image component mapping translation function
   * @param ANV Image component swizzle
   * @return Vulkan Image component swizzle
  */
  vk::ComponentSwizzle TranslateComponentSwizzle( image::view::component_swizzle ImageComponentMapping )
  {
    switch(ImageComponentMapping)
    {
    case image::view::component_swizzle::eR : return vk::ComponentSwizzle::eR;
    case image::view::component_swizzle::eG : return vk::ComponentSwizzle::eG;
    case image::view::component_swizzle::eB : return vk::ComponentSwizzle::eB;
    case image::view::component_swizzle::eA : return vk::ComponentSwizzle::eA;
    default                                 : return vk::ComponentSwizzle::eIdentity;
    }
  } /* TranslateComponentSwizzle */

  /**
   * @brief View building function
  */
  image::view * image::Build( view::builder &Builder )
  {
    view *View = new view(Builder.System);

    vk::ImageViewCreateInfo ImageViewCreateInfo;
    ImageViewCreateInfo
      .setFormat(TranslateFormat(Format))
      .setImage(Image)
      .setSubresourceRange(vk::ImageSubresourceRange()
        .setAspectMask(vk::ImageAspectFlagBits::eColor)
        .setBaseMipLevel(Builder.BaseMipLevel)
        .setLevelCount(Builder.MipLevelCount)
      )
      .setComponents(vk::ComponentMapping(
        TranslateComponentSwizzle(Builder.SwizzleR),
        TranslateComponentSwizzle(Builder.SwizzleG),
        TranslateComponentSwizzle(Builder.SwizzleB),
        TranslateComponentSwizzle(Builder.SwizzleA)
      ))
      ;
    View->ImageView = System.Device.createImageView(ImageViewCreateInfo);

    System.ResourcePool.Add(View);

    return View;
  } /* Build */

  /**
   * @brief Resource destroy callback
  */
  VOID image::view::OnDestroy( VOID )
  {
    Image.Release();
    delete this;
  } /* OnDestroy */
} /* namespace anv::render::core */