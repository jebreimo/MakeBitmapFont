cmake_minimum_required(VERSION 3.21)
project(MakeBitmapFont VERSION 0.1.2)

set(CMAKE_CXX_STANDARD 20)

find_package(Freetype REQUIRED)

include(FetchContent)

FetchContent_Declare(argos
    GIT_REPOSITORY "https://github.com/jebreimo/Argos.git"
    GIT_TAG "v1.1.271")
FetchContent_Declare(yimage
    GIT_REPOSITORY "https://github.com/jebreimo/Yimage.git"
    GIT_TAG v0.1.4)
FetchContent_Declare(yson
    GIT_REPOSITORY "https://github.com/jebreimo/Yson.git"
    GIT_TAG "v1.4.13")
FetchContent_Declare(ystring2
    GIT_REPOSITORY "https://github.com/jebreimo/Ystring2.git"
    GIT_TAG "v2.1.1")

FetchContent_MakeAvailable(argos yimage yson ystring2)

add_executable(MakeBitmapFont
    src/MakeBitmapFont/main.cpp
    src/MakeBitmapFont/FreeTypeWrapper.cpp
    src/MakeBitmapFont/FreeTypeWrapper.hpp
    src/MakeBitmapFont/BitmapFont.cpp
    src/MakeBitmapFont/BitmapFont.hpp
    )

target_link_libraries(MakeBitmapFont
    Argos::Argos
    Freetype::Freetype
    Yimage::Yimage
    Yson::Yson
    Ystring2::Ystring
    )
