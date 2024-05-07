cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 17)

project(SignerServer)

# 平台宏定义
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    add_definitions(-D _WIN_PLATFORM_)

    enable_language(ASM_MASM)

    link_libraries(ws2_32)
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
    add_definitions(-D _MAC_PLATFORM_)
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    add_definitions(-D _LINUX_PLATFORM_)
endif()


file(GLOB SOURCE_FILES "./src/*.cpp" "./src/*.asm")

add_library(SignerServer SHARED ${SOURCE_FILES})

if(MSVC)
    target_link_options(SignerServer PRIVATE
        /DEF:${CMAKE_CURRENT_SOURCE_DIR}/version.def
    )
endif()