cmake_minimum_required(VERSION 3.15)

project(SignerServer)

# 平台宏定义
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    add_definitions(-D _WIN_PLATFORM_)

    link_libraries(ws2_32)
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
    add_definitions(-D _MAC_PLATFORM_)
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    add_definitions(-D _LINUX_PLATFORM_)
endif()



# 设置CPP标准
set(CMAKE_CXX_STANDARD 17)

file(GLOB SOURCE_FILES "./src/*.cpp")

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} "./include/mongoose/mongoose.c")