# Copyright lowRISC contributors.
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0
subdir('base')
subdir('drivers')

# Otbn utilities.
sw_silicon_creator_lib_otbn_util = declare_dependency(
  link_with: static_library(
    'sw_silicon_creator_lib_otbn_util',
    sources: [
      'otbn_util.c',
    ],
    dependencies: [
      sw_silicon_creator_lib_driver_otbn,
    ],
  ),
)

# Must be declared after otbn_util, which is used in the crypto library.
subdir('crypto')

# Mask ROM fake dependencies.  These are temporary dependencies until we
# refactor or copy the other libraries we need (e.g. `runtime`).
sw_silicon_creator_lib_fake_deps = declare_dependency(
  link_with: static_library(
    'sw_silicon_creator_lib_fake_deps',
    sources: [
      'fake_deps.c',
    ],
    dependencies: [
    ],
  ),
)

# Boot data stored in the flash info partition.
sw_silicon_creator_lib_boot_data = declare_dependency(
  link_with: static_library(
    'sw_silicon_creator_lib_boot_data',
    sources: [
      hw_ip_flash_ctrl_reg_h,
      'boot_data.c',
    ],
    dependencies: [
      #TODO(#8777): Replace with silicon_creator version.
      #sw_silicon_creator_lib_driver_flash_ctrl,
      sw_lib_flash_ctrl,
      sw_silicon_creator_lib_driver_hmac,
    ],
  ),
)

# Manifest section for boot stage images stored in flash.
sw_silicon_creator_lib_manifest_section = declare_dependency(
  # Using link_whole so that the .manifest section is created even if a boot
  # stage doesn't use kManifest.
  link_whole: static_library(
    'sw_silicon_creator_lib_manifest_section',
    sources: [
      'manifest.S',
    ],
  ),
)

# Manifest library.
sw_silicon_creator_lib_manifest = declare_dependency(
  link_with: static_library(
    'sw_silicon_creator_lib_manifest',
    sources: [
      'manifest.c',
    ],
  ),
)

# Defined separately for sigverify_keys unit tests.
sw_silicon_creator_lib_sigverify_sources_for_boot_stage_tests = files([
  'sigverify_mod_exp_ibex.c',
  'sigverify.c',
])

# Signature verification.
sw_silicon_creator_lib_sigverify = declare_dependency(
  link_with: static_library(
    'sw_silicon_creator_lib_sigverify',
    sources: [
      'sigverify_mod_exp_ibex.c',
      'sigverify_mod_exp_otbn.c',
      'sigverify.c',
      hw_ip_otp_ctrl_reg_h,
    ],
    dependencies: [
      sw_silicon_creator_lib_driver_otp,
      sw_silicon_creator_lib_driver_lifecycle,
      sw_lib_bitfield,
      sw_silicon_creator_lib_otbn_util,
      sw_silicon_creator_otbn['rsa']['rv32embed_dependency'],
    ],
  ),
)

sw_silicon_creator_lib_shutdown = declare_dependency(
  link_with: static_library(
    'sw_silicon_creator_lib_shutdown',
    sources: [
      hw_ip_alert_handler_reg_h,
      hw_ip_otp_ctrl_reg_h,
      hw_ip_lc_ctrl_reg_h,
      hw_ip_keymgr_reg_h,
      hw_ip_sram_ctrl_reg_h,
      hw_ip_flash_ctrl_reg_h,
      'shutdown.c',
    ],
    dependencies: [
      sw_silicon_creator_lib_base_abs_mmio,
      sw_silicon_creator_lib_driver_alert,
    ],
  ),
)

# Silicon creator ePMP library.
sw_silicon_creator_lib_epmp = declare_dependency(
  link_with: static_library(
    'sw_silicon_creator_lib_epmp',
    sources: [
      'epmp.c',
    ],
  ),
)

# Silicon creator ePMP test utilities.
sw_silicon_creator_lib_epmp_test_unlock = declare_dependency(
  link_with: static_library(
    'sw_silicon_creator_lib_epmp_test_unlock',
    sources: [
      'epmp_test_unlock.c',
    ],
    dependencies: [
      sw_silicon_creator_lib_epmp,
    ],
  ),
)

# Silicon creator assembly exception handler.
sw_silicon_creator_lib_irq_asm = declare_dependency(
  link_with: static_library(
    'sw_silicon_creator_lib_irq_asm',
    sources: [
      hw_ip_aon_timer_reg_h,
      hw_ip_pwrmgr_reg_h,
      'irq_asm.S',
    ],
  ),
)

# Logging library.
sw_silicon_creator_lib_log = declare_dependency(
  link_with: static_library(
    'sw_silicon_creator_lib_log',
    sources: [
      'log.c',
    ],
    dependencies: [
      sw_silicon_creator_lib_driver_uart,
    ],
  ),
)

