custom_target(
    'plugin-file-trash',
    input: 'TrashApplet.plugin.in',
    output: 'TrashApplet.plugin',
    command : [intltool, '--desktop-style', podir, '@INPUT@', '@OUTPUT@'],
    install: true,
    install_dir: APPLET_INSTALL_DIR
)

applet_trash_deps = [
    dep_budgie,
    dep_gtk3,
    dep_glib
]

applet_trash_sources =[
    'TrashApplet.vala',
    'TrashPopover.vala',
    'TrashIconButton.vala',
    'TrashItem.vala'
]

bta_vala_args = [
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
