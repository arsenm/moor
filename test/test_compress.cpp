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

static bool testArchiveWrite(const std::string& path)
{
  try
  {
    std::vector<unsigned char> lout;
    //ArchiveWriter compressor("test1.tar.gz", Format::PAX, Filter::Gzip);
    ArchiveWriter compressor(lout, Format::PAX, Filter::Gzip);
    compressor.addFile("test_data_dir/bar.txt");
    compressor.addDirectory("test_data_dir/foo_dir");
    char a[] = { 64, 65, 66, 67, 68 };
    std::vector<char> l(10, 'A');
    std::vector<char> v(10, 'B');

    compressor.addFile("a_array.txt", a, a + 10);
    compressor.addFile("b_list.txt", l.begin(), l.end());
    compressor.addFile("vector_a.txt", v.begin(), v.end());

    compressor.close();
    std::ofstream of(path, std::ios::binary);
    for (auto a = lout.begin(), e = lout.end(); a != e; ++a)
      of << *a;
    of.close();
    return false;
  }
  catch (const std::runtime_error& ex)
  {
    std::cerr << "Error writing archive: " << ex.what() << '\n';
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
    while (data.first.length() > 0)
    {
      std::cout << data.first << " : " << data.second.size() << '\n';
      data = reader.extractNext();
    }

    data = reader1.extractNext();
    while (data.first.length() > 0)
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

  if (testArchiveWrite("test2.tar.gz"))
  {
    return 1;
  }

  if (testArchiveRead("test2.tar.gz"))
  {
    return 1;
  }

  return 0;
}
