project('wilk', 'c',
  version : '0.1',
  default_options : ['warning_level=3'])

glfw = dependency('glfw3')
exe = executable('wilk', ['src/glad/gl.c', 'src/wilk/main.c'],
  include_directories : 'include',
  dependencies : glfw,
  install : true)

test('basic', exe)
