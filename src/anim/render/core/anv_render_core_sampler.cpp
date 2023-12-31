#include "anv.h"

/* Renderer core namespace */
namespace anv::render::core
{
  /* Sampler builder implementation */
  ANV_BUILDER_IMPL(sampler);

  static vk::SamplerAddressMode TranslateAddressMode( sampler::address_mode AddressMode )
  {
    switch (AddressMode)
    {
    case sampler::address_mode::eRepeat         : return vk::SamplerAddressMode::eRepeat;
    case sampler::address_mode::eMirroredRepeat : return vk::SamplerAddressMode::eMirroredRepeat;
    case sampler::address_mode::eClampToBorder  : return vk::SamplerAddressMode::eClampToBorder;
    case sampler::address_mode::eClampToEdge    : return vk::SamplerAddressMode::eClampToEdge;
    default                                     : return vk::SamplerAddressMode::eRepeat;
    }
  }

  static vk::Filter TranslateFilter( sampler::filter Filter )
  {
    switch (Filter)
    {
    case sampler::filter::eNearest : return vk::Filter::eNearest;
    case sampler::filter::eLinear  : return vk::Filter::eLinear;
    default                        : return vk::Filter::eNearest;
    }
  }

  /**
   * @brief Sampler building function
   * @param SamplerBuilder Sampler builder reference
   * @return Sampler pointer
  */
  sampler * system::Build( sampler::builder &Builder )
  {
    vk::SamplerCreateInfo SamplerCreateInfo;
    SamplerCreateInfo
      .setAddressModeU(TranslateAddressMode(Builder.AddressModeU))
      .setAddressModeV(TranslateAddressMode(Builder.AddressModeV))
      .setMinFilter(TranslateFilter(Builder.MinFilter))
      .setMagFilter(TranslateFilter(Builder.MagFilter))
      .setMipmapMode(vk::SamplerMipmapMode::eLinear)
      ;

    sampler *Sampler = new sampler(this);
    Sampler->Sampler = Device.createSampler(SamplerCreateInfo);

    Sampler->Grab();

    ResourcePool.Add(Sampler);

    return Sampler;
  } /* Build */

  /**
    * @brief Resource destroy callback
  */
  VOID sampler::OnDestroy( VOID )
  {
    System.Device.destroySampler(Sampler);

    delete this;
  } /* OnDestroy */
} /* namespace anv::render::core */

/* anv_render_core_sampler.cpp */