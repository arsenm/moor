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

#include "archive.hpp"

#include <archive.h>


using namespace moor;


int Archive::fileCount() const
{
    return archive_file_count(m_archive);
}

int Archive::filterCount() const
{
    return archive_filter_count(m_archive);
}

const char* Archive::formatName() const
{
    return archive_format_name(m_archive);
}

const char* Archive::filterName() const
{
    return archive_filter_name(m_archive, -1);
}

int Archive::errorNumber() const
{
    return archive_errno(m_archive);
}

const char* Archive::errorString() const
{
    return archive_error_string(m_archive);
}

void Archive::throwError(int errCode,
                         bool closeBeforeThrow)
{
  std::system_error err = systemError();
  if (errCode == ARCHIVE_FATAL && closeBeforeThrow)
  {
    close();
  }

  throw err;
}

