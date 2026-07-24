#!/usr/bin/env python3
"""
JX3 / Kiếm Thế PAK V1 unpacker — pure Python, no deps.
Built and VERIFIED against real China3 (2010) paks: 3112/3112 entries extracted,
V1 filename hash reproduced 268/268 against the manifest.

Format (see jx3-pak-format-research.md):
  Header 32B: 'PACK' | count(u32) | index_off(u32) | data_off(u32) | crc(u32) | paktime(u32) | reserved[8]
  Index entry 16B: id(u32) | offset(u32) | size(u32, raw) | csize_flag(u32)
      method = csize_flag & 0xF0000000   (0x00=NONE, 0x10=FRAME, 0x20=UCL/NRV2B)
      csize  = csize_flag & 0x0FFFFFFF
  Compression: UCL NRV2B (ucl_nrv2b_decompress_8). NOTE: NRV2B uses the SIMPLE
  offset high-bit loop — NOT the interleaved NRV2D form.

Usage: python3 jx3-unpack-v1.py <file.pak> [outdir]  (default outdir: <file.pak>.out)
"""
import struct, os, sys


def nrv2b_decompress(src, out_len):
    ip = 0
    op = bytearray()
    bb = 0

    def getbit():
        nonlocal bb, ip
        if bb & 0x7f:
            bb = bb * 2
        else:
            bb = src[ip] * 2 + 1
            ip += 1
        return (bb >> 8) & 1

    last_m_off = 1
    while len(op) < out_len:
        while getbit():                       # literal run
            op.append(src[ip]); ip += 1
        m_off = 1                             # offset high bits (NRV2B simple loop)
        while True:
            m_off = m_off * 2 + getbit()
            if getbit():
                break
        if m_off == 2:
            m_off = last_m_off
        else:
            m_off = (m_off - 3) * 256 + src[ip]; ip += 1
            if m_off == 0xffffffff:           # EOF
                break
            m_off += 1
            last_m_off = m_off
        m_len = getbit()                      # match length
        m_len = m_len * 2 + getbit()
        if m_len == 0:
            m_len = 1
            while True:
                m_len = m_len * 2 + getbit()
                if getbit():
                    break
            m_len += 2
        if m_off > 0xd00:
            m_len += 1
        pos = len(op) - m_off                 # copy match (m_len + 1 bytes)
        for _ in range(m_len + 1):
            op.append(op[pos]); pos += 1
    return bytes(op)


def v1_hash(name):
    """g_FileNameHash — verified byte-exact vs libEngine_Lua5.so (JX3 2012)."""
    M = 0xFFFFFFFF
    hid = 0; index = 0
    for ch in name:
        c = ord(ch)
        if ord('A') <= c <= ord('Z'):
            c += ord('a') - ord('A')      # uppercase -> lowercase
        elif c == 0x2f:                   # '/' -> '\'
            c = 0x5c
        index += 1
        hid = ((hid + index * c) & M) % 0x8000000b
        hid = (hid * 0xffffffef) & M
    return hid ^ 0x12345678


def parse(path):
    d = open(path, 'rb').read()
    sig, count, idx_off, data_off, crc, paktime = struct.unpack_from('<4sIIIII', d, 0)
    assert sig == b'PACK', f'not a PACK V1 file: {sig!r}'
    ents = []
    for i in range(count):
        uId, uOff, lSize, lCsf = struct.unpack_from('<IIiI', d, idx_off + i * 16)
        ents.append((uId, uOff, lSize, lCsf & 0xf0000000, lCsf & 0x0fffffff))
    return d, ents


def get_data(d, uOff, lSize, method, csize):
    if method == 0x00000000:                  # stored
        return d[uOff:uOff + lSize]
    if method == 0x20000000:                  # UCL / NRV2B
        return nrv2b_decompress(d[uOff:uOff + csize], lSize)
    if method == 0x10000000:                  # fragmented
        nfrag, finfo_off = struct.unpack_from('<ii', d, uOff)
        out = bytearray()
        for f in range(nfrag):
            fo, fsz, fcsf = struct.unpack_from('<III', d, uOff + finfo_off + f * 12)
            frag = d[uOff + fo:uOff + fo + (fcsf & 0x0fffffff)]
            out += nrv2b_decompress(frag, fsz) if (fcsf & 0xf0000000) == 0x20000000 else frag
        return bytes(out)
    raise ValueError(f'unsupported method {hex(method)}')


def main():
    if len(sys.argv) < 2:
        print(__doc__); sys.exit(1)
    pak = sys.argv[1]
    outdir = sys.argv[2] if len(sys.argv) > 2 else pak + '.out'
    d, ents = parse(pak)
    ok = 0
    for uId, uOff, lSize, method, csize in ents:
        try:
            data = get_data(d, uOff, lSize, method, csize)
            assert len(data) == lSize
        except Exception as e:
            print(f'  FAIL {uId:08x}: {e}'); continue
        fp = os.path.join(outdir, f'{uId:08x}.bin')
        os.makedirs(outdir, exist_ok=True)
        open(fp, 'wb').write(data)
        ok += 1
    print(f'{pak}: extracted {ok}/{len(ents)} -> {outdir}/  (files named by hash id; supply a path list to rename)')


if __name__ == '__main__':
    main()
