// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#ifndef OPENTITAN_SW_DEVICE_SILICON_CREATOR_LIB_BASE_MOCK_SEC_MMIO_H_
#define OPENTITAN_SW_DEVICE_SILICON_CREATOR_LIB_BASE_MOCK_SEC_MMIO_H_

#include "sw/device/lib/base/testing/mock_mmio_test_utils.h"
#include "sw/device/silicon_creator/lib/base/sec_mmio.h"
#include "sw/device/silicon_creator/testing/mask_rom_test.h"

namespace mask_rom_test {
namespace internal {
/**
 * Mock class for abs_mmio.c.
 */
class MockSecMmio : public GlobalMock<MockSecMmio> {
 public:
  MOCK_METHOD(void, Init, (sec_mmio_shutdown_handler callee));
  MOCK_METHOD(uint32_t, Read32, (uint32_t addr));
  MOCK_METHOD(void, Write32, (uint32_t addr, uint32_t value));
  MOCK_METHOD(void, Write32Shadowed, (uint32_t addr, uint32_t value));
  MOCK_METHOD(void, WriteIncrement, (uint32_t value));
  MOCK_METHOD(void, CheckValues, (uint32_t rnd_offset));
  MOCK_METHOD(void, CheckCounters, (uint32_t expected_check_count));
};
}  // namespace internal

using MockSecMmio = testing::StrictMock<internal::MockSecMmio>;

/**
 * Expect a sec_mmio read at the given address, returning the given
 * 32-bit value.
 *
 * @param addr Read address.
 * @param ...  The value to return. May be an integer, a pointer to
 * little-endian data, or a `std::initializer_list<BitField>`.
 */
#define EXPECT_SEC_READ32(addr, ...)                                  \
  EXPECT_CALL(::mask_rom_test::MockSecMmio::Instance(), Read32(addr)) \
      .WillOnce(testing::Return(mock_mmio::ToInt<uint32_t>(__VA_ARGS__)))

/**
 * Expect a sec_mmio write to the given address with the given 32-bit value.
 *
 * @param addr Write address.
 * @param ...  Expected value to be written. May be an integer, a pointer to
 * little-endian data, or a `std::initializer_list<BitField>`.
 */
#define EXPECT_SEC_WRITE32(addr, ...)                   \
  EXPECT_CALL(::mask_rom_test::MockSecMmio::Instance(), \
              Write32(addr, mock_mmio::ToInt<uint32_t>(__VA_ARGS__)));

/**
 * Expect a shadowed write to the given offset with the given 32-bit value.
 *
 * @param addr Write address.
 * @param ...  Expected value to be written. May be an integer, a pointer to
 * little-endian data, or a `std::initializer_list<BitField>`.
 */
#define EXPECT_SEC_WRITE32_SHADOWED(addr, ...)          \
  EXPECT_CALL(::mask_rom_test::MockSecMmio::Instance(), \
              Write32Shadowed(addr, mock_mmio::ToInt<uint32_t>(__VA_ARGS__)));

/**
 * Expect a write counter increment with a given 32-bit increment value.
 *
 * @param val Increment value.
 */
#define EXPECT_SEC_WRITE_INCREMENT(val) \
  EXPECT_CALL(::mask_rom_test::MockSecMmio::Instance(), WriteIncrement(val));

extern "C" {

void sec_mmio_init(sec_mmio_shutdown_handler callee) {
  MockSecMmio::Instance().Init(callee);
}

uint32_t sec_mmio_read32(uint32_t addr) {
  return MockSecMmio::Instance().Read32(addr);
}

void sec_mmio_write32(uint32_t addr, uint32_t value) {
  MockSecMmio::Instance().Write32(addr, value);
}

void sec_mmio_write32_shadowed(uint32_t addr, uint32_t value) {
  MockSecMmio::Instance().Write32Shadowed(addr, value);
}

void sec_mmio_write_increment(uint32_t value) {
  MockSecMmio::Instance().WriteIncrement(value);
}

void sec_mmio_check_values(uint32_t rnd_offset) {
  MockSecMmio::Instance().CheckValues(rnd_offset);
}

void sec_mmio_check_counters(uint32_t expected_check_count) {
  MockSecMmio::Instance().CheckCounters(expected_check_count);
}

}  // extern "C"
}  // namespace mask_rom_test

#endif  // OPENTITAN_SW_DEVICE_SILICON_CREATOR_LIB_BASE_MOCK_SEC_MMIO_H_
