project(
  'sdeventplus',
  'cpp',
  version: '0.1',
  meson_version: '>=0.57.0',
  default_options: [
    'warning_level=3',
    'cpp_std=c++20',
    'tests=' + (meson.is_subproject() ? 'disabled' : 'auto'),
    'examples=' + (meson.is_subproject() ? 'false' : 'true'),
  ])  

cpp = meson.get_compiler('cpp')
subdir('dump')
