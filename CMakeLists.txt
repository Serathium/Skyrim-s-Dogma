cmake_minimum_required(VERSION 3.24)
project(SkyrimsDogma VERSION 0.1.1 LANGUAGES CXX)

option(DOGMA_BUILD_PLUGIN "Build the Windows SKSE observer" ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

add_library(DogmaCore STATIC src/core/ShadowModel.cpp src/core/Continuity.cpp)
target_include_directories(DogmaCore PUBLIC src)
if(MSVC)
    target_compile_options(DogmaCore PRIVATE /W4 /WX /permissive- /EHsc)
endif()

enable_testing()
add_executable(DogmaCoreTests tests/ShadowModelTests.cpp)
target_link_libraries(DogmaCoreTests PRIVATE DogmaCore)
add_test(NAME shadow-model COMMAND DogmaCoreTests)
add_executable(DogmaContinuityTests tests/ContinuityTests.cpp)
target_link_libraries(DogmaContinuityTests PRIVATE DogmaCore)
add_test(NAME observation-continuity COMMAND DogmaContinuityTests)

if(DOGMA_BUILD_PLUGIN)
    if(NOT WIN32 OR NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
        message(FATAL_ERROR "The SKSE plugin requires a 64-bit Windows toolchain.")
    endif()
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/.deps/CommonLibSSE-NG/CMakeLists.txt")
        message(FATAL_ERROR "Run scripts/setup.ps1 to fetch the pinned dependencies.")
    endif()
    find_package(Git REQUIRED)
    execute_process(COMMAND "${GIT_EXECUTABLE}" -C "${CMAKE_SOURCE_DIR}/.deps/CommonLibSSE-NG" rev-parse HEAD
        OUTPUT_VARIABLE COMMONLIB_COMMIT OUTPUT_STRIP_TRAILING_WHITESPACE COMMAND_ERROR_IS_FATAL ANY)
    if(NOT COMMONLIB_COMMIT STREQUAL "94faaed0c60eddd8347767f2d4d29a97c93bde8c")
        message(FATAL_ERROR "Unexpected CommonLib revision. Run scripts/setup.ps1.")
    endif()
    set(BUILD_TESTS OFF CACHE BOOL "CommonLib tests" FORCE)
    set(COMMONLIB_PREBUILT ON CACHE BOOL "Use verified matching-toolset prebuilt if available")
    set(COMMONLIB_PREBUILT_MULTICONFIG ON CACHE BOOL "Release-only prebuilt")
    set(COMMONLIB_ENABLE_IPO OFF CACHE BOOL "Faster source fallback" FORCE)
    add_subdirectory(.deps/CommonLibSSE-NG EXCLUDE_FROM_ALL)
    get_target_property(COMMONLIB_IS_IMPORTED CommonLibSSE IMPORTED)
    if(MSVC AND NOT COMMONLIB_IS_IMPORTED)
        # --parallel controls MSBuild projects; /MP also parallelizes this large
        # dependency's source files. Bound it to four compiler processes.
        target_compile_options(CommonLibSSE PRIVATE /MP4)
        # MSBuild evaluates raw AdditionalDependencies inside a single-quoted
        # expression. The project name contains an apostrophe. Use the library
        # basename and a separate search directory to avoid MSB4100.
        get_target_property(COMMONLIB_LINKS CommonLibSSE INTERFACE_LINK_LIBRARIES)
        string(REPLACE
            "${CMAKE_SOURCE_DIR}/.deps/CommonLibSSE-NG/extern/openvr/lib/win64/openvr_api.lib"
            "openvr_api" COMMONLIB_LINKS "${COMMONLIB_LINKS}")
        set_target_properties(CommonLibSSE PROPERTIES INTERFACE_LINK_LIBRARIES "${COMMONLIB_LINKS}")
        target_link_directories(CommonLibSSE INTERFACE
            "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/.deps/CommonLibSSE-NG/extern/openvr/lib/win64>")
    endif()

    add_library(SkyrimsDogma SHARED src/plugin/Plugin.cpp src/plugin/Observer.cpp src/plugin/Settings.cpp)
    target_link_libraries(SkyrimsDogma PRIVATE DogmaCore CommonLibSSE::CommonLibSSE)
    target_include_directories(SkyrimsDogma PRIVATE src)
    target_compile_definitions(SkyrimsDogma PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN DOGMA_VERSION="${PROJECT_VERSION}")
    target_compile_options(SkyrimsDogma PRIVATE /W4 /permissive- /EHsc /utf-8 /Zi)
    target_link_options(SkyrimsDogma PRIVATE /DEBUG:FULL)
    target_precompile_headers(SkyrimsDogma PRIVATE <RE/Skyrim.h> <SKSE/SKSE.h>)
    set_target_properties(SkyrimsDogma PROPERTIES PREFIX "")
endif()
