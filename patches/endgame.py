#!/usr/bin/env python3
# ============================================================================
# HISTORICAL / DO NOT RUN. Frozen into the git baseline commit.
# These edits are ALREADY BAKED into the tracked source (src/, include/).
# Source of truth is now git: fix = edit file + commit, not a patch here.
# Kept only as a record of WHY each drift fix was made (see DECISION.md).
# ============================================================================
"""
Endgame patches — áp lên cây scaffold (src/ + include/) SAU khi setup.sh copy source.
Chỉ vá 5 file SỐNG (xác nhận có trong binary server thật / có caller sống — xem DECISION.md §B2,§D).
KHÔNG vá KMissile/KPathFinder/KTrackList/KAI_Player: chúng CHẾT -> đã loại qua DEAD set trong build.sh.

Idempotent: mỗi patch kiểm marker, chạy nhiều lần không hại. Encoding latin-1 (source có GBK bytes).
Dùng: python3 patches/endgame.py <build_root>
"""
import sys, os, re

ROOT = sys.argv[1] if len(sys.argv) > 1 else os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

def patch(relpath, marker, transform, label):
    full = os.path.join(ROOT, relpath)
    if not os.path.isfile(full):
        print(f"  [MISS] {label}: {relpath} không tồn tại"); return
    text = open(full, "rb").read().decode("latin-1")
    if marker in text:
        print(f"  [skip] {label} (đã vá)"); return
    new = transform(text)
    if new == text:
        print(f"  [WARN] {label}: KHÔNG tìm thấy anchor — bỏ qua"); return
    open(full, "wb").write(new.encode("latin-1"))
    print(f"  [ok]   {label}")

INC = '\n#include "protocolbasic.h"  // _NAME_LEN (endgame patch)'

# 1-2: IDoodadFile.h / INpcFile.h — struct dùng _NAME_LEN ngay trong header
for p in ("include/Include/SO3World/IDoodadFile.h", "include/Include/SO3World/INpcFile.h"):
    patch(p, "protocolbasic.h",
          lambda t: t.replace('#include "KSUnknown.h"', '#include "KSUnknown.h"' + INC, 1), p)

# 3: KGMList.cpp — _NAME_LEN dùng trong .cpp
patch("src/SO3World/Src/KGMList.cpp", "protocolbasic.h",
      lambda t: t.replace('#include "KGMList.h"', '#include "KGMList.h"' + INC, 1), "KGMList.cpp")

# 4: KProtocolRecorderBase.h — in_addr/u_short cần header socket POSIX
patch("src/SO3World/Src/KProtocolRecorderBase.h", "netinet/in.h",
      lambda t: t.replace("#define _KPROTOCOLRECORDERBASE_H_\n",
                          "#define _KPROTOCOLRECORDERBASE_H_\n"
                          "#include <netinet/in.h>  // in_addr/u_short trên Linux (endgame patch)\n", 1),
      "KProtocolRecorderBase.h")

# 5: KNpcTeam — khối #ifdef _SERVER có 4 method, TẤT CẢ chết (GetMemberTarget/CheckNpcWorkDone
#    chỉ KAIBase-excluded gọi; OnMemberDeath/OnMemberRevive không ai gọi). Hai method
#    GetMemberTarget (dùng KThreatNode cũ) và CheckNpcWorkDone (dùng m_AIController cũ, KNpc
#    giờ dùng m_AIVM) KHÔNG compile với header mới. VM AI không có GetAIState() để port thật.
#    -> stub compile-clean cả hai. OnMemberDeath/OnMemberRevive compile OK, để nguyên.
GET_MEMBER_TARGET = (
    "KCharacter* KNpcTeam::GetMemberTarget()\n"
    "{\n"
    "\t// [endgame] GetMemberTarget dead: chi KAIBase (excluded) goi. API threat moi\n"
    "\t// GetFirstThreat() tra const KThreatLiteNode* (chi dwCharacterID, bo pCharacter cu);\n"
    "\t// khong accessor ID->KCharacter* trong tam. Path khong chay (VM-AI) -> NULL.\n"
    "\treturn NULL;\n"
    "}"
)
patch("src/SO3World/Src/KNpcTeam.cpp", "[endgame] GetMemberTarget dead",
      lambda t: re.sub(r"KCharacter\*\s*KNpcTeam::GetMemberTarget\(\)\s*\{.*?\n\}",
                       GET_MEMBER_TARGET, t, count=1, flags=re.DOTALL),
      "KNpcTeam.cpp GetMemberTarget")

CHECK_WORK_DONE = (
    "BOOL KNpcTeam::CheckNpcWorkDone()\n"
    "{\n"
    "\t// [endgame] CheckNpcWorkDone dead: chi KAIBase (excluded) goi; dung m_AIController\n"
    "\t// (AI-cu class-based). KNpc gio dung m_AIVM (VM AI, khong co GetAIState tuong duong).\n"
    "\t// Path khong chay (VM-AI) -> compile-clean, tra FALSE.\n"
    "\treturn FALSE;\n"
    "}"
)
patch("src/SO3World/Src/KNpcTeam.cpp", "[endgame] CheckNpcWorkDone dead",
      lambda t: re.sub(r"BOOL\s*KNpcTeam::CheckNpcWorkDone\(\)\s*\{.*?\n\}",
                       CHECK_WORK_DONE, t, count=1, flags=re.DOTALL),
      "KNpcTeam.cpp CheckNpcWorkDone")

