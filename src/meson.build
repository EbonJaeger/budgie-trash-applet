add_project_arguments('-Db_sanitize=address', language: 'c')

trash_applet_deps = [
    dependency('budgie-1.0', version: '>= 2'),
    dependency('gtk+-3.0', version: '>= 3.22.0'),
    dependency('glib-2.0', version: '>= 2.64.0'),
    dependency('gio-2.0', version: '>= 2.64.0'),
    dependency('libnotify', version: '>= 0.7'),
]

trash_applet_sources = [
    'trash_enum_types.c',
    'trash_icon_button.c',
    'trash_info.c',
    'trash_item.c',
    'trash_revealer.c',
    'trash_settings.c',
    'trash_store.c',
    'utils.c',
    'applet.c',
    'plugin.c',
    'notify.c',
]

# Compile our gresource file
trash_applet_sources += gnome.compile_resources('budgie-trash-applet-resources',
    'budgie-trash-applet.gresource.xml',
    dependencies: [ style ],
    c_name: 'budgie_trash_applet',
)

# Build the applet binary
shared_library(
    'trashapplet',
    trash_applet_sources,
    dependencies: trash_applet_deps,
    install: true,
    install_dir: APPLET_INSTALL_DIR,
)
