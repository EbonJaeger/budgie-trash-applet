project('budgie-trash-applet',
        ['vala', 'c'],
        version: '0.0.1',
)

am_cflags = [
    '-fstack-protector',
    '-pedantic',
    '-Wstrict-prototypes',
    '-Wundef',
    '-Werror-implicit-function-declaration',
    '-Wformat',
    '-Wformat-security',
    '-Werror=format-security',
    '-Wconversion',
    '-Wunused-variable',
    '-Wunreachable-code',
    '-Wall',
    '-W'
]

add_global_arguments(am_cflags, language: 'c')

# Dependencies
dep_budgie = dependency('budgie-1.0', version: '>= 2')
dep_gtk3 = dependency('gtk+-3.0', version: '>= 3.22.0')
dep_glib = dependency('glib-2.0', version: '>= 2.46.0')

intltool = find_program('intltool-merge')

# Global stuff
prefix = get_option('prefix')
libdir = join_paths(prefix, get_option('libdir'))
datadir = join_paths(prefix, get_option('datadir'))
localedir = join_paths(prefix, get_option('localedir'))
podir = join_paths(meson.source_root(), 'po')

APPLET_INSTALL_DIR = join_paths(libdir, 'budgie-desktop', 'plugins', meson.project_name())

subdir('src')
