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

#include "archive_entry.hpp"

#include <archive.h>

#include <functional>
#include <memory>


namespace moor
{
    typedef std::function<void(ArchiveEntry, void*)> ReadDiskMatchCallback;


    class ArchiveMatch
    {
        friend class ArchiveReadDisk;

    private:
        struct MatchCallbackData
        {
        public:
            ReadDiskMatchCallback* m_f;
            void* m_ud;

            static std::unique_ptr<MatchCallbackData> create(ReadDiskMatchCallback* f,
                                                             void* ud)
            {
                return std::unique_ptr<MatchCallbackData>(new MatchCallbackData(f, ud));
            }

        private:
            MatchCallbackData(ReadDiskMatchCallback* f_ = nullptr,
                              void* ud_ = nullptr)
                : m_f(f_),
                  m_ud(ud_) { }
        };

        archive* m_match;
        std::unique_ptr<MatchCallbackData> m_cb;


        archive* raw()
        {
            return m_match;
        }

    public:
        ArchiveMatch()
            : m_match(archive_match_new()),
              m_cb(nullptr)
        {
            if (!m_match)
            {
                throw std::bad_alloc();
            }
        }

        ~ArchiveMatch()
        {
            archive_match_free(m_match);
        }

        void addCallback(ReadDiskMatchCallback f, void* ud)
        {
            m_cb = MatchCallbackData::create(&f, ud);
        }

        bool hasCallback() const
        {
            return (m_cb != nullptr);
        }

        static void matchCallbackWrapper(archive* a, void* ud, archive_entry* e)
        {
            MatchCallbackData* mcd = reinterpret_cast<MatchCallbackData*>(ud);
            return (*mcd->m_f)(ArchiveEntry(a, e), mcd->m_ud);
        }

        // Return the wrapper user data which will be passed as the
        // userdata to the wrapper.
        void* callbackUserData() const
        {
            return m_cb.get();
        }

        bool excluded(ArchiveEntry& e)
        {
            return archive_match_excluded(m_match, e.raw());
        }

        int excludePattern(const char* pattern)
        {
            return archive_match_exclude_pattern(m_match, pattern);
        }

        int excludePatternW(const wchar_t* wpattern)
        {
            return archive_match_exclude_pattern_w(m_match, wpattern);
        }

        int includePattern(const char* pattern)
        {
            return archive_match_include_pattern(m_match, pattern);
        }

        int includePatternW(const wchar_t* wpattern)
        {
            return archive_match_include_pattern_w(m_match, wpattern);
        }

        int includeTime(int flag, time_t sec, long nsec)
        {
            return archive_match_include_time(m_match, flag, sec, nsec);
        }

        int includeDate(int flag, const char* datestr)
        {
            return archive_match_include_date(m_match, flag, datestr);
        }

        int includeDateW(int flag, const wchar_t* wdatestr)
        {
            return archive_match_include_date_w(m_match, flag, wdatestr);
        }

        int includeFileTime(int flag, const char* file)
        {
            return archive_match_include_file_time(m_match, flag, file);
        }

        int includeFileTime(int flag, const wchar_t* wfile)
        {
            return archive_match_include_file_time_w(m_match, flag, wfile);
        }

        int includeUID(std::int64_t id)
        {
            return archive_match_include_uid(m_match, id);
        }

        int includeGID(std::int64_t id)
        {
            return archive_match_include_gid(m_match, id);
        }

        int includeUname(const char* name)
        {
            return archive_match_include_uname(m_match, name);
        }

        int includeUnameW(const wchar_t* wname)
        {
            return archive_match_include_uname_w(m_match, wname);
        }

        int includeGname(const char* name)
        {
            return archive_match_include_gname(m_match, name);
        }

        int includeGnameW(const wchar_t* wname)
        {
            return archive_match_include_gname_w(m_match, wname);
        }
    };
}
