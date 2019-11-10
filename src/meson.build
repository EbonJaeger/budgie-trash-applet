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
    dep_glib
]

applet_trash_sources =[
    'TrashApplet.vala',
    'Widgets/MainPopover.vala',
    'Widgets/IconButton.vala',
    'Widgets/SettingsView.vala',
    'Widgets/TrashItem.vala',
    'Widgets/TrashStoreWidget.vala',
    'TrashHandler.vala',
    'TrashStore.vala',
    res
]

bta_vala_args = [
    '--gresources=' + gresource_file,
    '--pkg', 'gtk+-3.0',
    '--pkg', 'glib-2.0'
]

shared_library(
    'trashapplet',
    applet_trash_sources,
    dependencies: applet_trash_deps,
    vala_args: bta_vala_args,
    install: true,
    install_dir: APPLET_INSTALL_DIR
)
