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

#include <archive.h>

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

        virtual ~Archive()
        {

        }

        void throwError(int errCode, bool closeBeforeThrow)
        {
            std::system_error err = systemError();

            if (errCode == ARCHIVE_FATAL && closeBeforeThrow)
            {
                close();
            }

            throw err;
        }

    public:
        virtual void close() = 0;

        archive* raw()
        {
            return m_archive;
        }

        const archive* raw() const
        {
            return m_archive;
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
        int fileCount() const
        {
            return archive_file_count(m_archive);
        }

        int filterCount() const
        {
            return archive_filter_count(m_archive);
        }

        const char* formatName() const
        {
            return archive_format_name(m_archive);
        }

        const char* filterName() const
        {
            return archive_filter_name(m_archive, -1);
        }

        int errorNumber() const
        {
            return archive_errno(m_archive);
        }

        const char* errorString() const
        {
            return archive_error_string(m_archive);
        }

        inline void checkError(int errCode,
                               bool closeBeforeThrow = false)
        {
            if (errCode != ARCHIVE_OK && errCode != ARCHIVE_WARN)
            {
                throwError(errCode, closeBeforeThrow);
            }
        }
    };
}
