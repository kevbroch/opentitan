// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

// This file is auto-generated.

#include "sw/device/lib/dif/autogen/dif_rv_plic_autogen.h"

#include "gtest/gtest.h"
#include "sw/device/lib/base/mmio.h"
#include "sw/device/lib/base/testing/mock_mmio.h"

#include "rv_plic_regs.h"  // Generated.

namespace dif_rv_plic_autogen_unittest {
namespace {
using ::mock_mmio::MmioTest;
using ::mock_mmio::MockDevice;
using ::testing::Eq;
using ::testing::Test;

class RvPlicTest : public Test, public MmioTest {
 protected:
  dif_rv_plic_t rv_plic_ = {.base_addr = dev().region()};
};

class InitTest : public RvPlicTest {};

TEST_F(InitTest, NullArgs) {
  EXPECT_EQ(dif_rv_plic_init({.base_addr = dev().region()}, nullptr),
            kDifBadArg);
}

TEST_F(InitTest, Success) {
  EXPECT_EQ(dif_rv_plic_init({.base_addr = dev().region()}, &rv_plic_), kDifOk);
}

}  // namespace
}  // namespace dif_rv_plic_autogen_unittest
