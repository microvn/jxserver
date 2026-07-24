#!/usr/bin/env python3
"""
JX3 / Kiếm Thế PAK V1 packer — pure Python, no deps. Inverse of jx3-unpack-v1.py.

Produces a 'PACK' V1 archive with UCL NRV2B-compressed entries. The NRV2B encoder
mirrors, bit-for-bit, the ucl_nrv2b_decompress_8 stream format that jx3-unpack-v1.py
decodes — which is itself verified against 3112 real game entries. So a round-trip
(pack -> unpack) is a valid correctness proof for the game's decompressor too.

Index is sorted ascending by file-id hash (required: the game does binary search).

Usage:
  python3 jx3-pack-v1.py <dir> <out.pak>          # pack a dir tree (names hashed)
  python3 jx3-pack-v1.py --from-ids <dir> <out.pak>  # files named <hexid>.bin/.lua/...
"""
import struct, os, sys, glob
from collections import defaultdict

# ---------------- NRV2B compressor (mirror of the _8 decoder) ----------------

class BitWriter:
    def __init__(self):
        self.out = bytearray(); self.tag = -1; self.cnt = 0
    def bit(self, b):
        if self.cnt == 0:
            self.tag = len(self.out); self.out.append(0); self.cnt = 8
        self.cnt -= 1
        if b:
            self.out[self.tag] |= (1 << self.cnt)   # MSB-first within tag byte
    def byte(self, v):
        self.out.append(v & 0xff)

def _put_gamma(bw, value):
    # encode value>=2 as the NRV2B high-part code: rounds of (data-bit, flag-bit),
    # flag=1 ends. Mirrors: x=1; do x=x*2+data; while(!flag)
    k = value.bit_length() - 1            # value in [2^k, 2^(k+1)-1]
    for i in range(k):
        bw.bit((value >> (k - 1 - i)) & 1)   # data bit
        bw.bit(1 if i == k - 1 else 0)       # flag bit (1 on last round)

def nrv2b_compress(data):
    n = len(data)
    bw = BitWriter()
    heads = defaultdict(list)             # 3-byte key -> positions (match finder)
    ip = 0
    pending_lits = []

    def flush_lits():
        for b in pending_lits:
            bw.bit(1); bw.byte(b)
        pending_lits.clear()

    def find_match(pos):
        if pos + 3 > n:
            return 0, 0
        key = bytes(data[pos:pos + 3])
        best_len = 0; best_dist = 0
        for cand in reversed(heads.get(key, ())[-96:]):   # cap search width
            dist = pos - cand
            maxl = min(n - pos, 4096)
            l = 0
            while l < maxl and data[cand + l] == data[pos + l]:
                l += 1
            if l > best_len:
                best_len = l; best_dist = dist
        return best_dist, best_len

    while ip < n:
        dist, length = find_match(ip)
        # encodability: min length is 2 (dist<=0xd00) or 3 (dist>0xd00)
        min_len = 3 if dist > 0xd00 else 2
        if length >= min_len:
            flush_lits()
            bw.bit(0)                                    # end literal run
            val = dist - 1
            _put_gamma(bw, (val >> 8) + 3)               # offset high part (>=3)
            bw.byte(val & 0xff)                          # offset low byte
            bonus = 1 if dist > 0xd00 else 0
            E = (length - 1) - bonus                     # encoded length
            if E <= 3:
                bw.bit((E >> 1) & 1); bw.bit(E & 1)      # short: 2 bits
            else:
                bw.bit(0); bw.bit(0)                     # extended marker
                _put_gamma(bw, E - 2)
            # register hashes we skip over, advance
            for j in range(ip, min(ip + length, n - 2)):
                heads[bytes(data[j:j + 3])].append(j)
            ip += length
        else:
            if ip + 3 <= n:
                heads[bytes(data[ip:ip + 3])].append(ip)
            pending_lits.append(data[ip]); ip += 1

    flush_lits()
    bw.bit(0)                                            # end final literal run
    _put_gamma(bw, 0x1000002)                            # EOF: (H-3)*256+0xff == 0xffffffff
    bw.byte(0xff)
    return bytes(bw.out)

# ---------------- hash + pack ----------------

def v1_hash(name):
    # Verified byte-exact against g_FileNameHash in libEngine_Lua5.so (JX3 2012).
    M = 0xFFFFFFFF; hid = 0; index = 0
    for ch in name:
        c = ord(ch)
        if ord('A') <= c <= ord('Z'):
            c += ord('a') - ord('A')      # uppercase -> lowercase
        elif c == 0x2f:                   # '/' -> '\'  (real engine normalizes)
            c = 0x5c
        index += 1
        hid = ((hid + index * c) & M) % 0x8000000b
        hid = (hid * 0xffffffef) & M
    return hid ^ 0x12345678

def build_pak(entries, out_path):
    """entries: list of (id:int, raw:bytes). Compresses each with NRV2B."""
    HEADER = 32
    blobs = []; index = []
    cur = HEADER
    for fid, raw in entries:
        comp = nrv2b_compress(raw)
        if len(comp) < len(raw):                         # only keep if it helps
            method, payload = 0x20000000, comp
        else:
            method, payload = 0x00000000, raw
        csf = method | (len(payload) & 0x0fffffff)
        index.append((fid, cur, len(raw), csf))
        blobs.append(payload); cur += len(payload)
    index.sort(key=lambda e: e[0])                       # binary-search order
    idx_off = cur
    with open(out_path, 'wb') as f:
        f.write(struct.pack('<4sIIIII', b'PACK', len(index), idx_off, HEADER, 0, 0))
        f.write(b'\x00' * 8)                             # reserved
        for blob in blobs:
            f.write(blob)
        for fid, off, rawsz, csf in index:
            f.write(struct.pack('<IIiI', fid, off, rawsz, csf))
    return idx_off

def main():
    args = sys.argv[1:]
    from_ids = False
    if args and args[0] == '--from-ids':
        from_ids = True; args = args[1:]
    if len(args) != 2:
        print(__doc__); sys.exit(1)
    src, out = args
    entries = []
    for fp in sorted(glob.glob(os.path.join(src, '**', '*'), recursive=True)):
        if not os.path.isfile(fp):
            continue
        raw = open(fp, 'rb').read()
        if from_ids:
            base = os.path.splitext(os.path.basename(fp))[0]
            fid = int(base, 16)
        else:
            # engine hashes the path relative to root WITH its leading '\'
            rel = '\\' + os.path.relpath(fp, src).replace('/', '\\')
            fid = v1_hash(rel)
        entries.append((fid, raw))
    build_pak(entries, out)
    print(f'packed {len(entries)} files -> {out} ({os.path.getsize(out)} bytes)')

if __name__ == '__main__':
    main()
