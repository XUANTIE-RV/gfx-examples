/*
 * Source updates from:
 * - https://wayland-book.com/xdg-shell-basics/example-code.html
 * - weston's clients examples
 */

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#include "drm/drm.h"
#include "drm/drm_fourcc.h"
#include "xf86drm.h"

#include "linux-dmabuf-unstable-v1-client-protocol.h"
#include "xdg-shell-client-protocol.h"
#include <wayland-client.h>
#include "viewporter-client-protocol.h"
#include "xdg-output-unstable-v1-client-protocol.h"

#include "buffers.h"
#include "common.h"

int ms_buf_index = 0;
int raw_frame_index = 0;
static const int benchmark_interval = 5;

static void ms_flip(void *data, struct wl_callback *callback, uint32_t time);

static void wl_buffer_release(void *data, struct wl_buffer *wl_buffer) {
}

const struct wl_buffer_listener wl_buffer_listener = {
  .release = wl_buffer_release,
};

static void fb_buffer_release(void *data, struct wl_buffer *wl_buffer) {
  struct fb_buffer *buffer = data;

  buffer->busy = false;
}

const struct wl_buffer_listener fb_buffer_listener = {
  .release = fb_buffer_release,
};

static void xdg_surface_configure(void *data, struct xdg_surface *xdg_surface,
                                  uint32_t serial) {

  struct example_window *window = data;
  if ( !window->wait_for_configure)
    return;

  xdg_surface_ack_configure(xdg_surface, serial);

  if (window->initialized && window->wait_for_configure) {
    ms_flip(data, NULL, 0);
  }

  window->wait_for_configure = false;
}

const struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_configure,
};

static void xdg_wm_base_ping(void *data, struct xdg_wm_base *xdg_wm_base,
                             uint32_t serial) {
  xdg_wm_base_pong(xdg_wm_base, serial);
}

const struct xdg_wm_base_listener xdg_wm_base_listener = {
    .ping = xdg_wm_base_ping,
};

static void dmabuf_modifier(void *data,
                            struct zwp_linux_dmabuf_v1 *zwp_linux_dmabuf,
                            uint32_t format, uint32_t modifier_hi,
                            uint32_t modifier_lo) {}

static void dmabuf_format(void *data,
                          struct zwp_linux_dmabuf_v1 *zwp_linux_dmabuf,
                          uint32_t format) {}

const struct zwp_linux_dmabuf_v1_listener dmabuf_listener = {
    .format = dmabuf_format,
    .modifier = dmabuf_modifier,
};

/* Start of XDG Output */
/* Can got output infos to future usages */

static void
display_handle_geometry(void *data,
                        struct wl_output *wl_output,
                        int x, int y,
                        int physical_width,
                        int physical_height,
                        int subpixel,
                        const char *make,
                        const char *model,
                        int transform)
{
 //printf("YG: x=%d, y=%d, p-width=%d, p-height=%d, subpixel=%d, transform=%d, model=\"%s\"\n", x, y,
   //     physical_width, physical_height, subpixel, transform, model);
}

static void
display_handle_done(void *data,
                     struct wl_output *wl_output)
{
}

static void
display_handle_scale(void *data,
                     struct wl_output *wl_output,
                     int32_t scale)
{
}

static void
display_handle_mode(void *data,
                    struct wl_output *wl_output,
                    uint32_t flags,
                    int width,
                    int height,
                    int refresh)
{
 //printf("YG:width=%d, height=%d, flags=%d, refresh=%d\n", width, height, flags, refresh);
}

static const struct wl_output_listener output_listener = {
        display_handle_geometry,
        display_handle_mode,
        display_handle_done,
        display_handle_scale
};

static void
window_add_output(struct example_window *window, uint32_t id)
{
  struct output *output;

  output = malloc(sizeof *output);
  memset(output, 0, sizeof *output);
  output->output = wl_registry_bind(window->wl_registry, id, &wl_output_interface, 2);
  //wl_list_insert(window->output_list.prev, &output->link);

  wl_output_add_listener(output->output, &output_listener, output);
}

static void
handle_xdg_output_v1_logical_position(void *data, struct zxdg_output_v1 *output,
                                      int32_t x, int32_t y)
{
}

static void
handle_xdg_output_v1_logical_size(void *data, struct zxdg_output_v1 *output,
                                      int32_t width, int32_t height)
{
}

static void
handle_xdg_output_v1_done(void *data, struct zxdg_output_v1 *output)
{
}

static void
handle_xdg_output_v1_name(void *data, struct zxdg_output_v1 *output,
                          const char *name)
{
}

