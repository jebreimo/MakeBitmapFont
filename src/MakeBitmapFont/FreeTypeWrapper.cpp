//****************************************************************************
// Copyright © 2022 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2022-03-12.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "FreeTypeWrapper.hpp"

namespace freetype
{
    Library::Library()
    {
        FT_Library library;
        if (auto error = FT_Init_FreeType(&library))
            FREETYPE_THROW("FT_Init_FreeType returned " + std::to_string(error));
        library_ = LibraryPtr(library);
    }

    Library::Library(FT_Library library)
        : library_(library)
    {}

    Library::Library(Library&& rhs) noexcept
        : library_(move(rhs.library_))
    {}

    Library::~Library() = default;

    Library& Library::operator=(Library&& rhs) noexcept
    {
        library_ = move(rhs.library_);
        return *this;
    }

    LibraryPtr Library::release()
    {
        return move(library_);
    }

    Face Library::new_face(const std::string& font_path, FT_Long face_index)
    {
        FT_Face face;
        if (auto error = FT_New_Face(library_.get(),
                                     font_path.c_str(),
                                     face_index,
                                     &face))
        {
            FREETYPE_THROW("Can't load: " + font_path
                           + ". FT_New_Face returned "
                           + std::to_string(error));
        }

        return Face(face);
    }

    Face::Face() = default;

    Face::Face(FT_Face face)
        : face_(face)
    {}

    Face::Face(Face&& rhs) noexcept
        : face_(move(rhs.face_))
    {}

    Face::~Face() = default;

    Face& Face::operator=(Face&& rhs) noexcept
    {
        face_ = move(rhs.face_);
        return *this;
    }

    const FT_FaceRec* Face::operator->() const
    {
        return face_.get();
    }

    FT_Face Face::operator->()
    {
        return face_.get();
    }

    const FT_FaceRec* Face::get() const
    {
        return face_.get();
    }

    FT_Face Face::get()
    {
        return face_.get();
    }

    FacePtr Face::release()
    {
        return move(face_);
    }

    void Face::select_charmap(FT_Encoding encoding)
    {
        FT_Select_Charmap(face_.get(), encoding);
    }

    void Face::set_pixel_sizes(FT_UInt width, FT_UInt height)
    {
        if (auto error = FT_Set_Pixel_Sizes(face_.get(), width, height))
        {
            FREETYPE_THROW("FT_Set_Pixel_Sizes returned "
                           + std::to_string(error));
        }
    }

    void Face::load_char(FT_ULong char_code, FT_Int32 load_flags)
    {
        if (auto error = FT_Load_Char(face_.get(), char_code, load_flags))
        {
            FREETYPE_THROW("FT_Load_Char returned " + std::to_string(error));
        }
    }
}
