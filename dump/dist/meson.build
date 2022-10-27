systemd_system_unit_dir = systemd_dep.get_variable(
    pkgconfig:'systemdsystemunitdir')
conf_data = configuration_data()
conf_data.set('bindir', get_option('prefix') / get_option('bindir'))

configure_file(
  input: 'pvm_dump_offload@.service.in',
  output: 'pvm_dump_offload@.service',
  configuration: conf_data,
  install: true,
  install_dir: systemd_system_unit_dir)

systemd_alias = [[
    '../pvm_dump_offload@.service',
    'obmc-host-startmin@0.target.wants/pvm_dump_offload@0.service'
]]

foreach service: systemd_alias
    # Meson 0.61 will support this:
    #install_symlink(
    #      service,
    #      install_dir: systemd_system_unit_dir,
    #      pointing_to: link,
    #  )
    meson.add_install_script(
        'sh', '-c',
        'mkdir -p $(dirname $DESTDIR/@0@/@1@)'.format(systemd_system_unit_dir,
            service[1]),
    )   
    meson.add_install_script(
        'sh', '-c',
        'ln -s @0@ $DESTDIR/@1@/@2@'.format(service[0], systemd_system_unit_dir,
            service[1]),
    )   
endforeach

