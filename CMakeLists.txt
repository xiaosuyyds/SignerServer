cmake_minimum_required(VERSION 3.15)

project(SignerServer)

# 平台宏定义
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    add_definitions(-D _WIN_PLATFORM_)

    link_libraries(ws2_32)

    enable_language(ASM)
    SET(CMAKE_ASM_FLAGS "${CFLAGS} -x assembler-with-cpp")
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
    add_definitions(-D _MAC_PLATFORM_)
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    add_definitions(-D _LINUX_PLATFORM_)
endif()

# 设置CPP标准
set(CMAKE_CXX_STANDARD 17)

file(GLOB SOURCE_FILES "./src/*.cpp" "./src/*.asm")

add_library(SignerServer SHARED ${SOURCE_FILES})

if(MSVC)
    target_link_options(SignerServer PRIVATE
        /DEF:${CMAKE_CURRENT_SOURCE_DIR}/version.def
    )
endif()