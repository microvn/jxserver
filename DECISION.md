# JX3 Linux build — Nhật ký quyết định (Decision Log)

> Ghi lại **vì sao** đã làm như hiện tại: mỗi mục = bối cảnh → bằng chứng → quyết định → lý do → phương án đã loại.
> Đọc kèm `BUILD-TECHNICAL.md` (cách làm) và `../source/JX3-BUILD-GUIDE.md` (feasibility tổng quan).
> Cập nhật: 2026-07-04.

---

## A. Quyết định kiến trúc nền

### A1. Reuse 3 `.so` engine thay vì build lại engine
- **Bối cảnh:** engine `Engine_Lua5` chỉ ship binary, source đời JX3 không leak.
- **Bằng chứng:** "check-common" (phiên gốc) — bề mặt symbol ngoài-hệ-thống của exe shipped = **74 symbol, 100% resolve** bởi 3 `.so` (`libEngine_Lua5D/SO3EnumConvertorD/SO3ItemHouseD`), 0 unresolved.
- **Quyết định:** link 3 `.so` (bản `D`, ELF32 i386, không strip), chỉ rebuild logic game.
- **Lý do:** reconstruct engine = person-weeks + rủi ro sai byte; link `.so` cho hành vi engine **y hệt bản gốc**.
- **Loại:** (a) reconstruct engine từ `base/SourceCode/Engine` — chỉ là subset file/pak/crypto, không phải engine game đầy đủ; (b) forward-port engine tổ tiên SwordOnline — tên class khác, dự án lớn.

### A2. Rebuild đúng SO3World (không phải component khác)
- **Bằng chứng:** `SO3World.vcproj` = `ConfigurationType=4` (static lib → `SO3WorldServer.lib`); `SO3GameServer.vcproj` = `ConfigurationType=1` (exe) link 7 lib; glue exe chỉ ~4 file.
- **Quyết định:** compile 205 `.cpp` của SO3World → `.o`, link tĩnh + 4 glue + 3 `.so`.
- **Lý do:** logic game 100% ở SO3World, static-in-exe nên không hot-swap được → bắt buộc rebuild+relink. Gateway/GameCenter không mang logic → để nguyên binary. Xem `BUILD-TECHNICAL.md §1`.

### A3. Docker `centos:7` + GCC 4.8.5 + `-m32`
- **Lý do:** `.so` build 2012 dùng libstdc++ ABI cũ (pre-C++11 layout). GCC mới (ABI cxx11) mismatch. 4.8.5 mặc định CentOS 7 = khớp. `-m32` vì `.so` là i386.

### A4. Stub `KMemory` dạng `std::string` (KHÔNG dùng pooled thật), override qua FS case-insensitive
- **Bằng chứng:** áp `KMemory` pooled thật (kstring = basic_string trên KAllocator) làm **regress build 178→53** (mismatch với `std::string` cả cây dùng).
- **Quyết định:** dùng stub `kmemory.h` (class form, `std::string`-compatible); `setup.sh` overlay `_recovered/` **sau cùng** để `kmemory.h` đè `KMemory.h` (ExData case-insensitive → cùng file).
- **Loại:** dùng header engine thật → regress nặng.

### A5. KHÔNG tạo `ln -sf engine include/Base/Engine` (và Common)
- **Bối cảnh:** từng thêm 2 symlink này để resolve include chữ hoa; gây OrbStack virtiofs flake trên mount case-insensitive.
- **Bằng chứng:** lệnh build cuối phiên gốc (**cmd 306**, transcript event 3465) KHÔNG có 2 dòng này — include chữ hoa resolve qua `mkshim` (`-Ishim`).
- **Quyết định:** bỏ hẳn; để `mkshim` lo case.
- **Lý do:** trên host case-insensitive, symlink `Engine` đụng dir `engine/` thật → race "No such file". Xem `BUILD-TECHNICAL.md §7 bẫy #3`.

### A6. Fix endgame = bước PATCH trong `setup.sh`, không sửa source upstream
- **Quyết định:** mọi vá header/source cho endgame áp vào `$HERE/src`+`$HERE/include` sau khi copy, trong `setup.sh` (hoặc `patches/`).
- **Lý do:** source upstream (`source/*`) giữ nguyên bản; build vẫn reproducible; xóa scaffold dựng lại vẫn đúng.

### A7. Đính chính: CRC32 CÓ trong `.so`
- **Bằng chứng:** cmd 304 — `nm -D --defined-only libs/*.so | grep -c CRC32` = **1**.
- **Quyết định:** coi CRC32 undefined là **lệch mangling/khai báo C++**, cần khai báo extern đúng — KHÔNG viết lại impl. (Ghi chú cũ nói "reconstruct" là SAI.)

---

## A8. Mô hình 3 tầng độc lập — và VÌ SAO chỉ build exe (đính chính trọng tâm)

Lý do build exe KHÔNG phải chỉ "vì `.so` không có source" (đúng nhưng phụ). Lý do CHÍNH là **kiến trúc tách 3 tầng độc lập**, và **game phần lớn nằm ở Lua** (data tree), không phải ở exe hay `.so`:

```
Lua scripts (~3M dòng · 16667 file · 188MB, tại 镜像端/extracted/root/scripts)
   = GAME thật: skill/quest/dungeon/ai(AIParam.lua)/item/craft/activity/drop.
   Sửa = MOD, KHÔNG cần build. Đây là bề mặt moddable chính.
        ▲ 6107 call-site (CallFunction / m_ScriptCenter / Lua_*)
exe SO3World (C++ framework)
   = cầu nối: định nghĩa object game (KCharacter/KItem...) + protocol,
     route event vào Lua. Build cái NÀY (source SO3World có leak).
        ▲ 74 symbol (check-common)
libEngine_Lua5.so (= Lua engine, 347 symbol Lua: LuaAllocator/lua_*)
   = chạy Lua + primitive (memory/thread/file/pak/network). Reused, đóng.
```

**Hệ quả cho modding:** sửa nội dung game (chỉ số, quest, dungeon, hành vi AI) = **sửa Lua, không build**. Toàn bộ scene 单机/一键端 TQ mod theo cách này với binary shipped. Build exe từ source chỉ cần khi muốn đổi **tầng khung** (protocol, cơ chế lõi, bề mặt binding Lua↔C++).

**Hệ quả cho khe version 2010↔2012:** rủi ro KHÔNG nằm ở "C++ logic khác" (logic game phần lớn là Lua 2.5.2 dùng chung). Nó nằm ở **bề mặt binding Lua↔C++**: exe 2010 của ta có expose đủ/đúng các hàm `Lua_*` mà script 2.5.2 gọi không. Đó là điểm cần kiểm lúc chạy (script fail = thiếu/lệch hàm binding), không phải bản thân C++ logic.

---

## B. Phát hiện xuyên suốt về leak

