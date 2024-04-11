cmake_minimum_required(VERSION 3.16)
project(badecs VERSION 1.0 LANGUAGES CXX)

option(INSTALL_GTEST OFF)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
        GSL
        GIT_REPOSITORY "https://github.com/microsoft/GSL"
        GIT_TAG "v4.0.0"
        GIT_SHALLOW ON
)
FetchContent_MakeAvailable(GSL)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY "https://github.com/google/googletest"
        GIT_TAG "v1.14.0"
        GIT_SHALLOW ON
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)  # windows
FetchContent_MakeAvailable(googletest)

add_library(badecs INTERFACE)
target_include_directories(
        badecs
        INTERFACE
        include
)
target_link_libraries(
        badecs
        INTERFACE
        Microsoft.GSL::GSL
)

enable_testing()
add_executable(
        badecs_test
        test/badecs_test.cpp
)
target_link_libraries(
        badecs_test
        PRIVATE
        badecs
        gtest_main
)
if (WIN32)
    target_compile_options(
            badecs_test
            PRIVATE
            /W3
            /WX
            /wd4820 # compiler-inserted padding is fine
    )
else ()
    target_compile_options(
            badecs_test
            PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Werror
            -fprofile-arcs
            -ftest-coverage
    )
    target_link_options(
            badecs_test
            PRIVATE
            --coverage
    )
endif ()
include(GoogleTest)
gtest_discover_tests(badecs_test)

find_package(Doxygen)
if (DOXYGEN_FOUND)
    doxygen_add_docs(
            docs
            include
            COMMENT "Generate documentation"
    )
endif (DOXYGEN_FOUND)