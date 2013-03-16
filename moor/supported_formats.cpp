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

#include "supported_formats.hpp"

#include <archive.h>


#ifdef __clang__
  #pragma clang diagnostic ignored "-Wcovered-switch-default"
  #pragma clang diagnostic ignored "-Wunreachable-code"
#endif

namespace
{
    class ScopedWriteArchive
    {
    private:
        archive* m_archive;

    public:
        ScopedWriteArchive()
            : m_archive(archive_write_new())
        {

        }

        ~ScopedWriteArchive()
        {
            archive_write_free(m_archive);
        }

        operator archive*()
        {
            return m_archive;
        }
    };

    class SupportedWriteFilters
    {
    private:
        bool m_bzip2;
        bool m_compress;
        bool m_grzip;
        bool m_gzip;
        bool m_lrzip;
        bool m_lzip;
        bool m_lzma;
        bool m_lzop;
        bool m_none;
        bool m_uuencode;
        bool m_xz;
        bool m_rpm;

    public:
        SupportedWriteFilters();

        bool isSupported(moor::Filter fmt);
    };

    SupportedWriteFilters::SupportedWriteFilters()
    {
        ScopedWriteArchive a;

        m_bzip2 = (archive_write_add_filter_bzip2(a) == ARCHIVE_OK);
        m_compress = (archive_write_add_filter_compress(a) == ARCHIVE_OK);
        m_grzip = (archive_write_add_filter_grzip(a) == ARCHIVE_OK);
        m_gzip = (archive_write_add_filter_gzip(a) == ARCHIVE_OK);
        m_lrzip = (archive_write_add_filter_lrzip(a) == ARCHIVE_OK);
        m_lzip = (archive_write_add_filter_lzip(a) == ARCHIVE_OK);
        m_lzma = (archive_write_add_filter_lzma(a) == ARCHIVE_OK);
        m_lzop = (archive_write_add_filter_lzop(a) == ARCHIVE_OK);
        m_none = (archive_write_add_filter_none(a) == ARCHIVE_OK);
        m_uuencode = (archive_write_add_filter_uuencode(a) == ARCHIVE_OK);
        m_xz = (archive_write_add_filter_xz(a) == ARCHIVE_OK);
        m_rpm = false; // libarchive only supports read for it?
    }

    bool SupportedWriteFilters::isSupported(moor::Filter fmt)
    {
        switch (fmt)
        {
            case moor::Filter::None:
                return m_none;
            case moor::Filter::Gzip:
                return m_gzip;
            case moor::Filter::Bzip2:
                return m_bzip2;
            case moor::Filter::Compress:
                return m_compress;
            case moor::Filter::LZMA:
                return m_lzma;
            case moor::Filter::Xz:
                return m_xz;
            case moor::Filter::UU:
                return m_uuencode;
            case moor::Filter::RPM:
                return m_rpm;
            case moor::Filter::LZip:
                return m_lzip;
            case moor::Filter::LRZip:
                return m_lrzip;
            case moor::Filter::LZOP:
                return m_lzop;
            case moor::Filter::GRZip:
                return m_grzip;
            default:
                return false;
        }
    }

    class SupportedWriteFormats
    {
    private:
        bool m_cpio;
        bool m_pax;
        bool m_tar;
        bool m_iso9660;
        bool m_zip;
        bool m_empty;
        bool m_ar;
        bool m_cab;
        bool m_rar;
        bool m_7zip;
        bool m_xar;

    public:
        SupportedWriteFormats();
        bool isSupported(moor::Format fmt);
    };

    SupportedWriteFormats::SupportedWriteFormats()
    {
        ScopedWriteArchive a;

        m_cpio = (archive_write_set_format_cpio(a) == ARCHIVE_OK);
        m_pax = (archive_write_set_format_pax_restricted(a) == ARCHIVE_OK);
        m_tar = (archive_write_set_format_gnutar(a) == ARCHIVE_OK);
        m_iso9660 = (archive_write_set_format_iso9660(a) == ARCHIVE_OK);
        m_zip = (archive_write_set_format_zip(a) == ARCHIVE_OK);
        m_empty = true;
        m_ar = (archive_write_set_format_ar_bsd(a) == ARCHIVE_OK);
        m_cab = false; // Read support only?
        m_rar = false;
        m_7zip = (archive_write_set_format_7zip(a) == ARCHIVE_OK);
        m_xar = (archive_write_set_format_xar(a) == ARCHIVE_OK);
    }

    bool SupportedWriteFormats::isSupported(moor::Format fmt)
    {
        switch (fmt)
        {
            case moor::Format::CPIO:
                return m_cpio;
            case moor::Format::PAX:
                return m_pax;
            case moor::Format::Tar:
                return m_tar;
            case moor::Format::ISO9660:
                return m_iso9660;
            case moor::Format::Zip:
                return m_zip;
            case moor::Format::Empty:
                return m_empty;
            case moor::Format::Ar:
                return m_ar;
            case moor::Format::CAB:
                return m_cab;
            case moor::Format::RAR:
                return m_rar;
            case moor::Format::Zip7:
                return m_7zip;
            case moor::Format::XAR:
                return m_xar;
            default:
                return false;
        }
    }
}

bool moor::writeFilterIsSupported(moor::Filter fmt)
{
    static SupportedWriteFilters supported;
    return supported.isSupported(fmt);
}

bool moor::writeFormatIsSupported(moor::Format fmt)
{
    static SupportedWriteFormats supported;
    return supported.isSupported(fmt);
}

