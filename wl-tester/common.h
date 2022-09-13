#ifndef TESTER_COMMON_H
#define TESTER_COMMON_H

#define FB_BUFFER_NUM 3

struct output {
        struct wl_output *output;
        struct wl_list link;
};

struct example_window {
  struct wl_display *wl_display;
  char *drm_node;
  int  drm_dev_fd;
  int width;
  int height;
  struct fb_buffer ms_buffers[FB_BUFFER_NUM];
  char *raw_file;
  FILE *raw_fp;
  struct shm_buffer ss_buffer;
  int x;
  int y;
  struct wl_registry *wl_registry;
  struct wl_compositor *wl_compositor;
  struct wl_subcompositor *wl_subcompositor;
  struct xdg_wm_base *xdg_wm_base;
  struct wl_surface *wl_surface;
  struct wl_surface *wl_subsurface;
  struct wl_subsurface *subsurface;
  struct xdg_surface *xdg_surface;
  struct xdg_toplevel *xdg_toplevel;
  struct zwp_linux_dmabuf_v1 *dmabuf;
  struct wl_callback *callback;
  bool wait_for_configure;
  bool initialized;
  uint32_t frames_num;
  uint32_t benchmark_time;
  bool show_bar;
  bool sync;
  bool noread;
  bool ss_usedma;
  struct wl_list output_list;
  struct zxdg_output_manager_v1 *xdg_output_manager;
};

int init_window(struct example_window *window);
#endif
