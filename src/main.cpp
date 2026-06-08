#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <VkBootstrap.h>
#include <glm/glm.hpp>
#include <imgui.h>
#include <stb_image.h>
#include <tiny_obj_loader.h>
#include <vk_mem_alloc.h>

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#if defined(__APPLE__) && defined(VKGUIDE_VULKAN_LOADER)
#include <dlfcn.h>
#endif

/* HELPER FUNCTIONS */
void configureVulkanLoader()
{
#if defined(__APPLE__) && defined(VKGUIDE_MOLTENVK_ICD)
    setenv("VK_ICD_FILENAMES", VKGUIDE_MOLTENVK_ICD, 0);
#endif

#if defined(__APPLE__) && defined(VKGUIDE_VULKAN_LOADER)
    void* vulkanLoader = dlopen(VKGUIDE_VULKAN_LOADER, RTLD_NOW | RTLD_LOCAL);
    if (vulkanLoader == nullptr) return;

    const auto getInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(dlsym(vulkanLoader, "vkGetInstanceProcAddr"));
    if (getInstanceProcAddr != nullptr) {
        glfwInitVulkanLoader(getInstanceProcAddr);
    }
#endif
}

template <typename T>
T unwrap(vkb::Result<T>&& result, const std::string& label)
{
    if (!result) {
        throw std::runtime_error(label + ": " + result.error().message());
    }

    return result.value();
}

#include "vk.h"

int VK::init() {

    configureVulkanLoader();

    if (glfwInit() != GLFW_TRUE) {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    VK::window = glfwCreateWindow(1280, 720, "vkguide", nullptr, nullptr);
    if (VK::window == nullptr) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    if (glfwVulkanSupported() != GLFW_TRUE) {
        throw std::runtime_error("GLFW could not find a Vulkan loader");
    }

    uint32_t extensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);

    if (glfwExtensions == nullptr || extensionCount == 0) {
        throw std::runtime_error("GLFW did not provide required Vulkan instance extensions");
    }
    const std::vector<const char*> requiredExtensions = {glfwExtensions, glfwExtensions + extensionCount};

    VK::instance = unwrap(
        vkb::InstanceBuilder()
            .set_app_name("vkguide")
            .require_api_version(1, 2, 0)
            .enable_extensions(requiredExtensions)
            .build(),
        "Failed to create Vulkan instance");
    VK::hasInstance = true;

    if (glfwCreateWindowSurface(VK::instance.instance, VK::window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create GLFW Vulkan surface");
    }

    const vkb::PhysicalDevice physicalDevice = unwrap(
        vkb::PhysicalDeviceSelector(VK::instance)
            .set_surface(VK::surface)
            .set_minimum_version(1, 2)
            .select(),
        "Failed to select Vulkan physical device");

    VK::device = unwrap(vkb::DeviceBuilder(physicalDevice).build(),"Failed to create Vulkan logical device");
    VK::hasDevice = true;

    return 0;
}

int VK::cleanup() const {

    if (hasDevice) {
        vkb::destroy_device(VK::device);
    }

    if (VK::surface != VK_NULL_HANDLE && hasInstance) {
        vkb::destroy_surface(VK::instance, VK::surface);
    }

    if (hasInstance) {
        vkb::destroy_instance(VK::instance);
    }

    glfwDestroyWindow(VK::window);
    glfwTerminate();
    return 0;
}

int VK::run() const {

    glfwPollEvents();
    return glfwWindowShouldClose(VK::window) == GLFW_FALSE;
}

int main() {

    VK vk;

    try { vk.init(); while (vk.run()) {} }
    catch(const std::exception& error) {
        std::cerr << error.what() << '\n';
        return vk.cleanup();
    };

    return vk.cleanup();
}
