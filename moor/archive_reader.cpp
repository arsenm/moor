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

#include <boost/filesystem.hpp>

using namespace moor;

namespace
{
  class ScopedWriteDisk
  {
  private:
    archive* m_archive;

  public:
    ScopedWriteDisk(int flags)
      : m_archive(archive_write_disk_new())
    {
        if (!m_archive)
        {
            throw std::bad_alloc();
        }

        archive_write_disk_set_options(m_archive, flags);
        archive_write_disk_set_standard_lookup(m_archive);
    }

    ~ScopedWriteDisk()
    {
      archive_write_close(m_archive);
      archive_write_free(m_archive);
    }

    operator archive*()
    {
      return m_archive;
    }

    operator const archive*() const
    {
      return m_archive;
    }
  };
}


ArchiveReader::ArchiveReader(const std::string& archive_file_name_)
  : m_archive_file_name(archive_file_name_),
    m_in_buffer(),
    m_archive(archive_read_new()),
    m_open(true)
{
  init();
  int ec = archive_read_open_filename(m_archive,
                                      m_archive_file_name.c_str(),
                                      10240);
  checkError(ec, true);
}

ArchiveReader::ArchiveReader(void* in_buffer_, const size_t size_)
  : m_archive_file_name(),
    m_in_buffer(),
    m_archive(archive_read_new()),
    m_open(true)
{
  init();
  checkError(archive_read_open_memory(m_archive, in_buffer_, size_), true);
}

ArchiveReader::ArchiveReader(std::vector<unsigned char>&& in_buffer_)
  : m_archive_file_name(),
    m_in_buffer(std::move(in_buffer_)),
    m_archive(archive_read_new()),
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

int ArchiveReader::copyData(archive* ar, archive* aw)
{
  while (true)
  {
    const void* buff;
    size_t size;
    __LA_INT64_T offset;

    int r = archive_read_data_block(ar, &buff, &size, &offset);
    if (r == ARCHIVE_EOF)
      return ARCHIVE_OK;
    if (r != ARCHIVE_OK)
      return r;
    r = archive_write_data_block(aw, buff, size, offset);
    if (r != ARCHIVE_OK)
      return r;
  }
}

bool ArchiveReader::extractNext(const std::string& root_path_)
{
  /* Select which attributes we want to restore. */
  const int flags = ARCHIVE_EXTRACT_TIME
                  | ARCHIVE_EXTRACT_PERM
                  | ARCHIVE_EXTRACT_ACL
                  | ARCHIVE_EXTRACT_FFLAGS;
  ScopedWriteDisk a(flags);

  struct archive_entry* entry;
  auto r = archive_read_next_header(m_archive, &entry);
  if (r == ARCHIVE_EOF)
    return false;

  checkError(r);

  archive_entry_set_pathname(entry,
      (boost::filesystem::path(root_path_) /
      archive_entry_pathname(entry)).string().c_str());
  checkError(archive_write_header(a, entry));
  if (archive_entry_size(entry) > 0)
    checkError(copyData(m_archive, a));

  return true;
}

std::pair<std::string, std::vector<unsigned char>> ArchiveReader::extractNext()
{
  auto result = std::make_pair(std::string(), std::vector<unsigned char>());

  struct archive_entry* entry;
  int ec = archive_read_next_header(m_archive, &entry);
  if (ec == ARCHIVE_EOF)
    return result;

  checkError(ec);

  result.first = archive_entry_pathname(entry);
  std::int64_t entry_size = archive_entry_size(entry);
  if (entry_size > 0)
  {
    ssize_t read_index = 0;
    result.second.resize(entry_size);
    while (true)
    {
      ssize_t r = archive_read_data(m_archive,
                                    &result.second[read_index],
                                    result.second.size() - read_index);
      if (r == 0)
        break;
      if (r < ARCHIVE_OK)
        checkError(r);

      read_index += r;
      if (read_index == entry_size)
        break;
    }
  }

  return result;
}

void ArchiveReader::checkError(const int _err_code,
                               const bool _close_before_throw)
{
  int archiveErrno = 0;
  const char* errStr = nullptr;

  if (_err_code == ARCHIVE_FATAL)
  {
    // Close might clear the archive errno, so get it first
    archiveErrno = archive_errno(m_archive);
    errStr = archive_error_string(m_archive);

    if (_close_before_throw)
    {
      close();
    }
  }

  if (_err_code != ARCHIVE_OK && _err_code != ARCHIVE_WARN)
  {
    throw std::system_error(archiveErrno,
                            std::generic_category(),
                            errStr ? errStr : "");
  }
}

void ArchiveReader::close()
{
  if (m_open)
  {
    if (m_archive != NULL)
    {
      archive_read_close(m_archive);
      archive_read_free (m_archive);
    }

    m_open = false;
  }
}
