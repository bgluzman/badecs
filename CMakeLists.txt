cmake_minimum_required(VERSION 3.16)
project(badecs VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(GSL
        GIT_REPOSITORY "https://github.com/microsoft/GSL"
        GIT_TAG "v4.0.0"
        GIT_SHALLOW ON
)
FetchContent_MakeAvailable(GSL)

add_executable(badecs
        src/main.cpp
        src/World.h
        src/World.cpp
        src/Common.h
        src/Common.cpp
        src/Column.cpp
        src/Column.h
        src/ComponentRegistry.cpp
        src/ComponentRegistry.h
        src/EntityHandle.cpp
        src/EntityHandle.h
        src/EntityRegistry.cpp
        src/EntityRegistry.h
)
target_link_libraries(badecs PRIVATE Microsoft.GSL::GSL)