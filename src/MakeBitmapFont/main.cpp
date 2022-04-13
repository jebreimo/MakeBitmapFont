//****************************************************************************
// Copyright © 2021 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2021-12-04.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include <filesystem>
#include <iostream>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <Argos/Argos.hpp>
#include <Ystring/Utf32.hpp>

#include "BitmapFont.hpp"

argos::ParsedArguments parse_arguments(int argc, char* argv[])
{
    using namespace argos;
    return ArgumentParser(argv[0])
        .add(Argument("FONT"))
        .add(Argument("TEXT"))
        .add(Option{"-s", "--size"}.argument("PIXELS")
             .help("Set the font size (height)."))
        .parse(argc, argv);
}

int main(int argc, char* argv[])
{
    try
    {
        const auto args = parse_arguments(argc, argv);

        auto font_path = args.value("FONT").as_string();
        auto font_size = args.value("--size").as_uint(12);
        auto u32_chars = ystring::to_utf32(args.value("TEXT").as_string());
        auto font = make_bitmap_font(font_path,
                                     args.value("--size").as_uint(12),
                                     std::span(u32_chars));

        auto result_path = std::filesystem::path(font_path).filename()
                               .replace_extension().string()
                               + "_" + std::to_string(font_size);
        write_font(font, result_path);
   }
    catch (std::exception& ex)
    {
        std::cerr << ex.what() << "\n";
    }
    return 0;
}
