#!/usr/bin/env python3
"""Regenerate ATTRIBUTE_TYPE enum (KAttribute.h) + its string-map definition
(KAttribute.cpp) to EXACTLY match 2.5.2, from tools/_enum_ATTRIBUTE_TYPE.tsv
(extracted from libSO3EnumConvertorD.so). 2.5.2 interleaves new attributes, so
values differ from 2010 -> full regen, explicit values, not append.
Idempotent-ish: replaces the whole enum block and the whole DECLARE_STRING_MAP.
"""
import os, re
ROOT = "/Volumes/ExData/game/jx3/linux-build"
TSV  = os.path.join(ROOT, "tools/_enum_ATTRIBUTE_TYPE.tsv")
H    = os.path.join(ROOT, "include/Include/SO3World/KAttribute.h")
CPP  = os.path.join(ROOT, "src/SO3World/Src/KAttribute.cpp")

rows = [l.rstrip("\n").split("\t") for l in open(TSV)]
pairs = sorted(((n, int(v)) for n, v in rows), key=lambda kv: kv[1])
vals = [v for _, v in pairs]
assert vals == list(range(len(vals))), f"non-contiguous: missing {[i for i in range(max(vals)+1) if i not in set(vals)]}"
atTotal = len(vals)  # 454
print(f"loaded {len(pairs)} attrs, 0..{atTotal-1}, atTotal={atTotal}")

# ---- enum block (explicit values, authoritative) ----
enum_lines = ["enum ATTRIBUTE_TYPE",
              "{",
              "\t// ===================================================================",
              "\t// GENERATED from libSO3EnumConvertorD.so (2.5.2) via tools/extract_enum_maps.py",
              "\t// EXACT 2.5.2 values (interleaved vs 2010) - do not hand-edit; rerun gen.",
              "\t// ==================================================================="]
for n, v in pairs:
    enum_lines.append(f"\t{n} = {v},")
enum_lines.append(f"\tatTotal = {atTotal}")
enum_lines.append("};")
enum_block = "\n".join(enum_lines)

# ---- map block ----
map_lines = ["DECLARE_STRING_MAP_BEGIN(ATTRIBUTE_TYPE)"]
for n, _ in pairs:
    map_lines.append(f"\tREGISTER_MAP_VALUE({n})")
map_lines.append("DECLARE_STRING_MAP_END(ATTRIBUTE_TYPE)")
map_block = "\n".join(map_lines)

# ---- replace enum in .h (from 'enum ATTRIBUTE_TYPE' to matching first '};') ----
h = open(H, encoding="latin-1").read()
s = h.index("enum ATTRIBUTE_TYPE")
e = h.index("};", s) + 2
open(H, "w", encoding="latin-1").write(h[:s] + enum_block + h[e:])
print("KAttribute.h enum replaced")

# ---- replace map in .cpp (BEGIN..END inclusive) ----
c = open(CPP, encoding="latin-1").read()
bs = c.index("DECLARE_STRING_MAP_BEGIN(ATTRIBUTE_TYPE)")
be = c.index("DECLARE_STRING_MAP_END(ATTRIBUTE_TYPE)") + len("DECLARE_STRING_MAP_END(ATTRIBUTE_TYPE)")
open(CPP, "w", encoding="latin-1").write(c[:bs] + map_block + c[be:])
print("KAttribute.cpp map replaced")
print("DONE")
