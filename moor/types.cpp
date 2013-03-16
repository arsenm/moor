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

#include "types.hpp"


const char* moor::showFormat(moor::Format format)
{
    switch (format)
    {
        case moor::Format::CPIO:
            return "CPIO";

        case moor::Format::PAX:
            return "PAX";

        case moor::Format::Tar:
            return "GNU Tar";

        case moor::Format::ISO9660:
            return "ISO 9660";

        case moor::Format::Zip:
            return "Zip";

        case moor::Format::Empty:
            return "Empty";

        case moor::Format::Ar:
            return "Ar";

        case moor::Format::CAB:
            return "CAB";

        case moor::Format::RAR:
            return "CAB";

        case moor::Format::Zip7:
            return "7-Zip";

        case moor::Format::XAR:
            return "xar";

        default:
            return "unknown";
    }
}

const char* moor::showFilter(moor::Filter filter)
{
    switch (filter)
    {
        case moor::Filter::None:
            return "none";

        case moor::Filter::Gzip:
            return "gzip";

        case moor::Filter::Bzip2:
            return "bzip2";

        case moor::Filter::Compress:
            return "compress";

        case moor::Filter::LZMA:
            return "lzma";

        case moor::Filter::Xz:
            return "xz";

        case moor::Filter::UU:
            return "uu";

        case moor::Filter::RPM:
            return "rpm";

        case moor::Filter::LZip:
            return "lzip";

        case moor::Filter::LRZip:
            return "lrzip";

        case moor::Filter::LZOP:
            return "lzop";

        case moor::Filter::GRZip:
            return "grzip";

        default:
            return "unknown";
    }
}

const char* moor::showFileType(moor::FileType fileType)
{
    switch (fileType)
    {
        case moor::FileType::IFMT:
            return "ifmt";

        case moor::FileType::Regular:
            return "regular";

        case moor::FileType::Link:
            return "link";

        case moor::FileType::Socket:
            return "socket";

        case moor::FileType::Char:
            return "char";

        case moor::FileType::Block:
            return "block";

        case moor::FileType::Directory:
            return "directory";

        case moor::FileType::FIFO:
            return "fifo";

        default:
            return "unknown";
    }
}