static void
handle_xdg_output_v1_description(void *data, struct zxdg_output_v1 *output,
                          const char *description)
{
}

static const struct zxdg_output_v1_listener xdg_output_v1_listener = {
        .logical_position = handle_xdg_output_v1_logical_position,
        .logical_size = handle_xdg_output_v1_logical_size,
        .done = handle_xdg_output_v1_done,
        .name = handle_xdg_output_v1_name,
        .description = handle_xdg_output_v1_description,
};

static void
add_xdg_output_manager_v1_info(struct example_window *window, uint32_t id, uint32_t version)
{
        window->xdg_output_manager = wl_registry_bind(window->wl_registry, id,
                &zxdg_output_manager_v1_interface, version > 2 ? 2 : version);
}
/* End of XDG Output */

static void registry_global(void *data, struct wl_registry *wl_registry,
                            uint32_t name, const char *interface,
                            uint32_t version) {
  struct example_window *window = data;
  if (strcmp(interface, wl_shm_interface.name) == 0) {
    window->ss_buffer.wl_shm = wl_registry_bind(wl_registry, name, &wl_shm_interface, 1);
  } else if (strcmp(interface, wl_compositor_interface.name) == 0) {
    window->wl_compositor =
        wl_registry_bind(wl_registry, name, &wl_compositor_interface, 4);
  } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
    window->xdg_wm_base =
        wl_registry_bind(wl_registry, name, &xdg_wm_base_interface, 1);
    xdg_wm_base_add_listener(window->xdg_wm_base, &xdg_wm_base_listener, window);
  } else if (strcmp(interface, wl_subcompositor_interface.name) == 0) {
    window->wl_subcompositor =
        wl_registry_bind(wl_registry, name, &wl_subcompositor_interface, 1);
  } else if (strcmp(interface, zwp_linux_dmabuf_v1_interface.name) == 0) {
    window->dmabuf =
        wl_registry_bind(wl_registry, name, &zwp_linux_dmabuf_v1_interface, 3);
    zwp_linux_dmabuf_v1_add_listener(window->dmabuf, &dmabuf_listener, window);
  } else if (strcmp(interface, wl_output_interface.name) == 0) {
    window_add_output(window, name);
  } else if (strcmp(interface, zxdg_output_manager_v1_interface.name) == 0)
    add_xdg_output_manager_v1_info(window, name, version);
}

static void registry_global_remove(void *data, struct wl_registry *wl_registry,
                                   uint32_t name) {}

const struct wl_registry_listener wl_registry_listener = {
    .global = registry_global,
    .global_remove = registry_global_remove,
};

static const struct wl_callback_listener frame_listener = {
  ms_flip
};

static void ms_flip(void *data, struct wl_callback *callback, uint32_t time) {
  struct example_window *window = data;
   
  while(1){
    if (ms_buf_index >= FB_BUFFER_NUM)
      ms_buf_index = 0;

    if (!window->ms_buffers[ms_buf_index].busy){
      break;
    }
    ms_buf_index++;
  }

  if (!window->noread){ 
      int ret = fill_with_raw(window->raw_fp, window->width, window->height, (uint8_t *)(window->ms_buffers[ms_buf_index].buf_vma));
      if (ret != 0) {
          fprintf(stderr, "failed to fill the buf\n");
      }
  }

  wl_surface_attach(window->wl_surface, window->ms_buffers[ms_buf_index].buffer, 0, 0);
  wl_surface_damage(window->wl_surface, 0, 0, window->width, window->height);

  if (callback)
    wl_callback_destroy(callback);

  window->callback = wl_surface_frame(window->wl_surface);
  wl_callback_add_listener(window->callback, &frame_listener, window);
  wl_surface_commit(window->wl_surface);

  window->ms_buffers[ms_buf_index].busy = true;

  struct timeval tv;
  gettimeofday(&tv, NULL);
  time = tv.tv_sec * 1000 + tv.tv_usec / 1000;
  if (window->frames_num == 0)
    window->benchmark_time = time;

  if (time - window->benchmark_time > (benchmark_interval * 1000)) {
    printf("%d frames in %d seconds: %f fps\n",
           window->frames_num,
           benchmark_interval,
           (float) window->frames_num / benchmark_interval);
           window->benchmark_time = time;
           window->frames_num = 0;
    }
  window->frames_num++;
  return;
}

static void create_succeeded(void *data,
                             struct zwp_linux_buffer_params_v1 *params,
                             struct wl_buffer *new_buffer) {
  zwp_linux_buffer_params_v1_destroy(params);
}