# 6: RUNTIME tolerant config-reader — version drift 2010 (src) vs 2012 (data).
#    KGWConstList::LoadData (source 1.4.0) doc 81 (section,key); exe 2.5.2 doc 208.
#    14 key source doc ma data 2.5.2 da bo (MaxThreatCount/Slay*/MaxCampPrestige/ANTI_FARMER*)
#    -> GetInteger fail -> KGLOG_PROCESS_ERROR(nRetCode) crash. Noi long: key vang dung default.
#    ASCII comment only (file ghi latin-1). Xem DECISION.md §F2 + diff tools/diff_loaddata.py.
# Chiêu TỔNG: quét toàn SO3World, nới CHỈ cặp "get-field-from-file rồi check-retcode-ngay-sau".
# Pattern:  <var> = <expr>->Get{Integer|String|Float|Data}(...);   (dòng i)
#           KGLOG_PROCESS_ERROR(<var>);                            (dòng i+1, cùng var)
# -> đổi dòng check thành (void)<var>;  (key/cột vắng ở data 2.5.2 -> dùng default, không crash).
# KHÔNG đụng KGLOG_PROCESS_ERROR khác (assert logic, so sánh, con trỏ...). ASCII comment.
GET_RE = re.compile(r'^\s*([A-Za-z_]\w*)\s*=\s*.*->Get(?:Integer|String|Float|Data)\s*\(')
def relax_tree(reldir):
    import glob
    files = glob.glob(os.path.join(ROOT, reldir, "*.cpp"))
    total_files = 0; total_relax = 0
    for full in files:
        lines = open(full, encoding="latin-1").read().split("\n")
        changed = 0
        for i in range(len(lines) - 1):
            m = GET_RE.match(lines[i])
            if not m: continue
            var = m.group(1)
            want = f"KGLOG_PROCESS_ERROR({var});"
            # Get co the multi-line -> quet cua so i+1..i+6 tim dung check cua var nay
            for j in range(i+1, min(i+7, len(lines))):
                if lines[j].strip() == want:
                    indent = lines[j][:len(lines[j]) - len(lines[j].lstrip())]
                    lines[j] = f"{indent}(void){var}; /*[endgame] tolerant*/"
                    changed += 1
                    break
                # dừng nếu gặp Get khác cùng var (tránh nới nhầm)
                if GET_RE.match(lines[j]) and re.match(rf'^\s*{var}\s*=', lines[j]):
                    break
        if changed:
            open(full, "w", encoding="latin-1").write("\n".join(lines))
            total_files += 1; total_relax += changed
    print(f"  [ok]   tolerant data-loaders: noi long {total_relax} check tren {total_files} file")

relax_tree("src/SO3World/Src")

# 7: PORT hang-so tu Ghidra (khac tolerant — day la gia tri CHINH XAC 2.5.2, doc tu exe).
#    Struct-size/limit constant drift: source 2010 nho hon 2012 -> data 2012 vuot gioi han.
#    Ghidra decompile exe 2.5.2 cho gia tri that. Vd MakeQuestMap: dwQuestID < 0x4000 (16384).
#    KHONG the tolerant (noi = overflow mang); phai tang khop 2012.
def port_const(relpath, old, new, label):
    full = os.path.join(ROOT, relpath)
    if not os.path.isfile(full): print(f"  [MISS] {label}"); return
    t = open(full, encoding="latin-1").read()
    if new in t and old not in t: print(f"  [skip] {label}"); return
    if old not in t: print(f"  [WARN] {label}: anchor '{old}' khong thay"); return
    open(full, "w", encoding="latin-1").write(t.replace(old, new, 1))
    print(f"  [ok]   {label}: {old.strip()} -> {new.strip()}")

# MAX_QUEST_COUNT: 8192 -> 16384 (verify Ghidra exe 2.5.2 MakeQuestMap; max quest id data=8706)
port_const("include/Include/SO3World/KQuestInfoList.h",
           "#define MAX_QUEST_COUNT\t\t\t        8192",
           "#define MAX_QUEST_COUNT\t\t\t        16384 /*[endgame] Ghidra exe2.5.2=0x4000*/",
           "MAX_QUEST_COUNT 8192->16384")

port_const("include/Include/SO3World/KCoolDownList.h",
           "#define MAX_COOL_DOWN_COUNT 512",
           "#define MAX_COOL_DOWN_COUNT 1024 /*[endgame] Ghidra exe2.5.2=0x400*/",
           "MAX_COOL_DOWN_COUNT 512->1024")

print("endgame patches done.")
