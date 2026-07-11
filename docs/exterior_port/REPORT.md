# BÁO CÁO — RE & thiết kế port KExterior/KExteriorBox
**Bắt đầu:** 2026-07-08 03:14:26 +07 · **Kết thúc:** 2026-07-08 03:25:38 +07 · **Thời gian:** ~11 phút

## Đã làm
1. Xác nhận build 2010 KHÔNG có KExterior/KExteriorBox → port = dựng class mới trên cơ chế represent-ID sẵn có.
2. RE binary v246 qua pyghidra (local): map **76 hàm** KExterior/KExteriorBox từ __PRETTY_FUNCTION__, **decompile 12 hàm lõi**.
3. Giải trọn: kiến trúc 2 class, layout member KExteriorBox, KEXTERIOR_ITEM, KEXTERIOR_INFO, format DB block, **móc tích hợp represent** (map slot→index chính xác), **KExteriorBox nhúng KPlayer @0xb850**.
4. Viết PORT_DESIGN.md (thiết kế port 6 bước + verify) + WORKLOG.md (log RE chi tiết).
5. Backup source (KPlayer.h/.cpp, KLuaCharacter.cpp) — cách li sẵn. KHÔNG chỉnh gốc, KHÔNG xoá gì.
6. Tạo skill `jx3-re-port` (~/.claude/skills/) đóng gói pipeline + ràng buộc để dùng lâu dài.

## Chỉ số quyết định
- **Thời gian RE-hiểu:** ~11 phút (rất nhanh nhờ __PRETTY signature + pyghidra local).
- **Bề mặt port KExterior+Box:** ~57 hàm, chia 6 lát verify được.
- **Độ cô lập:** CAO — chỉ móc represent-ID (đã có) + player persistence + Lua. Không đụng combat/AI/skill.
- **Rủi ro:** (1) byte-layout DB nếu cần tương thích DB v246; (2) packet sync client. Nếu tự sinh DB thì bỏ được rủi ro (1).
- **Còn phải RE (bounded, 4 mục):** layout đầy đủ KEXTERIOR_INFO(nPrice), packet DoSyncExterior*, route Lua binding KPlayer, LoadExteriorBox mirror.

## Khuyến nghị (để bạn quyết)
- **Thiết kế đủ để BẮT ĐẦU CODE.** Đề xuất lát #1: struct + KExteriorBox skeleton + 4 LoadTable (verify: log "loaded 1949 exterior / 59 suit" khớp data).
- **Cân nhắc:** KExterior cô lập tốt NHƯNG là dựng-mới (scaffold DB/Lua/sync). Nếu muốn thắng-nhanh trước, **KDesignation** (đã có class 36 method, chỉ thêm ~15) rủi ro thấp hơn để chạy trọn pipeline 1 lần, rồi quay lại KExterior.

## Files
- /private/tmp/claude-501/-Volumes-ExData-game-jx3/a3f5a210-2f89-426d-a1ab-79e564a6aecd/scratchpad/exterior_port/PORT_DESIGN.md
- /private/tmp/claude-501/-Volumes-ExData-game-jx3/a3f5a210-2f89-426d-a1ab-79e564a6aecd/scratchpad/exterior_port/WORKLOG.md
- /private/tmp/claude-501/-Volumes-ExData-game-jx3/a3f5a210-2f89-426d-a1ab-79e564a6aecd/scratchpad/exterior_port/REPORT.md
- /private/tmp/claude-501/-Volumes-ExData-game-jx3/a3f5a210-2f89-426d-a1ab-79e564a6aecd/scratchpad/exterior_port/backup_source_*/  (backup)
- ~/.claude/skills/jx3-re-port/SKILL.md