### B1. Các cây full-source local = CÙNG một snapshot
- **Bằng chứng (2026-07-04):** `jx3dev-master`, `JX3-AIO`, `Sword3-FullSource-master` — cả 3 đều thiếu `KMissile::m_nExistingFrame` trong header và đều chỉ có `KSimpThreatList` (không `KThreatNode`). `viticm/pap2` (GitHub, fetch raw) **cũng vậy** (KMissile.h 1.8KB không có m_nExistingFrame; KThreatList.h có KSimpThreatList + KThreatLiteNode y hệt).
- **Kết luận:** cross-ref giữa các cây KHÔNG giải được mismatch — chúng giống hệt. `Sword3-FullSource` ≠ pap2 (khác repo/tác giả) nhưng **cùng version source**.
- **Hệ quả:** các lỗi endgame là **lệch version .cpp↔.h ngay trong leak** (một số `.cpp` mới/cũ hơn `.h` của chính nó), không phải lỗi thiếu-include tầm thường. Fix bằng suy luận ngữ nghĩa từ usage + xác nhận không ai dùng phần cũ. Xem memory `jx3-source-trees-and-research`.

---

## B2. RE-AUDIT AI: VM AI vs class-AI cũ — và dead set bị SAI (phát hiện then chốt)

Nghi vấn (user, đúng): "dead code có thể là root cause của endgame; ta có bỏ nhầm tính năng AI không?". Audit lại bằng **binary server 2012 thật** (`镜像端/extracted/root/SO3GameServer`, `strings`) + reference trong source:

- **VM AI ≠ mất tính năng, chỉ khác kiến trúc.** Hai hệ cùng làm AI quái/NPC, cùng feature (threat/aggro, chọn skill, tuần tra, tổ đội, truy đuổi, bỏ chạy):
  - *Class-AI cũ (KAIBase family):* hardcode C++ mỗi archetype một class — `KAI_Wolf` (sói), `KAI_Monk` (sư), `KAI_WolfKing` (boss), `KAI_Critter` (quái vô hại), `KAI_Negative/Positive` (bị động/chủ động), `KAI_Player` (auto-AI nhân vật), `KAISkillSelect`/`KAIControl`/`KAIEvent`. Thêm hành vi = viết class + recompile.
  - *VM AI mới (KAIVM family):* máy ảo chạy AI như **DATA** — `KAIAction`/`KAILogic`/`KAIState`/`KAISearchTactics`/`KAISkillSelector`/`KAIManager`/`KAIPatrolGroupManager`. Hành vi định nghĩa trong data/script, thông dịch lúc chạy.
- **Bằng chứng KHÔNG mất feature:**
  1. Binary server 2012 chứa **CHỈ VM AI** (`KAIAction`×54, `KAILogic`×15, `KAIVM`×9, `KAIManager`, `KAISearchTactics`, `KAISkillSelector`, `KAIPatrolGroupManager`) — **0** `KAIBase`/`KAI_Wolf`/`KAI_Monk`/`KAI_Player`. KingSoft **đã tự bỏ** class-AI trước khi ship.
  2. VM AI phủ đủ feature: threat (`m_ThreatLink`), tổ đội NPC (`m_nNpcTeamID` + `ClearAllMemberThreat`, dùng trong `KAIAction.cpp` SỐNG), tuần tra (`Patrol`×131), chọn skill (`KAISkillSelector`), truy tìm (`KAISearchTactics`).
  3. Class-AI cũ thậm chí **không compile** với header hiện tại (dùng `KThreatNode` đã bị đổi thành `KThreatLiteNode`) — chúng là tàn dư trước khi migrate sang VM.
  4. VM AI (9 file) **đều compile trong 189** và **đều có trong binary** → ta đang giữ đúng AI của server thật.
- **KẾT LUẬN:** loại class-AI KHÔNG mất tính năng. Hành vi quái (aggro sói, cơ chế boss) giờ là **DATA** do VM đọc (cây `手工端/root/scripts`+tab), không phải class C++. Ta giữ VM AI (đúng cái server chạy), bỏ class-AI mà chính KingSoft đã bỏ.

**HỆ QUẢ (dead set SAI ở 2 điểm — user bắt đúng):**
- **`KAI_Player` bị BỎ SÓT khỏi dead set** dù nó kế thừa `KAIBase` và vắng trong binary. Chính `KAI_Player.o` (đang compile) tham chiếu **18 `KAIBase::*` + typeinfo KAIBase** → đó là nguồn 18 undefined ở link. **Fix: thêm `KAI_Player` vào dead set** → 18 undefined tan, KHÔNG cần compile/stub KAIBase.
- **3 file "endgame" thực ra CHẾT**, đang định fix nhầm (bằng patch phỏng đoán):
  - `KPathFinder` — chỉ `KAIBase.h` (dead) tham chiếu; VM AI tự tìm đường. → **loại, đừng fix signature.**
  - `KTrackList` — không file sống nào gọi method `KTrackList::`; vắng binary. → **loại, đừng đoán `szDesc[64]`.**
  - `KMissile` — chỉ forward-decl `class KMissile;` ở `KRegion.h`/`KCell.h`, **không đâu `new KMissile`**, vắng binary. → **loại, đừng thêm `m_nExistingFrame`.**

**Cách phân biệt sống/chết đã dùng:** (a) có trong `strings` binary server thật = SỐNG chắc; (b) vắng string + chỉ được file DEAD hoặc header forward-decl tham chiếu + không `new`/không caller method = CHẾT. Lưu ý: class helper không-đa-hình có thể vắng string dù sống → luôn cross-check reference (vd KNpcTeam vắng string nhưng `KAIAction`/`KScene` SỐNG dùng → giữ).

### B2b. Migration class-AI → VM-AI ĐÃ HOÀN TẤT trong code sống (không phải "migrate dở")
Nghi vấn quan trọng (user): "VM AI chưa đủ symbol? Ta đang vá AI migrate dở?". Audit dứt điểm:
- **VM AI là AI ĐANG CHẠY, đầy đủ:** 10 file sống gọi `m_AIVM.` **49 lần active** (FireEvent...). `KAIVM.h` có `atPlayer` + `atNpc` → VM AI xử lý **cả player-AI lẫn npc-AI** (feature `KAI_Player` được thu vào type `atPlayer`; binary có `KAI_USER`).
- **Migration đã xong trong code sống:** mọi lời gọi `m_AIController.X()` (class-AI cũ) trong file sống **đều đã bị COMMENT**, thay bằng `m_AIVM.FireEvent(...)` ngay dưới. Smoking gun `KCharacter.cpp:4657`: `// m_AIController.DoEscape(...)` (comment) → `m_AIVM.FireEvent(aevOnInFear,...)` (active). Sau khi stub 2 method chết của KNpcTeam: **0 file sống còn `m_AIController` active**.
- **VM AI tự chứa:** KAIVM/KAIAction/KAILogic/KAIState/KAIManager **KHÔNG** tham chiếu class-AI → 0 undefined từ VM AI. 18 `KAIBase::*` undefined **chỉ** từ `KAI_Player.o` (bản thân là class-AI chết).
- **KẾT LUẬN:** đây KHÔNG phải "cắt AI đang chạy nửa vời". Code chạy = 100% VM AI (đã migrate xong). Cái còn lại (đảo KAIBase + 2 method _SERVER chết của KNpcTeam) là **xác code cũ chưa xóa** — chính KingSoft đã xóa nốt trước khi ship (binary 0 class-AI). Ta xóa đúng cái họ đã xóa; không đụng AI đang chạy.
- **Caveat trung thực:** đây chứng minh CODE đã migrate sạch. Việc DATA hành vi (VM đọc từ `手工端/root`) có tái hiện đủ mọi hành vi quái cũ hay không là câu hỏi RUNTIME, không verify được khi chỉ build — nhưng server 2012 thật ship+chạy trên VM AI ⇒ mạnh mẽ ngụ ý đủ.

