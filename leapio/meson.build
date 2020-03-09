leapio_lib = static_library(
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
