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

#include "archive_writer.hpp"
#include "memory_writer_callback.hpp"

#include <archive.h>
#include <archive_entry.h>

#include <boost/filesystem.hpp>

#include <fstream>
#include <stdexcept>
#include <system_error>

#include <sys/stat.h>

using namespace moor;

namespace
{
  class ScopedReadDisk
  {
  private:
    archive* m_archive;

  public:
    ScopedReadDisk()
      : m_archive(archive_read_disk_new())
    {
        if (!m_archive)
        {
            throw std::bad_alloc();
        }
    }

    ~ScopedReadDisk()
    {
      archive_read_close(m_archive);
      archive_read_free(m_archive);
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


void ArchiveWriter::checkError(const int err_code,
                               const bool close_before_throw)
{
  int archiveErrno = 0;
  const char* errStr = nullptr;
  if (err_code == ARCHIVE_FATAL)
  {
      archiveErrno = archive_errno(m_archive);
      errStr = archive_error_string(m_archive);
      if (close_before_throw)
      {
          close();
      }
  }

  if (err_code != ARCHIVE_OK && err_code != ARCHIVE_WARN)
  {
    throw std::system_error(archiveErrno,
                            std::generic_category(),
                            errStr ? errStr : "");
  }
}

ArchiveWriter::ArchiveWriter(const std::string& archive_file_name_,
                             const Format& format_,
                             const Filter& filter_)
  : Archive(archive_write_new(), archive_file_name_),
    m_entry(archive_entry_new()),
    m_format(format_),
    m_filter(filter_),
    m_open(true)
{
  // Set archive format
  checkError(archive_write_set_format(m_archive, (int) m_format), true);

  // Set archive compression
  checkError(archive_write_add_filter(m_archive, (int) m_filter), true);
  checkError(archive_write_open_filename(m_archive, cfilename()), true);
}

ArchiveWriter::ArchiveWriter(std::vector<unsigned char>& out_buffer_,
                             const Format& format_,
                             const Filter& filter_)
  : Archive(archive_write_new()),
    m_entry(archive_entry_new()),
    m_format(format_),
    m_filter(filter_),
    m_open(true)
{
  // Set archive format
  checkError(archive_write_set_format(m_archive, static_cast<int>(m_format)), true);

  // Set archive filter
  checkError(archive_write_add_filter(m_archive, static_cast<int>(m_filter)), true);
  checkError(write_open_memory(m_archive, &out_buffer_), true);
}

ArchiveWriter::ArchiveWriter(unsigned char* out_buffer_,
                             size_t* size_,
                             const Format& format_,
                             const Filter& filter_)
  : Archive(archive_write_new()),
    m_entry(archive_entry_new()),
    m_format(format_),
    m_filter(filter_),
    m_open(true)
{
  // Set archive format
  checkError(archive_write_set_format(m_archive, static_cast<int>(m_format)), true);

  // Set archive filter
  checkError(archive_write_add_filter(m_archive, static_cast<int>(m_filter)), true);
  checkError(archive_write_open_memory(m_archive, out_buffer_, *size_, size_), true);
}

ArchiveWriter::~ArchiveWriter()
{
  close();
}

void ArchiveWriter::addHeader(const std::string& entry_name_,
                              const FileType entry_type_,
                              const long long size_,
                              const int permission_)
{
  m_entry = archive_entry_clear(m_entry);
  archive_entry_set_pathname(m_entry, entry_name_.c_str());
  archive_entry_set_perm(m_entry, permission_);
  archive_entry_set_filetype(m_entry, static_cast<int>(entry_type_));
  archive_entry_set_size(m_entry, size_);
  checkError(archive_write_header(m_archive, m_entry));
}

void ArchiveWriter::addHeader(const std::string& file_path_)
{
  ScopedReadDisk a;

  m_entry = archive_entry_clear(m_entry);
  archive_entry_set_pathname(m_entry, file_path_.c_str());
  checkError(archive_read_disk_entry_from_file(a, m_entry, -1, 0));
  checkError(archive_write_header(m_archive, m_entry));
}

void ArchiveWriter::addContent(const char b)
{
  archive_write_data(m_archive, &b, 1);
}

void ArchiveWriter::addContent(const void* data, const unsigned long long size)
{
  archive_write_data(m_archive, data, size);
}

void ArchiveWriter::addFinish()
{
  archive_write_finish_entry(m_archive);
}

void ArchiveWriter::addFile(const std::string& file_path)
{
  boost::filesystem::file_status file_stat = boost::filesystem::status(file_path);
  if (!boost::filesystem::exists(file_stat))
  {
    throw std::system_error(std::make_error_code(std::errc::no_such_file_or_directory));
  }

  addHeader(file_path);

  if (!boost::filesystem::is_regular_file(file_stat))
  {
    throw std::system_error(std::make_error_code(std::errc::not_supported));
  }

  std::fstream entry_file(file_path.c_str(), std::ios::in);
  char buf[8192];
  while (entry_file.good())
  {
    entry_file.read(buf, sizeof(buf));
    archive_write_data(m_archive, buf, static_cast<size_t>(entry_file.gcount()));
  }
  entry_file.close();

  addFinish();
}

void ArchiveWriter::addFile(const std::string& entry_name,
                            const void* data,
                            unsigned long long size)
{
  addHeader(entry_name, FileType::Regular, size);
  addContent(data, size);
  addFinish();
}

void ArchiveWriter::addDirectory(const std::string& directory_name)
{
  addHeader(directory_name, FileType::Directory, 0777);
  addFinish();
}

void ArchiveWriter::close()
{
  if (m_open)
  {
    m_open = false;

    if (m_archive)
    {
      archive_write_close(m_archive);
      archive_write_free(m_archive);
    }

    if (m_entry)
    {
      archive_entry_free(m_entry);
    }
  }
}
