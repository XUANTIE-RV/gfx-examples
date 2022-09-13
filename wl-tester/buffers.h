/*
 * DRM based mode setting test program
 * Copyright 2008 Tungsten Graphics
 *   Jakob Bornecrantz <jakob@tungstengraphics.com>
 * Copyright 2008 Intel Corporation
 *   Jesse Barnes <jesse.barnes@intel.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef __BUFFERS_H__
#define __BUFFERS_H__

#define FORMAT_MAX_PLANES 4

struct fb_buffer {
        struct wl_buffer *buffer;
        struct bo *bo;
        void *buf_vma;
        bool busy;
};

struct shm_buffer {
        struct wl_buffer *buffer;
        struct wl_shm *wl_shm;
        void *buf_vma;
        int fd;
        bool busy;
};

struct bo {
  int fd;
  void *ptr;
  size_t size;
  size_t offset;
  size_t pitch;
  unsigned handle;
};

int allocate_shm_file(size_t size);
int fill_with_raw(FILE *fp, uint32_t width, uint32_t height, uint8_t *data);

int bo_map(struct bo *bo, void **out);
void bo_unmap(struct bo *bo);
struct bo *bo_create(int fd, unsigned int format, unsigned int width,
                     unsigned int height, unsigned int handles[4],
                     unsigned int pitches[4], unsigned int offsets[4]);
void bo_destroy(struct bo *bo);

#endif
