#pragma once

class VK {

    public:
        int init();
        int run() const;
        int cleanup() const;

        GLFWwindow* window = nullptr;
        vkb::Instance instance{};
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        vkb::Device device{};

    private:
        bool hasInstance = false;
        bool hasDevice = false;
};
