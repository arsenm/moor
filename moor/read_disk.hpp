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

#include <archive.h>

#include <functional>


namespace moor
{
  class ArchiveReadDisk
  {
  private:
      /*
    struct FilterCallbackData
    {
      std::function<int(ArchiveEntry, void*)>* m_f;
      void* m_ud;

      FilterCallbackData(std::function<int(ArchiveEntry, void*)>* f_ = nullptr,
                         void* ud_ = nullptr
            )
        : m_f(f_),
          m_ud(ud_) { }
    };
      */

    archive* m_archive;
      //FilterCallbackData m_fcd;

  public:
    ArchiveReadDisk()
      : m_archive(archive_read_disk_new())
    {
      if (!m_archive)
      {
        throw std::bad_alloc();
      }

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

    /*
    int setFilterCallback(std::function<int(ArchiveEntry, void*)>& f, void* ud)
    {
      m_fcd.m_f = &f;
      m_fcd.m_ud = ud;
      return archive_read_disk_set_matching(
        m_archive,
        [](archive* a, void* ud, archive_entry* e) -> int
        {
          FilterCallbackData* fcd = reinterpret_cast<FilterCallbackData*>(ud);
          return (*fcd->m_f)(ArchiveEntry(a, e), fcd->m_ud);
        },
        &m_fcd);
    }

    void clearFilterCallback()
    {
        m_fcd.m_f = nullptr;
        m_fcd.m_ud = nullptr;
        archive_read_disk_set_metadata_filter_callback(m_archive,
                                                       nullptr,
                                                       nullptr);
    }
    */
  };
}

