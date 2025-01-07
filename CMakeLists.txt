cmake_minimum_required(VERSION 3.10)
project(MiniWar)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


file(GLOB_RECURSE CPP_SOURCES 
    "cpp/*.cpp"
    "cpp/**/*.cpp"
)

file(GLOB IMGUI_SOURCES 
    "include/imgui/*.cpp"
)

set(SOURCES
    ${CPP_SOURCES}
    ${IMGUI_SOURCES}
)


include_directories(
    header
    include
    include/GL
    include/GLFW
    include/imgui
    include/stb
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-D_DEBUG)
endif()

add_executable(${PROJECT_NAME} ${SOURCES})

find_package(OpenGL REQUIRED)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/lib/glew32.lib
        ${CMAKE_SOURCE_DIR}/lib/glfw3dll.lib
        ${CMAKE_SOURCE_DIR}/lib/OpenGL32.lib
        ${CMAKE_SOURCE_DIR}/lib/bass.lib
    )
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE
        GLEW
        glfw
        OpenGL::GL
        bass
    )
endif()

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /MP)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(${PROJECT_NAME} PRIVATE /O2)
    endif()
endif()

file(GLOB SHADER_FILES "shaders/*.*")
file(COPY ${SHADER_FILES} DESTINATION ${CMAKE_BINARY_DIR}/shaders)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/resources)
file(COPY "MiniWar/resources"
     DESTINATION ${CMAKE_BINARY_DIR}
     PATTERN "*.*")

file(GLOB CONFIG_FILES "MiniWar/*.json")
file(COPY ${CONFIG_FILES} DESTINATION ${CMAKE_BINARY_DIR})

if(WIN32)
    file(GLOB DLL_FILES "MiniWar/*.dll")
    file(COPY ${DLL_FILES} DESTINATION ${CMAKE_BINARY_DIR})
endif()

if(WIN32)
    file(GLOB DLL_FILES "lib/*.dll")
    file(COPY ${DLL_FILES} DESTINATION ${CMAKE_BINARY_DIR})
# linux
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # 指定bass.so的路径
    # x86平台
    # 检查处理器架构并复制相应的 BASS 库
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        # 32位平台 (x86)
        file(COPY "lib/linux/bass/x86/libbass.so" DESTINATION ${CMAKE_BINARY_DIR})
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
            # 64位 x86 平台 (x86_64)
            file(COPY "lib/linux/bass/x86_64/libbass.so" DESTINATION ${CMAKE_BINARY_DIR})
        elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
            # 64位 ARM 平台 (aarch64 或 arm64)
            file(COPY "lib/linux/bass/aarch64/libbass.so" DESTINATION ${CMAKE_BINARY_DIR})
        else()
            message(FATAL_ERROR "Unsupported 64-bit architecture: ${CMAKE_SYSTEM_PROCESSOR}")
        endif()
    else()
        message(FATAL_ERROR "Unsupported platform: ${CMAKE_SIZEOF_VOID_P}-bit")
    endif()

    # 设置链接器路径
    link_directories(${CMAKE_BINARY_DIR})

    # 查找 BASS 库
    find_library(BASS_LIBRARY bass PATHS ${CMAKE_BINARY_DIR} NO_DEFAULT_PATH)

    if(NOT BASS_LIBRARY)
        message(FATAL_ERROR "BASS library not found in ${CMAKE_BINARY_DIR}")
    endif()

    # 将找到的 BASS 库链接到你的目标
    target_link_libraries(MiniWar PRIVATE ${BASS_LIBRARY})


endif()
