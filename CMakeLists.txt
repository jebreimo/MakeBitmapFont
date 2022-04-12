cmake_minimum_required(VERSION 3.21)
project(MakeBitmapFont)

set(CMAKE_CXX_STANDARD 20)

if (DEFINED LOCAL_SOURCE_PACKAGE_DIR OR DEFINED ENV{LOCAL_SOURCE_PACKAGE_DIR})
    if (NOT LOCAL_SOURCE_PACKAGE_DIR)
        set(LOCAL_SOURCE_PACKAGE_DIR $ENV{LOCAL_SOURCE_PACKAGE_DIR})
    endif ()
endif ()

find_package(Freetype REQUIRED)

include(FetchContent)

if (DEFINED LOCAL_SOURCE_PACKAGE_DIR)
    FetchContent_Declare(yimage
        URL "file://${LOCAL_SOURCE_PACKAGE_DIR}/Yimage-0.0.8.tar.gz")
else()
    FetchContent_Declare(yimage
        GIT_REPOSITORY "https://github.com/jebreimo/Yimage.git"
        GIT_TAG main)
endif()

FetchContent_Declare(argos
    GIT_REPOSITORY "https://github.com/jebreimo/Argos.git"
    GIT_TAG "v1.1.3")
FetchContent_Declare(xyz
    GIT_REPOSITORY "https://github.com/jebreimo/Xyz.git"
    GIT_TAG "v0.1.3")
FetchContent_Declare(yson
    GIT_REPOSITORY "https://github.com/jebreimo/Yson.git"
    GIT_TAG "v1.4.3"
    )
FetchContent_Declare(ystring2
    GIT_REPOSITORY "https://github.com/jebreimo/Ystring2.git"
    GIT_TAG "v2.1.1")

FetchContent_MakeAvailable(argos xyz yimage yson ystring2)

add_executable(MakeBitmapFont main.cpp FreeTypeWrapper.cpp FreeTypeWrapper.hpp BitmapFont.cpp BitmapFont.hpp)

target_link_libraries(MakeBitmapFont
    Argos::Argos
    Freetype::Freetype
    Xyz::Xyz
    Yimage::Yimage
    Yson::Yson
    Ystring2::Ystring
    )
