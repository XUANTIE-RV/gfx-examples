tester_sources = files(
  'tester.c',
  'common.c',
  'buffers.c',
)

cc = meson.get_compiler('c')
wl_req = '>= 1.15'
wl_scanner = find_program('wayland-scanner')
drm_dep = dependency('libdrm')
rt_dep = cc.find_library('rt', required: false)
wl_client_dep = dependency('wayland-client', version: wl_req)
wl_protocol_dep = dependency('wayland-protocols', version: wl_req)

protocols_datadir = wl_protocol_dep.get_pkgconfig_variable('pkgdatadir')
protocol_defs = [
        ['/stable/viewporter/viewporter.xml', 'viewporter-protocol.c', 'viewporter-client-protocol.h'],
        ['/unstable/linux-dmabuf/linux-dmabuf-unstable-v1.xml',
         'linux-dmabuf-unstable-v1-protocol.c', 'linux-dmabuf-unstable-v1-client-protocol.h'],
        ['/unstable/xdg-output/xdg-output-unstable-v1.xml',
         'xdg-output-unstable-v1-protocol.c', 'xdg-output-unstable-v1-client-protocol.h'],
        ['/stable/xdg-shell/xdg-shell.xml', 'xdg-shell-protocol.c', 'xdg-shell-client-protocol.h'],
]
protocols_files = []

foreach protodef: protocol_defs
	xmlfile = protocols_datadir + protodef.get(0)

	protocols_files += [custom_target(protodef.get(1),
	output : protodef.get(1),
	input : xmlfile,
	command : [wl_scanner, 'code', '@INPUT@', '@OUTPUT@'])]

	protocols_files += [custom_target(protodef.get(2),
	output : protodef.get(2),
	input : xmlfile,
	command : [wl_scanner, 'client-header', '@INPUT@', '@OUTPUT@'])]
endforeach

dep_common = [drm_dep, rt_dep, wl_client_dep, wl_protocol_dep]
wl_tester = executable('wl-tester', tester_sources + protocols_files, dependencies : dep_common, install : true)
