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

add_library(badecs
        SHARED
        src/World.h
        src/World.cpp
        src/Common.h
        src/Common.cpp
        src/Column.cpp
        src/Column.h
        src/ComponentRegistry.cpp
        src/ComponentRegistry.h
        src/EntityRegistry.cpp
        src/EntityRegistry.h
        src/Commands.cpp
        src/Commands.h
        src/Query.cpp
        src/Query.h
        src/View.cpp
        src/View.h
)
target_link_libraries(badecs PUBLIC Microsoft.GSL::GSL)

add_executable(badecs_main
        src/main.cpp
)
target_link_libraries(badecs_main PRIVATE badecs)