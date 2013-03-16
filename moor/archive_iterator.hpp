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

#include "archive_entry.hpp"
#include "types.hpp"

#include <archive.h>

#include <cassert>
#include <system_error>


namespace moor
{
    class MOOR_API ArchiveIterator
    {
    private:
        archive* m_archive;
        ArchiveEntry m_state;

        void throwArchiveError()
        {
            assert(m_archive);
            const char* errStr = archive_error_string(m_archive);

            throw std::system_error(archive_errno(m_archive),
                                    std::generic_category(),
                                    errStr ? errStr : "");
        }

    public:
        typedef ArchiveEntry value_type;
        typedef value_type& reference;
        typedef value_type* pointer;

        ArchiveIterator(ArchiveIterator&& old)
            : m_archive(old.m_archive),
              m_state(std::move(old.m_state))
        {

        }

        reference operator*()
        {
            return m_state;
        }

        pointer operator->()
        {
            return &m_state;
        }

        ArchiveIterator& operator++()
        {
            assert(m_archive && "Trying to iterate past end");

            int r = m_state.nextHeader();
            if (r != ARCHIVE_OK)
            {
                throwArchiveError();
            }

            return *this;
        }

        // There's no sense using operator== since
        // archive_read_next_header gives us back the same pointer each
        // time. The position is also side-effecty on the archive
        // itself, so that isn't useful either
        bool isAtEnd() const
        {
            return (m_state.entry() == nullptr);
        }

        explicit ArchiveIterator(archive* a = nullptr)
            : m_archive(a),
              m_state(a)
        {
            if (!m_archive)
            {
                return;
            }

            int r = m_state.nextHeader();
            if (r != ARCHIVE_OK)
            {
                throwArchiveError();
            }
        }
    };
}
