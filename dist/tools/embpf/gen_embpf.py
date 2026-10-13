#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# vim:fenc=utf-8

# Copyright (C) 2021 Inria
# Copyright (C) 2021 Koen Zandberg <koen@bergzand.net>
#
# This file is subject to the terms and conditions of the GNU Lesser
# General Public License v2.1. See the file LICENSE in the top level
# directory for more details.

import argparse
import logging
import csv

from embpf.embpf_factory import EmBPFBinarySrcFactory, EmBPFELFSrcFactory


def dump(arguments):
    binary = EmBPFBinarySrcFactory.create(arguments.file)
    binary.dump()


def stats(arguments):
    binary = EmBPFBinarySrcFactory.create(arguments.file)
    header = ["category", "class", "bit_width", "source", "mode", "size", "name"]
    rows = [
        i.get_stats()
        for i in binary.instructions()
    ]
    with arguments.output as output:
        writer = csv.DictWriter(output, fieldnames=header)
        writer.writeheader()
        writer.writerows(rows)


def generate(arguments):
    binary = EmBPFELFSrcFactory.create(
        arguments.input, arguments.patch_functions)
    data = binary.to_bytes()
    arguments.output.write(data)


if __name__ == "__main__":
    parser = argparse.ArgumentParser("RIOT emBPF format utility")
    parser.add_argument('--verbose', '-v', help="Verbose output",
                        action='store_true', default=False)
    parser.add_argument('--debug', '-d', help="All debug output",
                        action='store_true', default=False)
    subparsers = parser.add_subparsers(help="sub commands")

    parser_dump = subparsers.add_parser('dump')
    parser_dump.set_defaults(func=dump)
    parser_dump.add_argument('file', type=argparse.FileType(
        'rb'), help='emBPF file to dump')

    parser_stats = subparsers.add_parser('stats')
    parser_stats.set_defaults(func=stats)
    parser_stats.add_argument('file', type=argparse.FileType(
        'rb'), help='emBPF get stats from')
    parser_stats.add_argument('output', type=argparse.FileType(
        'w'), help='File to write stats to')

    parser_gen = subparsers.add_parser('generate')
    parser_gen.set_defaults(func=generate)
    parser_gen.add_argument('--patch-functions', '-p',
                            action='store_true', default=False)
    parser_gen.add_argument('input', type=argparse.FileType(
        'rb'), help='ELF file to read')
    parser_gen.add_argument('output', type=argparse.FileType(
        'wb'), help='emBPF file to write')

    args = parser.parse_args()

    logging.basicConfig(format='%(message)s')
    logger = logging.getLogger()
    if args.debug:
        logger.setLevel(logging.DEBUG)
    elif args.verbose:
        logger.setLevel(logging.INFO)
    else:
        logger.setLevel(logging.WARNING)

    args.func(args)
