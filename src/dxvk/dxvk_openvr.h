#pragma once

#include <mutex>
#include <vector>

#include "dxvk_extension_provider.h"

namespace vr {
  class IVRCompositor;
  class IVRSystem;
}

namespace dxvk {

  class DxvkInstance;

  /**
   * \brief OpenVR instance
   * 
   * Loads Initializes OpenVR to provide
   * access to Vulkan extension queries.
   */
  class VrInstance : public DxvkExtensionProvider {
    
  public:
    
    VrInstance();
    ~VrInstance();

    std::string_view getName();

    DxvkExtensionList getInstanceExtensions();

    DxvkExtensionList getDeviceExtensions(
            uint32_t      adapterId);
    
    void initInstanceExtensions();

    void initDeviceExtensions(
      const DxvkInstance* instance);

    static VrInstance s_instance;

  private:

    dxvk::mutex           m_mutex;
    HKEY                  m_vr_key     = nullptr;
    vr::IVRCompositor*    m_compositor = nullptr;
    HMODULE               m_ovrApi     = nullptr;

    bool m_no_vr;
    bool m_loadedOvrApi      = false;
    bool m_initializedOpenVr = false;
    bool m_initializedInsExt = false;
    bool m_initializedDevExt = false;

    DxvkExtensionList              m_insExtensions;
    std::vector<DxvkExtensionList> m_devExtensions;
    
    DxvkExtensionList queryInstanceExtensions() const;

    DxvkExtensionList queryDeviceExtensions(
            Rc<DxvkAdapter>           adapter) const;

    DxvkExtensionList parseExtensionList(
      const std::string&              str) const;
    
    vr::IVRCompositor* getCompositor();

    void shutdown();

    HMODULE loadLibrary();

    void freeLibrary();

    void* getSym(const char* sym);

    bool waitVrKeyReady() const;
  };

  extern VrInstance g_vrInstance;
  
}
