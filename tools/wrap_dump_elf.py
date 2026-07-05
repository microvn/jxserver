#!/usr/bin/env python3
"""Wrap the decrypted center memory dump (base 0x08048000) into a minimal ELF so Ghidra
loads it at the right vaddr (absolute code refs -> strings resolve)."""
import struct, sys
DUMP="/private/tmp/claude-501/-Volumes-ExData-game-jx3/8c29d561-e7a2-4447-8a95-6903d68bd924/scratchpad/center_dump.bin"
OUT ="/private/tmp/claude-501/-Volumes-ExData-game-jx3/8c29d561-e7a2-4447-8a95-6903d68bd924/scratchpad/center_decrypted.elf"
BASE=0x08048000
data=open(DUMP,"rb").read()
phoff=52; dataoff=0x1000
ehdr=b"\x7fELF"+bytes([1,1,1,0])+b"\x00"*8
ehdr+=struct.pack("<HHIIIIIHHHHHH",
    2,3,1, BASE, phoff,0,0, 52,52,1, 0,0,0)
phdr=struct.pack("<IIIIIIII", 1, dataoff, BASE, BASE, len(data), len(data), 7, 0x1000)
blob=ehdr+phdr
blob+=b"\x00"*(dataoff-len(blob))
blob+=data
open(OUT,"wb").write(blob)
print("wrote", OUT, len(blob), "bytes; base", hex(BASE))