static void create_failed(void *data,
                          struct zwp_linux_buffer_params_v1 *params) {
  zwp_linux_buffer_params_v1_destroy(params);
  fprintf(stderr, "Error: zwp_linux_buffer_params.create failed.\n");
}

static const struct zwp_linux_buffer_params_v1_listener params_listener = {
    create_succeeded, create_failed};


int init_buffers(struct example_window *window) {
  int ret = 0;

  window->drm_dev_fd = open(window->drm_node, O_RDWR);
  if (window->drm_dev_fd < 0) {
    fprintf(stderr, "failed to open device %s: %s\n", window->drm_node, strerror(errno));
    return -1;
  }

  //Init main surface buffers
  for(int i_buf = 0; i_buf < FB_BUFFER_NUM; i_buf++){
    uint32_t handles[4] = {0}, pitches[4] = {0}, offsets[4] = {0};
 
    window->ms_buffers[i_buf].bo = bo_create(window->drm_dev_fd, DRM_FORMAT_NV12, window->width, window->height, handles, pitches, offsets);
    window->ms_buffers[i_buf].busy = false;
    if (!window->ms_buffers[i_buf].bo) {
      fprintf(stderr, "failed to create bo\n");
      close(window->drm_dev_fd);
      return -1;
    }

    //TODO: need unmap in handling of exception or exit
    ret = bo_map(window->ms_buffers[i_buf].bo, &(window->ms_buffers[i_buf].buf_vma));
    if (ret) {
      fprintf(stderr, "failed to map buffer: %s\n", strerror(-errno));
      bo_destroy(window->ms_buffers[i_buf].bo);
      return -1;
    }

    struct zwp_linux_buffer_params_v1 *params;
    params = zwp_linux_dmabuf_v1_create_params(window->dmabuf);

    int fd_plane = 0;
    for (int i = 0; i < FORMAT_MAX_PLANES; ++i) {
      if (!handles[i])
        continue;

      int ret = drmPrimeHandleToFD(window->drm_dev_fd, handles[i], 0, &fd_plane);
      if (ret < 0 || fd_plane < 0) {
        fprintf(stderr, "error: failed to get dmabuf_fd\n");
        bo_destroy(window->ms_buffers[i_buf].bo);
        close(window->drm_dev_fd);
        return -1;
      }

      zwp_linux_buffer_params_v1_add(params, fd_plane, i, offsets[i], pitches[i],
                                   DRM_FORMAT_MOD_INVALID >> 32,
                                   DRM_FORMAT_MOD_INVALID & 0xffffffff);
    }

    uint32_t flags = 0;
    zwp_linux_buffer_params_v1_add_listener(params, &params_listener, window);
    window->ms_buffers[i_buf].buffer = zwp_linux_buffer_params_v1_create_immed(params, window->width, window->height, WL_SHM_FORMAT_NV12, flags);
    wl_buffer_add_listener(window->ms_buffers[i_buf].buffer, &fb_buffer_listener, &(window->ms_buffers[i_buf]));
  } 

  //Init sub surface buffers
  if (window->show_bar) {
    int ss_stride = window->width * 4;
    int ss_height = window->width/5;
    int ss_size = ss_stride * (ss_height);

    if (window->ss_usedma){
      uint32_t ss_handles[4] = {0}, ss_pitches[4] = {0}, ss_offsets[4] = {0};
      //Using BGR as DMA Buf in Light can't support RGB
      struct bo *ss_bo = bo_create(window->drm_dev_fd, DRM_FORMAT_ARGB8888, window->width, ss_height, ss_handles, ss_pitches, ss_offsets);
      if (!ss_bo) {
          fprintf(stderr, "failed to create ss_bo\n");
          close(window->drm_dev_fd);
          return -1;
      }

      ret = bo_map(ss_bo, &(window->ss_buffer.buf_vma));
      if (ret) {
        fprintf(stderr, "failed to map buffer: %s\n", strerror(-errno));
        bo_destroy(ss_bo);
        return -1;
      }

      struct zwp_linux_buffer_params_v1 *ss_params;
      ss_params = zwp_linux_dmabuf_v1_create_params(window->dmabuf);

      int ss_fd_plane = 0;
      for (int i = 0; i < FORMAT_MAX_PLANES; ++i) {
        if (!ss_handles[i])
          continue;

        ret = drmPrimeHandleToFD(window->drm_dev_fd, ss_handles[i], 0, &ss_fd_plane);
        if (ret < 0 || ss_fd_plane < 0) {
          fprintf(stderr, "error: failed to get ss dmabuf fd\n");
          bo_destroy(ss_bo);
          close(window->drm_dev_fd);
          return -1;
        }

        zwp_linux_buffer_params_v1_add(ss_params, ss_fd_plane, i, ss_offsets[i], ss_pitches[i],
                                   DRM_FORMAT_MOD_INVALID >> 32,
                                   DRM_FORMAT_MOD_INVALID & 0xffffffff);
      }

      uint32_t ss_flags = 0;
      zwp_linux_buffer_params_v1_add_listener(ss_params, &params_listener, window);
      window->ss_buffer.buffer = zwp_linux_buffer_params_v1_create_immed(ss_params, window->width, 
                                 ss_height, 0x34325241, ss_flags);
    } else {
      window->ss_buffer.fd = allocate_shm_file(ss_size);
      if (window->ss_buffer.fd == -1) {
        return -1;
      }
  
      struct wl_shm_pool *pool = wl_shm_create_pool(window->ss_buffer.wl_shm, window->ss_buffer.fd, ss_size);
      window->ss_buffer.buffer = wl_shm_pool_create_buffer(pool, 0, window->width, ss_height, ss_stride, WL_SHM_FORMAT_XRGB8888);
      wl_shm_pool_destroy(pool);

      window->ss_buffer.buf_vma = mmap(NULL, ss_size, PROT_READ | PROT_WRITE, MAP_SHARED, window->ss_buffer.fd, 0);
      if (window->ss_buffer.buf_vma == MAP_FAILED) {
        close(window->ss_buffer.fd);
        return -1;
      }
    }

    //Start fill sub surface buffer
    for (int y = 0; y < ss_height; ++y) {
      for (int x = 0; x < window->width; ++x) {
        if ((x + y / 8 * 8) % 16 < 8)
          ((uint32_t *)window->ss_buffer.buf_vma)[y * window->width + x] = 0xFF666666;
        else
          ((uint32_t *)window->ss_buffer.buf_vma)[y * window->width + x] = 0xFFEEEEEE;
      }
    }

    munmap(window->ss_buffer.buf_vma, ss_size);

    wl_surface_attach(window->wl_subsurface, window->ss_buffer.buffer, 0, 0);
    wl_surface_damage(window->wl_subsurface, 0, 0, window->width, ss_height);
    wl_surface_commit(window->wl_subsurface); 
    wl_buffer_add_listener(window->ss_buffer.buffer, &wl_buffer_listener, NULL);
  }

  return 0; 
}


