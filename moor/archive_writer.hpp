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

#include "moor_build_config.hpp"
#include "types.hpp"
#include "archive.hpp"

#include <iterator>
#include <string>
#include <vector>


struct archive;
struct archive_entry;
struct stat;

namespace moor
{
  class ArchiveMatch;

  class MOOR_API ArchiveWriter : public Archive
  {
  public:
    ArchiveWriter(const std::string& archive_file_name,
                  const Format format,
                  const Filter compression);
    ArchiveWriter(std::vector<unsigned char>& out_buffer,
                  const Format format,
                  const Filter compression);
    ArchiveWriter(unsigned char* out_buffer,
                  size_t* size,
                  const Format format,
                  const Filter compression);
    virtual ~ArchiveWriter() override;

    // Add the file / directories under file_path and their content to
    // the archive
    void addDiskPath(const std::string& file_path,
                     ArchiveMatch* match = nullptr);

    // Add an entry and its content from a real file
    void addFile(const std::string& file_path);

    // Add a file entry with the given data for the content
    template <class Iter>
    void addFile(const std::string& entry_name,
                 const Iter entry_contents_begin,
                 const Iter entry_contents_end,
                 long long size = -1);
    void addFile(const std::string& entry_name,
                 const void* data,
                 const unsigned long long size);

    void addFile(const std::string& entry_name,
                 const std::string& content)
    {
        addFile(entry_name, content.c_str(), content.size());
    }

    void addDirectory(const std::string& directory_name);
    virtual void close() override;

  private:
    void addHeader(const std::string& entry_name,
                   const FileType entry_type,
                   const long long size = 0,
                   const int permission = 0644);
    void addHeader(const std::string& file_path,
                   const struct stat* file_stat = nullptr);
    void addContent(const char byte);
    void addContent(const void* bytes,
                    const unsigned long long size);
    void addFinish();

    void writeFileData(const char* path);

    archive_entry* m_entry;
    const Format m_format;
    const Filter m_filter;
    bool m_open;
  };

  template <class Iter>
  void ArchiveWriter::addFile(const std::string& entry_name,
                              const Iter entry_contents_begin,
                              const Iter entry_contents_end,
                              long long size)
  {
    size = size > -1
         ? size
         : std::distance(entry_contents_begin, entry_contents_end);
    addHeader(entry_name, FileType::Regular, size);
    for (Iter it = entry_contents_begin; it != entry_contents_end; ++it)
    {
      addContent(*it);
    }

    addFinish();
  }
}

