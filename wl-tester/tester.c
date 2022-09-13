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
#include <wayland-client.h>

#include "buffers.h"
#include "common.h"

static void print_usage_and_exit(void) {
  printf("Usages:\n\n"
         "wl-tester -d /dev/dri/card0 -w 1280 -h 720 -f /path-to/test.yuv -x 100 -y 200 -s -b\n\n"
         "\t-d: DRM device node path\n"
         "\t-w: RAW YUV file's width\n"
         "\t-h: RAW YUV file's height\n"
         "\t-f: RAW YUV file's path\n"
         "\t-x: Window Surface's x position if compositor supports window position\n"
         "\t-y: Window Surface's y position if compositor supports window position\n"
         "\t-b: Show Sub Surface/bar\n"
         "\t-s: sync mode of sub surface\n"
         "\t-n: not read YUV file in loop, for perf debug purpose\n"
         "\t-u: Sub Surface use DMA Buf also, default is using Share Memory\n"
         );
  exit(0);
}

int main(int argc, char *argv[]) {
  struct example_window window = {0};
  int c, option_index;
  int ret = 0;
  char default_drm_node[32] = "/dev/dri/card0";

  window.drm_node = default_drm_node;

  static struct option long_options[] = {
      {"drm-node", required_argument, 0, 'd'},
      {"raw-file", required_argument, 0, 'f'},
      {"width", required_argument, 0, 'w'},
      {"height", required_argument, 0, 'h'},
      {"sx", required_argument, 0, 'x'},
      {"sy", required_argument, 0, 'y'},
      {"show-bar", no_argument, NULL, 'b'},
      {"sync", no_argument, NULL, 's'},
      {"noread", no_argument, NULL, 'n'},
      {"ss-usedma", no_argument, NULL, 'u'},
      {"help", no_argument, 0, 0},
      {0, 0, 0, 0}};

  while ((c = getopt_long(argc, argv, "bsnud:f:w:h:x:y:", long_options, &option_index)) !=
         -1) {
    switch (c) {
    case 'w':
      window.width = atoi(optarg);
      break;
    case 'h':
      window.height = atoi(optarg);
      break;
    case 'x':
      window.x = atoi(optarg);
      break;
    case 'y':
      window.y = atoi(optarg);
      break;
    case 'd':
      window.drm_node = optarg;
      break;
    case 'f':
      window.raw_file = optarg;
      break;
    case 's':
      window.sync = true;
      break;
    case 'n':
      window.noread = true;
      break;
    case 'u':
      window.ss_usedma = true;
      break;
    case 'b':
      window.show_bar = true;
      break;
    default:
      print_usage_and_exit();
    }
  }

  window.raw_fp = fopen(window.raw_file, "rb");
  if (!window.raw_fp) {
    fprintf(stderr, "Error opening yuv image for read\n");
    return -1;
  }

  ret = init_window(&window);
  if (ret){
    return 1;
  }

  return 0;
}