int init_window(struct example_window *window) {

  window->frames_num = 0;
  window->wl_display = wl_display_connect(NULL);
  window->wl_registry = wl_display_get_registry(window->wl_display);
  wl_registry_add_listener(window->wl_registry, &wl_registry_listener, window);
  wl_display_roundtrip(window->wl_display);

  window->wl_surface = wl_compositor_create_surface(window->wl_compositor);
  window->wl_subsurface =
      wl_compositor_create_surface(window->wl_compositor);
  window->subsurface = wl_subcompositor_get_subsurface(
      window->wl_subcompositor, window->wl_subsurface, window->wl_surface);

  //we are using xdg_surface_set_window_geometry() to set window position now.
  //wl_subsurface_set_position(window->subsurface, window->x, window->y);

  window->xdg_surface =
      xdg_wm_base_get_xdg_surface(window->xdg_wm_base, window->wl_surface);
  xdg_surface_add_listener(window->xdg_surface, &xdg_surface_listener, window);
  window->xdg_toplevel = xdg_surface_get_toplevel(window->xdg_surface);
  xdg_toplevel_set_title(window->xdg_toplevel,
                         "Graphics Wayland Buf Display Example");
  xdg_toplevel_set_app_id(window->xdg_toplevel, "wl-tester");
  xdg_surface_set_window_geometry(window->xdg_surface, window->x, window->y, window->width, window->height);

  //Not sure if it is workable?
  //xdg_toplevel_set_fullscreen();

  window->wait_for_configure = true;
  if (window->sync)
    wl_subsurface_set_sync(window->subsurface);
  else
    wl_subsurface_set_desync(window->subsurface);

  wl_surface_commit(window->wl_surface);

  int ret = init_buffers(window);
  if (ret != 0){
    fprintf(stderr, "Error init buffers\n");
    return -1;
  }
  wl_display_roundtrip(window->wl_display);

  if (!window->wait_for_configure){
    ms_flip(window, NULL, 0);
  }
  window->initialized = true;

  while (wl_display_dispatch(window->wl_display)) {
  }
 
  return 0; 
}
