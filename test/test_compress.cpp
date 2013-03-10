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

#include <moor/archive_reader.hpp>
#include <moor/archive_writer.hpp>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <system_error>
#include <vector>


using namespace moor;

static bool testDoesNotExist()
{
  try
  {
    ArchiveReader badReader("xxxx_does_not_exist");
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
  const std::string entryName("lorem_ipsum.txt");
  std::vector<unsigned char> buf;

  {
      ArchiveWriter compressor(buf, Format::PAX, Filter::Gzip);
      compressor.addFile(entryName, testDataString);
  }

  ArchiveReader reader(std::move(buf));
  auto item = reader.extractNext();
  if (item.first != entryName)
  {
      std::cerr << "Entry name does not match: "
                << item.first
                << " vs. expected "
                << entryName
                << '\n';
      return true;
  }

  const std::vector<unsigned char>& out = item.second;

  std::string extractedCopy(out.begin(), out.end());
  if (extractedCopy != testDataString)
  {
      std::cerr << "Extracted string does not match:\n"
                << "  Original string size: " << testDataString.size() << '\n'
                << "  Extracted string size: " << extractedCopy.size() << '\n';

      return true;
  }

  return false;
}

// Write result directly to path
static bool testArchiveWriteFile(const std::string& path)
{
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

static bool testArchiveRead(const std::string& path)
{
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
    auto data = reader.extractNext();
    while (!data.first.empty())
    {
      std::cout << data.first << " : " << data.second.size() << '\n';
      data = reader.extractNext();
    }

    data = reader1.extractNext();
    while (!data.first.empty())
    {
      std::cout << data.first << " : " << data.second.size() << '\n';
      data = reader1.extractNext();
    }

    return false;
  }
  catch (const std::runtime_error& ex)
  {
    std::cerr << "Error reading archive: " << ex.what() << '\n';
    return true;
  }
}

int main()
{
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

  return 0;
}
