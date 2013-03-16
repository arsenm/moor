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

#include <moor/archive_iterator.hpp>
#include <moor/archive_match.hpp>
#include <moor/archive_reader.hpp>
#include <moor/archive_writer.hpp>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <system_error>
#include <vector>

#define PRINT_TEST_NAME() do                            \
    {                                                   \
        std::cout << "\nStart test " << __func__ << std::endl;  \
    } while (0)



using namespace moor;

static bool testDoesNotExist()
{
    PRINT_TEST_NAME();

    try
    {
        ArchiveReader badReader("xxxx_does_not_exist");

        std::cerr << "Should not have reached this point\n";
        return true; // Failure, we expect an exception
    }
    catch (const std::system_error& ex)
    {
        std::cerr << "Expected exception reading non-existent file: " << ex.what() << '\n';

        if (ex.code() != std::errc::no_such_file_or_directory)
        {
            std::cerr << "Unexpected error code for non-existent file\n";
            return true;
        }

        return false;
    }
}

static void writeArrayToFile(const std::string& path,
                             const std::vector<unsigned char>& v)
{
    std::ofstream of(path, std::ios::binary);

    for (auto a : v)
    {
        of << a;
    }
}

static const char testDataAArray[] = { 64, 65, 66, 67, 68 };
static const std::vector<char> testDataA10(10, 'A');
static const std::vector<char> testDataB10(10, 'B');

const std::string testDataString =
    "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod "
    "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
    "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. "
    "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat"
    "nulla pariatur. Excepteur sint occaecat cupidatat non proident, "
    "sunt in culpa qui officia deserunt mollit anim id est laborum.";


static bool testArchiveWriteImpl(ArchiveWriter& compressor)
{
    compressor.addFile("test_data_dir/bar.txt");
    compressor.addDirectory("test_data_dir/foo_dir");

    compressor.addFile("a_array.txt",
                       testDataAArray,
                       sizeof(testDataAArray) / sizeof(testDataAArray[0]));

    compressor.addFile("b_list.txt",
                       testDataA10.begin(),
                       testDataA10.end());

    compressor.addFile("vector_b.txt",
                       testDataB10.begin(),
                       testDataB10.end());
    compressor.close();

    return false;
}

static bool testArchiveDataCheck()
{
    PRINT_TEST_NAME();
    const std::string entryName("lorem_ipsum.txt");
    std::vector<unsigned char> buf;

    {
        ArchiveWriter compressor(buf, Format::PAX, Filter::Gzip);
        compressor.addFile(entryName, testDataString);
    }

    ArchiveReader reader(std::move(buf));

    for (auto it = reader.begin(); !it.isAtEnd(); ++it)
    {
        std::string path(it->pathname());

        if (entryName != path)
        {
            std::cerr << "Entry name does not match: "
                      << path
                      << " vs. expected "
                      << entryName
                      << '\n';
            return true;
        }

        std::vector<unsigned char> out;

        if (!it->extractData<std::vector<unsigned char>>(out))
        {
            std::cerr << "Error extracting test data\n";
            return true;
        }

        std::string extractedCopy(out.begin(), out.end());

        if (extractedCopy != testDataString)
        {
            std::cerr << "Extracted string does not match:\n"
                      << "  Original string size: " << testDataString.size() << '\n'
                      << "  Extracted string size: " << extractedCopy.size() << '\n';

            return true;
        }
    }

    // TODO: Is this actually useful? It isn't updated until you archive_read_next_header?
    if (reader.fileCount() != 1)
    {
        std::cerr << "Expected 1 files, found " << reader.fileCount() << '\n';
        return true;
    }

    return false;
}

// Write result directly to path
static bool testArchiveWriteFile(const std::string& path)
{
    PRINT_TEST_NAME();

    try
    {
        ArchiveWriter compressor(path, Format::PAX, Filter::Gzip);
        return testArchiveWriteImpl(compressor);
    }
    catch (const std::runtime_error& ex)
    {
        std::cerr << "Error writing file archive: " << ex.what() << '\n';
        return true;
    }
}

// Write to path and first go through temporary buffer
static bool testArchiveWriteMemory(const std::string& path)
{
    PRINT_TEST_NAME();

    try
    {
        std::vector<unsigned char> lout;
        ArchiveWriter compressor(lout, Format::PAX, Filter::Gzip);

        if (testArchiveWriteImpl(compressor))
        {
            return true;
        }

        writeArrayToFile(path, lout);
        return false;
    }
    catch (const std::runtime_error& ex)
    {
        std::cerr << "Error writing memory archive: " << ex.what() << '\n';
        return true;
    }
}

static bool testArchiveWriteCallback()
{
    PRINT_TEST_NAME();
    std::vector<unsigned char> out;

    try
    {
        ArchiveWriter compressor(
            [](ArchiveWriter&, void*) -> int
        {
            std::cout << "Open callback\n";
            return ARCHIVE_OK;
        },
        [&out](ArchiveWriter&, void*, const void * buf, size_t size) -> ssize_t
        {
            std::cout << "Write callback size " << size << '\n';
            const unsigned char* p = reinterpret_cast<const unsigned char*>(buf);
            std::copy(p,
            p + size,
            std::back_inserter(out));
            return size;
        },
        [](ArchiveWriter&, void*) -> int
        {
            std::cout << "Close callback\n";
            return ARCHIVE_OK;
        },
        Format::PAX,
        Filter::Gzip);

        compressor.addFile("lorem_ipsum.txt", testDataString);
        compressor.close();

        ArchiveReader reader(std::move(out));

        std::vector<unsigned char> copied;

        for (auto it = reader.begin(); !it.isAtEnd(); ++it)
        {
            if (!it->extractData<std::vector<unsigned char>>(copied))
            {
                std::cerr << "Error extracting test data\n";
                return true;
            }
        }

        std::string copyStr(copied.begin(), copied.end());

        if (copyStr != testDataString)
        {
            std::cerr << "Extracted string does not match:\n"
                      << "  Original string size: " << testDataString.size() << '\n'
                      << "  Extracted string size: " << copyStr.size() << '\n';

            return true;
        }

        return false;
    }
    catch (const std::runtime_error& ex)
    {
        std::cerr << "Error writing callback archive: " << ex.what() << '\n';
        return true;
    }
}

