// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include "sw/device/silicon_creator/mask_rom/rsa_verify.h"

#include <memory.h>
#include <stddef.h>

/**
 * Subtracts `b` from `a` in-place, i.e. `a -= b`.
 *
 * `a` must be greater than or equal to `b`.
 *
 * See Handbook of Applied Cryptography, Ch. 14, Alg. 14.9.
 *
 * @param a A `kRsaNumWords` long buffer, little-endian.
 * @param b A `kRsaNumWords` long buffer, little-endian.
 */
static void subtract(uint32_t *a, const uint32_t *b) {
  uint32_t borrow = 0;
  for (size_t i = 0; i < kRsaNumWords; ++i) {
    uint64_t diff = (uint64_t)a[i] - b[i] - borrow;
    borrow = diff > a[i];
    a[i] = (uint32_t)diff;
  }
}

/**
 * Checks if `a` is greater than or equal to `b`.
 *
 * @param a A `kRsaNumWords` long buffer, little-endian.
 * @param b A `kRsaNumWords` long buffer, little-endian.
 * @return Comparison result.
 */
static bool greater_equal(const uint32_t *a, const uint32_t *b) {
  // TODO(#33): Hardening?
  // Note: Loop terminates when `i` wraps around.
  for (size_t i = kRsaNumWords - 1; i < kRsaNumWords; --i) {
    if (a[i] != b[i]) {
      return a[i] > b[i];
    }
  }
  return true;
}

// FIXME: Merge this comment with the one in the header file.
// This function implements Alg. 14.36 in Handbook of Applied Cryptography:
// 1. result = 0
// 2. For i from 0 to (n - 1):
// 		2.1. u_i = (result_0 + x_i * y_0) * m' mod b
// 		2.2. result = (result + x_i * y + u_i * m) / b
// 3. If result >= m then result = result - m
// 4. Return result
void mont_mul(const uint32_t *x, const uint32_t *y, const uint32_t *m,
              const uint32_t m0_inv, uint32_t *result) {
  memset(result, 0, kRsaNumWords * sizeof(uint32_t));

  for (size_t i = 0; i < kRsaNumWords; ++i) {
    // The loop below reads one word ahead of writes to avoid a separate loop
    // for the division by `b` in step 2.2 of the algorithm. Thus, `acc0` and
    // `acc1` are initialized here before the loop. `acc0` holds the sum of
    // first two addends in step 2.2 while `acc1` holds the entire sum. Carries
    // of these operations are stored separately in the upper words of `acc0`
    // and `acc1`. `acc0` and `acc1` can safely store these intermediate values,
    // i.e. without wrapping, because UINT32_MAX^2 + 2*UINT32_MAX is
    // 0xffff_ffff_ffff_ffff.

    // Holds the sum of the first two addends in step 2.2.
    uint64_t acc0 = (uint64_t)x[i] * y[0] + result[0];
    const uint32_t u_i = (uint32_t)acc0 * m0_inv;
    // Holds the sum of the all three addends in step 2.2.
    uint64_t acc1 = (uint64_t)u_i * m[0] + (uint32_t)acc0;

    // Process the i^th digit of `x`, i.e. `x[i]`.
    for (size_t j = 1; j < kRsaNumWords; ++j) {
      acc0 = (uint64_t)x[i] * y[j] + result[j] + (acc0 >> 32);
      acc1 = (uint64_t)u_i * m[j] + (uint32_t)acc0 + (acc1 >> 32);
      result[j - 1] = (uint32_t)acc1;
    }
    acc0 = (acc0 >> 32) + (acc1 >> 32);
    result[kRsaNumWords - 1] = (uint32_t)acc0;

    // The intermediate result of this algorithm before the check below is
    // bounded by R + m (Eq. (4) in Montgomery Arithmetic from a Software
    // Perspective, Bos. J. W, Montgomery, P. L.) where `m` is an integer with
    // `kRsaNumWords` base 2^32 digits, `R` is 2^(`kRsaNumWords`*32), and m < R.
    // Therefore, if there is a carry, then `result` is not the least
    // non-negative residue of x*y*R^-1 mod m. Since `acc0 >> 32` here is at
    // most 1, we can subtract `m` from `result` without taking it into account
    // and fit `result` into `kRsaNumWords`. Since this is not a direct
    // comparison with `m`, the final result is not guaranteed to be the the
    // least non-negative residue of x*y*R^-1 mod m.
    if (acc0 >> 32) {
      subtract(result, m);
    }
  }
}

bool mod_exp(const uint32_t *sig, const rsa_verify_exponent_t e,
             const uint32_t *r_square, const uint32_t *m, const uint32_t m0_inv,
             uint32_t *result) {
  uint32_t buf[kRsaNumWords];

  if (e == kRsaVerifyExponent3) {
    // result = sig * R mod m
    mont_mul(sig, r_square, m, m0_inv, result);
    // buf = sig^2 * R mod m
    mont_mul(result, result, m, m0_inv, buf);
  } else if (e == kRsaVerifyExponent65537) {
    // buf = sig * R mod m
    mont_mul(sig, r_square, m, m0_inv, buf);
    for (size_t i = 0; i < 8; ++i) {
      // result = sig^{2*i+1} * R mod m (sig's exponent: 2, 8, 32, ..., 32768)
      mont_mul(buf, buf, m, m0_inv, result);
      // buf = sig^{4*i+2} * R mod m (sig's exponent: 4, 16, 64, ..., 65536)
      mont_mul(result, result, m, m0_inv, buf);
    }
  } else {
    return false;
  }
  // result = sig^e mod m
  mont_mul(buf, sig, m, m0_inv, result);

  // We need this check because the result of `mont_mul` is not guaranteed to be
  // the least non-negative residue. We need to subtract `m` from `result` at
  // most once because  `m` is the modulus of an RSA public key and therefore
  // R/2 < m < R.
  if (greater_equal(result, m)) {
    subtract(result, m);
  }

  return true;
}
