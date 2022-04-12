//****************************************************************************
// Copyright © 2021 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2021-12-04.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include <cmath>
#include <filesystem>
#include <iostream>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <Argos/Argos.hpp>
#include <Yimage/Yimage.hpp>
#include <Yson/JsonWriter.hpp>
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

void write(Yson::Writer& writer, const BitmapCharData& data)
{
    constexpr auto FLAT = Yson::JsonParameters(Yson::JsonFormatting::FLAT);

    writer.beginObject();
    writer.key("position").beginArray(FLAT);
    writer.value(data.x).value(data.y);
    writer.endArray();
    writer.key("size").beginArray(FLAT);
    writer.value(data.width).value(data.height);
    writer.endArray();
    writer.key("bearing").beginArray(FLAT);
    writer.value(data.bearing_x).value(data.bearing_y);
    writer.endArray();
    writer.key("advance").value(data.advance);
    writer.endObject();
}

int main(int argc, char* argv[])
{
    try
    {
        const auto args = parse_arguments(argc, argv);

        auto font_path = args.value("FONT").as_string();
        auto font_size = args.value("--size").as_uint(12);
        auto font = make_bitmap_font(font_path,
                                     args.value("--size").as_uint(12),
                                     args.value("TEXT").as_string());

        auto result_path = std::filesystem::path(font_path).filename()
                               .replace_extension().string()
                               + "_" + std::to_string(font_size);
        Yson::JsonWriter writer(result_path + ".json",
                                 Yson::JsonFormatting::FORMAT);
        writer.beginObject();
        for (auto [ch, data] : font.all_char_data())
        {
            writer.key(ystring::from_utf32(ch));
            write(writer, data);
        }
        writer.endObject();

        yimage::write_png(result_path + ".png", font.image());
   }
    catch (std::exception& ex)
    {
        std::cerr << ex.what() << "\n";
    }
    return 0;
}