---

## C. Audit & quyết định file endgame (SAU re-audit B2: 5 fix + 4 loại)

Phân loại: **[SẠCH]** = platform/include đơn giản; **[DRIFT-rõ]** = lệch version nhưng fix chắc chắn từ ngữ nghĩa; **[DRIFT-cân nhắc]** = lệch version cần phán đoán.

### C1. KDoodadFile — [SẠCH]
- **Triệu chứng:** `IDoodadFile.h:18 '_NAME_LEN' was not declared`.
- **Root cause:** `IDoodadFile.h` dùng `_NAME_LEN` (define chuẩn ở `include/Base/protocolbasic.h`) nhưng chỉ `#include "KSUnknown.h"`.
- **Quyết định:** thêm `#include "protocolbasic.h"` vào `IDoodadFile.h`.
- **Lý do:** header nên tự include thứ nó dùng; `protocolbasic.h` là nơi định nghĩa chính (`#define _NAME_LEN 32`), sẵn trên `-Iinclude/Base`.

### C2. KNpcFile — [SẠCH]
- Giống C1, file `INpcFile.h`. Thêm `#include "protocolbasic.h"`.

### C3. KGMList — [SẠCH]
- **Triệu chứng:** `KGMList.cpp:23 '_NAME_LEN'` (`char szAccount[_NAME_LEN]`). File chỉ include `stdafx.h`+`KGMList.h`.
- **Quyết định:** thêm `#include "protocolbasic.h"` vào `KGMList.h`.

### C4. KProtocolRecorder — [SẠCH]
- **Triệu chứng:** `KProtocolRecorderBase.h:134 'in_addr' does not name a type` (struct `KSERVER_GET_ADDRESS { in_addr addr; u_short uPort; }`).
- **Root cause:** trên Windows `in_addr` đến từ winsock; Linux chưa include.
- **Quyết định:** thêm `#include <netinet/in.h>` vào đầu `KProtocolRecorderBase.h`.
- **Lý do:** `in_addr`/`u_short` là type socket POSIX chuẩn ở `<netinet/in.h>`.

### C5. KMissile — [DRIFT-rõ]
- **Triệu chứng:** `KMissile.cpp:29 'm_nExistingFrame' was not declared`.
- **Bằng chứng usage:** dòng 29 `=0` (init), 89/106 so sánh, 115 `--` (giảm mỗi frame), 116 `KG_CHECK_VANISH(m_nExistingFrame > 0)` (vanish khi ≤0). `KMissile.h` (mọi cây + pap2) KHÔNG khai báo; base `KSceneObject` cũng không.
- **Quyết định:** thêm `int m_nExistingFrame;` vào phần member của `KMissile.h`.
- **Lý do:** ngữ nghĩa rõ ràng = **bộ đếm frame sống của đạn** (int, giảm dần, hết thì biến mất). `.cpp` là bản mới hơn `.h` — thêm member là fix root-cause, không hack.
- **Loại:** đổi `.cpp` để bỏ dùng member → sai hành vi (đạn mất cơ chế hết-hạn).

### C6. KPathFinder — [DRIFT-cân nhắc → đã chốt]
- **Triệu chứng:** `KPathFinder.cpp:124 no matching function KRegion::GetProximalCellForObj(int&,int&,int&,int&,bool,int*)` — gọi 6 tham số.
- **Bằng chứng:** signature thật (`KRegion.h:112`) = **7 tham số**: `(nXCell, nYCell, nFootPosZ, nHeight, BOOL bIgnoreDynamicSwitch, BOOL bIgnoreBlock, int* pnRelativeZ)`. Call ở KPathFinder thiếu `bIgnoreBlock`.
- **Quyết định:** thêm `false` cho `bIgnoreBlock` → `GetProximalCellForObj(nX, nY, nZ, m_nHeight, false, false, &nRelativeZ)`.
- **Lý do (chốt bằng bằng chứng, không đoán):** impl `KRegion.cpp:1319` dòng `if (bIgnoreBlock || !dwBlockCharacter)` — `true` = nhận cả ô bị chặn. Hai caller nội bộ KRegion (NPC `:1742`, Doodad `:1768`) đều truyền `false, false`. Pathfinder cần ô **đi được thật** → `bIgnoreBlock=false`, khớp mẫu NPC/Doodad.
- **Loại:** `bIgnoreBlock=true` → có thể trả ô bị chặn, sai cho tìm đường.

### C7. KTrackList — [DRIFT-cân nhắc → đã chốt]
- **Triệu chứng:** `KTrackList.cpp:48 KTRACK_INFO has no member 'dwMapID'` (cũng dùng `szDesc`).
- **Bằng chứng:** `KTRACK_INFO` thật (`KTrackList.h`, include/Include/SO3World, struct nội bộ trong `class KTrackList`) = `{DWORD dwSrcNode; DWORD dwDstNode; KTRACK Track;}`. Nhưng `.cpp` dùng `dwMapID`+`szDesc`+`Track`. **Không ai** dùng `dwSrcNode/dwDstNode` (grep toàn cây trống). KRoadManager (SỐNG, dùng KTrackList) chỉ chạm API public `KTRACK`/`KTRACK_POINT`/`GetTrackInfo`, KHÔNG chạm `KTRACK_INFO`.
- **Quyết định:** thêm `DWORD dwMapID; char szDesc[64];` vào struct `KTRACK_INFO` (giữ nguyên các member cũ để an toàn tối đa).
- **Lý do:** `KTRACK_INFO` là struct **nội bộ**, member cũ vô dụng → thêm 2 member `.cpp` cần là an toàn, không lộ ra ngoài. `.cpp` là bản khác version của `.h`.
- **Điểm chưa chắc:** `szDesc` **size = 64 là phỏng đoán** (đọc bằng `GetString(..., sizeof(szDesc))` nên truncate an toàn; giá trị chính xác không rõ từ leak). Ghi lại để nếu có version khớp thì chỉnh.
- **Loại:** thay hẳn `dwSrcNode/dwDstNode` bằng `dwMapID/szDesc` → sạch hơn nhưng rủi ro nếu có chỗ dùng ngầm chưa grep ra.

