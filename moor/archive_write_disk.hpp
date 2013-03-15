/*
 * Copyright (c) 2013 Mohammad Mehdi Saboorian
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

#include "archive_writer.hpp"

#include <cstdint>


namespace moor
{
    class MOOR_API ArchiveWriteDisk : public ArchiveWriter
    {
    public:
        ArchiveWriteDisk(int flags)
            : ArchiveWriter(archive_write_disk_new())
        {
            archive_write_disk_set_options(m_archive, flags);
            archive_write_disk_set_standard_lookup(m_archive);
        }

        ~ArchiveWriteDisk()
        {
            close();
        }

        virtual void close() override
        {
            archive_write_close(m_archive);
            archive_write_free(m_archive);
        }

        int writeHeader(archive_entry* entry)
        {
            return archive_write_header(m_archive, entry);
        }

        int writeDataBlock(const void* buf, size_t size, std::int64_t offset)
        {
            return archive_write_data_block(m_archive, buf, size, offset);
        }
    };
}
