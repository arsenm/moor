/*
 * Copyright (c) 2013 Matthew Arsenault
 *
 * This is part of moor, a wrapper for libarchive
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include "moor_build_config.hpp"

#include <cassert>
#include <string>
#include <system_error>


struct archive;

namespace moor
{
    class MOOR_API Archive
    {
    private:
        const std::string m_archive_file_name;

    protected:
        archive* m_archive;

        Archive(archive* a) :
            m_archive_file_name(),
            m_archive(a)
        {
          if (!m_archive)
          {
            throw std::bad_alloc();
          }
        }

        Archive(archive* a, const std::string& filename_) :
            m_archive_file_name(filename_),
            m_archive(a)
        {

        }

        ~Archive()
        {
          close();
        }

        operator archive*()
        {
            return m_archive;
        }

        operator const archive*() const
        {
            return m_archive;
        }

    public:
        virtual void close()
        {
            assert(false && "Close not implemented for type");
        }

        const std::string& filename() const
        {
            return m_archive_file_name;
        }

        const char* cfilename() const
        {
            return m_archive_file_name.c_str();
        }

        std::system_error systemError() const
        {
            const char* errStr = errorString();
            return std::system_error(errorNumber(),
                                     std::generic_category(),
                                     errStr ? errStr : "");
        }

        // Returns a count of the number of files processed by this archive object.
        int fileCount() const;
        int filterCount() const;
        const char* formatName() const;
        const char* filterName() const;

        int errorNumber() const;
        const char* errorString() const;

        void checkError(int errCode,
                        bool closeBeforeThrow = false);
    };
}