test('sw_silicon_creator_lib_shutdown_unittest', executable(
    'sw_silicon_creator_lib_shutdown_unittest',
    sources: [
      hw_ip_alert_handler_reg_h,
      hw_ip_lc_ctrl_reg_h,
      hw_ip_otp_ctrl_reg_h,
      hw_ip_keymgr_reg_h,
      hw_ip_sram_ctrl_reg_h,
      hw_ip_flash_ctrl_reg_h,
      'shutdown.c',
      'shutdown_unittest.cc',
    ],
    dependencies: [
      sw_vendor_gtest,
      sw_silicon_creator_lib_base_mock_abs_mmio,
    ],
    native: true,
    c_args: ['-DMOCK_ABS_MMIO', '-DOT_OFF_TARGET_TEST'],
    cpp_args: ['-DMOCK_ABS_MMIO', '-DOT_OFF_TARGET_TEST'],
  ),
  suite: 'mask_rom',
)

test('sw_silicon_creator_lib_error_unittest', executable(
    'sw_silicon_creator_lib_error_unittest',
    sources: [
      'error_unittest.cc',
    ],
    dependencies: [
      sw_vendor_gtest,
    ],
    native: true,
  ),
  suite: 'mask_rom',
)

test('sw_silicon_creator_lib_manifest_unittest', executable(
    'sw_silicon_creator_lib_manifest_unittest',
    sources: [
      'manifest_unittest.cc',
    ],
    dependencies: [
      sw_vendor_gtest,
    ],
    native: true,
  ),
  suite: 'mask_rom',
)

test('sw_silicon_creator_mask_rom_sigverify_mod_exp_ibex_unittest', executable(
    'sw_silicon_creator_mask_rom_sigverify_mod_exp_ibex_unittest',
    sources: [
      'sigverify_mod_exp_ibex.c',
      'sigverify_mod_exp_ibex_unittest.cc',
    ],
    dependencies: [
      sw_vendor_gtest,
    ],
    native: true,
  ),
  suite: 'mask_rom',
)

test('sw_silicon_creator_lib_sigverify_unittest', executable(
    'sw_silicon_creator_lib_sigverify_unittest',
    sources: [
      'sigverify_unittest.cc',
      'sigverify.c',
      hw_ip_otp_ctrl_reg_h,
    ],
    dependencies: [
      sw_vendor_gtest,
      sw_lib_testing_bitfield,
    ],
    native: true,
  ),
  suite: 'mask_rom',
)

test('sw_silicon_creator_lib_epmp_unittest', executable(
    'sw_silicon_creator_lib_epmp_unittest',
    sources: [
      'epmp_unittest.cc',
      'epmp.c',
    ],
    dependencies: [
      sw_vendor_gtest,
      sw_silicon_creator_lib_base_mock_csr,
      sw_lib_testing_bitfield,
    ],
    native: true,
    c_args: ['-DMOCK_CSR'],
    cpp_args: ['-DMOCK_CSR'],
    ),
  suite: 'mask_rom',
)

test('sw_silicon_creator_lib_log_unittest', executable(
    'sw_silicon_creator_lib_log_unittest',
    sources: [
      'log.c',
      'log_unittest.cc',
    ],
    dependencies: [
      sw_vendor_gtest,
    ],
    native: true,
  ),
  suite: 'mask_rom',
)

sw_silicon_creator_lib_boot_data_functest = declare_dependency(
  link_with: static_library(
    'sw_silicon_creator_lib_boot_data_functest',
    sources: [
      hw_ip_flash_ctrl_reg_h,
      'boot_data_functest.c'
    ],
    dependencies: [
      sw_lib_flash_ctrl,
      sw_silicon_creator_lib_boot_data,
    ],
  ),
)
mask_rom_tests += {
  'sw_silicon_creator_lib_boot_data_functest': {
    'library': sw_silicon_creator_lib_boot_data_functest,
  }
}

sw_silicon_creator_lib_sigverify_functest = declare_dependency(
  link_with: static_library(
    'sw_silicon_creator_lib_sigverify_functest',
    sources: [
      'sigverify_functest.c'
    ],
    dependencies: [
      sw_silicon_creator_lib_sigverify,
      sw_silicon_creator_lib_driver_hmac,
    ],
  ),
)
mask_rom_tests += {
  'sw_silicon_creator_lib_sigverify_functest': {
    'library': sw_silicon_creator_lib_sigverify_functest,
  }
}

sw_silicon_creator_lib_irq_asm_functest = declare_dependency(
  link_with: static_library(
    'sw_silicon_creator_lib_irq_asm_functest',
    sources: [
      hw_ip_rstmgr_reg_h,
      'irq_asm_functest.c'
    ],
    dependencies: [
      sw_silicon_creator_lib_irq_asm,
      sw_silicon_creator_lib_driver_retention_sram,
      sw_silicon_creator_lib_driver_rstmgr,
      sw_silicon_creator_lib_driver_watchdog,
    ],
  ),
)
mask_rom_tests += {
  'sw_silicon_creator_lib_irq_asm_functest': {
    'library': sw_silicon_creator_lib_irq_asm_functest,
  }
}
