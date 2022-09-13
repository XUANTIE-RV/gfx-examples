project(
  'gfx-examples',
  'c',
  version : '0.0.1',
  license : 'MIT',
  meson_version : '>= 0.47',
  default_options : ['c_std=gnu99', 'warning_level=2']
)

if get_option('c_std') != 'gnu99'
  error('c_std must be gnu99')
endif

subdir('gtk01')
subdir('wl-tester')
