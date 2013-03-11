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

#include "archive_reader.hpp"

#include <archive.h>
#include <archive_entry.h>
#include <stdexcept>
#include <system_error>

using namespace moor;

// Select which attributes we want to restore.
const int ArchiveReader::s_defaultExtractFlags = ARCHIVE_EXTRACT_TIME
                                               | ARCHIVE_EXTRACT_PERM
                                               | ARCHIVE_EXTRACT_ACL
                                               | ARCHIVE_EXTRACT_FFLAGS;

ArchiveReader::ArchiveReader(const std::string& archive_file_name_)
  : Archive(archive_read_new(), archive_file_name_),
    m_in_buffer(),
    m_open(true)
{
  init();
  int ec = archive_read_open_filename(m_archive,
                                      cfilename(),
                                      10240);
  checkError(ec, true);
}

ArchiveReader::ArchiveReader(void* in_buffer_, const size_t size_)
  : Archive(archive_read_new()),
    m_in_buffer(),
    m_open(true)
{
  init();
  checkError(archive_read_open_memory(m_archive, in_buffer_, size_), true);
}

ArchiveReader::ArchiveReader(std::vector<unsigned char>&& in_buffer_)
  : Archive(archive_read_new()),
    m_in_buffer(std::move(in_buffer_)),
    m_open(true)
{
  init();
  int ec = archive_read_open_memory(m_archive,
                                    m_in_buffer.data(),
                                    m_in_buffer.size());
  checkError(ec, true);
}

void ArchiveReader::init()
{
  checkError(archive_read_support_format_all(m_archive), true);
  checkError(archive_read_support_filter_all(m_archive), true);
}

ArchiveReader::~ArchiveReader()
{
  close();
}

ArchiveIterator ArchiveReader::begin()
{
  return ArchiveIterator(m_archive);
}

void ArchiveReader::close()
{
  if (m_open)
  {
    if (m_archive)
    {
      archive_read_close(m_archive);
      archive_read_free(m_archive);
    }

    m_open = false;
  }
}
