project('budgie-trash-applet', 'c',
        version: '2.1.2',
        meson_version: '>= 0.57.0',
        default_options: [
            'warning_level=2',
            'werror=true',
            'c_std=gnu11',
        ],
)

gnome = import('gnome')

#
# Global stuff
#

prefix = get_option('prefix')
libdir = join_paths(prefix, get_option('libdir'))
datadir = join_paths(prefix, get_option('datadir'))
localedir = join_paths(prefix, get_option('localedir'))
podir = join_paths(meson.current_source_dir(), 'po')

APPLET_INSTALL_DIR = join_paths(libdir, 'budgie-desktop', 'plugins', 'com.github.EbonJaeger.budgie-trash-applet')

subdir('data')
subdir('src')

gnome.post_install(
    glib_compile_schemas: true
)
