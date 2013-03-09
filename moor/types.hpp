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

#include <archive.h>
#include <archive_entry.h>


namespace moor
{
  enum class Format
  {
    CPIO = ARCHIVE_FORMAT_CPIO,
    PAX = ARCHIVE_FORMAT_TAR,
    Tar = ARCHIVE_FORMAT_TAR_GNUTAR,
    ISO9660 = ARCHIVE_FORMAT_ISO9660,
    Zip = ARCHIVE_FORMAT_ZIP,
    Empty = ARCHIVE_FORMAT_EMPTY,
    Ar = ARCHIVE_FORMAT_AR,
    CAB = ARCHIVE_FORMAT_CAB,
    RAR = ARCHIVE_FORMAT_RAR,
    Zip7 = ARCHIVE_FORMAT_7ZIP
  };

  enum class Filter
  {
    None = ARCHIVE_FILTER_NONE,
    Gzip = ARCHIVE_FILTER_GZIP,
    Bzip2 = ARCHIVE_FILTER_BZIP2,
    Compress = ARCHIVE_FILTER_COMPRESS,
    LZMA = ARCHIVE_FILTER_LZMA,
    Xz = ARCHIVE_FILTER_XZ,
    UU = ARCHIVE_FILTER_UU,
    RPM = ARCHIVE_FILTER_RPM,
    LZip = ARCHIVE_FILTER_LZIP,
    LRZip = ARCHIVE_FILTER_LRZIP,
    LZOP = ARCHIVE_FILTER_LZOP,
    GRZip = ARCHIVE_FILTER_GRZIP
  };

  enum class FileType
  {
    Regular = AE_IFREG,
    Directory = AE_IFDIR
  };
}
