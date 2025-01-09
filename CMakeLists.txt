cmake_minimum_required(VERSION 3.10)
project(MiniWar)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    # 设置输出目录为build目录
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR})
    
    string(REPLACE "/INCREMENTAL" "" CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG}")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi /Od /Ob0 /RTC1")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /DEBUG /INCREMENTAL:NO")
    set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /DEBUG")
endif()

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
    # ָ��bass.so��·��
    # x86ƽ̨
    # ��鴦�����ܹ���������Ӧ�� BASS ��
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        # 32λƽ̨ (x86)
        file(COPY "lib/linux/bass/x86/libbass.so" DESTINATION ${CMAKE_BINARY_DIR})
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
            # 64λ x86 ƽ̨ (x86_64)
            file(COPY "lib/linux/bass/x86_64/libbass.so" DESTINATION ${CMAKE_BINARY_DIR})
        elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
            # 64λ ARM ƽ̨ (aarch64 �� arm64)
            file(COPY "lib/linux/bass/aarch64/libbass.so" DESTINATION ${CMAKE_BINARY_DIR})
        else()
            message(FATAL_ERROR "Unsupported 64-bit architecture: ${CMAKE_SYSTEM_PROCESSOR}")
        endif()
    else()
        message(FATAL_ERROR "Unsupported platform: ${CMAKE_SIZEOF_VOID_P}-bit")
    endif()

    # ����������·��
    link_directories(${CMAKE_BINARY_DIR})

    # ���� BASS ��
    find_library(BASS_LIBRARY bass PATHS ${CMAKE_BINARY_DIR} NO_DEFAULT_PATH)

    if(NOT BASS_LIBRARY)
        message(FATAL_ERROR "BASS library not found in ${CMAKE_BINARY_DIR}")
    endif()

    # ���ҵ��� BASS �����ӵ����Ŀ��
    target_link_libraries(MiniWar PRIVATE ${BASS_LIBRARY})


endif()
