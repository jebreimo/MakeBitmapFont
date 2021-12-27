//****************************************************************************
// Copyright © 2021 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2021-12-04.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include <cmath>
#include <iostream>
#include <ft2build.h>

#include <Argos/Argos.hpp>
#include <Xyz/Xyz.hpp>
#include <Yimage/Yimage.hpp>
#include <Yson/JsonWriter.hpp>

#include FT_FREETYPE_H

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

struct CharData
{
    Xyz::Vector<unsigned, 2> size;
    Xyz::Vector<int, 2> bearing;
    int advance = 0;
};

std::pair<unsigned, unsigned> get_best_grid_size(unsigned count)
{
    if (count == 0)
        return {0, 0};

    struct Best {unsigned width; unsigned height; unsigned remainder;};
    Best best = {count, 1, count};
    auto width = unsigned(ceil(sqrt(count)));
    while (true)
    {
        auto height = count / width;
        const auto n = count % width;
        if (n)
            ++height;
        if (width > height * 2)
            break;
        if (n == 0)
            return {width, height};
        if (width - n < best.remainder)
            best = {width, height, width - n};
        ++width;
    }
    return {best.width, best.height};
}

void write(Yson::Writer& writer, const CharData& data,
           const Xyz::Vector<unsigned, 2>& point)
{
    constexpr auto FLAT = Yson::JsonParameters(Yson::JsonFormatting::FLAT);

    writer.beginObject();
    writer.key("position").beginArray(FLAT);
    writer.value(point[0]).value(point[1]);
    writer.endArray();
    writer.key("size").beginArray(FLAT);
    writer.value(data.size[0]).value(data.size[1]);
    writer.endArray();
    writer.key("bearing").beginArray(FLAT);
    writer.value(data.bearing[0]).value(data.bearing[1]);
    writer.endArray();
    writer.key("advance").value(data.advance);
    writer.endObject();
}

CharData get_size(FT_Face face, uint32_t c)
{
    if (auto error = FT_Load_Char(face, c, FT_LOAD_BITMAP_METRICS_ONLY))
    {
        std::cerr << "FreeType error: " << error << ".\n";
        return {};
    }
    auto glyph = face->glyph;
    auto bmp = glyph->bitmap;
    return {.size = {bmp.width, bmp.rows},
            .bearing = {glyph->bitmap_left, glyph->bitmap_top},
            .advance = int(glyph->advance.x)};
}

std::pair<unsigned, unsigned>
get_max_glyph_size(FT_Face face, const std::string& chars)
{
    unsigned width = 0, height = 0;
    for (const auto  c : chars)
    {
        auto cdata = get_size(face, c);
        if (cdata.size[0] > width)
            width = cdata.size[0];
        if (cdata.size[1] > height)
            height = cdata.size[1];
    }
    return {width, height};
}

int main(int argc, char* argv[])
{
    const auto args = parse_arguments(argc, argv);

    FT_Library library;

    if (auto error = FT_Init_FreeType(&library))
    {
        std::cerr << "FreeType error: " << error << ".\n";
        return 1;
    }

    auto font_path = args.value("FONT").as_string();

    FT_Face face;
    if (auto error = FT_New_Face(library,
                                 font_path.c_str(),
                                 0,
                                 &face))
    {
        std::cerr << "FreeType can't load: " << font_path
                  << ". Error: " << error << ".\n";
        return 1;
    }

    auto font_size = args.value("--size").as_uint(12);
    if (auto error = FT_Set_Pixel_Sizes(face, 0, font_size))
    {
        std::cerr << "FreeType error: " << error << ".\n";
        return 1;
    }

    auto text = args.value("TEXT").as_string();
    auto [glyph_width, glyph_height] = get_max_glyph_size(face, text);
    glyph_width += 1;
    glyph_height += 1;
    const auto [grid_width, grid_height] = get_best_grid_size(text.size());
    std::cout << glyph_width << ", " << glyph_height << "\n"
              << grid_width << ", " << grid_height << "\n";
    yimage::Image image(glyph_width * grid_width,
                        glyph_height * grid_height,
                        yimage::PixelType::MONO8);
    yimage::MutableImageView mut_image = image;
    Yson::JsonWriter writer(text + ".json", Yson::JsonFormatting::FORMAT);
    writer.beginObject();
    for (unsigned i = 0; i < text.size(); ++i)
    {
        const auto c = text[i];
        writer.key(std::string(1, c));
        const auto x = (i % grid_width) * glyph_width;
        const auto y = (i / grid_width) * glyph_height;
        write(writer, get_size(face, c), {x, y});

        if (auto error = FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cerr << "FreeType error: " << error << ".\n";
            return -1;
        }
        auto glyph = face->glyph;
        yimage::ImageView glyph_img(glyph->bitmap.buffer,
                                    glyph->bitmap.width,
                                    glyph->bitmap.rows,
                                    yimage::PixelType::MONO8);
        mut_image.paste(glyph_img, x, y);
    }
    yimage::write_png(text + ".png", image);
    writer.endObject();

    return 0;
}
