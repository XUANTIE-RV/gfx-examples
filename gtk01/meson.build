gtk01_sources = files(
  'gtk01.c',
)

cc = meson.get_compiler('c')
dep_gtk3 = dependency('gtk+-3.0', version : '>=3.20')
dep_common = [dep_gtk3]

executable('gtk01', gtk01_sources, dependencies : dep_common, install : true)
