chuniio_dll = shared_library(
    'chuniio',
    name_prefix : '',
    include_directories : [inc, leap_inc],
    implicit_include_directories : false,
    c_pch : '../precompiled.h',
    link_with: [leapio_lib],
    dependencies: leap_lib,
    vs_module_defs : 'chuniio.def',
    sources : [
        'chuniio.c',
        'chuniio.h'
    ],
)
