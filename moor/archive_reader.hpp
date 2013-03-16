/*
 * Copyright (c) 2013 Mohammad Mehdi Saboorian
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

#include <string>
#include <utility>
#include <vector>

#include "moor_build_config.hpp"
#include "archive.hpp"
#include "types.hpp"

#include "archive_iterator.hpp"


namespace moor
{
    // The ArchiveReaderImpl is where all the functionality is.  The
    // ArchiveReader is the owning version which will destroy the
    // underlying archive.
    class MOOR_API ArchiveReaderImpl : public Archive
    {
        friend class ArchiveMatch;
    public:
        ArchiveReaderImpl(const std::string& archive_file_name);
        ArchiveReaderImpl(void* in_buffer, const size_t size);
        ArchiveReaderImpl(std::vector<unsigned char>&& in_buffer);
        virtual ~ArchiveReaderImpl() override
        {

        }

        // Check ArchiveIterator::isAtEnd for EOF
        ArchiveIterator begin();

    protected:
        ArchiveReaderImpl(archive* a)
            : Archive(a),
              m_in_buffer()
        {

        }

        virtual void close() override;

    private:
        static const int s_defaultExtractFlags;

        void init();
        int openFilename(const char* path, size_t blockSize = 10240);
        int openMemory(void* buffer, size_t bufferSize);

        static int copyData(archive* ar, archive* aw);
        int readDataBlock(const void** buf, size_t* size, std::int64_t* offset);

        std::vector<unsigned char> m_in_buffer;
    };

    class MOOR_API ArchiveReader : public ArchiveReaderImpl
    {
    protected:
        ArchiveReader(archive* a)
            : ArchiveReaderImpl(a) { }

    public:
        ArchiveReader(const std::string& archive_file_name)
            : ArchiveReaderImpl(archive_file_name) { }
        ArchiveReader(void* buffer, const size_t size)
            : ArchiveReaderImpl(buffer, size) { }
        ArchiveReader(std::vector<unsigned char>&& buffer)
            : ArchiveReaderImpl(std::move(buffer)) { }
        virtual ~ArchiveReader() override
        {
            close();
        }
    };
}

