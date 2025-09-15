project(
  'gjiten', 'c',
  version: '3.2.1',
  meson_version: '>=0.54.0',
  default_options: ['warning_level=2', 'c_std=gnu99']
)

gnome = import('gnome')
i18n = import('i18n')
#i18n.gettext('gjiten', languages: ['ja', 'de', 'de_CH'])

gtk_dep     = dependency('gtk+-3.0', version: '>=3.22.0')
glib_dep    = dependency('glib-2.0')
gmodule_dep = dependency('gmodule-2.0')

config_h = configuration_data()
config_h.set_quoted('GETTEXT_PACKAGE', 'gjiten')
config_h.set_quoted('PACKAGE_TARNAME', 'gjiten')
config_h.set_quoted('VERSION', meson.project_version())
config_h.set_quoted('PACKAGE_VERSION', meson.project_version())
config_h.set_quoted('PACKAGE_STRING', meson.project_name() + ' ' + meson.project_version())


if get_option('enable_debug')
  add_project_arguments('-ggdb', '-DDEBUG_GJITEN', '-O0', language: 'c')
  config_h.set('ENABLE_DEBUG', 1)
endif


prefix = get_option('prefix')
bindir = get_option('bindir')
datadir = get_option('datadir')

config_h.set_quoted('SHARE_DIR', join_paths(datadir))
config_h.set_quoted('PIXMAPDIR', join_paths(prefix, get_option('datadir'), 'pixmaps'))
config_h.set_quoted('PIXMAPDIR_LOGO', join_paths(prefix, datadir, 'pixmaps', 'gjiten'))
config_h.set_quoted('PACKAGE_LOCALE_DIR', join_paths(datadir, 'locale'))
config_h.set_quoted('GJITEN_DATADIR', join_paths(datadir, 'gjiten'))
config_h.set_quoted('GJITEN_BINDIR', bindir)


configure_file(
  output: 'config.h',
  configuration: config_h
)

subdir('src')
subdir('gnome')
subdir('pixmaps')
subdir('po')
subdir('dicfiles')
