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

#include "memory_writer_callback.hpp"

#include <archive.h>

#include <iterator>

using namespace moor;

struct write_memory_data
{
    //size_t used;
    //size_t size;
    //size_t* client_size;
    std::vector<unsigned char>* buff;
};

static int moor_memory_write_open(archive* a, void* /* client_data */)
{
    /*write_memory_data *mine;
      mine = client_data; mine->used = 0;
      if (mine->client_size != 0)
       *mine->client_size = mine->used;*/
    /* Disable padding if it hasn't been set explicitly. */
    if (archive_write_get_bytes_in_last_block(a) == -1)
    {
        archive_write_set_bytes_in_last_block(a, 1);
    }

    return ARCHIVE_OK;
}
static ssize_t moor_memory_write(archive*,
                                 void* client_data,
                                 const void* buff,
                                 size_t length)
{
    write_memory_data* mine = static_cast<write_memory_data*>(client_data);

    /*if (mine->used + length > mine->size)
    {
      archive_set_error(a, ENOMEM, "Buffer exhausted");
      return (ARCHIVE_FATAL);
    }
    memcpy(mine->buff + mine->used, buff, length);
    mine->used += length;
    if (mine->client_size != 0)
      *mine->client_size = mine->used;*/
    const unsigned char* p = reinterpret_cast<const unsigned char*>(buff);
    std::copy(p,
              p + length,
              std::back_inserter(*mine->buff));
    return static_cast<ssize_t>(length);
}

static int moor_memory_write_close(archive* a, void* client_data)
{
    write_memory_data* mine = static_cast<write_memory_data*>(client_data);
    (void)a; /* UNUSED */
    delete mine;
    return ARCHIVE_OK;
}

int moor::write_open_memory(archive* a, std::vector<unsigned char>& buff)
{
    write_memory_data* mine = new write_memory_data();

    mine->buff = &buff;
    //mine->size = buffSize;
    //mine->client_size = used;
    return archive_write_open(a,
                              mine,
                              moor_memory_write_open,
                              moor_memory_write,
                              moor_memory_write_close);
}

