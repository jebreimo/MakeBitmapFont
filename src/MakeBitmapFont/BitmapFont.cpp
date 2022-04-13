//****************************************************************************
// Copyright © 2022 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2022-03-15.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "BitmapFont.hpp"

#include <filesystem>
#include <Yimage/Yimage.hpp>
#include <Yson/JsonReader.hpp>
#include <Yson/JsonWriter.hpp>
#include <Yson/ReaderIterators.hpp>
#include <Ystring/Ystring.hpp>

#include "FreeTypeWrapper.hpp"

BitmapFont::BitmapFont(std::unordered_map<char32_t, BitmapCharData> char_data,
                       yimage::Image image)
    : char_data_(move(char_data)),
      image_(std::move(image))
{}

const BitmapCharData* BitmapFont::char_data(char32_t ch) const
{
    if (auto it = char_data_.find(ch); it != char_data_.end())
        return &it->second;
    return nullptr;
}

const std::unordered_map<char32_t, BitmapCharData>& BitmapFont::all_char_data() const
{
    return char_data_;
}

std::pair<int, int> BitmapFont::vertical_extremes() const
{
    int max_hi = 0, min_lo = 0;
    for (auto& data: char_data_)
    {
        int hi = data.second.bearing_y;
        if (hi > max_hi)
            max_hi = hi;
        int lo = hi - int(data.second.height);
        if (lo < min_lo)
            min_lo = lo;
    }
    return {min_lo, max_hi};
}

const yimage::Image& BitmapFont::image() const
{
    return image_;
}

yimage::Image BitmapFont::release_image()
{
    return std::move(image_);
}

namespace
{
    std::pair<unsigned, unsigned> get_best_grid_size(unsigned count)
    {
        if (count == 0)
            return {0, 0};

        struct Best
        {
            unsigned width;
            unsigned height;
            unsigned remainder;
        };
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

    BitmapCharData make_char_data(FT_GlyphSlot glyph, unsigned x, unsigned y)
    {
        auto bmp = glyph->bitmap;
        return {.x = x,
                .y = y,
                .width = bmp.width,
                .height = bmp.rows,
                .bearing_x = glyph->bitmap_left,
                .bearing_y = glyph->bitmap_top,
                .advance = int(glyph->advance.x)};
    }

    std::pair<unsigned, unsigned> get_size(freetype::Face& face, char32_t ch)
    {
        face.load_char(ch, FT_LOAD_BITMAP_METRICS_ONLY);
        auto glyph = face->glyph;
        auto bmp = glyph->bitmap;
        return {bmp.width, bmp.rows};
    }

    std::pair<unsigned, unsigned>
    get_max_glyph_size(freetype::Face& face, std::span<char32_t> chars)
    {
        unsigned max_width = 0, max_height = 0;
        for (const auto ch: chars)
        {
            auto [width, height] = get_size(face, ch);
            if (width > max_width)
                max_width = width;
            if (height > max_height)
                max_height = height;
        }
        return {max_width, max_height};
    }
}

BitmapFont make_bitmap_font(const std::string& font_path,
                            unsigned font_size,
                            std::span<char32_t> chars)
{
    freetype::Library library;
    auto face = library.new_face(font_path);
    face.select_charmap(FT_ENCODING_UNICODE);
    face.set_pixel_sizes(0, font_size);
    auto [glyph_width, glyph_height] = get_max_glyph_size(face, chars);
    glyph_width += 1;
    glyph_height += 1;
    const auto[grid_width, grid_height] = get_best_grid_size(chars.size());
    auto image_width = glyph_width * grid_width;
    if (auto n = image_width % 8)
        image_width += 8 - n;
    std::unordered_map<char32_t, BitmapCharData> char_map;
    yimage::Image image(yimage::PixelType::MONO_8,
                        image_width,
                        glyph_height * grid_height);
    yimage::MutableImageView mut_image = image;
    for (unsigned i = 0; i < chars.size(); ++i)
    {
        const auto x = (i % grid_width) * glyph_width;
        const auto y = (i / grid_width) * glyph_height;

        const auto ch = chars[i];
        face.load_char(ch, FT_LOAD_RENDER);
        auto glyph = face->glyph;
        const auto& data = char_map.insert({ch,
                                            make_char_data(glyph, x, y)}).first->second;
        yimage::ImageView glyph_img(glyph->bitmap.buffer,
                                    yimage::PixelType::MONO_8,
                                    data.width,
                                    data.height);
        paste(glyph_img, x, y, mut_image);
    }

    return BitmapFont(std::move(char_map), std::move(image));
}

namespace
{
    std::pair<std::string, std::string>
    get_json_and_png_paths(std::filesystem::path path)
    {
        auto extension = ystring::to_lower(path.extension().string());
        if (extension == ".json")
        {
            const auto json_path = path;
            path.replace_extension(".png");
            return {json_path.string(), path.string()};
        }
        else if (extension == ".png")
        {
            const auto png_path = path;
            path.replace_extension(".json");
            return {path.string(), png_path.string()};
        }
        else
        {
            return {path.string() + ".json", path.string() + ".png"};
        }
    }
}

std::unordered_map<char32_t, BitmapCharData> read_font(Yson::Reader& reader)
{
    using Yson::get;
    std::unordered_map<char32_t, BitmapCharData> result;
    for (const auto& key: keys(reader))
    {
        auto item = reader.readItem();
        auto position = item["position"];
        auto size = item["size"];
        auto bearing = item["bearing"];
        auto[range, ch] = ystring::get_code_point(key, 0);
        result.insert({ch,
                       {get<unsigned>(position[0].value()),
                        get<unsigned>(position[1].value()),
                        get<unsigned>(size[0].value()),
                        get<unsigned>(size[1].value()),
                        get<int>(bearing[0].value()),
                        get<int>(bearing[1].value()),
                        get<int>(item["advance"])}});
    }
    return result;
}

BitmapFont read_font(const std::string& font_path)
{
    auto [json_path, png_path] = get_json_and_png_paths(font_path);
    Yson::JsonReader reader(json_path);
    return {read_font(reader), yimage::read_png(png_path)};
}

namespace
{
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
}

void write_font(const std::unordered_map<char32_t, BitmapCharData>& font,
                Yson::Writer& writer)
{
    writer.beginObject();
    for (auto [ch, data] : font)
    {
        writer.key(ystring::from_utf32(ch));
        write(writer, data);
    }
    writer.endObject();
}

void write_font(const BitmapFont& font, const std::string& file_name)
{
    Yson::JsonWriter writer(file_name + ".json",
                            Yson::JsonFormatting::FORMAT);
    writer.beginObject();
    for (auto [ch, data] : font.all_char_data())
    {
        writer.key(ystring::from_utf32(ch));
        write(writer, data);
    }
    writer.endObject();

    yimage::write_png(file_name + ".png", font.image());
}
