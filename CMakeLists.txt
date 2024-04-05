cmake_minimum_required(VERSION 3.16)
project(badecs VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(INSTALL_GTEST OFF)

include(FetchContent)
FetchContent_Declare(GSL
        GIT_REPOSITORY "https://github.com/microsoft/GSL"
        GIT_TAG "v4.0.0"
        GIT_SHALLOW ON
)
FetchContent_MakeAvailable(GSL)
FetchContent_Declare(googletest
        GIT_REPOSITORY "https://github.com/google/googletest"
        GIT_TAG "v1.14.0"
        GIT_SHALLOW ON
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

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
        src/View.cpp
        src/View.h
)
target_link_libraries(badecs PUBLIC Microsoft.GSL::GSL)

add_executable(badecs_main
        src/main.cpp
)
target_link_libraries(badecs_main PRIVATE badecs)

enable_testing()
add_executable(badecs_test
        test/badecs_test.cpp
)
target_link_libraries(badecs_test PRIVATE badecs gtest_main)
include(GoogleTest)
gtest_discover_tests(badecs_test)

find_package(Doxygen)
if (DOXYGEN_FOUND)
    doxygen_add_docs(
            docs
            src
            COMMENT "Generate documentation"
    )
endif (DOXYGEN_FOUND)