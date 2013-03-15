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

#include "archive_match.hpp"
#include "archive_reader.hpp"

#include <archive.h>


namespace moor
{
    class MOOR_API ArchiveReadDisk : public ArchiveReader
    {
    public:
        ArchiveReadDisk()
            : ArchiveReader(archive_read_disk_new())
        {
#if !defined(_WIN32) || defined(__CYGWIN__)
            int ec = archive_read_disk_set_standard_lookup(m_archive);

            if (ec != ARCHIVE_OK)
            {
                throw std::bad_alloc();
            }

#endif
        }

        ~ArchiveReadDisk()
        {
            close();
        }

        virtual void close() override
        {
            archive_read_close(m_archive);
            archive_read_free(m_archive);
        }

        int open(const char* path)
        {
            return archive_read_disk_open(m_archive, path);
        }

        int setMatchFilter(ArchiveMatch& match)
        {
            if (match.hasCallback())
            {
                return archive_read_disk_set_matching(m_archive,
                                                      match,
                                                      ArchiveMatch::matchCallbackWrapper,
                                                      match.callbackUserData());
            }
            else
            {
                return archive_read_disk_set_matching(m_archive,
                                                      match,
                                                      nullptr,
                                                      nullptr);
            }
        }

        int descend()
        {
            return archive_read_disk_descend(m_archive);
        }

        int nextHeader2(archive_entry* e)
        {
            return archive_read_next_header2(m_archive, e);
        }

        int entryFromFile(archive_entry* e,
                          int fd = -1,
                          const struct stat* statBuf = nullptr)
        {
            return archive_read_disk_entry_from_file(m_archive,
                                                     e,
                                                     fd,
                                                     statBuf);
        }
    };
}