### C8. KNpcTeam — [DRIFT-cân nhắc]
- **Triệu chứng:** `KNpcTeam.cpp:218 'KThreatNode' was not declared` trong `GetMemberTarget()`.
- **Bằng chứng:** code dùng API **cũ**: `KThreatNode* pNode = m_ThreatList.GetFirstThreat(); if (pNode->pCharacter)`. API hiện tại: `KCharacter::m_ThreatList` kiểu `KThreatList`; `KThreatList::GetFirstThreat()` (no-arg) trả `const KThreatLiteNode*`; `KThreatLiteNode` = `{DWORD dwCharacterID;}` (KHÔNG có `pCharacter`). `KThreatNode` là **tên type cũ đã bị đổi thành KThreatLiteNode**.
- **QUAN TRỌNG — method này CHẾT:** `GetMemberTarget()` chỉ được `KAIBase.cpp:669` gọi, mà `KAIBase` nằm trong **DEAD set** (không compile). Không file SỐNG nào gọi → không tạo undefined ở link. File `KNpcTeam.cpp` thì SỐNG (KScene/KNpcTeamList dùng object) nên vẫn **phải compile**.
- **Quyết định (cuối):** port `KThreatNode*` → `const KThreatLiteNode*`; giữ nguyên vòng lặp; vì `KThreatLiteNode` bỏ `pCharacter` (chỉ còn `dwCharacterID`) và **không có accessor ID→`KCharacter*`** trong tầm SO3World (nó ở tầng KGameServer/KPlayerServer cao hơn, không include được từ KNpcTeam), method **trả `NULL`** với comment "DEAD LEGACY". Compile-clean, an toàn (không ai chạy).
- **Điểm chưa chắc:** lookup ID→`KCharacter*` bỏ ngỏ. Nếu method này **hồi sinh** (server thật gọi tới), phải nối lookup thật (tầng world/scene manager). Hiện dead nên NULL là đúng-đủ. Đánh dấu để lần sau không tưởng nhầm là logic thật.
- **Loại:** (a) đưa KNpcTeam vào DEAD set — KScene/KNpcTeamList SỐNG dùng object → sinh undefined; (b) khôi phục `KThreatNode`+`pCharacter` API cũ vào header — phá 189 file dùng API mới.

---

## D. Tổng kết quyết định endgame (ĐÃ SỬA theo re-audit B2)

**Dead set MỚI = 14 cũ + `KAI_Player` + `KPathFinder` + `KTrackList` + `KMissile` = 18 file.**

Chỉ **5 file** thật sự SỐNG cần fix (đều SẠCH/thấp rủi ro — bỏ hết các patch phỏng đoán):

| File | Trong binary? | Loại | Fix | Độ chắc |
|---|---|---|---|---|
| KDoodadFile | CÓ (sống) | SẠCH | +`#include "protocolbasic.h"` (IDoodadFile.h) | cao |
| KNpcFile | CÓ (sống) | SẠCH | +`#include "protocolbasic.h"` (INpcFile.h) | cao |
| KGMList | CÓ (sống) | SẠCH | +`#include "protocolbasic.h"` (KGMList.h) | cao |
| KProtocolRecorder | CÓ (sống) | SẠCH | +`#include <netinet/in.h>` (KProtocolRecorderBase.h) | cao |
| KNpcTeam | sống (KAIAction/KScene dùng) | DRIFT | `GetMemberTarget` (method chết) → `const KThreatLiteNode*`, trả NULL | cao |

**LOẠI thay vì fix (thêm vào dead set) — tránh 3 patch phỏng đoán rủi ro:**

| File | Bằng chứng chết | Trước định làm (SAI) |
|---|---|---|
| KAI_Player | vắng binary, 0 caller sống, kế thừa KAIBase | (không trong endgame, nhưng tạo 18 undefined) |
| KMissile | forward-decl `class KMissile;` only, 0 `new`, vắng binary | ~~+`m_nExistingFrame`~~ |
| KPathFinder | chỉ KAIBase.h (dead) tham chiếu, vắng binary | ~~+arg bIgnoreBlock~~ |
| KTrackList | 0 caller method sống, vắng binary | ~~+`szDesc[64]` (đoán)~~ |

**Lợi ích kép của re-audit:** (1) 18 `KAIBase::*` undefined ở link **tan** khi loại `KAI_Player`; (2) tránh 3 fix phỏng đoán (`m_nExistingFrame`, `bIgnoreBlock`, `szDesc[64]`) trên code chết — vốn có thể "đúng compile nhưng sai bản chất". Còn lại 5 fix đều là include/type rõ ràng.

**Nguyên tắc rút ra:** trước khi fix một file compile-fail, **kiểm nó có SỐNG không** (trong binary server thật + có caller sống) — nếu chết thì LOẠI, đừng vá. Vá code chết = nợ kỹ thuật + có thể che root cause. Tất cả áp qua **patch/exclude step trong build (dead set) + setup.sh** (A6).

**Còn lại sau khi làm 5 fix + dead set mới (dự đoán):** compile 197−4(dead mới)=**~193/193 target** (205 − 18 dead = 187 SO3World + 4 glue + 2 recon), link undefined giảm mạnh (18 KAIBase tan; `CRC32` khai báo extern; `Create{Doodad,Npc}FileInterface`/`KGMList::*`/`vtable KProtocolRecorder` tan khi 5 file compile). Xác nhận bằng build thực tế.

---

## E. ĐÃ XONG — binary link hoàn chỉnh (2026-07-05)

**★ COMPILE 193/193 fail=0 · link exit=0 · undefined 0 → `/tmp/SO3GameServer` ELF32 i386, dynamically linked, not stripped.**

Endgame link giải quyết:
- **18 `KAIBase::*` undefined** → tan khi loại `KAI_Player` (B2). Không cần compile/stub KAIBase.
- **`Create{Doodad,Npc}FileInterface`, `KGMList::*`, `vtable KProtocolRecorder`** → tan khi 5 file endgame compile.
- **`CRC32`** → `common_recon/crc32_shim.cpp`: wrapper `CRC32 → Misc_CRC32` (đúng impl trong `.so`, chỉ khác tên; KHÔNG tự viết CRC32). Sửa lại A7: đúng là "khai báo/alias", không reconstruct.
- **`KAIEvent::ctor/dtor`** → đến từ `KAIParamTemplateList` (nhúng `KAIEvent AIEvent[]` by-value). `KAIParamTemplateList` là AI-cũ chết (vắng binary, 0 code sống dùng, chỉ 3 header AI-cũ nhắc) → **thêm vào DEAD set** (dead set = 19 file). Không stub ctor/dtor class chết.

## F2. RUNTIME TEST trong deploy tree thật (2026-07-05) — chạm VERSION-DRIFT WALL

