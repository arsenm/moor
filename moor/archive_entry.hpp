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
#include "types.hpp"

#include <archive_entry.h>

#include <cstdint>
#include <limits>
#include <vector>

#include <iostream>

namespace moor
{
    // Non-owning references to archive*, archive_entry*
    class MOOR_API ArchiveEntry
    {
    private:
        archive* m_archive; // Archive the entry belongs to
        archive_entry* m_entry;

        static const int s_defaultExtractFlags;

        bool extractDataImpl(archive* a,
                             unsigned char* ptr,
                             ssize_t size,
                             ssize_t entrySize);
        int copyData(archive* ar, archive* aw);

    public:
        explicit ArchiveEntry(archive* a,
                              archive_entry* e = nullptr)
            : m_archive(a),
              m_entry(e) { }

        archive_entry* entry()
        {
            return m_entry;
        }

        const archive_entry* entry() const
        {
            return m_entry;
        }

        template <class Resizeable>
        bool extractData(Resizeable& out)
        {
            typedef typename Resizeable::value_type value_type;

            if (!size_is_set())
            {
                return false;
            }

            std::int64_t entrySize = size();

            if (entrySize < 0 || entrySize >= std::numeric_limits<ssize_t>::max())
            {
                return false;
            }

            out.resize(entrySize);
            return extractDataImpl(m_archive,
                                   reinterpret_cast<unsigned char*>(out.data()),
                                   sizeof(value_type) * out.size(),
                                   entrySize);
        }

        bool extractData(std::vector<unsigned char>& out);
        bool extractData(void* out, size_t size);

        // Like extract data but extract to the given filepath instead
        bool extractDisk(const std::string& rootPath);

        time_t atime() const
        {
            return archive_entry_atime(m_entry);
        }

        long atime_nsec() const
        {
            return archive_entry_atime_nsec(m_entry);
        }

        bool atime_is_set() const
        {
            return (archive_entry_atime_is_set(m_entry) != 0);
        }

        time_t birthtime() const
        {
            return archive_entry_birthtime(m_entry);
        }

        long birthtime_nsec() const
        {
            return archive_entry_birthtime_nsec(m_entry);
        }

        bool birthtime_is_set() const
        {
            return (archive_entry_birthtime_is_set(m_entry) != 0);
        }

        time_t ctime() const
        {
            return archive_entry_ctime(m_entry);
        }

        long ctime_nsec() const
        {
            return archive_entry_ctime_nsec(m_entry);
        }

        bool ctime_is_set() const
        {
            return (archive_entry_ctime_is_set(m_entry) != 0);
        }

        FileType filetype() const
        {
            return static_cast<FileType>(archive_entry_filetype(m_entry));
        }

        void fflags(unsigned long* set, unsigned long* clear)
        {
            archive_entry_fflags(m_entry, set, clear);
        }

        std::int64_t gid() const
        {
            return archive_entry_gid(m_entry);
        }

        const char* gname() const
        {
            return archive_entry_gname(m_entry);
        }

        const wchar_t* gname_w() const
        {
            return archive_entry_gname_w(m_entry);
        }

        const char* hardlink() const
        {
            return archive_entry_hardlink(m_entry);
        }

        const wchar_t* hardlink_w() const
        {
            return archive_entry_hardlink_w(m_entry);
        }

        std::int64_t ino() const
        {
            return archive_entry_ino(m_entry);
        }

        std::int64_t ino64() const
        {
            return archive_entry_ino64(m_entry);
        }

        bool ino_is_set() const
        {
            return (archive_entry_ino_is_set(m_entry) != 0);
        }

        __LA_MODE_T mode() const
        {
            return archive_entry_mode(m_entry);
        }

        time_t mtime() const
        {
            return archive_entry_mtime(m_entry);
        }

        long mtime_nsec() const
        {
            return archive_entry_mtime_nsec(m_entry);
        }

        bool mtime_is_set() const
        {
            return (archive_entry_mtime_is_set(m_entry) != 0);
        }

