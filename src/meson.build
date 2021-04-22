gnome = import('gnome')

custom_target(
    'plugin-file-trash',
    input: 'TrashApplet.plugin.in',
    output: 'TrashApplet.plugin',
    command : [intltool, '--desktop-style', podir, '@INPUT@', '@OUTPUT@'],
    install: true,
    install_dir: APPLET_INSTALL_DIR
)

data_dir = join_paths(meson.source_root(), 'data')

gresource_file = join_paths(data_dir, 'com.github.EbonJaeger.budgie-trash-applet.gresource.xml')
res = gnome.compile_resources('bta-resources', gresource_file, source_dir: data_dir)

applet_trash_deps = [
    dep_budgie,
    dep_gtk3,
    dep_glib,
    dep_gio,
    dep_notify
]

applet_trash_sources = [
    'trash_icon_button.c',
    'trash_item.c',
    'trash_revealer.c',
    'trash_store.c',
    'utils.c',
    'applet.c',
    'plugin.c',
    'notify.c',
    res
]

shared_library(
    'trashapplet',
    applet_trash_sources,
    dependencies: applet_trash_deps,
    include_directories: includeDir,
    install: true,
    install_dir: APPLET_INSTALL_DIR
)
