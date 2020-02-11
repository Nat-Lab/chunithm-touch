leap_configurator = executable(
    'leapconfig',
    name_prefix : '',
    include_directories : [inc, leap_inc],
    implicit_include_directories : false,
    c_pch : '../precompiled.h',
    link_with: [leapio_lib],
    dependencies: leap_lib,
    sources : [
        'leap-configurator.c'
    ]
)
