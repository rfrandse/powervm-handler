conf_data = configuration_data()
conf_data.set('bindir', get_option('prefix') / get_option('bindir'))
configure_file(
  input: 'pvm_dump_offload.service.in',
  output: 'pvm_dump_offload.service',
  configuration: conf_data,
  install: true,
  install_dir: systemd_dep.get_pkgconfig_variable('systemdsystemunitdir'))
