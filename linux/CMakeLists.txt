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

if(WIN32)
    file(GLOB DLL_FILES "lib/*.dll")
    file(COPY ${DLL_FILES} DESTINATION ${CMAKE_BINARY_DIR})
endif()
