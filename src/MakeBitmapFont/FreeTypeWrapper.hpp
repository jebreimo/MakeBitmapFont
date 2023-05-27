//****************************************************************************
// Copyright © 2022 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2022-03-12.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace Freetype
{
    struct FreeTypeException : std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };

    #define FREETYPE_THROW_3_(file, line, msg) \
        throw ::Freetype::FreeTypeException(file ":" #line ": " msg)

    #define FREETYPE_THROW_2_(file, line, msg) \
        FREETYPE_THROW_3_(file, line, msg)

    #define FREETYPE_THROW(msg) \
        FREETYPE_THROW_2_(__FILE__, __LINE__, msg)

    struct FaceDeleter
    {
        void operator()(FT_Face library)
        {
            FT_Done_Face(library);
        }
    };

    using FacePtr = std::unique_ptr<std::decay_t<decltype(*FT_Face())>,
        FaceDeleter>;

    class Face
    {
    public:
        Face();

        explicit Face(FT_Face face);

        Face(Face&& rhs) noexcept;

        ~Face();

        Face& operator=(Face&& rhs) noexcept;

        const FT_FaceRec* operator->() const;

        FT_Face operator->();

        [[nodiscard]]
        const FT_FaceRec* get() const;

        [[nodiscard]]
        FT_Face get();

        FacePtr release();

        void select_charmap(FT_Encoding encoding);

        void set_pixel_sizes(FT_UInt width, FT_UInt height);

        void load_char(FT_ULong char_code, FT_Int32 load_flags);
    private:
        FacePtr face_;
    };

    struct LibraryDeleter
    {
        void operator()(FT_Library library)
        {
            FT_Done_FreeType(library);
        }
    };

    using LibraryPtr = std::unique_ptr<std::decay_t<decltype(*FT_Library())>,
                                       LibraryDeleter>;

    class Library
    {
    public:
        Library();

        explicit Library(FT_Library library);

        Library(Library&& rhs) noexcept;

        ~Library();

        Library& operator=(Library&& rhs) noexcept;

        LibraryPtr release();

        Face new_face(const std::string& font_path, FT_Long face_index = 0);
    private:
        LibraryPtr library_;
    };
}
