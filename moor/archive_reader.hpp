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

struct archive;
struct archive_entry;

namespace moor
{
  class MOOR_API ArchiveReader : public Archive
  {
  public:
    ArchiveReader(const std::string& archive_file_name);
    ArchiveReader(void* in_buffer, const size_t size);
    ArchiveReader(std::vector<unsigned char>&& in_buffer);
    ~ArchiveReader();

    // Returns false at EOF
    bool extractNext(const std::string& root_path);

    // Returns empty filename at EOF
    std::pair<std::string, std::vector<unsigned char>> extractNext();

    // Check ArchiveIterator::isAtEnd for EOF
    ArchiveIterator begin();

  private:
    static const int s_defaultExtractFlags;

    void init();
    void checkError(const int err_code,
                    const bool close_before_throw = false);
    void close();
    static int copyData(archive* ar, archive* aw);

    std::vector<unsigned char> m_in_buffer;

    bool m_open;
  };
}

