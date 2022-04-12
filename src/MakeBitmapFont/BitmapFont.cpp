//****************************************************************************
// Copyright © 2022 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2022-03-15.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "BitmapFont.hpp"
#include "FreeTypeWrapper.hpp"
#include "Ystring/Utf32.hpp"

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

    BitmapCharData get_size(FT_GlyphSlot glyph, unsigned x, unsigned y)
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

    std::pair<unsigned, unsigned> get_size(freetype::Face& face, uint32_t ch)
    {
        face.load_char(ch, FT_LOAD_BITMAP_METRICS_ONLY);
        auto glyph = face->glyph;
        auto bmp = glyph->bitmap;
        return {bmp.width, bmp.rows};
    }

    std::pair<unsigned, unsigned>
    get_max_glyph_size(freetype::Face& face, std::string_view chars)
    {
        unsigned max_width = 0, max_height = 0;
        for (const auto ch: ystring::to_utf32(chars))
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
                            std::string_view chars)
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
        image_width += (8 - n);
    std::unordered_map<char32_t, BitmapCharData> char_map;
    yimage::Image image(yimage::PixelType::MONO_8,
                        image_width,
                        glyph_height * grid_height);
    yimage::MutableImageView mut_image = image;
    auto u32_chars = ystring::to_utf32(chars);
    for (unsigned i = 0; i < u32_chars.size(); ++i)
    {
        const auto ch = u32_chars[i];
        face.load_char(ch, FT_LOAD_RENDER);
        const auto x = (i % grid_width) * glyph_width;
        const auto y = (i / grid_width) * glyph_height;

        auto glyph = face->glyph;
        const auto& data = char_map.insert({ch,
                                            get_size(glyph, x, y)}).first->second;
        yimage::ImageView glyph_img(glyph->bitmap.buffer,
                                    yimage::PixelType::MONO_8,
                                    data.width,
                                    data.height);
        paste(glyph_img, x, y, mut_image);
    }

    return BitmapFont(std::move(char_map), std::move(image));
}
