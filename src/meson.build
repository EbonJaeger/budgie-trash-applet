add_project_arguments('-Db_sanitize=address', language: 'c')

trash_applet_deps = [
    dependency('budgie-1.0', version: '>= 2'),
    dependency('gtk+-3.0', version: '>= 3.22.0'),
    dependency('glib-2.0', version: '>= 2.64.0'),
    dependency('gio-2.0', version: '>= 2.64.0'),
    dependency('libnotify', version: '>= 0.7'),
]

gnome = import('gnome')
intltool = find_program('intltool-merge')

custom_target(
    'plugin-file-trash',
    input: 'TrashApplet.plugin.in',
    output: 'TrashApplet.plugin',
    command : [intltool, '--desktop-style', podir, '@INPUT@', '@OUTPUT@'],
    install: true,
    install_dir: APPLET_INSTALL_DIR
)

trash_applet_sources = [
    'trash_icon_button.c',
    'trash_info.c',
    'trash_item.c',
    'trash_revealer.c',
    'trash_store.c',
    'utils.c',
    'applet.c',
    'plugin.c',
    'notify.c',
]

trash_applet_sources += gnome.compile_resources('trash-applet-resources',
    'trash-applet.gresource.xml',
    dependencies: [ style ],
    c_name: 'budgie_trash_applet',
)

shared_library(
    'trashapplet',
    trash_applet_sources,
    dependencies: trash_applet_deps,
    install: true,
    install_dir: APPLET_INSTALL_DIR
)
