chuniio_dll = shared_library(
    'ctw',
    name_prefix : '',
    include_directories : inc,
    implicit_include_directories : false,
    c_pch : '../precompiled.h',
    dependencies : [
        capnhook.get_variable('hook_dep')
    ],
    sources : [
        'dllmain.c',
        'create-touch-window.c',
        'create-touch-window.h'
    ],
)
