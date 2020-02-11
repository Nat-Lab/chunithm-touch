project('chunithm-touch', 'c', version: '0.2.0', default_options: ['warning_level=2'])

add_project_arguments(
    '-DCOBJMACROS',
    '-DDIRECTINPUT_VERSION=0x0800',
    '-DWIN32_LEAN_AND_MEAN',
    '-D_WIN32_WINNT=_WIN32_WINNT_WIN8',
    '-DMINGW_HAS_SECURE_API=1',
    language: 'c',
)

if meson.get_compiler('c').get_id() != 'msvc'
    add_project_arguments(
        '-ffunction-sections',
        '-fdata-sections',
        language: 'c',
    )

    add_project_link_arguments(
        '-Wl,--exclude-all-symbols',
        '-Wl,--gc-sections',
        '-static-libgcc',
        language: 'c',
    )
endif

cc = meson.get_compiler('c')

inc = include_directories('.')

leap_inc = include_directories('3rdparty/LeapSDK/include')
arch = 'x86'
if host_machine.cpu_family() == 'x86_64'
    arch = 'x64'
endif
leap_lib_path = meson.source_root() + '/3rdparty/LeapSDK/lib/' + arch
leap_lib = meson.get_compiler('c').find_library('LeapC', dirs: leap_lib_path)

capnhook = subproject('capnhook')

subdir('chuniio')
subdir('create-touch-window')
subdir('leapio')
subdir('leap-configurator')