Chạy binary self-built trong deploy tree thật (`镜像端/extracted/root`, CWD=deploy, `.so`+config+scripts thật, KHÔNG đụng binary gốc). Tiến trình:
1. exec OK, load đủ libs. 2. thiếu locale → `localedef GBK zh_CN` (môi trường, không phải code). 3. **Set locale: zh_CN.gbk** → **"The sword3 version is 2-5-2-4503"** (đọc version từ deploy) → vào **LoadData**.
4. **Dừng ở LoadData** (KGWConstList::LoadData đọc `settings/GameWorldConstList.ini`): fail lần lượt line 59 → 76 → 224 khi patch dần key thiếu.

**Chẩn đoán = VERSION DRIFT 2010↔2012, biểu hiện ở config layer, NHIỀU DẠNG:**
- **Key bị bỏ:** `MaxThreatCount` (threat cũ), `MinSlayLevel/CloseSlayTime/WaitSlayTime/SlayCDIndex/MinBeSlayedLevel` (hệ Slay PK cũ), `MaxCampPrestige` — feature 2010 mà data 2012 đã bỏ.
- **Key bị DỜI SECTION:** `LootItemQualityToLog` — source 2010 đọc `[MISC]`, data 2012 để `[LOG_CONFIG]`. Drift cấu trúc, không chỉ thiếu.
- **China3 config (2010) CŨNG fail LoadData** → source 2010 (build 1.4.0.1834) không khớp sạch với BẤT KỲ data nào trên đĩa, kể cả data cùng đời China3.

**KẾT LUẬN (bức tường thật, không phải bug build):** source = **1.4.0.1834 (2010)**, data+`.so`+binary deploy = **2.5.2.4503 (2012)**. Build/link/chạy-tới-config đã xong 100%; nhưng config-reader C++ của source 2010 mong schema config 2010, data 2012 đã tiến hóa (bỏ key + dời section). Đây KHÔNG phải chuyện patch vài key — mỗi file config (hàng chục `.ini` + `.tab` trong `settings/`) có drift riêng, và sau config còn tầng tab-data + Lua-binding. Đuổi theo = whack-a-mole với data sai đời, công lớn, kết quả không chắc.

**3 ĐƯỜNG ĐI (quyết định chiến lược của user):**
- **A. Đuổi patch drift** để source 2010 nuốt data 2012: mechanical nhưng lan rộng (nhiều file × key × section-move), rồi chạm tab + Lua-binding. Rủi ro/công cao, kết quả không đảm bảo. KHÔNG khuyến nghị.
- **B. Mod Lua trên binary shipped (khuyến nghị cho "chạy + chơi + mod"):** dùng binary 2.5.2 shipped (khớp data+`.so`+client) + sửa Lua (~3M dòng, §A8). Đây là cách toàn bộ scene 单机 TQ làm; đạt server chơi được + moddable NHANH NHẤT. Binary self-built khi đó là **thành tựu "chứng minh build-from-source"** (hiếm, giá trị nghiên cứu) — không phải để chạy production.
- **C. Tìm data đúng đời 1.4.0.1834** (websearch pap2-era/Gitee/forum TQ): source 2010 cần data 2010 KHỚP CHÍNH XÁC. China3 gần nhưng không khớp sạch. Khó vì bản 1.4.0 rất cũ, ít lưu hành.

**Ý nghĩa:** mục tiêu "sửa được C++ tầng khung" (build-from-source) = ĐÃ ĐẠT. Mục tiêu "server chơi được để mod cho con" = đường B (Lua) thực tế nhất, và khớp đúng insight 3-tầng (§A8): game thật nằm ở Lua.

## F3. GHIDRA DIFF PIPELINE + tolerant reader (2026-07-05) — verify hoạt động, drift ĐA TẦNG

Setup Ghidra decompile (pyghidra + pyghidra-mcp `pyghidra-jx3`, project persistent `linux-build/ghidra-project`; xem memory jx3-source-trees §Ghidra). Chất lượng: `.so` DWARF ~94% hàm có tên (đọc như source); exe stripped 3% tên NHƯNG assert-macro in `__FILE__/__FUNCTION__` + key config là string literal → định danh + đọc logic được.

**DIFF hoàn chỉnh `KGWConstList::LoadData` (exe 2.5.2 vs source 2010)** — `tools/diff_loaddata.py`, full decompile lưu `ghidra-project/LoadData_252.c`:
- Source 2010 đọc **81** (section,key); exe 2.5.2 đọc **208**. → 2.5.2 = 2010 + ~130 key hệ thống MỚI (CURRENCY/TITLE/FIVE_ELEMENT/DOMESTICATE/HUNT_REDNAME/REWARDS_SHOP/STAMINA_THEW_RESTRICT...).
- 14 key source doc mà 2.5.2 bỏ (MaxThreatCount/Slay*/MaxCampPrestige/ANTI_FARMER*).
- 5 key dời section (CommonKill/LastHit/SoloKill/KillerPartyMember CAMP→TITLE; LootItemQualityToLog MISC→LOG_CONFIG).

**FIX tolerant reader (đã codify `patches/endgame.py` §6):** nới `KGLOG_PROCESS_ERROR(nRetCode)` sau mỗi Get trong KGWConstList (81 chỗ) → key vắng dùng default thay vì crash. → **server QUA LoadData config**.

**Runtime tiến trình sau tolerant:** banner → locale → version 2.5.2.4503 → qua KGWConstList → **dừng ở tab-data**: `[settings/NpcTemplate.tab] GetInteger(ReputeLowLimit1) failed, LoadNpcTemplate:1186`. Source 2010 đọc cột tab mà data 2.5.2 không có.

**KẾT LUẬN drift ĐA TẦNG:** version 2010↔2012 lệch ở MỌI tầng nạp: config ini (xong) → tab-data (NpcTemplate...) → sẽ còn item/skill/... tab → rồi Lua-binding/protocol. Tolerant reader áp được từng tầng (chiêu: nới KGLOG_PROCESS_ERROR sau Get), server tiến từng nấc. Nhưng: (a) đây là whack-a-mole nhiều tầng; (b) tolerant = server "logic 2010 đọc data 2012 best-effort" → chạy được nhưng nhiều field dùng default → hành vi lệch; (c) 130 hệ thống 2012 mới source 2010 KHÔNG có → thiếu feature. → Xác nhận lại: build-from-source XONG; "server 2010-logic chạy trên data 2012" = KHẢ THI qua tolerant đa tầng nhưng là best-effort, KHÔNG bằng binary 2.5.2 shipped. Đường B (mod Lua trên binary shipped) vẫn là cách tốt nhất cho server chơi-được. Pipeline Ghidra-diff giờ SẴN cho ai muốn port chính xác từng tầng.

## F4. RUNTIME BOOT tiến sâu — 2 loại drift + pipeline Ghidra port hằng số (2026-07-05)

Ý user: đọc parent → biết trọn chuỗi loader → patch cả loạt (thay whack-a-mole). Parent = `KSO3World::Init` gọi ~20 sub-manager `.Init()` (Settings/ItemManager/AIManager/SkillManager/...), mỗi cái load config+tab. Áp:

