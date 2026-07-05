#!/usr/bin/env python3
"""
Case-insensitive #include shim for the Windows->Linux JX3 server build.
Windows/macOS resolve #include "Engine/KMemory.h" against on-disk engine/kmemory.h
because their FS is case-insensitive; Linux won't. This scans every #include "..."
in the source + headers, and for any target that doesn't match a real file's exact
case, drops a symlink at shim/<exact-target-case> -> the real file.

Symlink targets use the CONTAINER path (/work/...) since the shim is only consumed
inside the centos:7 build container (build dir mounted at /work).
Run on host: python3 mkshim.py
"""
import os, re

HOST = os.path.dirname(os.path.abspath(__file__))          # .../scratchpad/build
CONTAINER = "/work"                                          # mount point in container
SHIM = os.path.join(HOST, "shim")

# roots (relative to build/) where real headers live; source dirs included so
# cross-module and local includes resolve too.
ROOTS = [
    "include/Include", "include/Include/SO3World", "include/Include/SO3Represent",
    "include/Base", "include/Base/engine", "include/Base/common",
    "src/SO3World/Src", "src/SO3World", "src/SO3Represent/Src", "src/SO3GameServer",
    "compat",
]
SCAN_EXT = (".h", ".hpp", ".inl", ".cpp", ".c", ".cxx", ".cc")
INC_RE = re.compile(rb'#\s*include\s*"([^"]+)"')

# 1. index every real file: exact relpath-within-a-root, and lowercase lookups
exact_rel = set()                 # relpaths (with '/') that resolve as-is in a root
low_rel   = {}                    # lower(relpath) -> path-relative-to-build
low_base  = {}                    # lower(basename) -> [paths-relative-to-build]
for root in ROOTS:
    ap = os.path.join(HOST, root)
    if not os.path.isdir(ap):
        continue
    for dirpath, _, files in os.walk(ap):
        for fn in files:
            full = os.path.join(dirpath, fn)
            rel_root = os.path.relpath(full, ap).replace(os.sep, "/")   # within this root
            rel_bld  = os.path.relpath(full, HOST).replace(os.sep, "/") # within build/
            exact_rel.add(rel_root)
            low_rel.setdefault(rel_root.lower(), rel_bld)
            low_base.setdefault(fn.lower(), []).append(rel_bld)

# 2. collect all #include "..." targets
targets = set()
for root in ROOTS:
    ap = os.path.join(HOST, root)
    if not os.path.isdir(ap):
        continue
    for dirpath, _, files in os.walk(ap):
        for fn in files:
            if not fn.lower().endswith(SCAN_EXT):
                continue
            try:
                data = open(os.path.join(dirpath, fn), "rb").read()
            except Exception:
                continue
            for m in INC_RE.finditer(data):
                t = m.group(1).decode("latin-1").replace("\\", "/").lstrip("./")
                if t:
                    targets.add(t)

# 3. for each target that won't resolve by exact case, create a shim symlink
made = amb = miss = 0
for t in sorted(targets):
    if t in exact_rel:
        continue                                    # resolves as-is somewhere
    real = low_rel.get(t.lower())                   # same path, different case
    if real is None:                                # fall back to basename match
        cands = low_base.get(os.path.basename(t).lower())
        if not cands:
            miss += 1; continue
        if len(cands) > 1:
            # prefer a candidate whose parent dir matches the target's parent
            par = os.path.basename(os.path.dirname(t)).lower()
            pref = [c for c in cands if os.path.basename(os.path.dirname(c)).lower() == par]
            cands = pref or cands
            amb += 1
        real = cands[0]
    link = os.path.join(SHIM, t)
    os.makedirs(os.path.dirname(link), exist_ok=True)
    if os.path.islink(link) or os.path.exists(link):
        try: os.remove(link)
        except OSError: pass
    os.symlink(CONTAINER + "/" + real, link)        # container-absolute target
    made += 1

print(f"shim: {made} symlinks created, {amb} ambiguous(basename), {miss} unresolved")
print(f"shim dir: {SHIM}")
