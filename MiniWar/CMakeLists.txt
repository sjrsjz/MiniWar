set(PROJECT_NAME MiniWar)

################################################################################
# Source groups
################################################################################
set(no_group_source_files
    "../README.md"
)
source_group("" FILES ${no_group_source_files})

set(cpp
    "../cpp/Engine.cpp"
    "../cpp/globals.cpp"
    "../cpp/Logic/Army.cpp"
    "../cpp/Logic/Building.cpp"
    "../cpp/Logic/Player.cpp"
    "../cpp/Logic/Region.cpp"
    "../cpp/Logic/RegionManager.cpp"
    "../cpp/Logic/Weapon.cpp"
    "../cpp/Main.cpp"
    "../cpp/mash.cpp"
    "../cpp/output.cpp"
    "../cpp/shader.cpp"
    "../cpp/utils/Array.cpp"
    "../cpp/utils/Config.cpp"
    "../cpp/utils/GlobalTimer.cpp"
    "../cpp/utils/Operation.cpp"
    "../cpp/utils/Point.cpp"
)
source_group("cpp" FILES ${cpp})

set(header
    "../header/ByteArray.h"
    "../header/Camera.h"
    "../header/debug.h"
    "../header/Engine.h"
    "../header/Exception/FullHpException.h"
    "../header/Exception/SurrenderNotAttackedException.h"
    "../header/globals.h"
    "../header/Logic/AI.h"
    "../header/Logic/Army.h"
    "../header/Logic/Building.h"
    "../header/Logic/Player.h"
    "../header/Logic/region.h"
    "../header/Logic/RegionManager.h"
    "../header/Logic/Weapon.h"
    "../header/mash.h"
    "../header/output.h"
    "../header/passes/FlameRenderPass.h"
    "../header/passes/FragmentBuffer.h"
    "../header/passes/SSBO.h"
    "../header/passes/SSBOByteArray.h"
    "../header/points.h"
    "../header/shader.h"
    "../header/Timer.h"
    "../header/utils/Array.h"
    "../header/utils/Config.h"
    "../header/utils/FloatBuffer.h"
    "../header/utils/ImageLoader.h"
    "../header/utils/Operation.h"
    "../header/utils/Point.h"
    "../header/utils/RegionData.h"
    "../header/utils/RegionSelector.h"
    "../header/utils/RegionSSBOBuffer.h"
    "../header/utils/SmoothMove.h"
)
source_group("header" FILES ${header})

set(include
    "../include/bass/bass.h"
    "../include/GL/eglew.h"
    "../include/GL/glew.h"
    "../include/GL/glxew.h"
    "../include/GL/wglew.h"
    "../include/GLFW/glfw3.h"
    "../include/GLFW/glfw3native.h"
    "../include/imgui/imconfig.h"
    "../include/imgui/imgui.cpp"
    "../include/imgui/imgui.h"
    "../include/imgui/imgui_demo.cpp"
    "../include/imgui/imgui_draw.cpp"
    "../include/imgui/imgui_impl_glfw.cpp"
    "../include/imgui/imgui_impl_glfw.h"
    "../include/imgui/imgui_impl_opengl3.cpp"
    "../include/imgui/imgui_impl_opengl3.h"
    "../include/imgui/imgui_impl_opengl3_loader.h"
    "../include/imgui/imgui_internal.h"
    "../include/imgui/imgui_tables.cpp"
    "../include/imgui/imgui_widgets.cpp"
    "../include/imgui/imstb_rectpack.h"
    "../include/imgui/imstb_textedit.h"
    "../include/imgui/imstb_truetype.h"
    "../include/json.hpp"
    "../include/linmath.h"
    "../include/stb/stb_image.h"
)
source_group("include" FILES ${include})

