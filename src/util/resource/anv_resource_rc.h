/**
 * @brief       ANIM-VK Project
 * @file        src/util/resource/anv_resource_rc.h
 * @description Reference-counter resource implementaion module
 * @last_update 31.12.2023
*/

#ifndef ANV_RESOURCE_RC_H_
#define ANV_RESOURCE_RC_H_

#include "anv_common.h"

/**
 * @brief RefCounter resource namespace
*/
namespace anv::rc
{
  /* Resource base structure */
  class resource abstract
  {
    // Friend all kinds of 'type' types
    template <std::derived_from<resource> type>
      friend class pool;

    INT32 UseCount = 0; // Use count tracker

  protected:

    /**
     * @brief Resource destroy callback
    */
    virtual VOID OnDestroy( VOID )
    {

    } /* OnDestroy */
  public:

    /**
     * @brief UseCount getting function
     * @return Resource use count
    */
    INT32 GetUseCount( VOID ) const
    {
      return UseCount;
    } /* GetUseCount */

    /**
     * @brief Resource grabbing function
    */
    VOID Grab( VOID )
    {
      UseCount++;
    } /* Grab */

    /**
     * @brief Resource releasing function
    */
    VOID Release( VOID )
    {
      UseCount--;
    } /* Release */
  }; /* class resource */

  /* Resource garbage collector */
  template <std::derived_from<resource> resource_type>
    class pool
    {
      std::vector<resource_type *> Resources; // Registered resource list

    public:
      /**
       * @brief Begin iterator getting funciton
       * @return Begin iterator
      */
      auto begin( VOID )
      {
        return Resources.begin();
      } /* begin */

      /**
       * @brief End iterator getting funciton
       * @return End iterator
      */
      auto end( VOID )
      {
        return Resources.end();
      } /* end */

      /**
       * @brief Resource to resource pool adding function
       * @param Resource Resource to add to pool
      */
      VOID Add( resource_type *Resource )
      {
        Resources.push_back(Resource);
      } /* Add */

      /**
       * @brief Garbage collection function
      */
      BOOL CollectGarbage( VOID )
      {
        UINT32 LastFreeResourceIndex = 0;

        for (resource_type *Resource : Resources)
        {
          if (Resource->UseCount <= 0)
            Resource->OnDestroy();
          else
            Resources[LastFreeResourceIndex++] = Resource;
        }

        Resources.resize(LastFreeResourceIndex);

        return LastFreeResourceIndex != 0;
      } /* CollectGarbage */

      /**
       * @brief Resource pool clearing function
       * @return TRUE if cleared, FALSE otherwise.
      */
      BOOL Clear( VOID )
      {
        CollectGarbage();

        // Release somehow unreleased resources
        for (resource *R : Resources)
          R->OnDestroy();

        return Resources.empty();
      } /* Clear */
    }; /* class resource_manager */
} /* namespace anv::resource */

#endif // !defined(ANV_RESOURCE_RC_H_)

/* file anv_resource_rc.h */
