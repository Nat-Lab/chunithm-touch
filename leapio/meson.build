leap_inc = include_directories('../3rdparty/LeapSDK/include')
arch = 'x86'
if host_machine.cpu_family() == 'x86_64'
    arch = 'x64'
endif
leap_lib_path = meson.source_root() + '/3rdparty/LeapSDK/lib/' + arch
leap_lib = meson.get_compiler('c').find_library('LeapC', dirs: leap_lib_path)

leapio_dll = static_library(
    'leapio',
    name_prefix : '',
    include_directories : [inc, leap_inc],
    implicit_include_directories : false,
    c_pch : '../precompiled.h',
    dependencies: leap_lib,
    sources : [
        'leapio.c',
        'leapio.h'
    ],
)
