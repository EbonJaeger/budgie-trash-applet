install_data('TrashApplet.plugin', 
    install_dir: APPLET_INSTALL_DIR
)

install_data('com.github.ebonjaeger.budgie-trash-applet.gschema.xml',
    install_dir: join_paths(get_option('datadir'), 'glib-2.0', 'schemas')
)

compile_schemas = find_program('glib-compile-schemas', required: false)
if compile_schemas.found()
  test('Validate schema file', compile_schemas,
    args: ['--strict', '--dry-run', meson.current_source_dir()]
  )
endif
