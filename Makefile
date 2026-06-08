BUILD_DIR = cmake-build-debug
TARGET = vkguide

ifeq ($(OS),Windows_NT)
	EXE = $(BUILD_DIR)/Debug/$(TARGET).exe
else
	EXE = $(BUILD_DIR)/$(TARGET)
endif

.PHONY: all deps configure build run clean add

all: deps configure build run

deps:
	@cmake -E echo "Using vendored libraries from Vendor/"
	@cmake -E echo "GLFW, GLM, ImGui, STB, tinyobjloader, vk-bootstrap, VMA, Vulkan-Headers"

configure:
	cmake -S . -B $(BUILD_DIR)

build:
	cmake --build $(BUILD_DIR) --target $(TARGET)

run:
	$(EXE)

clean:
	cmake -E rm -rf $(BUILD_DIR) compile_commands.json

add:
	git status
	git add CMakeLists.txt Makefile src Vendor
	git status
