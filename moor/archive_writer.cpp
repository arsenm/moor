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
#include <boost/scope_exit.hpp>

#include <fstream>
#include <stdexcept>
#include <system_error>

#include <sys/stat.h>

using namespace moor;


void ArchiveWriter::checkError(const int _err_code
  , const bool _close_before_throw)
{
  if (_close_before_throw && _err_code == ARCHIVE_FATAL)
    Close();
  if (_err_code != ARCHIVE_OK && _err_code != ARCHIVE_WARN)
  {
    throw std::system_error(archive_errno(m_archive),
                            std::generic_category(),
                            archive_error_string(m_archive));
  }
}


ArchiveWriter::ArchiveWriter(const std::string& _archive_file_name
    , const Format& _format, const Filter& _filter)
  : m_open (true), m_archive (archive_write_new()), m_entry (archive_entry_new())
  , m_archive_file_name (_archive_file_name)
   , m_format(_format), m_filter(_filter)
{
  //set archive format
  checkError(archive_write_set_format(m_archive, (int) m_format), true);
  //set archive compression
  checkError(archive_write_add_filter(m_archive, (int) m_filter), true);
  checkError(archive_write_open_filename(m_archive, m_archive_file_name.c_str())
      , true);
}

ArchiveWriter::ArchiveWriter(std::vector<unsigned char>& _out_buffer
    , const Format& _format, const Filter& _filter)
  : m_open(true), m_archive(archive_write_new()), m_entry (archive_entry_new())
  , m_archive_file_name (""), m_format(_format)
  , m_filter(_filter)
{
  //set archive format
  checkError(archive_write_set_format(m_archive, (int) m_format), true);
  //set archive filter
  checkError(archive_write_add_filter(m_archive, (int) m_format), true);
  checkError(write_open_memory(m_archive, &_out_buffer), true);
}

ArchiveWriter::ArchiveWriter(unsigned char* _out_buffer, size_t* _size
    , const Format& _format, const Filter& _filter)
  : m_open(true), m_archive(archive_write_new()), m_entry (archive_entry_new())
  , m_archive_file_name (""), m_format(_format)
  , m_filter(_filter)
{
  //set archive format
  checkError(archive_write_set_format(m_archive, (int) m_format), true);
  //set archive filter
  checkError(archive_write_add_filter(m_archive, (int) m_filter), true);
  checkError(archive_write_open_memory(m_archive, _out_buffer, *_size, _size)
   , true);
}

ArchiveWriter::~ArchiveWriter()
{
  Close();
}

void ArchiveWriter::addHeader(const std::string& _entry_name
  , const FileType _entry_type, const long long _size, const int _permission)
{
  m_entry = archive_entry_clear(m_entry);
  archive_entry_set_pathname(m_entry, _entry_name.c_str());
  archive_entry_set_perm(m_entry, _permission);
  archive_entry_set_filetype(m_entry, (int) _entry_type);
  archive_entry_set_size(m_entry, _size);
  checkError(archive_write_header(m_archive, m_entry));
}

void ArchiveWriter::addHeader(const std::string& _file_path)
{
  struct archive* a = archive_read_disk_new();
  BOOST_SCOPE_EXIT (&a)
  {
    if(a != NULL)
    {
      archive_read_close(a);
      archive_read_free(a);
    }
  }
  BOOST_SCOPE_EXIT_END

  m_entry = archive_entry_clear(m_entry);
  archive_entry_set_pathname(m_entry, _file_path.c_str());
  checkError(archive_read_disk_entry_from_file(a, m_entry, -1, 0));
  checkError(archive_write_header(m_archive, m_entry));
}

void ArchiveWriter::addContent(const char _byte)
{
  archive_write_data(m_archive, &_byte, 1);
}

void ArchiveWriter::addContent(const char* _bytes, const unsigned long long _size)
{
  archive_write_data(m_archive, _bytes, _size);
}

void ArchiveWriter::addFinish()
{
  archive_write_finish_entry(m_archive);
}

void ArchiveWriter::AddFile (const std::string& _file_path)
{
  if (!boost::filesystem::exists(_file_path))
  {
      throw std::system_error(std::make_error_code(std::errc::no_such_file_or_directory));
  }

  boost::filesystem::file_status file_stat
    = boost::filesystem::status(_file_path);
  boost::filesystem::perms perm = file_stat.permissions();
  long long file_size = boost::filesystem::file_size(_file_path);

  addHeader(_file_path);

  if (!boost::filesystem::is_regular_file(file_stat))
  {
    throw std::system_error(std::make_error_code(std::errc::not_supported));
  }

  std::fstream entry_file(_file_path.c_str(), std::ios::in);
  char buff[8192];
  while (entry_file.good())
  {
    entry_file.read(buff, 8192);
    archive_write_data(m_archive, buff
      , static_cast<size_t>(entry_file.gcount()));
  }
  entry_file.close();

  addFinish();
}
void ArchiveWriter::AddFile (const std::string& _entry_name
  , const unsigned char * _data , const unsigned long long _size)
{
  addHeader(_entry_name, FileType::Regular, _size);
  addContent((char*)_data, _size);
  addFinish();
}

void ArchiveWriter::AddDirectory(const std::string& _directory_name)
{
  addHeader(_directory_name, FileType::Directory, 0777);
  addFinish();
}

void ArchiveWriter::Close()
{
  if (m_open)
  {
    if (m_archive != NULL)
    {
      archive_write_close(m_archive);
      archive_write_free (m_archive);
    }
    if (m_entry != NULL)
      archive_entry_free(m_entry);

    m_open = false;
  }
}
