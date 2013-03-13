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
#include "archive_entry.hpp"
#include "archive_read_disk.hpp"
#include "memory_writer_callback.hpp"

#include <archive.h>
#include <archive_entry.h>

#include <fstream>
#include <stdexcept>
#include <system_error>

#include <sys/types.h>
#include <sys/stat.h>


int moor::ArchiveWriter::openCallbackWrapper(archive*, void* ud)
{
    WriterCallbackData* wcb = reinterpret_cast<WriterCallbackData*>(ud);
    return wcb->m_open(wcb->m_writer, wcb->m_userData);
}

ssize_t moor::ArchiveWriter::writeCallbackWrapper(archive*,
                                                  void* ud,
                                                  const void* buffer,
                                                  size_t size)
{
    WriterCallbackData* wcb = reinterpret_cast<WriterCallbackData*>(ud);
    return wcb->m_write(wcb->m_writer, wcb->m_userData, buffer, size);
}

int moor::ArchiveWriter::closeCallbackWrapper(archive*, void* ud)
{
    WriterCallbackData* wcb = reinterpret_cast<WriterCallbackData*>(ud);
    return wcb->m_close(wcb->m_writer, wcb->m_userData);
}

moor::ArchiveWriter::ArchiveWriter(const std::string& archive_file_name_,
                                   const moor::Format format_,
                                   const moor::Filter filter_)
    : Archive(archive_write_new(), archive_file_name_),
      m_entry(m_archive, archive_entry_new()),
      m_format(format_),
      m_filter(filter_),
      m_callbackData(),
      m_buffer(new char[bufferSize()]),
      m_open(true)
{
    // Set archive format
    checkError(archive_write_set_format(m_archive, static_cast<int>(m_format)), true);

    // Set archive compression
    checkError(archive_write_add_filter(m_archive, static_cast<int>(m_filter)), true);
    checkError(openFilename(cfilename()), true);
}

moor::ArchiveWriter::ArchiveWriter(std::vector<unsigned char>& out_buffer_,
                                   const moor::Format format_,
                                   const moor::Filter filter_)
    : Archive(archive_write_new()),
      m_entry(m_archive, archive_entry_new()),
      m_format(format_),
      m_filter(filter_),
      m_callbackData(),
      m_buffer(new char[bufferSize()]),
      m_open(true)
{
    // Set archive format
    checkError(archive_write_set_format(m_archive, static_cast<int>(m_format)), true);

    // Set archive filter
    checkError(archive_write_add_filter(m_archive, static_cast<int>(m_filter)), true);
    checkError(openMemory(out_buffer_), true);
}

moor::ArchiveWriter::ArchiveWriter(unsigned char* out_buffer_,
                                   size_t* size_,
                                   const moor::Format format_,
                                   const moor::Filter filter_)
    : Archive(archive_write_new()),
      m_entry(m_archive, archive_entry_new()),
      m_format(format_),
      m_filter(filter_),
      m_callbackData(nullptr),
      m_buffer(new char[bufferSize()]),
      m_open(true)
{
    // Set archive format
    checkError(archive_write_set_format(m_archive, static_cast<int>(m_format)), true);

    // Set archive filter
    checkError(archive_write_add_filter(m_archive, static_cast<int>(m_filter)), true);
    checkError(openMemory(out_buffer_, size_), true);
}

moor::ArchiveWriter::ArchiveWriter(OpenCallback openCB,
                                   WriteCallback writeCB,
                                   CloseCallback closeCB,
                                   const moor::Format format_,
                                   const moor::Filter filter_,
                                   void* userData)
    : Archive(archive_write_new()),
      m_entry(m_archive, archive_entry_new()),
      m_format(format_),
      m_filter(filter_),
      m_buffer(new char[bufferSize()]),
      m_callbackData(WriterCallbackData::create(*this,
                                                openCB,
                                                writeCB,
                                                closeCB,
                                                userData)),
      m_open(true)
{
    // Set archive format
    checkError(archive_write_set_format(m_archive, static_cast<int>(m_format)), true);

    // Set archive filter
    checkError(archive_write_add_filter(m_archive, static_cast<int>(m_filter)), true);

    int r = archive_write_open(m_archive,
                               m_callbackData.get(),
                               ArchiveWriter::openCallbackWrapper,
                               ArchiveWriter::writeCallbackWrapper,
                               ArchiveWriter::closeCallbackWrapper);
    checkError(r);
}

