#!/usr/bin/env python3
# Copyright lowRISC contributors.
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0
r"""Generate RTL and documentation collateral from OTP memory
map definition file (hjson).
"""
import argparse
import logging as log
import random
from pathlib import Path

import hjson
from lib.common import wrapped_docstring
from lib.OtpMemMap import OtpMemMap
from mako.template import Template

TABLE_HEADER_COMMENT = '''<!--
DO NOT EDIT THIS FILE DIRECTLY.
It has been generated with ./util/design/gen-otp-mmap.py
-->

'''

# memory map source
MMAP_DEFINITION_FILE = "hw/ip/otp_ctrl/data/otp_ctrl_mmap.hjson"
# documentation tables to generate
PARTITIONS_TABLE_FILE = "hw/ip/otp_ctrl/doc/otp_ctrl_partitions.md"
DIGESTS_TABLE_FILE = "hw/ip/otp_ctrl/doc/otp_ctrl_digests.md"
MMAP_TABLE_FILE = "hw/ip/otp_ctrl/doc/otp_ctrl_mmap.md"
# code templates to render
TEMPLATES = [
    "hw/ip/otp_ctrl/data/otp_ctrl.hjson.tpl",
    "hw/ip/otp_ctrl/rtl/otp_ctrl_part_pkg.sv.tpl"
]


def main():
    log.basicConfig(level=log.WARNING,
                    format="%(levelname)s: %(message)s")

    parser = argparse.ArgumentParser(
        prog="gen-otp-mmap",
        description=wrapped_docstring(),
        formatter_class=argparse.RawDescriptionHelpFormatter)

    # Generator options for compile time random netlist constants
    parser.add_argument('--seed',
                        type=int,
                        metavar='<seed>',
                        help='Custom seed for RNG to compute default values.')

    args = parser.parse_args()

    with open(MMAP_DEFINITION_FILE, 'r') as infile:
        config = hjson.load(infile)

        # If specified, override the seed for random netlist constant computation.
        if args.seed:
            log.warning('Commandline override of seed with {}.'.format(
                args.seed))
            config['seed'] = args.seed
        # Otherwise, we either take it from the .hjson if present, or
        # randomly generate a new seed if not.
        else:
            random.seed()
            new_seed = random.getrandbits(64)
            if config.setdefault('seed', new_seed) == new_seed:
                log.warning(
                    'No seed specified, setting to {}.'.format(new_seed))

        try:
            otp_mmap = OtpMemMap(config)
        except RuntimeError as err:
            log.error(err)
            exit(1)

        with open(PARTITIONS_TABLE_FILE, 'w') as outfile:
            outfile.write(TABLE_HEADER_COMMENT +
                          otp_mmap.create_partitions_table())

        with open(DIGESTS_TABLE_FILE, 'w') as outfile:
            outfile.write(TABLE_HEADER_COMMENT +
                          otp_mmap.create_digests_table())

        with open(MMAP_TABLE_FILE, 'w') as outfile:
            outfile.write(TABLE_HEADER_COMMENT + otp_mmap.create_mmap_table())

        # render all templates
        for template in TEMPLATES:
            with open(template, 'r') as tplfile:
                tpl = Template(tplfile.read())
                with open(
                        Path(template).parent.joinpath(Path(template).stem),
                        'w') as outfile:
                    outfile.write(tpl.render(otp_mmap=otp_mmap))


if __name__ == "__main__":
    main()
