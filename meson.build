project(
    'powervm-handler',
    'cpp',
    default_options: [
        'buildtype=debugoptimized',
        'cpp_std=c++20',
        'warning_level=3',
        'werror=true',
    ],
    meson_version: '>=0.57.0',
)

cpp = meson.get_compiler('cpp')

sdbusplus_dep = dependency(
    'sdbusplus',
    fallback: ['sdbusplus', 'sdbusplus_dep'],
)

sdeventplus_dep = dependency(
    'sdeventplus',
    fallback: ['sdeventplus', 'sdeventplus_dep'],
)

phosphor_dbus_interfaces_dep = dependency(
    'phosphor-dbus-interfaces',
    fallback: [
        'phosphor-dbus-interfaces',
        'phosphor_dbus_interfaces_dep'
    ],
)

phosphor_logging_dep = dependency(
    'phosphor-logging',
    fallback: ['phosphor-logging', 'phosphor_logging_dep'],
)

systemd_dep = dependency('systemd')

configure_file(
    input: 'config.h.in',
    output: 'config.h',
    configuration: configuration_data(),
)

dump_offload_deps = [
    phosphor_dbus_interfaces_dep,
    phosphor_logging_dep,
    sdbusplus_dep,
    sdeventplus_dep,
]

executable(
    'pvm_dump_offload',
    'dump_offload_main.cpp',
    'dump_offload_mgr.cpp',
    dependencies: dump_offload_deps,
    install: true,
)