set(shader
    "../shaders/direct_tex.frag"
    "../shaders/direct_tex.vert"
    "../shaders/flame_render_pass.frag"
    "../shaders/flame_render_pass.vert"
    "../shaders/gaussian_blur.frag"
    "../shaders/gaussian_blur.vert"
#    "../shaders/main_game_pass.frag"
#    "../shaders/main_game_pass.vert"
    "../shaders/map_renderer.frag"
    "../shaders/map_renderer.vert"
    "../shaders/normal_gl.frag"
    "../shaders/normal_gl.vert"
    "../shaders/points_renderer.frag"
    "../shaders/points_renderer.vert"
)
source_group("shader" FILES ${shader})

set(ALL_FILES
    ${no_group_source_files}
    ${cpp}
    ${header}
    ${include}
    ${shader}
)

################################################################################
# Target
################################################################################
add_executable(${PROJECT_NAME} ${ALL_FILES})

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE MiniWar)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_KEYWORD "Win32Proj"
)
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
    )
endif()
################################################################################
# MSVC runtime library
################################################################################
get_property(MSVC_RUNTIME_LIBRARY_DEFAULT TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY)
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    string(CONCAT "MSVC_RUNTIME_LIBRARY_STR"
        $<$<CONFIG:Debug>:
            MultiThreadedDebugDLL
        >
        $<$<CONFIG:Release>:
            MultiThreadedDLL
        >
        $<$<NOT:$<OR:$<CONFIG:Debug>,$<CONFIG:Release>>>:${MSVC_RUNTIME_LIBRARY_DEFAULT}>
    )
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY ${MSVC_RUNTIME_LIBRARY_STR})

################################################################################
# Include directories
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    target_include_directories(${PROJECT_NAME} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/../include/GL;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../include/GLFW"
    )
endif()

################################################################################
# Compile definitions
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:Debug>:"
            "_DEBUG"
        ">"
        "$<$<CONFIG:Release>:"
            "NDEBUG"
        ">"
        "_CONSOLE;"
        "UNICODE;"
        "_UNICODE"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:Debug>:"
            "_DEBUG"
        ">"
        "$<$<CONFIG:Release>:"
            "NDEBUG"
        ">"
        "WIN32;"
        "_CONSOLE;"
        "UNICODE;"
        "_UNICODE"
    )
endif()

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:
                /Oi;
                /Gy;
                /Ot
            >
            /permissive-;
            /std:c++17;
            /sdl;
            /W3;
            ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
            ${DEFAULT_CXX_EXCEPTION_HANDLING}
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:
                /Oi;
                /Gy
            >
            /permissive-;
            /std:c++17;
            /sdl;
            /W3;
            ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
            ${DEFAULT_CXX_EXCEPTION_HANDLING}
        )
    endif()
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:
                /OPT:REF;
                /OPT:ICF
            >
            /NODEFAULTLIB:MSVCRT;
            /DEBUG;
            /SUBSYSTEM:CONSOLE
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:
                /OPT:REF;
                /OPT:ICF
            >
            /NODEFAULTLIB:MSVCRT;
            /DEBUG;
            /SUBSYSTEM:CONSOLE
        )
    endif()
endif()

################################################################################
# Dependencies
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    set(ADDITIONAL_LIBRARY_DEPENDENCIES
        "$<$<CONFIG:Release>:"
            "msvcrt;"
            "vcruntime"
        ">"
        "$(ProjectDir)../lib/glew32;"
        "$(ProjectDir)../lib/glfw3dll;"
        "$(ProjectDir)../lib/OpenGL32;"
        "$(ProjectDir)../lib/bass"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    set(ADDITIONAL_LIBRARY_DEPENDENCIES
        "$(ProjectDir)../lib/glew32;"
        "$(ProjectDir)../lib/glfw3dll;"
        "$(ProjectDir)../lib/OpenGL32;"
        "$(ProjectDir)../lib/bass"
    )
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE "${ADDITIONAL_LIBRARY_DEPENDENCIES}")

if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    target_link_directories(${PROJECT_NAME} PRIVATE
        "${CMAKE_SOURCE_DIR}/lib/"
    )
endif()

