#!/usr/bin/env python
# Extracts the target v2.5 Fast-codec key table into the transient native build directory.
from __future__ import print_function

import hashlib
import struct
import sys

EXPECTED_TARGET_SHA256 = (
    '47716c73e8de281c95759cdc4a478e70e7c61322fb46e8c4e04954e51124b94a',
    '3002bf4ad08d4c0fb9d4fc10f2d66b1f6cd8f70d398ff792136a3b7d5c416f11',
)
EXPECTED_TABLE_SHA256 = '5491f08643c87d4ad6c08e3b5f44457acbc78a3cd0d288b6d6f4d99dafe66047'
TABLE_VA = 0x084d9d20
TABLE_COUNT = 5679
TABLE_SIZE = TABLE_COUNT * 4


def fail(message):
    sys.stderr.write('ksg-fast-table: %s\n' % message)
    sys.exit(1)


def main():
    if len(sys.argv) != 3:
        fail('usage: extract_ksg_fast_table.py <target-elf> <output-inc>')
    source_path, output_path = sys.argv[1:]
    blob = open(source_path, 'rb').read()
    target_sha256 = hashlib.sha256(blob).hexdigest()
    if target_sha256 not in EXPECTED_TARGET_SHA256:
        fail('target SHA-256 mismatch')
    if blob[:4] != b'\x7fELF' or ord(blob[4:5]) != 1 or ord(blob[5:6]) != 1:
        fail('expected little-endian ELF32 target')
    phoff = struct.unpack_from('<I', blob, 0x1c)[0]
    phentsize = struct.unpack_from('<H', blob, 0x2a)[0]
    phnum = struct.unpack_from('<H', blob, 0x2c)[0]
    raw = None
    for index in range(phnum):
        offset = phoff + index * phentsize
        p_type, p_offset, p_vaddr, _p_paddr, p_filesz, _p_memsz, _p_flags, _p_align = struct.unpack_from('<IIIIIIII', blob, offset)
        if p_type == 1 and p_vaddr <= TABLE_VA and TABLE_VA + TABLE_SIZE <= p_vaddr + p_filesz:
            begin = p_offset + (TABLE_VA - p_vaddr)
            raw = blob[begin:begin + TABLE_SIZE]
            break
    if raw is None or len(raw) != TABLE_SIZE:
        fail('target key table is outside file-backed PT_LOAD data')
    table_sha256 = hashlib.sha256(raw).hexdigest()
    if table_sha256 != EXPECTED_TABLE_SHA256:
        fail('target key table SHA-256 mismatch')
    values = struct.unpack('<%dI' % TABLE_COUNT, raw)
    output = open(output_path, 'w')
    output.write('/* target: %s; data SHA-256: %s */\n' % (target_sha256, table_sha256))
    for index in range(0, TABLE_COUNT, 8):
        output.write('    %s,\n' % ', '.join('0x%08xu' % value for value in values[index:index + 8]))
    output.close()


if __name__ == '__main__':
    main()
