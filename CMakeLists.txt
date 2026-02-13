# Added by Gemini3, on 26.02.11
################################################################################################################################
cmake_minimum_required(VERSION 3.20)
project(AdaptiveArena VERSION 0.1.0 LANGUAGES CXX)

# C++ Standard configuration
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# MSVC Specific flags for UTF-8 coverage
if(MSVC)
    add_compile_options(/utf-8)
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Dear ImGui setup via FetchContent
include(FetchContent)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        docking
)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.3.8 # Stable version
)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(imgui glfw)

# Manually define imgui target because it doesn't provide a native CMakeLists.txt
set(IMGUI_DIR ${imgui_SOURCE_DIR})
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    # Backends
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC 
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${glfw_SOURCE_DIR}/include
)

# Include directories
include_directories(include)

# Sources
set(SOURCE_FILES
    src/main.cpp
    src/AdaptiveArena.cpp
    src/LearningEngine.cpp
    src/Visualizer.cpp
    src/UltrasoundArena.cpp
)

# Executable
add_executable(${PROJECT_NAME} ${SOURCE_FILES})


# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE 
    imgui 
    glfw
    opengl32
)

# Benchmark Test
add_executable(ultrasound_test 
    tests/ultrasound_test.cpp
    src/UltrasoundArena.cpp
    src/LearningEngine.cpp
    src/AdaptiveArena.cpp
    src/Visualizer.cpp
)
target_link_libraries(ultrasound_test PRIVATE
    imgui
    glfw
    opengl32
)