---
## Cập nhật (round 2) — RE-6: giải trọn 4 follow-up + chuẩn hoá docs
- **Đã RE nốt cả 4 mục** (không còn "còn phải RE" chặn thiết kế):
  1. nPrice/KEXTERIOR_INFO — pin đủ 18 cột ExteriorInfo.tab, giá ×10000.
  2. Packet sync — **protocol 0x11a**, header 6B + item 7B {WORD id; BYTE flag; DWORD expire}.
  3. Lua routing — LuaApplyExterior: flag|=0x80 + loop 5 slot + sync.
  4. LoadExteriorBox mirror — DB item 16B, count-prefixed, gọi _Add + sync.
- **Rủi ro (1)+(2) đã hạ**: DB byte-layout + packet đều pin → port ĐỦ (khớp v246) được.
- Caveat: client pap2 (build 4550) không có Exterior → chỉ client v246/BVTLocal render.
- **Docs chuẩn hoá** (canonical: linux-build/docs/exterior_port/): PORT_DESIGN thêm §1 LINKED/RELEVANT + thứ tự port bé→to; README (convention thư mục port); backup có MANIFEST + tên có ngày giờ.
- **Skill** (project .claude/skills/jx3-re-port) nâng cấp: bước chi tiết + workflow SYSTEMATIC-SEARCH + WRITE-FILE.
- Tổng: decompile 15 hàm lõi, map 76 hàm. RE coi như **đủ để bắt đầu code** theo §7 (bé→to).

---

# PORT IMPLEMENTATION COMPLETE — 2026-07-08 ~14:00 +07

**Trạng thái: XONG cả 8 lát (PORT_DESIGN §7).** Mỗi lát: compile sạch + verify pass + WORKLOG [PORT-n].
Thời gian code: ~10:40→14:00 +07 (~3h20).

## Build/verify tổng
- Host Linode x86 (docker jx3build, gcc4.8/-m32): **COMPILE ok=191 fail=0, link 0 undefined, BINARY PRODUCED** (+KExterior.o, +KExteriorBox.o).
- Runtime: **"Load game settings [OK]"**, 0 lỗi exterior, 0 crash/assert; 4 bảng nạp trên data thật; không hồi quy startup.
- Verify/lát: #1 layout static_assert · #2 runtime settings-OK · #3 verbatim v246 switch + enum-guard 2010==v246 · #4 _Add oracle 6 · #5 Apply oracle 6 · #6 Save/Load roundtrip oracle · #7 item-7B oracle · #8 regression-boot 0 lỗi. TẤT CẢ PASS.

## Port đủ dùng (script-drivable): KExterior (config+grant), KExteriorBox (own/set/apply/unapply/save/sync), persistence rbtExteriorData, packet 2010-native, 9 Lua binding KPlayer đăng ký Luna.

## Phát hiện then chốt
- `grep -a` bắt buộc (source GBK). Crux giải: m_wRepresentId + lock + Lua get/set có sẵn 2010.
- Version-drift PASS: enum represent 2010 index {2,3,5,6,8,9,11,12,14,15} = v246 → apply đúng slot.
- player+0x9a30 = m_bHideHat. 2 mâu thuẫn RE cũ (INFO memory-order; DB expire@3 unaligned) giải bằng decompile.
- unApply dùng 2010-native GetEquipPos(reprIdx), không transcribe v246 raw (enum eit drift).

## Deferred (có lý do, không chặn):
- Buy chain client-purchase: coin-path version-gapped (KDiamond vắng 2010) + protocol-coupled + unreachable (client 2010 không có shop UI). Grant-path LuaAddExterior phủ "cấp ngoại trang".
- Query bindings Luna<KExterior>; latest-buy populate; render (cần client v246).
- Live Lua test: docs/exterior_port/test_exterior.lua (cần cluster + login).

## Isolation: file mới KExterior*.{h,cpp}; sửa (backup backup_source_20260708_104341/): KWorldSettings, KPlayer, KRoleDBDataDef.h, GS_Client_Protocol.h, KPlayerServer, KLuaPlayer.cpp. KHÔNG xoá. Fix source + rebuild (không patch exe). Chưa commit (chờ lệnh).