moor::ArchiveWriter::~ArchiveWriter()
{
    close();
}

int moor::ArchiveWriter::writeHeader(archive_entry* e)
{
    return archive_write_header(m_archive, e);
}

int moor::ArchiveWriter::openFilename(const char* path)
{
    return archive_write_open_filename(m_archive, path);
}

int moor::ArchiveWriter::openMemory(std::vector<unsigned char>& outBuf)
{
    return write_open_memory(m_archive, &outBuf);
}

int moor::ArchiveWriter::openMemory(void* buf, size_t* bufSize)
{
    return archive_write_open_memory(m_archive, buf, *bufSize, bufSize);
}

void moor::ArchiveWriter::addHeader(const std::string& entry_name_,
                                    const FileType entry_type_,
                                    const long long size_,
                                    const int permission_)
{
    m_entry.clear();
    m_entry.set_pathname(entry_name_.c_str());
    m_entry.set_perm(permission_);
    m_entry.set_filetype(entry_type_);
    m_entry.set_size(size_);
    checkError(writeHeader(m_entry));
}

void moor::ArchiveWriter::addHeader(const std::string& filePath,
                                    const struct stat* statBuf)
{
    ArchiveReadDisk disk;

    m_entry.clear();
    m_entry.set_pathname(filePath.c_str());
    checkError(disk.entryFromFile(m_entry, -1, statBuf));
    checkError(writeHeader(m_entry));
}

void moor::ArchiveWriter::addContent(const char b)
{
    archive_write_data(m_archive, &b, 1);
}

void moor::ArchiveWriter::addContent(const void* data, const unsigned long long size)
{
    archive_write_data(m_archive, data, size);
}

void moor::ArchiveWriter::addFinish()
{
    archive_write_finish_entry(m_archive);
}

ssize_t moor::ArchiveWriter::writeData(const void* buf, size_t bufSize)
{
    return archive_write_data(m_archive, buf, bufSize);
}

void moor::ArchiveWriter::writeFileData(const char* path)
{
    std::ifstream file(path, std::ios::in);

    while (file.good())
    {
        file.read(m_buffer.get(), bufferSize());
        writeData(m_buffer.get(), static_cast<size_t>(file.gcount()));
    }

#if 0
    int fd = open(archive_entry_sourcepath(m_entry), O_RDONLY);
    auto len = read(fd, buf, sizeof(buf));

    while (len > 0)
    {
        archive_write_data(m_archive, buf, len);
        len = read(fd, buf, sizeof(buf));
    }

    ::close(fd);
#endif
}

void moor::ArchiveWriter::addFile(const std::string& file_path)
{
    struct stat file_stat;

    if (stat(file_path.c_str(), &file_stat) < 0)
    {
        throw std::system_error(std::error_code(errno, std::generic_category()));
    }

    addHeader(file_path, &file_stat);

    if (!S_ISREG(file_stat.st_mode))
    {
        throw std::system_error(std::make_error_code(std::errc::not_supported));
    }

    writeFileData(file_path.c_str());
    addFinish();
}

void moor::ArchiveWriter::addFile(const std::string& entry_name,
                                  const void* data,
                                  unsigned long long size)
{
    addHeader(entry_name, FileType::Regular, size);
    addContent(data, size);
    addFinish();
}

void moor::ArchiveWriter::addDiskPath(const std::string& path, ArchiveMatch* match)
{
    moor::ArchiveReadDisk disk;

    checkError(disk.open(path.c_str()), true);

    if (match)
    {
        disk.checkError(disk.setMatchFilter(*match), true);
    }

    while (true)
    {
        int r = disk.nextHeader2(m_entry);
        if (r == ARCHIVE_EOF)
        {
            break;
        }

        disk.checkError(r, true);
        disk.descend();

        r = writeHeader(m_entry);
        checkError(r, true);

        writeFileData(m_entry.sourcepath());
    }
}

void moor::ArchiveWriter::addDirectory(const std::string& directory_name)
{
    addHeader(directory_name, FileType::Directory, 0777);
    addFinish();
}

void moor::ArchiveWriter::close()
{
    if (m_open)
    {
        m_open = false;

        if (m_archive)
        {
            archive_write_close(m_archive);
            archive_write_free(m_archive);
        }

        if (m_entry)
        {
            archive_entry_free(m_entry);
        }
    }
}
