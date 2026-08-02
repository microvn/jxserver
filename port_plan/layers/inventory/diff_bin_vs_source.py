#!/usr/bin/env python3
"""Đo drift 2010->2.5.2: so tập hàm Class::method giữa BINARY (hoặc dump giải mã) và SOURCE.
Binary: lấy signature từ __PRETTY_FUNCTION__ strings. Source: grep định nghĩa (CR->LF).

Usage: diff_bin_vs_source.py <binary_or_elf> <src_dir> [<src_dir> ...]
"""
import re, subprocess, glob, sys, os

BIN = sys.argv[1]
SRCS = sys.argv[2:]
sig_re = re.compile(r'\b([A-Za-z_]\w*)::([A-Za-z_]\w*)\s*\(')

raw = subprocess.run(["strings","-a",BIN], capture_output=True).stdout.decode("latin-1","replace")
bin_funcs=set()
for line in raw.splitlines():
    for m in sig_re.finditer(line):
        bin_funcs.add((m.group(1), m.group(2)))

src_funcs=set()
nfiles=0
for d in SRCS:
    for f in glob.glob(os.path.join(d,"**","*.cpp"), recursive=True):
        if "/Test/" in f or "TestProject" in f: continue
        nfiles+=1
        data=open(f,"rb").read().replace(b"\r",b"\n").decode("latin-1","replace")
        for m in re.finditer(sig_re, data):
            src_funcs.add((m.group(1), m.group(2)))

common=bin_funcs&src_funcs; bin_only=bin_funcs-src_funcs; src_only=src_funcs-bin_funcs
print("binary:", BIN)
print("  distinct Class::method =", len(bin_funcs), " classes =", len(set(c for c,_ in bin_funcs)))
print("source:", nfiles, "files; distinct Class::method =", len(src_funcs))
print("\n=== DRIFT ===")
print("common               :", len(common))
print("binary-only (2.5.2 +) :", len(bin_only))
print("source-only (removed) :", len(src_only))
pct = 100*len(common)/max(1,len(bin_funcs))
print("coverage: %.1f%% of binary funcs exist in 2010 source" % pct)

# top classes by number of NEW methods in binary
from collections import Counter
newc=Counter(c for c,_ in bin_only)
print("\n=== classes with most NEW methods in 2.5.2 ===")
for c,n in newc.most_common(20):
    print("  %-32s +%d" % (c,n))