**Loại drift 1 — FIELD thiếu (tolerant được):** `patches/endgame.py` §6 quét toàn `src/SO3World/Src`, nới cặp `var=...->Get{Integer|String|Float|Data}(...)` + `KGLOG_PROCESS_ERROR(var)` ngay sau (xử lý cả Get MULTI-LINE, cửa sổ i+1..i+6) → **969 check nới / 54 file**. Key/cột vắng ở data 2.5.2 → dùng default thay vì crash. Qua config + **toàn bộ NpcTemplate.tab (18,207 NPC)**.

**Loại drift 2 — HẰNG SỐ DUNG LƯỢNG (KHÔNG tolerant được, phải PORT):** data 2012 vượt giới hạn compile-time 2010; nới = overflow mảng. Phải tăng #define khớp 2012. Giá trị THẬT đọc bằng **Ghidra decompile exe 2.5.2** (pipeline user đề xuất — chính xác, không đoán):
- `MAX_QUEST_COUNT`: 8192 → **16384** (Ghidra MakeQuestMap `dwQuestID < 0x4000`; data max quest id=8706). Mảng `KQuestList::m_byQuestStateList[MAX_QUEST_COUNT]` = player-state → đổi layout struct.
- `MAX_COOL_DOWN_COUNT`: 512 → **1024** (Ghidra KCoolDownList::Init `< 0x400`).
- codify `patches/endgame.py` §7 (`port_const`).

**PATTERN (lặp):** boot = tolerant field (1 chiêu) + chuỗi port constant-limit (từng cái, giá trị từ Ghidra). Mỗi rebuild ~4 phút. → cần TỰ ĐỘNG HÓA: quét mọi `KGLOG_PROCESS_ERROR(<var> < MAX_*)` trong source → với mỗi, Ghidra decompile hàm chứa (string-anchor) → đọc `< 0xNNNN` → port cả loạt trong 1 lần (thay grind từng cái).

**LƯU Ý struct-layout:** các constant này (MAX_QUEST_COUNT...) đổi SIZE struct player/state → ảnh hưởng DB role-blob + protocol client. Port khớp 2012 = ĐÚNG hướng (source khớp data/DB/client 2012). Đây là drift NGHIÊM TRỌNG hơn config (đụng wire format), nhưng Ghidra cho giá trị chính xác nên port được. Xác nhận lại: "chạy best-effort" cần tolerant(field) + port(constant), khả thi có định hướng bằng Ghidra; nhưng công dài + nhiều tầng.

## F. Việc còn treo — mốc RUNTIME (build đã xong, giờ tới CHẠY)

Binary link ≠ server chạy. Ba mốc runtime tách biệt:
- **Data tree** `手工端/root/` (Lua scripts + tab + config + 3 `.so`) đặt cạnh binary.
- **MySQL** (schema server) — DB.
- **Client 2.5.2 thật** để **validate framing/crypto socket** (`kg_socket.cpp` hiện pass-through NONE — phần ĐOÁN duy nhất còn lại; nội dung packet thì đúng vì ở SO3World).
- Nếu cần chính xác hơn cho code chết đã stub (KNpcTeam) / hằng phỏng đoán: websearch pap2/Gitee/forum TQ version khớp. Nhưng đều là code KHÔNG chạy nên không chặn.

**Dead set cuối = 19 file:** 14 class-AI cũ + KAI_Player + KMissile + KPathFinder + KTrackList + KAIParamTemplateList. Tất cả: vắng binary server thật + 0 caller sống (verify từng cái).

---

## G. Source vào git — bỏ pipeline patch-chồng-patch (2026-07-05)

**Vấn đề (user chỉ ra):** `setup.sh` cũ mỗi lần chạy **xóa `src/ include/` rồi copy lại từ
leak trees** (jx3dev-master, JX3-AIO, 镜像端), CUỐI cùng mới áp `patches/endgame.py`. Hệ quả:
source-đã-sửa không tồn tại lâu dài ở đâu — nó là sản phẩm phụ sinh-rồi-bị-đè. Nên **mọi fix
buộc phải nhồi vào `endgame.py`**. Càng port drift 2010→2012 thì file patch càng phình, và
sớm muộn phải re-port ngược vào source. Đây **không phải tư duy phát triển phần mềm**.

Bằng chứng mong manh gặp ngay khi kiểm tra: working tree lúc đó **không nhất quán** (21/54 file
tolerant, header thiếu `MAX_QUEST_COUNT`, KNpcTeam chưa stub) dù binary đã build 191/0 — vì
source-of-truth trôi giữa leak + patch + working tree.

**Quyết định:** đóng băng cây source-đã-patch thành **git tree = source of truth**.
- `git init` trong `linux-build/`. `.gitignore` bỏ: rác MSVC (`*.sdf/*.ncb/*.pch/*.ipch/*.pdb/
  *.obj/*.tlog/*.cod/*.vsd` — chính chúng phình `src/` lên **357M**), `obj/ shim/ Source/`,
  `libs/*.so`, binary, `ghidra-project/`. Track = **1596 file / 19MB** source thật.
- **Baseline commit** = output của pipeline assembly cũ, đã baked toàn bộ endgame (6 fix +
  969 tolerant/54 file + 2 const). Từ giờ **fix = sửa file + commit**, không qua endgame.py.
- `setup.sh` viết gầy lại: chỉ chuẩn bị build-INPUT gitignore (copy 3 `.so` từ 镜像端, tạo
  bridge symlink `Source/Common/*`, gen shim). **Bỏ copy-từ-leak, bỏ endgame.py.**
- `bootstrap-from-leak.sh` = pipeline assembly cũ, giữ làm **provenance** (không chạy thường).
- `patches/endgame.py` = gắn banner HISTORICAL/DO NOT RUN, giữ làm **record VÌ SAO** mỗi fix
  (map sang §C/§F). Đã baked vào source.

**Đánh đổi:** mất khả năng `setup.sh` refresh-từ-leak (cố ý — không còn muốn thế). Provenance
"assembled từ leak thế nào" nằm ở `bootstrap-from-leak.sh` + baseline commit message; lý do
từng fix nằm ở endgame.py + DECISION.md; thay đổi mới nằm ở git history. Port drift tiếp theo
(enum `wdtBigSword`...) giờ là **sửa source thật + commit**, diff/blame được, không nhồi patch.

---

## H. Pipeline reverse-engineer diff 2010→2.5.2 — tài sản binary & bài học version (2026-07-05)

**Câu hỏi user:** có cách export "word-by-word" so từng file, dùng Ghidra extract 2 exec
theo tên file trong `src/SO3World/Src/*`, phân loại add/mod/del rồi port lên 2012?

