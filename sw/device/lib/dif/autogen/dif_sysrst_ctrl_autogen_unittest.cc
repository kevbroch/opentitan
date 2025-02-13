// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

// This file is auto-generated.

#include "sw/device/lib/dif/autogen/dif_sysrst_ctrl_autogen.h"

#include "gtest/gtest.h"
#include "sw/device/lib/base/mmio.h"
#include "sw/device/lib/base/testing/mock_mmio.h"

#include "sysrst_ctrl_regs.h"  // Generated.

namespace dif_sysrst_ctrl_autogen_unittest {
namespace {
using ::mock_mmio::MmioTest;
using ::mock_mmio::MockDevice;
using ::testing::Eq;
using ::testing::Test;

class SysrstCtrlTest : public Test, public MmioTest {
 protected:
  dif_sysrst_ctrl_t sysrst_ctrl_ = {.base_addr = dev().region()};
};

class InitTest : public SysrstCtrlTest {};

TEST_F(InitTest, NullArgs) {
  EXPECT_EQ(dif_sysrst_ctrl_init({.base_addr = dev().region()}, nullptr),
            kDifBadArg);
}

TEST_F(InitTest, Success) {
  EXPECT_EQ(dif_sysrst_ctrl_init({.base_addr = dev().region()}, &sysrst_ctrl_),
            kDifOk);
}

class IrqGetStateTest : public SysrstCtrlTest {};

TEST_F(IrqGetStateTest, NullArgs) {
  dif_sysrst_ctrl_irq_state_snapshot_t irq_snapshot = 0;

  EXPECT_EQ(dif_sysrst_ctrl_irq_get_state(nullptr, &irq_snapshot), kDifBadArg);

  EXPECT_EQ(dif_sysrst_ctrl_irq_get_state(&sysrst_ctrl_, nullptr), kDifBadArg);

  EXPECT_EQ(dif_sysrst_ctrl_irq_get_state(nullptr, nullptr), kDifBadArg);
}

TEST_F(IrqGetStateTest, SuccessAllRaised) {
  dif_sysrst_ctrl_irq_state_snapshot_t irq_snapshot = 0;

  EXPECT_READ32(SYSRST_CTRL_INTR_STATE_REG_OFFSET,
                std::numeric_limits<uint32_t>::max());
  EXPECT_EQ(dif_sysrst_ctrl_irq_get_state(&sysrst_ctrl_, &irq_snapshot),
            kDifOk);
  EXPECT_EQ(irq_snapshot, std::numeric_limits<uint32_t>::max());
}

TEST_F(IrqGetStateTest, SuccessNoneRaised) {
  dif_sysrst_ctrl_irq_state_snapshot_t irq_snapshot = 0;

  EXPECT_READ32(SYSRST_CTRL_INTR_STATE_REG_OFFSET, 0);
  EXPECT_EQ(dif_sysrst_ctrl_irq_get_state(&sysrst_ctrl_, &irq_snapshot),
            kDifOk);
  EXPECT_EQ(irq_snapshot, 0);
}

class IrqIsPendingTest : public SysrstCtrlTest {};

TEST_F(IrqIsPendingTest, NullArgs) {
  bool is_pending;

  EXPECT_EQ(dif_sysrst_ctrl_irq_is_pending(nullptr, kDifSysrstCtrlIrqSysrstCtrl,
                                           &is_pending),
            kDifBadArg);

  EXPECT_EQ(dif_sysrst_ctrl_irq_is_pending(
                &sysrst_ctrl_, kDifSysrstCtrlIrqSysrstCtrl, nullptr),
            kDifBadArg);

  EXPECT_EQ(dif_sysrst_ctrl_irq_is_pending(nullptr, kDifSysrstCtrlIrqSysrstCtrl,
                                           nullptr),
            kDifBadArg);
}

TEST_F(IrqIsPendingTest, BadIrq) {
  bool is_pending;
  // All interrupt CSRs are 32 bit so interrupt 32 will be invalid.
  EXPECT_EQ(
      dif_sysrst_ctrl_irq_is_pending(
          &sysrst_ctrl_, static_cast<dif_sysrst_ctrl_irq_t>(32), &is_pending),
      kDifBadArg);
}

TEST_F(IrqIsPendingTest, Success) {
  bool irq_state;

  // Get the first IRQ state.
  irq_state = false;
  EXPECT_READ32(SYSRST_CTRL_INTR_STATE_REG_OFFSET,
                {{SYSRST_CTRL_INTR_STATE_SYSRST_CTRL_BIT, true}});
  EXPECT_EQ(dif_sysrst_ctrl_irq_is_pending(
                &sysrst_ctrl_, kDifSysrstCtrlIrqSysrstCtrl, &irq_state),
            kDifOk);
  EXPECT_TRUE(irq_state);
}

class AcknowledgeAllTest : public SysrstCtrlTest {};

TEST_F(AcknowledgeAllTest, NullArgs) {
  EXPECT_EQ(dif_sysrst_ctrl_irq_acknowledge_all(nullptr), kDifBadArg);
}

TEST_F(AcknowledgeAllTest, Success) {
  EXPECT_WRITE32(SYSRST_CTRL_INTR_STATE_REG_OFFSET,
                 std::numeric_limits<uint32_t>::max());

  EXPECT_EQ(dif_sysrst_ctrl_irq_acknowledge_all(&sysrst_ctrl_), kDifOk);
}

class IrqAcknowledgeTest : public SysrstCtrlTest {};

TEST_F(IrqAcknowledgeTest, NullArgs) {
  EXPECT_EQ(
      dif_sysrst_ctrl_irq_acknowledge(nullptr, kDifSysrstCtrlIrqSysrstCtrl),
      kDifBadArg);
}

TEST_F(IrqAcknowledgeTest, BadIrq) {
  EXPECT_EQ(dif_sysrst_ctrl_irq_acknowledge(
                nullptr, static_cast<dif_sysrst_ctrl_irq_t>(32)),
            kDifBadArg);
}

TEST_F(IrqAcknowledgeTest, Success) {
  // Clear the first IRQ state.
  EXPECT_WRITE32(SYSRST_CTRL_INTR_STATE_REG_OFFSET,
                 {{SYSRST_CTRL_INTR_STATE_SYSRST_CTRL_BIT, true}});
  EXPECT_EQ(dif_sysrst_ctrl_irq_acknowledge(&sysrst_ctrl_,
                                            kDifSysrstCtrlIrqSysrstCtrl),
            kDifOk);
}

class IrqForceTest : public SysrstCtrlTest {};

TEST_F(IrqForceTest, NullArgs) {
  EXPECT_EQ(dif_sysrst_ctrl_irq_force(nullptr, kDifSysrstCtrlIrqSysrstCtrl),
            kDifBadArg);
}

TEST_F(IrqForceTest, BadIrq) {
  EXPECT_EQ(dif_sysrst_ctrl_irq_force(nullptr,
                                      static_cast<dif_sysrst_ctrl_irq_t>(32)),
            kDifBadArg);
}

TEST_F(IrqForceTest, Success) {
  // Force first IRQ.
  EXPECT_WRITE32(SYSRST_CTRL_INTR_TEST_REG_OFFSET,
                 {{SYSRST_CTRL_INTR_TEST_SYSRST_CTRL_BIT, true}});
  EXPECT_EQ(
      dif_sysrst_ctrl_irq_force(&sysrst_ctrl_, kDifSysrstCtrlIrqSysrstCtrl),
      kDifOk);
}

class IrqGetEnabledTest : public SysrstCtrlTest {};

TEST_F(IrqGetEnabledTest, NullArgs) {
  dif_toggle_t irq_state;

  EXPECT_EQ(dif_sysrst_ctrl_irq_get_enabled(
                nullptr, kDifSysrstCtrlIrqSysrstCtrl, &irq_state),
            kDifBadArg);

  EXPECT_EQ(dif_sysrst_ctrl_irq_get_enabled(
                &sysrst_ctrl_, kDifSysrstCtrlIrqSysrstCtrl, nullptr),
            kDifBadArg);

  EXPECT_EQ(dif_sysrst_ctrl_irq_get_enabled(
                nullptr, kDifSysrstCtrlIrqSysrstCtrl, nullptr),
            kDifBadArg);
}

TEST_F(IrqGetEnabledTest, BadIrq) {
  dif_toggle_t irq_state;

  EXPECT_EQ(
      dif_sysrst_ctrl_irq_get_enabled(
          &sysrst_ctrl_, static_cast<dif_sysrst_ctrl_irq_t>(32), &irq_state),
      kDifBadArg);
}

TEST_F(IrqGetEnabledTest, Success) {
  dif_toggle_t irq_state;

  // First IRQ is enabled.
  irq_state = kDifToggleDisabled;
  EXPECT_READ32(SYSRST_CTRL_INTR_ENABLE_REG_OFFSET,
                {{SYSRST_CTRL_INTR_ENABLE_SYSRST_CTRL_BIT, true}});
  EXPECT_EQ(dif_sysrst_ctrl_irq_get_enabled(
                &sysrst_ctrl_, kDifSysrstCtrlIrqSysrstCtrl, &irq_state),
            kDifOk);
  EXPECT_EQ(irq_state, kDifToggleEnabled);
}

class IrqSetEnabledTest : public SysrstCtrlTest {};

TEST_F(IrqSetEnabledTest, NullArgs) {
  dif_toggle_t irq_state = kDifToggleEnabled;

  EXPECT_EQ(dif_sysrst_ctrl_irq_set_enabled(
                nullptr, kDifSysrstCtrlIrqSysrstCtrl, irq_state),
            kDifBadArg);
}

TEST_F(IrqSetEnabledTest, BadIrq) {
  dif_toggle_t irq_state = kDifToggleEnabled;

  EXPECT_EQ(
      dif_sysrst_ctrl_irq_set_enabled(
          &sysrst_ctrl_, static_cast<dif_sysrst_ctrl_irq_t>(32), irq_state),
      kDifBadArg);
}

TEST_F(IrqSetEnabledTest, Success) {
  dif_toggle_t irq_state;

  // Enable first IRQ.
  irq_state = kDifToggleEnabled;
  EXPECT_MASK32(SYSRST_CTRL_INTR_ENABLE_REG_OFFSET,
                {{SYSRST_CTRL_INTR_ENABLE_SYSRST_CTRL_BIT, 0x1, true}});
  EXPECT_EQ(dif_sysrst_ctrl_irq_set_enabled(
                &sysrst_ctrl_, kDifSysrstCtrlIrqSysrstCtrl, irq_state),
            kDifOk);
}

class IrqDisableAllTest : public SysrstCtrlTest {};

TEST_F(IrqDisableAllTest, NullArgs) {
  dif_sysrst_ctrl_irq_enable_snapshot_t irq_snapshot = 0;

  EXPECT_EQ(dif_sysrst_ctrl_irq_disable_all(nullptr, &irq_snapshot),
            kDifBadArg);

  EXPECT_EQ(dif_sysrst_ctrl_irq_disable_all(nullptr, nullptr), kDifBadArg);
}

TEST_F(IrqDisableAllTest, SuccessNoSnapshot) {
  EXPECT_WRITE32(SYSRST_CTRL_INTR_ENABLE_REG_OFFSET, 0);
  EXPECT_EQ(dif_sysrst_ctrl_irq_disable_all(&sysrst_ctrl_, nullptr), kDifOk);
}

TEST_F(IrqDisableAllTest, SuccessSnapshotAllDisabled) {
  dif_sysrst_ctrl_irq_enable_snapshot_t irq_snapshot = 0;

  EXPECT_READ32(SYSRST_CTRL_INTR_ENABLE_REG_OFFSET, 0);
  EXPECT_WRITE32(SYSRST_CTRL_INTR_ENABLE_REG_OFFSET, 0);
  EXPECT_EQ(dif_sysrst_ctrl_irq_disable_all(&sysrst_ctrl_, &irq_snapshot),
            kDifOk);
  EXPECT_EQ(irq_snapshot, 0);
}

TEST_F(IrqDisableAllTest, SuccessSnapshotAllEnabled) {
  dif_sysrst_ctrl_irq_enable_snapshot_t irq_snapshot = 0;

  EXPECT_READ32(SYSRST_CTRL_INTR_ENABLE_REG_OFFSET,
                std::numeric_limits<uint32_t>::max());
  EXPECT_WRITE32(SYSRST_CTRL_INTR_ENABLE_REG_OFFSET, 0);
  EXPECT_EQ(dif_sysrst_ctrl_irq_disable_all(&sysrst_ctrl_, &irq_snapshot),
            kDifOk);
  EXPECT_EQ(irq_snapshot, std::numeric_limits<uint32_t>::max());
}

class IrqRestoreAllTest : public SysrstCtrlTest {};

TEST_F(IrqRestoreAllTest, NullArgs) {
  dif_sysrst_ctrl_irq_enable_snapshot_t irq_snapshot = 0;

  EXPECT_EQ(dif_sysrst_ctrl_irq_restore_all(nullptr, &irq_snapshot),
            kDifBadArg);

  EXPECT_EQ(dif_sysrst_ctrl_irq_restore_all(&sysrst_ctrl_, nullptr),
            kDifBadArg);

  EXPECT_EQ(dif_sysrst_ctrl_irq_restore_all(nullptr, nullptr), kDifBadArg);
}

TEST_F(IrqRestoreAllTest, SuccessAllEnabled) {
  dif_sysrst_ctrl_irq_enable_snapshot_t irq_snapshot =
      std::numeric_limits<uint32_t>::max();

  EXPECT_WRITE32(SYSRST_CTRL_INTR_ENABLE_REG_OFFSET,
                 std::numeric_limits<uint32_t>::max());
  EXPECT_EQ(dif_sysrst_ctrl_irq_restore_all(&sysrst_ctrl_, &irq_snapshot),
            kDifOk);
}

TEST_F(IrqRestoreAllTest, SuccessAllDisabled) {
  dif_sysrst_ctrl_irq_enable_snapshot_t irq_snapshot = 0;

  EXPECT_WRITE32(SYSRST_CTRL_INTR_ENABLE_REG_OFFSET, 0);
  EXPECT_EQ(dif_sysrst_ctrl_irq_restore_all(&sysrst_ctrl_, &irq_snapshot),
            kDifOk);
}

}  // namespace
}  // namespace dif_sysrst_ctrl_autogen_unittest