        unsigned int nlink() const
        {
            return archive_entry_nlink(m_entry);
        }

        const char* pathname() const
        {
            return archive_entry_pathname(m_entry);
        }

        const wchar_t* pathname_w() const
        {
            return archive_entry_pathname_w(m_entry);
        }

        __LA_MODE_T perm() const
        {
            return archive_entry_perm(m_entry);
        }

        const char* sourcepath() const
        {
            return archive_entry_sourcepath(m_entry);
        }

        const wchar_t* sourcepath_w() const
        {
            return archive_entry_sourcepath_w(m_entry);
        }

        std::int64_t size() const
        {
            return archive_entry_size(m_entry);
        }

        bool size_is_set() const
        {
            return archive_entry_size_is_set(m_entry);
        }

        const char* strmode() const
        {
            return archive_entry_strmode(m_entry);
        }

        const char* symlink() const
        {
            return archive_entry_symlink(m_entry);
        }

        const wchar_t* symlink_w() const
        {
            return archive_entry_symlink_w(m_entry);
        }

        std::int64_t uid() const
        {
            return archive_entry_uid(m_entry);
        }

        const char* uname() const
        {
            return archive_entry_uname(m_entry);
        }

        const wchar_t* uname_w() const
        {
            return archive_entry_uname_w(m_entry);
        }

        void set_atime(std::int64_t t, long x)
        {
            archive_entry_set_atime(m_entry, t, x);
        }

        void unset_atime()
        {
            archive_entry_unset_atime(m_entry);
        }

        void set_birthtime(std::int64_t t, long x)
        {
            archive_entry_set_birthtime(m_entry, t, x);
        }

        void unset_birthtime()
        {
            archive_entry_unset_birthtime(m_entry);
        }

        void set_ctime(std::int64_t t, long x)
        {
            archive_entry_set_ctime(m_entry, t, x);
        }

        void unset_ctime()
        {
            archive_entry_unset_ctime(m_entry);
        }

        void set_filetype(FileType type)
        {
            archive_entry_set_filetype(m_entry, static_cast<int>(type));
        }

        void set_fflags(long set, long clear)
        {
            archive_entry_set_fflags(m_entry, set, clear);
        }

        void set_gid(std::int64_t g)
        {
            archive_entry_set_gid(m_entry, g);
        }

        void set_gname(const char* n)
        {
            archive_entry_set_gname(m_entry, n);
        }

        void set_hardlink(const char* s)
        {
            archive_entry_set_hardlink(m_entry, s);
        }

        void set_ino(std::int64_t i)
        {
            archive_entry_set_ino(m_entry, i);
        }

        void set_ino64(std::int64_t i)
        {
            archive_entry_set_ino64(m_entry, i);
        }

        void set_mode(__LA_MODE_T mode)
        {
            archive_entry_set_mode(m_entry, mode);
        }

        void set_mtime(time_t t, long i)
        {
            archive_entry_set_mtime(m_entry, t, i);
        }

        void unset_mtime()
        {
            archive_entry_unset_mtime(m_entry);
        }

        void set_nlink(unsigned int n)
        {
            archive_entry_set_nlink(m_entry, n);
        }

        void set_pathname(const char* n)
        {
            archive_entry_set_pathname(m_entry, n);
        }

        void set_perm(__LA_MODE_T perm)
        {
            archive_entry_set_perm(m_entry, perm);
        }

        void set_size(std::int64_t s)
        {
            archive_entry_set_size(m_entry, s);
        }

        void unset_size()
        {
            archive_entry_unset_size(m_entry);
        }

        void set_symlink(const char* s)
        {
            archive_entry_set_symlink(m_entry, s);
        }

        void set_uid(std::int64_t i)
        {
            archive_entry_set_uid(m_entry, i);
        }

        void set_uname(const char* s)
        {
            archive_entry_set_uname(m_entry, s);
        }

        int update_uname_utf8(const char* s)
        {
            return archive_entry_update_uname_utf8(m_entry, s);
        }
    };
}