**Kiểm kê binary tìm được (find toàn collection):**
| Binary | Nền | Version thực (theo CONTENT) | Symbol |
|---|---|---|---|
| SO3GameServer ta build | Linux ELF32 | từ source 2010 | đầy đủ (build ta) |
| `镜像端/.../SO3GameServer` | Linux ELF | **2.5.2.4503 (data ta)** | STRIP, tự đặt tên qua assert |
| `source/China3/SO3GameServerD.exe` + **`.pdb` 39MB** | Win VC80 Debug | nhánh KHÁC (ít weapon type hơn) | đầy đủ qua PDB |
| `lv90/.../SO3GameServerD.exe` + .pdb | Win | nhánh khác/mới | đầy đủ qua PDB |

SO3World static-link vào exe → mọi exe CHỨA toàn bộ hàm SO3World. Exe strings tự tài liệu
hoá cực mạnh: **1840 tên `Class::Method`** từ macro assert `KGLOG_PROCESS_ERROR` (in
`__FUNCTION__`), nên hàm tự-đặt-tên kể cả khi strip.

**Sự thật về "word-by-word":** KHÔNG khả thi. Decompiler ra pseudo-C, không phải source;
diff pseudo-C với C++ gốc là nhiễu (tên biến bịa, mất macro/comment, inline, control-flow
compiler biến đổi). Cái KHẢ THI & đúng granularity add/mod/del:
- **add/del = kiểm kê hàm/struct**: dump symbol → set `Class::Method`, gom theo class=file,
  set-diff với source. `llvm-pdbutil dump -types/-symbols` (đã cài llvm 22) đọc PDB không cần
  Windows; với binary strip Linux dùng Ghidra `search_symbols`/`list_xrefs`.
- **struct/field diff**: PDB có layout struct đầy đủ → gốc của mọi field-missing/size drift.
- **mod = logic từng hàm**: LLM đọc pseudo-C (đặt tên) + source, so NGỮ NGHĨA rồi port. Đắt →
  làm có mục tiêu.

**BÀI HỌC then chốt — version-match binary với DATA, không tin ngày file:**
Pilot enum `WEAPON_DETAIL_TYPE` phơi bày: PDB China3/lv90 (ngày file 2014/2020) chỉ có **9**
weapon type (giống 2010), KHÔNG có `wdtBigSword`. Nhưng data 2.5.2 (`WeaponType.tab`) dùng
**13** loại (thêm BigSword/Bow/Flute/Knife). → **PDB là nhánh content KHÁC/CŨ hơn** data ta;
ngày file chỉ là ngày đóng gói. **Ground-truth cho data 2.5.2 = binary Linux `镜像端`
(strip) + Ghidra**, KHÔNG phải PDB. PDB vẫn quý cho phần CẤU TRÚC framework/struct (ít đổi
theo content patch) nhưng KHÔNG phải chân lý cho enum/const content. → mọi port content phải
verify trên binary 2.5.2 (`/SO3GameServer-3c8199` trong ghidra project), cross-check PDB.

**Pilot hoàn chỉnh (chứng minh pipeline end-to-end trên đúng blocker):** xem §I.

---

## I. Pilot hoàn tất + migrate sang host x86 native + frontier mới: Lua state (2026-07-05)

**Pilot WEAPON_DETAIL_TYPE + MAX_ACHIEVEMENT_ID = chứng minh pipeline end-to-end:**
mỗi boot lộ blocker → decompile hàm-khung 2.5.2 (Ghidra) lấy giá trị CHÍNH XÁC → sửa source
→ commit → rebuild → boot xa hơn. 2 fix (§H), giá trị verify từ binary, không đoán.

**Nút thắt KHÔNG phải method mà là test-env:** OrbStack emulated-x86 trên mac bất ổn +
chậm dưới flood 254k warning NpcTemplate (14 cột 2.5.2 đã bỏ × 18207 NPC, tolerant xử lý
nhưng ồn). Mỗi boot mất vài phút, thất thường (đúng cảnh báo build.sh).

**GIẢI: migrate sang host x86_64 Linux NATIVE** (Linode 172.105.112.239, AMD EPYC 6-core,
Ubuntu 20.04, Docker 28, 102G trống). Layout giữ nguyên: `/root/jx3/{linux-build,镜像端/
extracted/root}`. rsync source repo (19MB) + data thiết yếu (settings/scripts/.so/ini ~341MB,
BỎ maps 3.8G/recorder 875M/logs). Image jx3build dựng lại từ Dockerfile (self-contained).
Kết quả: **build 191/0 trong 84s, boot xử lý 18207 NPC trong ~3 GIÂY** (mac emulated: phút).
Native = hết bất ổn, quan sát được. Chia việc: Ghidra RE ở mac (pyghidra-mcp), build+boot ở host.
BÀI HỌC rsync: `--exclude 'SO3GameServer'` khớp CẢ thư mục `src/SO3GameServer/` → phải anchor
`--exclude '/SO3GameServer'` (giống bug .gitignore §G).
Đọc log THẬT ở `logs/SO3GameServer/<date>/*.log` (fflush từng dòng, authoritative) — KHÔNG
tin console qua pipe (buffer, phân mảnh) hay file-redirect (server tắt console khi stdout≠tty).

**Frontier mới (sau khi settings drift sạch):** boot vượt QUA mọi settings tab → dừng ở
`KGLOG_PROCESS_ERROR(pLuaState) at line 693 in CreateScriptHolder`. `CreateScriptHolder`
KHÔNG có trong source ta → nằm trong `libEngine_Lua5D.so` (Lua engine binary). pLuaState NULL
= tạo Lua state thất bại. Đây là lớp MỚI: tích hợp exe↔.so (Lua), không phải data drift.
Nghi vấn: stub `KLuaScriptEx.h` tái dựng (_recovered) không init Lua state đúng như engine cần,
hoặc exe thiếu bước init mà server 2.5.2 thật gọi. Cần điều tra tiếp.

---

## J. Boot qua Lua + item + enum lớn (2026-07-05, host native) — giải nối tiếp §I

### J1. Lua state NULL = BUG ABI vtable (không phải data drift) — fix lớn nhất
- **Bối cảnh:** §I dừng ở `KGLOG_PROCESS_ERROR(pLuaState) at line 693 in CreateScriptHolder`.
- **Điều tra (Ghidra):** `KScriptCenter::CreateScriptHolder` (source ta, KScriptCenter.cpp:684):
  `piScript=CreateLuaInterface(...); piScript->RegisterFunctions(); piScript->RegisterConstList();
  pLuaState=piScript->GetLuaState()` → NULL. `.so` decompile: `CreateLuaInterface`→`KLuaScriptEx::Create`
  gọi `luaL_newstate()` (BỎ QUA allocator arg) rồi `GetLuaState()` chỉ `return m_pLuaState`.
  Nếu Create thành công thì lua_State phải non-NULL → mâu thuẫn.
- **Root cause:** stub `_recovered/include/Base/engine/KLuaScriptEx.h` khai báo `interface ILuaScriptEx`
  **SAI thứ tự virtual** — thừa `Init()/UnInit()` (slot 1,2) mà interface thật KHÔNG có → mọi slot
  lệch +2. Code ta gọi qua `ILuaScriptEx*` → `GetLuaState()` (slot ta 15) dispatch vào **slot 15 thật
  = `GetActiveScriptID`** → trả int (0) đọc thành lua_State* NULL. `RegisterFunctions` (slot ta 6)
  đã gọi nhầm `SafeCallBegin` → Lua hỏng từ gốc.
