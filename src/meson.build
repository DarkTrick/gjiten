sources = files(
  'utils.c',
  'conf.c',
  'data_store.c',
  'dicfile.c',
  'dicutil.c',
  'error.c',
  'gconf_reader.c',
  'gjiten.c',
  'kanjidic.c',
  'main.c',
  'migration2-6_3-0.c',
  'pref.c',
  'radicals.c',
  'radicals_ui.c',
  'utils.c',
  'worddic.c',

)

resources = gnome.compile_resources(
  'gjiten-resources',
  '../resources.xml',
  c_name : 'resources',
  source_dir: meson.source_root(),
)

inc = include_directories('..')

executable(
  'gjiten',
  sources + [resources],
  dependencies: [
    dependency('gtk+-3.0'),
    dependency('glib-2.0'),
    dependency('gobject-2.0'),
    # Add more dependencies if needed
  ],
  include_directories: inc,
  install: true
)