static bool printArchiveEntries(ArchiveReader& reader)
{
    for (ArchiveIterator it = reader.begin(); !it.isAtEnd(); ++it)
    {
        std::cout << "Extracting " << it->pathname()
                  << " size " << it->size()
                  << '\n';

        std::vector<unsigned char> data;

        if (!it->extractData<std::vector<unsigned char>>(data))
        {
            std::cerr << "Error extracting data\n";
            return true;
        }

        if (static_cast<size_t>(it->size()) != data.size())
        {
            std::cerr << "Read data size doesn't match entry size\n";
            return true;
        }
    }

    return false;
}

static bool testArchiveRead(const std::string& path)
{
    PRINT_TEST_NAME();

    try
    {
        ArchiveReader reader1(path);
        std::ifstream iff(path, std::ios::binary);
        iff.seekg(0, std::ios::end);
        auto size = iff.tellg();
        iff.seekg(0, std::ios::beg);
        std::vector<unsigned char> ff(size);

        while (iff.good())
        {
            iff.read(reinterpret_cast<char*>(ff.data()), size);
        }

        ArchiveReader reader(std::move(ff));
        return printArchiveEntries(reader)
               && printArchiveEntries(reader1);
    }
    catch (const std::runtime_error& ex)
    {
        std::cerr << "Error reading archive: " << ex.what() << '\n';
        return true;
    }
}

static bool testCompressDirectory(const std::string& path)
{
    PRINT_TEST_NAME();

    try
    {
        {
            ArchiveWriter compressor(path, Format::PAX, Filter::Gzip);
            compressor.addDiskPath("test_data_dir");
        }

        ArchiveReader reader(path);
        return printArchiveEntries(reader);
    }
    catch (const std::runtime_error& ex)
    {
        std::cerr << "Error writing archive from directory: " << ex.what() << '\n';
        return true;
    }
}

static bool testExtractDirectory(const std::string& path)
{
    PRINT_TEST_NAME();

    try
    {
        ArchiveReader reader(path);

        std::string extractedPath("extracted_");
        extractedPath += path;

        for (auto it = reader.begin(); !it.isAtEnd(); ++it)
        {
            if (!it->extractDisk(extractedPath))
            {
                std::cerr << "Error extracting directory\n";
                return true;
            }
        }

        return false;
    }
    catch (const std::runtime_error& ex)
    {
        std::cerr << "Exception extracting directory: " << ex.what() << '\n';
        return true;
    }
}

static bool testMatch(const std::string& path, bool useCallback)
{
    PRINT_TEST_NAME();

    try
    {
        std::vector<unsigned char> buf;

        ArchiveMatch match;
        match.excludePattern("foo*");

        if (useCallback)
        {
            match.addCallback(
                [](ArchiveEntry entry, void*) -> void
            {
                const char* uname = entry.uname();
                const char* gname = entry.gname();

                std::cout << "\nCallback: skipping path " << entry.pathname() << '\n'
                          << " entry properties:\n"
                          << "   ctime: " << entry.ctime() << '\n'
                          << "   mtime: " << entry.mtime() << '\n'
                          << "   size: " << entry.size() << '\n'
                          << "   uname: " << (uname ? uname : "") << '\n'
                          << "   gname: " << (gname ? gname : "") << '\n';
            },
            nullptr);
        }

        {
            ArchiveWriter compressor(buf, Format::PAX, Filter::Gzip);
            compressor.addDiskPath(path, &match);
        }

        ArchiveReader reader(std::move(buf));

        for (auto it = reader.begin(); !it.isAtEnd(); ++it)
        {
            std::string entryName(it->pathname());

            if (entryName.find("foo") != std::string::npos)
            {
                std::cerr << "File should have been excluded from archive";
                return true;
            }
        }

        return false;
    }
    catch (const std::runtime_error& ex)
    {
        std::cerr << "Error testing match: " << ex.what() << '\n';
        return true;
    }
}

int main()
{
    {
        ArchiveWriter compressor("cmake_compressed.tar.gz", Format::PAX, Filter::Gzip);
        compressor.addDiskPath("CMakeLists.txt");
    }

    if (testCompressDirectory("test_data_dir.tar.gz"))
    {
        return 1;
    }

    if (testExtractDirectory("test_data_dir.tar.gz"))
    {
        return 1;
    }

    if (testDoesNotExist())
    {
        return 1;
    }

    if (testArchiveWriteFile("test_write_file.tar.gz"))
    {
        return 1;
    }

    if (testArchiveRead("test_write_file.tar.gz"))
    {
        return 1;
    }

    if (testArchiveWriteMemory("test_write_memory.tar.gz"))
    {
        return 1;
    }

    if (testArchiveRead("test_write_memory.tar.gz"))
    {
        return 1;
    }

    if (testArchiveDataCheck())
    {
        return 1;
    }

    if (testMatch("test_data_dir", false))
    {
        return 1;
    }

    if (testMatch("test_data_dir", true))
    {
        return 1;
    }

    if (testArchiveWriteCallback())
    {
        return 1;
    }

    return 0;
}
