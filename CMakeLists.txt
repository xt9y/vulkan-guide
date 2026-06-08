cmake_minimum_required(VERSION 3.22)
project(vkguide LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(TINYOBJLOADER_BUILD_TEST_LOADER OFF CACHE BOOL "" FORCE)
set(TINYOBJLOADER_BUILD_OBJ_STICHER OFF CACHE BOOL "" FORCE)
set(VK_BOOTSTRAP_TEST OFF CACHE BOOL "" FORCE)
set(VK_BOOTSTRAP_INSTALL OFF CACHE BOOL "" FORCE)
set(VK_BOOTSTRAP_VULKAN_HEADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Vendor/Vulkan-Headers/include" CACHE PATH "" FORCE)

add_subdirectory(Vendor/glfw)
add_subdirectory(Vendor/glm)
add_subdirectory(Vendor/tinyobjloader)
add_subdirectory(Vendor/vk-bootstrap)
add_subdirectory(Vendor/VulkanMemoryAllocator)

add_library(stb STATIC
        src/stb_image.cpp)
target_include_directories(stb PUBLIC Vendor/stb)

add_library(imgui STATIC
        Vendor/imgui/imgui.cpp
        Vendor/imgui/imgui_draw.cpp
        Vendor/imgui/imgui_tables.cpp
        Vendor/imgui/imgui_widgets.cpp
        Vendor/imgui/backends/imgui_impl_glfw.cpp
        Vendor/imgui/backends/imgui_impl_vulkan.cpp)
target_include_directories(imgui PUBLIC
        Vendor/imgui
        Vendor/imgui/backends)
target_compile_definitions(imgui PUBLIC
        IMGUI_IMPL_VULKAN_NO_PROTOTYPES
        VK_NO_PROTOTYPES)
target_link_libraries(imgui PUBLIC glfw Vulkan::Headers)

add_executable(vkguide
        src/main.cpp)

target_link_libraries(vkguide PRIVATE
        Vulkan::Headers
        glm::glm
        imgui
        stb
        tinyobjloader
        vk-bootstrap::vk-bootstrap
        GPUOpen::VulkanMemoryAllocator)

if(APPLE)
    find_library(VKGUIDE_VULKAN_LOADER
            NAMES vulkan.1 vulkan MoltenVK
            PATHS /opt/homebrew/lib /usr/local/lib
            NO_DEFAULT_PATH)
    find_file(VKGUIDE_MOLTENVK_ICD
            NAMES MoltenVK_icd.json
            PATHS /opt/homebrew/etc/vulkan/icd.d /usr/local/etc/vulkan/icd.d
            NO_DEFAULT_PATH)

    if(VKGUIDE_VULKAN_LOADER)
        target_compile_definitions(vkguide PRIVATE
                VKGUIDE_VULKAN_LOADER="$<SHELL_PATH:${VKGUIDE_VULKAN_LOADER}>")
        set_target_properties(vkguide PROPERTIES
                BUILD_RPATH "/opt/homebrew/lib;/usr/local/lib")
    endif()

    if(VKGUIDE_MOLTENVK_ICD)
        target_compile_definitions(vkguide PRIVATE
                VKGUIDE_MOLTENVK_ICD="$<SHELL_PATH:${VKGUIDE_MOLTENVK_ICD}>")
    endif()
endif()
