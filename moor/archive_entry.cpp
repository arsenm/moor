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

#include "archive_entry.hpp"
#include "moor_utils.hpp"
#include "archive_write_disk.hpp"

#include <cassert>
#include <system_error>


// Select which attributes we want to restore.
const int moor::ArchiveEntry::s_defaultExtractFlags = ARCHIVE_EXTRACT_TIME
                                                    | ARCHIVE_EXTRACT_PERM
                                                    | ARCHIVE_EXTRACT_ACL
                                                    | ARCHIVE_EXTRACT_FFLAGS;

bool moor::ArchiveEntry::extractDataImpl(archive* a,
                                         unsigned char* out,
                                         ssize_t outSize,
                                         ssize_t entrySize)
{
  ssize_t readIndex = 0;

  while (true)
  {
    ssize_t r = archive_read_data(a,
                                  &out[readIndex],
                                  outSize - readIndex);
    if (r == 0)
        return true;

    if (r < ARCHIVE_OK && r != ARCHIVE_WARN)
    {
      const char* errStr = archive_error_string(a);
      throw std::system_error(archive_errno(a),
                              std::generic_category(),
                              errStr ? errStr : "");
    }

    readIndex += r;
    if (readIndex == entrySize)
      return true;
  }
}

bool moor::ArchiveEntry::extractData(void* out, size_t outSize)
{
  assert(m_entry);

  if (!size_is_set())
  {
      return false;
  }

  std::int64_t entrySize = size();
  if (entrySize < 0 || entrySize >= std::numeric_limits<ssize_t>::max())
  {
      return false;
  }

  return extractDataImpl(m_archive,
                         static_cast<unsigned char*>(out),
                         outSize,
                         static_cast<ssize_t>(entrySize));
}

int moor::ArchiveEntry::copyData(archive* ar, archive* aw)
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

  return ARCHIVE_OK;
}

bool moor::ArchiveEntry::extractDisk(const std::string& rootPath)
{
  ArchiveWriteDisk disk(s_defaultExtractFlags);
  std::string fullPath(rootPath);
  fullPath += '/';
  fullPath.append(pathname());

  set_pathname(fullPath.c_str());

  int rc = disk.writeHeader(m_entry);
  if (rc != ARCHIVE_OK)
  {
      moor::throwArchiveError(disk);
  }

  if (!size_is_set())
  {
    return true;
  }

  std::int64_t entrySize = size();
  if (entrySize < 0)
  {
    return false;
  }

  rc = copyData(m_archive, disk);
  if (rc != ARCHIVE_OK)
  {
      moor::throwArchiveError(disk);
  }

  return true;
}
