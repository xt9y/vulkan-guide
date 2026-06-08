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

namespace {

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

    std::vector<const char*> getRequiredInstanceExtensions()
    {
        uint32_t extensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);

        if (glfwExtensions == nullptr || extensionCount == 0) {
            throw std::runtime_error("GLFW did not provide required Vulkan instance extensions");
        }

        return {glfwExtensions, glfwExtensions + extensionCount};
    }

    template <typename T>
    T unwrap(vkb::Result<T>&& result, const std::string& label)
    {
        if (!result) {
            throw std::runtime_error(label + ": " + result.error().message());
        }

        return result.value();
    }

} // namespace

int main()
{
    configureVulkanLoader();

    if (glfwInit() != GLFW_TRUE) {
        std::cerr << "Failed to initialize GLFW\n";
        return 1;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "vkguide", nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return 1;
    }

    vkb::Instance instance{};
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    vkb::Device device{};
    bool hasInstance = false;
    bool hasDevice = false;

    try
    {
        if (glfwVulkanSupported() != GLFW_TRUE) {
            throw std::runtime_error("GLFW could not find a Vulkan loader");
        }

        const std::vector<const char*> requiredExtensions = getRequiredInstanceExtensions();

        instance = unwrap(
            vkb::InstanceBuilder()
                .set_app_name("vkguide")
                .require_api_version(1, 2, 0)
                .enable_extensions(requiredExtensions)
                .build(),
            "Failed to create Vulkan instance");
        hasInstance = true;

        if (glfwCreateWindowSurface(instance.instance, window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create GLFW Vulkan surface");
        }

        const vkb::PhysicalDevice physicalDevice = unwrap(
            vkb::PhysicalDeviceSelector(instance)
                .set_surface(surface)
                .set_minimum_version(1, 2)
                .select(),
            "Failed to select Vulkan physical device");

        device = unwrap(vkb::DeviceBuilder(physicalDevice).build(),"Failed to create Vulkan logical device");
        hasDevice = true;

        const glm::vec3 origin{0.0f, 0.0f, 0.0f};
        (void)origin;
        (void)ImGui::GetVersion();
        (void)stbi_failure_reason();
        (void)tinyobj::ObjReaderConfig{};
        (void)sizeof(VmaAllocator);

        while (glfwWindowShouldClose(window) == GLFW_FALSE) {
            glfwPollEvents();
        }

        vkb::destroy_device(device);
        vkb::destroy_surface(instance, surface);
        vkb::destroy_instance(instance);
    }
    catch (const std::exception& error)
    {
        if (hasDevice) {
            vkb::destroy_device(device);
        }

        if (surface != VK_NULL_HANDLE && hasInstance) {
            vkb::destroy_surface(instance, surface);
        }

        if (hasInstance) {
            vkb::destroy_instance(instance);
        }

        std::cerr << error.what() << '\n';
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
