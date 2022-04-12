//****************************************************************************
// Copyright © 2022 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2022-03-15.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once

#include <string_view>
#include <unordered_map>
#include <Yimage/Image.hpp>

struct BitmapCharData
{
    unsigned x = 0;
    unsigned y = 0;
    unsigned width = 0;
    unsigned height = 0;
    int bearing_x = 0;
    int bearing_y = 0;
    int advance = 0;
};

class BitmapFont
{
public:
    BitmapFont() = default;

    explicit BitmapFont(std::unordered_map<char32_t, BitmapCharData> char_data,
                        yimage::Image image);

    [[nodiscard]]
    const BitmapCharData* char_data(char32_t ch) const;

    [[nodiscard]]
    const std::unordered_map<char32_t, BitmapCharData>& all_char_data() const;

    [[nodiscard]]
    std::pair<int, int> vertical_extremes() const;

    [[nodiscard]]
    const yimage::Image& image() const;

    yimage::Image release_image();
private:
    std::unordered_map<char32_t, BitmapCharData> char_data_;
    yimage::Image image_;
};

BitmapFont make_bitmap_font(const std::string& font_path,
                            unsigned font_size,
                            std::string_view text);