- **Bằng chứng vtable THẬT:** đọc `_ZTV12KLuaScriptEx @0xe0100` (read_bytes, 23 slot) + đối chiếu
  offset exe 2.5.2 gọi: RegisterFunctions=0x10/slot4, RegisterConstList=0x14/slot5, GetLuaState=0x34/slot13.
  Map 23 địa chỉ→tên (search_symbols). Thứ tự thật: Release, LoadFromFile, LoadFromBuffer,
  RegisterFunction, RegisterFunctions, RegisterConstList, SafeCallBegin, SafeCallEnd, IsFuncExist,
  CallFunction, AddParamCount, GetValuesFromStack×2, GetLuaState, IsScriptExist, GetActiveScriptID,
  GetScriptData, DumpStrt, NewFunctionRefID, DeleteFunctionRefID, PushFunctionBeforeParam,
  FastCallFunction, GetFunctionData.
- **Quyết định:** viết lại `interface ILuaScriptEx` khớp đúng 23 slot; bỏ phantom (Init/UnInit/
  ScriptNameToID/RegisterErrorHandler/GetErrorHandler — không có trong vtable, code không gọi qua iface);
  thêm filler slot 17-22 (không gọi, chỉ giữ offset). Verify: mọi method code ta gọi qua iface đều
  có trong vtable thật; `KLuaScriptEx` derived không bị instantiate ở đâu (chỉ .so tạo) → thành abstract OK.
- **Lý do:** đây là ABI binary-interface (như reuse .so), sai slot = gọi nhầm hàm. **Unblock 16,997 Lua script.**
- **Loại:** đoán/ retry — vô nghĩa với ABI; phải đọc vtable thật.

### J2. ITEM_GENRE 8→14 (append, verify từng value)
- Data item tab (`Other.tab`) có Genre tới 13; enum ta tới 7 (igTotal=8).
- **Verify Ghidra (append-only, 0-7 giữ nguyên):** đọc bảng `LUA_CONST_ITEM_GENRE` trong exe
  (array @0x84c9320 → strings @0x8432c75) + decompile chéo: OnOpenBox `nGenre==8`→igBox;
  OnApplyUseItem `==8||9`→igBox/igBoxKey; LuaSetItemMountDiamond `==10`→igDiamond. Kết quả:
  igBox=8, igBoxKey=9, igDiamond=10, igColorDiamond=11, igCub=12 (pet), igFodder=13, igTotal=14.
- **LƯU Ý:** lần đầu scan data bằng `awk|sort|uniq|head` → cắt mất, tưởng max=9. Thực tế 13. Đừng head khi cần max.

### J3. bCanStack — nới assert (2.5.2 cho box stack)
- `KItemInfoList::LoadLine:207` assert `!EquipInfo.bCanStack` cho box item. Binary 2.5.2 KHÔNG có
  assert này (chỉ 1 ref bCanStack ở material-property) → 2.5.2 bỏ. Nới thành tolerant.

### J4. ATTRIBUTE_TYPE — RE-AUDIT rộng: lỗ hổng hệ thống, regen exact (Option A)
- **User yêu cầu audit kỹ trước khi làm fix lớn.** Re-audit phát hiện 3 điều bỏ sót ban đầu:
  1. **Hệ thống, không 1 enum:** `libSO3EnumConvertorD.so` sở hữu **3 map** (ATTRIBUTE_TYPE,
     REQUIRE_TYPE, KTONG_OPERATION_TYPE) + export `EnumStr2Int`. 2.5.2 tập trung enum vào .so;
     source 2010 giữ **map local stale** (`KAttribute.cpp`) đè lên → dùng bản 2010 dù link .so 2.5.2.
  2. **INTERLEAVED không append:** `atAdjustProfessionLevel` = 343 (2010) vs **425** (2.5.2, đọc byte
     `mov [esp+8],0x1a9` trong init .so). ~82 attr chèn giữa → append kiểu số riêng sẽ khiến 344 value
     cũ **lệch với client/.so → hỏng chỉ số trang bị ÂM THẦM**. Code ta có 24 `case atX:` → value BẮT BUỘC khớp.
  3. **KHÔNG phải struct-size ABI:** attribute lưu `vector/map{type,value}`, **không** array `[atTotal]`
     → mở enum không đổi struct → không vỡ DB role-blob/protocol layout. (Khác MAX_QUEST_COUNT.)
- **Đã loại là không-lỗ-hổng:** REQUIRE_TYPE của ta **đã khớp hệt** 2.5.2 (rqtInvalid..rqtBodytype,
  rqtTotal=9) → không sửa. WEAPON_DETAIL_TYPE/ITEM_GENRE không thuộc EnumConvertor (map exe-local),
  đã verify append từ binary → không dính lỗi shadow. 0 attr bị bỏ tên → 24 switch không vỡ compile.
- **Quyết định (Option A — regen khớp CHÍNH XÁC 2.5.2, do user chọn):**
  - `tools/extract_enum_maps.py`: quét `make_pair("name",value)` trong init .so (byte pattern
    `c7 44 24 08 <imm> c7 44 24 04 <ptr>`, JArray(JByte) đọc block) → name→value authoritative.
    ATTRIBUTE_TYPE = **454 tên, value 0..453**, atTotal=454. 1 entry scan sót
    (`atAddSprintPowerReviveOnWall=444`, codegen khác) → phục hồi bằng suy diễn gap+series.
  - `tools/gen_attribute_enum.py`: regen enum (KAttribute.h, explicit value) + DECLARE_STRING_MAP
    (KAttribute.cpp) từ tsv. Map các file khác chỉ MAP_STRING_EXTERN → 1 định nghĩa duy nhất.
- **Lý do:** value khớp .so+client+data trong 1 lần; tránh lỗi âm thầm; sinh bằng script (không gõ 454 dòng).
- **Verify:** build 191/0, boot qua toàn bộ nạp attribute → tiến tới `MAX_RECIPE_ID` (constant-limit kế).
- **Còn treo:** `LUA_CONST_ATTRIBUTE_TYPE` (nếu script 2.5.2 dùng hằng attribute mới); `KTONG_OPERATION_TYPE`
  (bang hội, map dùng tên trần "Speak"/"ProductItem" ≠ tên enum → cần xử lý riêng, chưa trên đường boot).

### J5. Boot hiện tại (mốc)
config → 18,207 NPC → mọi settings tab → **16,997 Lua script** → item lib (Custom/Other/genre/bCanStack)
→ **ATTRIBUTE_TYPE (454)** → dừng ở `MAX_RECIPE_ID` (LoadCraft). Bốn+ drift đã port đúng, verify từ binary, trong git.
