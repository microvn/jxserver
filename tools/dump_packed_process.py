#!/usr/bin/env python3
"""Defeat the SO3GameCenter/SO3Gateway packer by dumping the DECRYPTED image from a
RUNNING process (the packer self-decrypts into an RWE segment to execute).

Run ON THE HOST while the center is running in a container:
    HPID=$(pgrep -x SO3GameCenter | head -1)
    python3 dump_packed_process.py $HPID 0x08048000 0x088ca000 /tmp/center_dump.bin
Then wrap into an ELF (tools/wrap_dump_elf.py) and load in Ghidra to RE the real code.
Proven 2026-07-06: recovered KGameServer::OnHandshakeRequest/ProcessNetwork + all strings.
Range comes from `readelf -l SO3GameCenter` (LOAD segments; the RWE one holds decrypted code)."""
import sys
pid, lo, hi, out = sys.argv[1], int(sys.argv[2],16), int(sys.argv[3],16), sys.argv[4]
maps=[]
for line in open("/proc/%s/maps"%pid):
    p=line.split(); a,b=[int(x,16) for x in p[0].split("-")]; maps.append((a,b,p[1]))
mem=open("/proc/%s/mem"%pid,"rb",0); cur=lo
with open(out,"wb") as f:
    while cur<hi:
        seg=next((m for m in maps if m[0]<=cur<m[1] and "r" in m[2]), None)
        if seg:
            end=min(seg[1],hi)
            try: mem.seek(cur); f.write(mem.read(end-cur))
            except Exception: f.write(b"\x00"*(end-cur))
            cur=end
        else:
            nxt=min([m[0] for m in maps if m[0]>cur]+[hi]); f.write(b"\x00"*(nxt-cur)); cur=nxt
import os; print("dumped", hex(lo),"-",hex(hi),"->",os.path.getsize(out),"bytes")
