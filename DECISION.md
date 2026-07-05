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

---

## K. QUYẾT ĐỊNH: build + boot trên host x86 native (playbook vận hành, 2026-07-05)

**Quyết định:** môi trường build + boot-test CHÍNH THỨC = **host Linode x86_64 native**, KHÔNG
dùng OrbStack emulated-x86 trên mac nữa. Lý do đầy đủ ở §I (emulation bất ổn + chậm dưới flood
log; native: build 84s, boot 18k NPC ~3s, ổn định, quan sát được).

**Phân vai (2 máy):**
- **Mac** = source-of-truth git (`/Volumes/ExData/game/jx3/linux-build`) + Ghidra RE
  (pyghidra-mcp `pyghidra-jx3`, binary 2.5.2 = `/SO3GameServer-3c8199`, .so EnumConvertor =
  `/libSO3EnumConvertorD.so-7ad338`, Lua5 = `/libEngine_Lua5D.so-ebbc88`). SỬA SOURCE + COMMIT ở đây.
- **Host** `root@172.105.112.239` = build + boot. Layout `/root/jx3/{linux-build, 镜像端/extracted/root}`
  (data thiết yếu đã rsync: settings/scripts/center_scripts/.so/ini ~341MB; KHÔNG có maps/recorder).
  Image `jx3build` đã dựng từ Dockerfile. `boottest.sh` sẵn ở `/root/jx3/`.

**Vòng lặp fix (chuẩn):**
1. Sửa source + `git commit` trên mac.
2. Sync (từ `/Volumes/ExData/game/jx3`), rsync `linux-build/` → host, exclude ANCHOR `/`:
   `rsync -az -e ssh --exclude 'obj/' --exclude 'shim/' --exclude '/Source/' --exclude '/libs/' --exclude '/SO3GameServer' --exclude '/ghidra-project/' --exclude '/logs/' --exclude '_deploy.log' --exclude '_build.log' --exclude '.DS_Store' linux-build/ root@172.105.112.239:/root/jx3/linux-build/`
   (anchor `/` — nếu không, `--exclude 'SO3GameServer'` khớp cả thư mục `src/SO3GameServer/`, §I).
3. Build + boot host (1 lệnh ssh):
   `cd /root/jx3/linux-build && ./setup.sh >/dev/null 2>&1 && rm -rf SO3GameServer && ./build.sh 2>&1 | grep -E "COMPILE|link exit|BINARY|FAIL" && bash /root/jx3/boottest.sh 2>&1 | tail -18`
   - `boottest.sh`: copy binary → `SO3GameServer_ours` trong deploy tree (KHÔNG đè binary 2.5.2 gốc),
     localedef GBK, chạy `timeout 90` qua pipe, đọc ĐÚNG logfile run này
     (`logs/SO3GameServer/<date>/*.log` mới hơn `/tmp/START`) lọc `grep -av "]:Get"`.
4. Đọc blocker kế ở log → RE trên mac lấy giá trị đúng → lặp.

**Nguyên tắc bất biến (mọi fix):** value/schema lấy từ **binary 2.5.2 qua Ghidra, KHÔNG đoán**
(§H bài học version-match). Sửa source thật + commit (không endgame.py, §G). Comment ASCII khi
ghi file latin-1 (GBK) — UTF-8 làm truncate file (§J lặp lại lỗi này). Đọc log fflush thật,
không tin console pipe hay file-redirect (§I).

---

## L. MAX_RECIPE_ID 512→1024 + frontier AI-type signedness (2026-07-05, host native)

### L1. MAX_RECIPE_ID: constant-limit drift, value CHÍNH XÁC từ binary (không đoán)
- **Blocker (§J5):** `KGLOG_PROCESS_ERROR(CraftRecipe.dwID < MAX_RECIPE_ID)` tại `KRecipe.cpp:396`
  trong `KRecipe<KCraftRecipe>::Init` (load `Craft/Recipe/*.tab`). Cùng lớp drift MAX_QUEST_COUNT/
  MAX_ACHIEVEMENT_ID (§F4/§J): data 2.5.2 có recipe ID vượt hằng của source 2010.
- **RE binary 2.5.2 (`/SO3GameServer-3c8199`, Ghidra):**
  - `KRecipe<KCraftRecipe>::Init` = `FUN_080b6c76`: nhánh lỗi `if (0x3ff < dwID)` in ra
    `"CraftRecipe.dwID < MAX_RECIPE_ID"` (line 0x1c5) → assert `dwID < MAX_RECIPE_ID` với
    **MAX_RECIPE_ID = 0x400 = 1024**. (String map: `search_strings "MAX_RECIPE_ID"` →
    3 assert CraftRecipe/CraftEnchant/CraftCopy @0x83e0d00/0af0/0bd0; xref → Init/LoadLine.)
  - Xác nhận chéo `KRecipeList::IsRecipeLearned` = `FUN_083b64fe`: `dwRecipeID <= MAX_RECIPE_ID`
    fail khi `0x400 < dwRecipeID`; `dwCraftID <= MAX_CRAFT_COUNT` fail khi `0x10 < dwCraftID`
    (MAX_CRAFT_COUNT=16, khớp source ta). **Bitmap stride `(craftID-1)*0x80`** = 0x80 = 128 byte
    = `(1024+7)/8` → đây là size bitmap DB/wire 2.5.2, KHÔNG chỉ bound in-memory.
- **Fix:** `Global.h:107` `#define MAX_RECIPE_ID 512` → `1024` (OLD_MAX_RECIPE_ID giữ 256).
  Sửa bằng `LC_ALL=C sed -i ''` (file GBK — sed thường báo "illegal byte sequence"; LC_ALL=C
  xử byte thô, chỉ đổi 1 dòng, +1 byte, không đụng encoding phần còn lại). Commit `f89cb85`.
- **Tác động struct-size (như MAX_QUEST_COUNT §F4 — ghi rõ):**
  - `m_byRecipeState[16][MAX_RECIPE_ID]`: 8K→16K **in-memory** mỗi player (không ra wire/DB).
  - `KRecipeDBItem.byRecipeData[(MAX_RECIPE_ID+7)/8]`: 64→**128 byte** = bitmap role-blob/wire
    → giá trị 1024 làm ta **KHỚP** 2.5.2 (client/DB kỳ vọng 128 byte). Vì thế phải đúng 1024,
    không phải "đủ lớn". (Server tự build, chưa có char-DB cũ nên không vướng migrate blob.)
- **Verify data:** `Craft/Recipe/tailoring.tab` max ID=**713** (>512 → thủ phạm), founding=506,
  cooking=198, medicine=210; Enchant=221, Read/Copy=185, Collection≤46. Tất cả <1024 ✓.
- **Verify boot host:** build 191/0, link 0 → boot QUA recipe, tiến tới blocker AI (L2).

### L2. FRONTIER MỚI: `nAIType >= 0` fail trong LoadAITabFile = drift signedness (int vs DWORD)
- **Blocker kế:** `KGLOG_PROCESS_ERROR(nAIType >= 0) at line 182 in LoadAITabFile`
  (`KAIManager.cpp:180-182`): `GetInteger(nRow,"AIType",0,&nAIType)` vào `int nAIType`.
- **Điều tra data (`settings/AIType.tab`, 25417 dòng):** cột AIType KHÔNG chỉ 0..N tuần tự;
  từ ~row 25020 có ID map-specific rất lớn (vd `2097152009`=0x7D000009, script `scripts\Map\...\ai\...BOSS...`)
  và các giá trị unsigned high-bit (đọc thành âm `-2063597567`=0x84A3E401). `int nAIType` nhận
  high-bit → âm → assert vỡ. Nghi 2.5.2 khai **DWORD nAIType** (unsigned) + `m_AITable` key DWORD.
  Đây là drift signedness, KHÔNG phải constant-limit.
- **CHƯA fix — cần RE trước (nguyên tắc §K):** decompile `KAIManager::LoadAITabFile` trong
  `/SO3GameServer-3c8199`, xác nhận (a) kiểu đọc AIType (DWORD?) và (b) có assert `>=0` không / bound
  thật. Nếu 2.5.2 dùng DWORD: đổi `int nAIType`→`DWORD` (và các chỗ dùng: `m_AITable`, `ReloadAILogic`,
  `GetAILogic`, `CreateAI`, `Setup` — grep `nAIType`/`AIType` trong KAIManager/KAILogic/KCharacter).
  Kiểm tác động: key map đổi int→DWORD không đổi wire (AI runtime server-side) nhưng cần đồng bộ mọi
  chữ ký hàm. Liên quan §B2 (VM-AI): xác nhận đường load này là VM-AI hiện hành.

### L3. Boot hiện tại (mốc, cập nhật §J5)
config → 18,207 NPC → mọi settings tab → 16,997 Lua script → item lib → ATTRIBUTE_TYPE(454)
→ **MAX_RECIPE_ID(1024) [QUA]** → dừng ở `LoadAITabFile nAIType>=0` (AI type signedness, L2).
Năm+ drift đã port đúng, verify từ binary, trong git.

---

## M. AUDIT CHỦ ĐỘNG: quét drift TĨNH (không chờ boot) — 2026-07-05

**Bối cảnh (ý user):** MAX_RECIPE_ID là constant-limit drift THỨ TƯ cùng khuôn (sau
MAX_QUEST_COUNT/MAX_COOL_DOWN_COUNT/MAX_ACHIEVEMENT_ID). Thay vì chờ boot chạm từng cái →
quét trước cả lớp. Câu hỏi 2: có cách phát hiện KHÔNG cần debug? Có — 3 kỹ thuật tĩnh.

### M0. Ba kỹ thuật phát hiện drift KHÔNG cần boot
1. **Binary-assert-literal diff (mạnh nhất cho constant-limit):** exe 2.5.2 nhúng SẴN text
   mọi assert KGLOG (`search_strings "MAX_"` → vd `"CraftRecipe.dwID < MAX_RECIPE_ID"`), và
   literal bound nằm ngay cạnh trong decompile (`if (0x3ff < dwID)`). So `#define` source vs
   literal binary: source < binary ⇒ DRIFT chắc chắn (2.5.2 nâng limit, source stale, data vượt).
2. **Data-max offline (không cần cả binary):** quét cột ID/level trong `.tab` (LC_ALL=C awk max)
   so `#define`. data_max >= define ⇒ sẽ vỡ assert khi boot. Trả lời trực tiếp "boot có crash không".
3. **Enum name-set diff:** map string->int (EnumConvertor .so + map exe-local) chứa tên value
   authoritative 2.5.2; so tên/đếm với enum source. binary có tên source thiếu ⇒ `Str2Int=-1`
   ⇒ blocker "Map string X failed" (đúng dạng ATTRIBUTE_TYPE §J4).

### M1. KẾT QUẢ audit CONSTANT-LIMIT (kỹ thuật 1+2) — nhóm coi như ĐÃ CẠN
Quét 198 `#define` limit trong source, giao với ~60 assert `MAX_*` trong binary. Bảng
(source vs 2.5.2, đọc literal qua Ghidra hoặc data-max):
- **4 drift THẬT — tất cả ĐÃ sửa:** MAX_QUEST_COUNT 16384(0x4000), MAX_COOL_DOWN_COUNT 1024(0x400),
  MAX_ACHIEVEMENT_ID 4000, MAX_RECIPE_ID 1024(0x400).
- **12 hằng boot-path KHỚP/AN TOÀN (không sửa):** MAX_TRACK_ID 2000=2000, MAX_MAP_ID 255=0xff,
  MAX_BUFF_REPRESENT_ID 256=256, MAX_BUFF_DECAY_TYPE 16=16, MAX_DESIGNATION_FIX_ID 255=255,
  MAX_READ_BOOK_ID 512=0x200 (bit9), MAX_READ_BOOK_SUB_ID 8=8 (bit3), PROFESSION_MAX_LEVEL 100=0x64
  (`(nHeight-1)<0x65`), MAX_CRAFT_COUNT 16=0x10, MAX_SKILL_LEVEL 127=SCHAR_MAX (data max=127),
  MAX_PROFESSION_COUNT 128 (profession.tab max=12), MAX_FORCE_COUNT 128 (data nhỏ).
- **Kết luận:** lớp constant-limit gần như hết drift trên boot-path. Các blocker kế KHÔNG còn
  thuộc lớp này (ID-space grow theo content = quest/achi/recipe/cooldown, đã xong).

### M2. NHÓM RỦI RO KẾ (chưa quét hết) — enum name-set (đã drift 3 lần)
24 enum có string-map trong source (load-by-name từ data = nhóm hay drift nhất):
`ATTRIBUTE_TYPE, REQUIRE_TYPE, WEAPON_DETAIL_TYPE(=TYPE), ITEM_TABLE_TYPE, KTONG_OPERATION_TYPE,
KBUFF_TYPE, KBUFF_PERSIST_ON_FIGHT/HORSE/TERRAIN, KBUFF_PERSIST_SHIELD_TYPE, KBUFF_REPRESENT_POS,
KSKILL_CAST_MODE/CAST_EFFECT_TYPE/COMMON_ACTIVE_MODE/DAMAGE_TYPE/EFFECT_PLAY_TYPE/EVENT_TYPE/
FUNCTION_TYPE/KIND_TYPE/WEAPON_REQUEST_TYPE, KEVENT_SKILL_CASTER_TARGET_TYPE,
SKILL_HORSE_STATE_CONDITION` + ITEM_GENRE (exe-local array).
- **Đã kiểm SẠCH:** ATTRIBUTE_TYPE(454 regen), REQUIRE_TYPE(9 khớp), WEAPON_DETAIL_TYPE(+4),
  ITEM_GENRE(14). **~19 enum KSKILL_*/KBUFF_*/KEVENT_* CHƯA quét** — blocker enum kế (nếu có) ở đây.
- **Cách quét (chưa chạy):** với mỗi enum, trích name-set 2.5.2 (EnumConvertor .so cho map .so;
  byte-scan DECLARE_STRING_MAP static-init trong exe cho map exe-local — mở rộng
  `tools/extract_enum_maps.py`) rồi so enum source. HOẶC offline: quét cột enum trong skill/buff tab
  → tập distinct name → check có trong enum source. Skill/buff tab NẰM trên boot-path (sau AI).

### M3. NHÓM signedness/struct (khó quét hàng loạt tĩnh)
`nAIType` (§L2) = int vs DWORD. Loại này cần so kiểu field từng cái, không có
chữ-ký string như assert/enum → chỉ lộ khi RE per-field hoặc boot. Ít tái diễn hơn 2 lớp trên.

### M4. THỰC THI enum sweep (offline diff → RE value) — 4 drift sửa loạt, skill 0→6161
Chạy offline diff: quét cột string trong skill/buff tab (data boot) vs tập string source map,
BINDING chính xác từ `MAP_STRING_LOCAL` (KHÔNG đoán cột). Tìm được 4 drift THẬT:
- **KSKILL_CAST_MODE (INTERLEAVE):** chèn scmTargetAngleSector=2, scmRectangle=3,
  scmTargetAngleRectangle=4 sau scmSector=1 → dịch scmCasterArea..scmTeamArea +3. Value đọc từ
  binary (make_pair byte-scan `c744 2408 <imm> c744 2404 <ptr>` @FUN_08169170): Sector=1..PartyArea=14.
  Append sẽ gán SAI value mọi cast-mode cũ vs client → desync targeting. Fix = chèn 3 enumerator
  (KSkill.h) + 3 REGISTER (KSkillManager.cpp).
- **SKILL_HORSE_STATE_CONDITION (append):** shcIgnore=4, shcEquipSpecialHorse=5.
- **KBUFF_PERSIST_SHIELD_TYPE (append):** pmsGlobalTherapyAbsorb=14 (@FUN_0826edec).
- **KSKILL_EVENT_TYPE (append):** seParry=20 (SkillEvent.tab, @FUN_0816769e: Cast=1..BeMiss=19 khớp).
- **BÀI HỌC phương pháp (2 lần suýt sai):** (1) so với **ENUM** (có Invalid=0 đầu), KHÔNG so với
  string-map trần → nếu không sẽ đọc nhầm SHIELD thành interleave. (2) offline diff PHẢI dùng đúng
  binding: skills.tab cột `FunctionType` KHÔNG được skill-loader map (chỉ buff map) → Blow/Fly/Heal
  là FALSE POSITIVE, bỏ. Chỉ cột thật sự Str2Int mới tính.
- **Kết quả boot:** build 191/0, `[AI] 25414 AI loaded` (nAIType OK), skill load **0 → 6161 skill
  loaded** rồi dừng ở lớp MỚI (§N).

---

## N. FRONTIER MỚI: Lua-binding (GetEditorString nil) — 2026-07-05

Sau khi 4 enum skill/buff sạch, skill load tới 6161 rồi dừng:
`[Lua] scripts/Include/GlobalStrings.ls:10: attempt to call global 'GetEditorString' (a nil value)`
→ script load fail → skill id=2 script fail → `LoadSkillDataFull:1358` → skill mgr Init fail.
- **Bản chất:** lớp exe↔Lua **binding** (đăng ký hàm Lua global), KHÔNG phải data/enum/constant drift.
  `GetEditorString` KHÔNG có trong source ta. "Editor" gợi ý hàm editor-tool (như KGodServer dead §E)
  hoặc hàm exe 2.5.2 đăng ký mà build ta thiếu.
- **RE kết quả:** `GetEditorString` KHÔNG phải C++ binding — nó là hàm LUA, định nghĩa file-scope
  trong `scripts/LuaEnvInit/EditorExportedStrings/EditorStringHeader.li`, được nạp qua
  `scripts/LuaEnvInit/LuaEnvInit.li` (Include chuỗi + hàm `LuaEnvInit()` append helper globals:
  GetEditorString, String/Math/Player/Npc/Misc/SceneCustomValue/VersionControl/TongRaidQuest).
- **Root cause:** exe 2.5.2 `KScriptCenter::Init` (FUN_08113e90) gọi `ScriptEnvInit("scripts")`
  (FUN_081139bc) NGAY sau CreateScriptHolder, TRƯỚC search/load: load `LuaEnvInit\LuaEnvInit.li`
  → IsFuncExist("LuaEnvInit") → CallFunction("LuaEnvInit"). Source ta THIẾU HẲN bước này → helper
  globals không bao giờ nạp. `.li` không khớp `IsLuaFile` (chỉ lua/lh/ls) → LuaEnvInit.li không vào
  bảng auto-load → phải load tường minh.
- **FIX (làm rồi, commit):** thêm `KScriptCenter::ScriptEnvInit(const char*)` khớp 2.5.2 + gọi trong
  `Init()` sau CreateScriptHolder. Verify boot: lỗi GetEditorString BIẾN MẤT. (LoadFromFile =
  compile-only; execute xảy ra khi Include/CallFunction, nên ScriptEnvInit phải chạy trước skill exec.)

### N2. FRONTIER KẾ (sau GetEditorString): filename GBK HỎNG khi giải nén — DATA, không phải code
Sau ScriptEnvInit, skill vẫn dừng ở skill id=2 (script `npc\副本BOSS\空雾峰1号欠债.lua`), `IsScriptExist`
false. Điều tra: **96% (16,485/17,091) file script có tên GBK bị hỏng thành U+FFFD (bytes EF BF BD)**
trên CẢ mac lẫn host — do giải nén `手工端.7z` bằng công cụ macOS diễn giải tên GBK thành UTF-8 lỗi.
- **Cơ chế:** skills.tab (nội dung GBK, ĐÚNG) trỏ path GBK; file trên đĩa mang tên U+FFFD → `g_FileNameHash`
  của path-đúng ≠ hash path-hỏng → không tìm thấy → `IsScriptExist` false. Skill path-ASCII (TestSkill)
  OK; skill path-GBK ĐẦU TIÊN (id=2) fail → abort (2.5.2 skill loader cũng strict, FUN_0816d346).
- **Nội dung file CÒN NGUYÊN** (0 collision trên full-path → không đè mất) — chỉ TÊN sai. U+FFFD lossy →
  không suy ngược tên gốc → PHẢI re-extract từ archive gốc với GBK filename decoding.
- **Trở ngại tooling:** mac không có 7z; host chỉ có `unzip` (không 7z/convmv); archive 902MB ở mac.
  Cần: 7z có codepage GBK, hoặc script đọc archive lấy raw filename bytes decode GBK. LỚN + tách biệt.
- **Loại:** vấn đề ENVIRONMENT/DATA (giải nén sai encoding), KHÔNG phải version-drift source. Chờ user quyết.

### N2b. FIX filename-encoding + AUDIT mất file (2026-07-05) — re-extract GBK trên host Linux
- **Archive:** `download/手工端.7z` = `.7z` bọc `jjjx3.tar` (7.2GB). TAR lưu filename raw-bytes; entry
  bên trong = **byte GBK ĐÚNG** (`\270\261\261\276BOSS` = B8B1 B1BE = 副本 GBK). Corruption cũ do
  giải nén jjjx3.tar bằng tool macOS: APFS ép tên UTF-8 hợp lệ → byte GBK không hợp lệ → U+FFFD.
- **KHÔNG extract được trên mac/APFS** (chỉ nhận tên UTF-8 hợp lệ) → phải extract trên **host Linux/ext4**.
  Cài `p7zip-full` host, copy .7z (đặt tên ASCII `shougong.7z` — rsync macOS cũ + remote path GBK lỗi),
  stream: `7z x -so shougong.7z jjjx3.tar | tar xf - root/scripts` (tar Linux giữ nguyên byte GBK).
- **Swap:** `镜像端/extracted/root/{scripts,settings}` → `*_corrupt_bak`, thay bằng cây extract đúng.
- **AUDIT mất file (user yêu cầu kiểm kỹ, so bằng md5 NỘI DUNG không phải tên):**
  scripts 19245 file (18676 unique-content) vs corrupt 17091 (16632); settings 9262 vs 8940.
  `comm` trên tập content-hash: **NEW\CORRUPT = 2048 (scripts) + 280 (settings) nội dung MẤT THẬT**;
  CORRUPT\NEW = 4+1 file, **TẤT CẢ là `.DS_Store`** (rác macOS, non-DS_Store=0). → cây mới là superset
  nội dung thật, không mất gì; corrupt mất 2328 file game.
- **Cơ chế collision (chứng minh):** #U+FFFD = #byte-GBK = 2×#ký-tự-Hán → tên cùng số ký tự Hán +
  cùng phần ASCII → CÙNG chuỗi U+FFFD → đè nhau (last-writer-wins). Bằng chứng: corrupt còn 1 dir
  `<UFFFD×4>BOSS` (115 file) trong khi new có `副本BOSS`(126)+`25人副本BOSS`+... riêng biệt.
- **Kết quả boot:** scripts fix → skill loading VƯỢT QUA (không còn abort skill id=2). settings fix →
  DropList/锦囊 hết. Tiến tới: (a) non-fatal `LuaAddAttribute:53` attribute key ngoài range
  (LUA_CONST_ATTRIBUTE_TYPE, §J4 "còn treo"); (b) blocker `MapDropInit:542` mở `settings/DropList/10`
  (path bị cắt tại byte GBK — nghi buffer/GetString truncate, đang điều tra).

### N2c. FRONTIER KẾ: map-drop system THIẾT KẾ LẠI ở 2.5.2 (drift cấu trúc, chưa fix)
Sau GBK fix, dừng ở `KDropCenter::MapDropInit:542` mở `settings/DropList/10` (fail).
- **Điều tra:** `MapList.tab` cột `MapDrop` = "10","7","3"... (SỐ, không phải tên file). Source 2010
  `MapDropInit` coi `pMapParams->szDropName` là filename → `settings/DropList/10` → không tồn tại
  (archive không có DropList tên số; 110 dir đều tên GBK).
- **RE 2.5.2 (FUN_082bf6be = MapDropInit):** hệ thống KHÁC HẲN — mở `settings/**MapDrop.tab**`
  (ID, MapDrop1..8, DropType1..8). MapList.MapDrop = **MapDropID** → tra MapDrop.tab → mỗi slot
  `MapDrop%d` là tên drop-list → `settings/DropList/<name>`, `DropType%d` = loại. Dùng struct
  `m_mapMapDropID2MapDropTabItem`. `settings/MapDrop.tab` CÓ trong data (row 10 → MapDrop1=
  "MapDrop\wanhua_gear_MapDrop.tab", DropType1=2).
- **Bản chất:** 2.5.2 thêm tầng gián tiếp MapDropID (1 map-drop-set dùng lại cho nhiều map). Source
  2010 chưa có tầng này → phải REIMPLEMENT MapDropInit (đọc MapDrop.tab, struct ID→slots, DropType
  semantics). Drift CẤU TRÚC (như migration class-AI→VM-AI), không phải constant/enum/tên-file.
- **Lựa chọn:** (A) port đúng theo 2.5.2 (đọc MapDrop.tab, dựng m_mapMapDropID2MapDropTabItem) — lớn;
  (B) tolerant tạm (skip map-drop lỗi) để boot xa hơn xem blocker kế, port sau. Chờ user quyết.

### N3. Tổng kết lớp drift (6 loại đã gặp)
constant-limit (§M1, cạn) · enum name-set (§M4, 4 fix) · signedness (nAIType §L2) ·
Lua-global-binding (GetEditorString/ScriptEnvInit §N, fix) · data/filename-encoding (§N2, fix +
audit: 2328 file thật mất do collision, khôi phục) · **struct/subsystem-redesign (map-drop §N2c, mới)**.

---

## O. MỐC LỚN: "Load game settings [OK]" — hết pha nạp data, tới pha network/center (2026-07-05)

Sau full GBK re-extract (§N2b, cả cây `root/` 7.1G/152559 file, U+FFFD=0, có maps) + tolerant
map-drop (§N2c hướng B): boot **QUA TOÀN BỘ pha nạp data** → `Load game settings ... ... [OK]`.
Server tiến sang pha MỚI: **kết nối center server** `192.168.200.105:5003` → [Failed]
(`Connect:45 piSocket` → `KSocketConnation::Init:207` → Init:52). Đây là tầng **hạ tầng đa tiến
trình** (§E: SO3GameCenter sở hữu MySQL, GameServer nối qua socket), KHÔNG phải version-drift.

**Ý nghĩa:** mọi bức tường version-drift trên đường NẠP DATA đã vượt (config→NPC→settings→Lua→
item→attribute→recipe→AI→skill→buff→drop→...→settings [OK]). Server tự-build 2010-source chạy
data 2.5.2 tới tận pha networking.

**Việc còn treo (không chặn "settings OK", cần cho chạy thật):**
- **Hạ tầng center/DB (pha kế):** cần chạy SO3GameCenter + MySQL (config `relay_settings.ini`
  `[MySQL]` DB=jx3_25, §E) và/hoặc trỏ center IP 192.168.200.105→localhost. Đây là bước "dựng cụm
  3 tiến trình" (GameCenter/Gateway/GameServer), không phải sửa source.
- **Non-fatal đã thấy:** `script_server.lua:1581 index global 'DIAMOND_SUB_TYPE' (nil)` (LUA_CONST
  thiếu — cùng lớp LUA_CONST_ATTRIBUTE_TYPE §J4 "còn treo"; script_server.lua fail → có thể ảnh
  hưởng runtime sau, chưa chặn settings). Vài script Map (HoroSystem/CheckTime/QiXi...) fail load —
  runtime-map deps, non-fatal.
- **map-drop port đúng (§N2c hướng A):** chưa làm; hiện skip → NPC trên map không rơi đồ.

**Playbook (§K bổ sung):** data GBK PHẢI extract trên host Linux/ext4 (`7z x -so shougong.7z jjjx3.tar
| tar xf - root`) — tar giữ nguyên byte GBK khớp .tab GBK. KHÔNG extract qua macOS/APFS (ép UTF-8 →
GBK không hợp lệ → U+FFFD, mất file do collision). Host là nơi canonical của data tree.

---

## P. AUDIT đối chiếu log THẬT vs bản ta — root cause để 2 log khớp (2026-07-05)

Chạy binary GỐC 2.5.2 (`SO3GameServer`) cùng deploy tree, diff normalized với log bản ta.
**Cả hai đạt `Load game settings [OK]` → cùng fail center-connect 192.168.200.105:5003** (đúng mốc,
center chưa chạy). Binary thật log 56 dòng SẠCH; bản ta 10785 dòng (ngập warning tolerant). Binary
thật CŨNG fail đúng các script Map (HoroSystem/CheckTime/QiXi...) → bình thường, không phải bug ta.

**Blast radius THẬT (từ log, không đoán) — chỉ 8 symbol KSkill + 2 const:**
| symbol | loại | #fail | root cause |
|---|---|---|---|
| SetCheckCoolDown | method | 1323 | 2.5.2 thêm category cooldown "Check" (MAX_SKILL_CHECKONLY_COOL_DOWN_TIMER, string có trong binary) song song Normal/Public; source 2010 thiếu |
| AddSlowCheckDestOwnBuff | method | 141 | 2.5.2 thêm biến thể buff-check "Own" (buff do chính mình cast); ta chỉ có DestBuff/SelfBuff |
| AddSlowCheckSelfOwnBuff | method | 2 | như trên |
| nHeight / nRectWidth / nProtectRadius | field | 176/117/100 | field hình học cho cast-mode MỚI Rectangle/TargetAngleSector (đã thêm enum §M4); KSkill thiếu member+binding |
| nCostManaBasePercent / nCostEnergy | field | 5/1 | field cost MỚI 2.5.2 |
| LUA_ATTRIBUTE_TYPE (LuaAddAttribute out-range) | const | 1376 | array Lua-const KLuaConstList.cpp:533 chỉ 342 entry vs C++ enum 454 (§J4) → tên attr mới = nil → AddAttribute(0) → fail `>atInvalid`. Regen array từ binary (name→value) như §J4 |
| DIAMOND_SUB_TYPE | const | 1 | LUA_CONST chưa đăng ký; script_server.lua:1581 dùng .GOLD/.WOOD/.WATER... |

RE tên KSkill thật (@0x08451xxx): danh sách method 2.5.2 = ...AddCheckSelfLearntSkill,
AddSlowCheckSelfOwnBuff, AddSlowCheckDestOwnBuff, AddSlowCheckSelfBuff, AddSlowCheckDestBuff,
...SetPublicCoolDown, SetCheckCoolDown, GetCheckCoolDownCount, GetCheckCoolDownID, SetNormalCoolDown,
GetNormalCooldownCount, GetNormalCooldownID... Chỉ 3 method + 5 field là bị script gọi & fail thật.

**Khác biệt phụ (không chặn, chưa chốt):** AI count gốc 25627 vs ta 25414 (nAIType skip `==0` khác
cách đếm gốc — cần RE lại LoadAITabFile count); scripts searched 19150 vs 19245 (+95, minor);
binary thật log `Append [...]` của LuaEnvInit + `CPU in`/`VersionEx` + SkillManager timing — bản ta
KHÔNG log Append → nghi `ScriptEnvInit` chưa thực gọi `LuaEnvInit()` (dù GetEditorString chạy nhờ
file-scope); cần verify CallFunction("LuaEnvInit").

**SOLUTION (chờ user double-check, có repo tham chiếu SetCheckCoolDown) — audit-then-fix:**
1. Port 3 method KSkill (SetCheckCoolDown + GetCheckCoolDownCount/ID; AddSlowCheckDestOwnBuff/
   SelfOwnBuff): thêm member (check-cooldown array + own-buff vector), method Lua, REGISTER_LUA_FUNC,
   wire vào CheckCoolDown/require-buff runtime. RE `KSkill::LuaSetCheckCoolDown` (@0x08451700) +
   AddSlowCheck* từ binary để impl ĐÚNG (không đoán).
2. Thêm 5 field (nHeight/nRectWidth/nProtectRadius/nCostManaBasePercent/nCostEnergy): member KSkill +
   REGISTER_LUA_INTEGER + đọc từ skill tab (SkillTableLine) như field cũ.
3. Regen LUA_ATTRIBUTE_TYPE (342→454) từ binary LUA_CONST array (name UPPER_SNAKE→value), khớp C++ enum.
4. Thêm LUA_CONST_DIAMOND_SUB_TYPE (đọc value GOLD/WOOD/WATER/... từ binary).
5. (phụ) verify LuaEnvInit() được gọi; điều tra AI count 213.

---

## Q. LOG-PARITY: audit real-vs-ours + 7 fix để 2 log khớp (2026-07-05, goal)

Chạy binary GỐC 2.5.2 cùng deploy tree, diff log. Cả hai đạt `Load game settings [OK]` → center-fail.
Ban đầu ours 10785 dòng (ngập warning tolerant) vs real 56. Audit root-cause từng bug (không đoán),
fix, đưa ours meaningful (lọc `]:Get`) về ~176; **Map-script fails KHỚP CHÍNH XÁC real (19=19, 0 lệch
2 chiều)**; LuaAddAttribute/SetCheckCoolDown/... = 0.

**7 fix (mỗi cái RE binary + đối chiếu JX3CalcBE repo user cấp):**
1. **KSkill Lua binding (3 method)** — `SetCheckCoolDown`/`GetCheckCoolDownCount`/`ID` (category
   "Check" cooldown, MAX_SKILL_CHECKONLY_COOL_DOWN_TIMER=3, RE FUN_08356b3e: idx a-1, m_dwCheckCoolDownID);
   `AddSlowCheck{Self,Dest}OwnBuff` (biến thể "Own", vector riêng). ~1466 fail.
2. **KSkill field (5)** — nHeight/nRectWidth/nProtectRadius (geometry Rect/Sector cast mode) +
   nCostManaBasePercent/nCostEnergy. macro REGISTER_LUA_INTEGER thêm 'n' → lua name nHeight. ~400 fail.
3. **LUA_ATTRIBUTE_TYPE regen 342→451** — script dùng attr-const thiếu → nil → AddAttribute(0) → out-range
   (1376 fail). Trích array exe @0x084ca1a0 qua **tools/extract_lua_attr.py (pyghidra, không transcription)**;
   tên UPPER_SNAKE không suy máy móc được (KiloNum≠KILONUM...) → lấy verbatim.
4. **DIAMOND_SUB_TYPE** LUA_CONST (INVALID=-1..TOTAL=5, exe @0x084c9500). 1 fail.
5. **LuaBindBuff 3→3/4/5 args** (RE FUN_08357882; opt arg4 bool/arg5 int). 63 fail.
6. **SetSun/MoonSubsectionSkill** (RE FUN_0835645c: 4 args, MAX_SUN/MOON_POWER_VALUE=1). 11 fail.
7. (map-drop §N2c tolerant-skip đã có).

**Phương pháp học được:** hand-transcribe hex name-block SAI (CURRENT_ENERGY 49→51) → chuyển pyghidra
standalone đọc thẳng array (reliable). so tên với ENUM không phải string-map trần. binding từ binary +
JX3CalcBE verify, không đoán.

**Residual (còn ours-only, non-fatal, boot vẫn [OK]):** 13 dòng `index a nil value` từ **4 BOSS skill
script** = quirk DATA per-script: `天工坊_机甲龙` MaxLevel=**21** nhưng tSkillData chỉ **20** entry →
tSkillData[21]=nil (off-by-one tab-vs-script); `南诏皇宫_枯荣大师_*` MaxLevel=2 nil khác. KHÔNG phải
drift hệ thống — là bất nhất data trong 4 script (of 6161). Real binary xử lý (có thể cũng log ở file
rotate khác — harness real bất ổn sau full-tree swap, không đo dứt điểm được). Boot outcome KHỚP real.

**Kết luận:** mọi drift HỆ THỐNG (binding/const/enum/field/args) đã fix, log khớp real trên toàn bộ
nội dung hệ thống + Map-fails y hệt. Còn 13 dòng edge-case data-quirk (4/6161 skill), non-fatal.
### Q2. Kiem real dut diem — real SACH, 13 residual la ours-specific
- Real complete log = 56 dong, 1 FILE (real khong flood ]:Get nen khong rotate; ours flood NpcTemplate
  tolerant warning 18724x2 -> rotate 65536 dong/file = nguon nhieu do truoc). Real: index-nil=0,
  GetSkillLevelData-fail=0, Failed-load=25(=ours), 6161 skill sach. => 13 index-nil CONFIRM ours-specific.
- Root-cause 13: tSkillData[skill.dwLevel] nil du m_dwLevel set TRUOC CallInitializeScript (dwLevel dung)
  + tSkillData >= dwLevel entries => tSkillData GLOBAL collision/nil khi GetSkillLevelData chay. NHUNG
  ours CallInitializeScript CAU TRUC Y HET real (SafeCallBegin/PushValueToStack(this)/CallFunction/
  SafeCallEnd = FUN_08112a72/081567ec/081131fe/08112a1c) => khac o tang Lua script-env isolation sau hon,
  can debug Lua-runtime. 4/6161 skill, non-fatal (boot van [OK]).
- KET LUAN: moi drift HE THONG fixed, log khop real toan bo noi dung he thong + Map-fails y het;
  residual 13 dong (4 script) edge-case Lua-scope chua fix (tach biet, can Lua-runtime debug).

### Q3. Root-cause 13 index-nil chinh xac (2026-07-05) — data off-by-one MaxLevel vs tSkillData
- 13 index-nil DETERMINISTIC (tai dung: 10x line35 nam-chieu-hoang-cung, 2x line66 + 1x line72 thien-cong-fang).
- 4 script fail deu: tab MaxLevel > so entry tSkillData script khai. VD `天工坊_3号BOSS_震天甲士_冲击`
  MaxLevel=**21** nhung tSkillData chi **20** entry -> level 21: tSkillData[21]=nil -> index nil (line 66).
  南诏皇宫_南诏皇宫_* (line35) tuong tu. => bat nhat DATA trong 4/6161 script (tac gia khai tSkillData thieu).
- Da loai tru: dwLevel dung (m_dwLevel set truoc CallInitializeScript), tSkillData khong nil (co entries),
  CallInitializeScript CAU TRUC Y HET real (SafeCallBegin/PushValueToStack/CallFunction/SafeCallEnd).
- CHUA pin: vi sao real (cung MaxLevel=21, cung tSkillData=20, cung loop 1..MaxLevel FUN_081648fc) KHONG
  fail. Nghi real doc/cap dwMaxLevel khac (TranslateTableInfo/LoadSkillTableLine) HOAC engine xu level>data
  khac. Runtime-instrument (error() trong script) KHONG fire duoc do harness multi-log (server rotate
  65536 dong/file) + find-newer flaky; can harness tot hon (tat rotate / lua debug hook) de dao tiep.
- Trang thai: non-fatal (boot [OK]), 4 script edge. Moi drift HE THONG da khop real. Residual nay = data
  off-by-one + 1 an so real-MaxLevel-handling, tach biet.

### Q4. Option A ket qua — DBGML instrument xac nhan DATA off-by-one, real-mechanism opaque
Instrument KSkill::CallInitializeScript (temp log dwMaxLevel/dwLevel/script khi GetSkillLevelData fail),
build, boot, grep theo timestamp (reliable). KET QUA CHINH XAC:
- id=838/844/855: MaxLevel=21, fail tai lv=21 (tSkillData literal=20 entry) -> tSkillData[21]=nil.
- id=4413 (南诏皇宫_陈和尚_肉毒地藏): MaxLevel=20, fail lv=11..20 (tSkillData literal=**10** entry, dem chinh xac)
  -> tSkillData[11..20]=nil.
=> 13 index-nil = 4 script co tSkillData literal < MaxLevel. Script KHONG co loop/helper fill tSkillData
   (thuan literal). Day la BUG DATA trong 4 script game (tac gia khai tSkillData thieu vs MaxLevel tab).
- Ours doc dwMaxLevel dung tu cot "MaxLevel" (=21/20), loop 1..MaxLevel = Y HET real (FUN_081648fc).
- MAU THUAN chua giai: real cung file (tSkillData 10/20), cung tab (MaxLevel 20/21), cung loop -> dang le
  cung fail lv>tSkillData. Real (ban 56-dong good) index-nil=0. Static RE khong giai thich duoc; KHONG
  instrument duoc real (docker harness flaky: real luc dat [OK] luc 4-dong early-exit -> khong co
  instrumented-real run on dinh de so). Co the real cap level/doc MaxLevel khac o tang sau chua pin duoc.
- Non-fatal (boot [OK] giong real). Option de dong tuyet doi: (a) sua DATA 4 script (extend tSkillData ->
  MaxLevel, HOAC giam MaxLevel tab) -> guarantee 0 index-nil; (b) on dinh harness (tat log-rotate/fix
  docker flaky) roi instrument real pin co che; (c) log GetSkillLevelData-fail o muc DEBUG thay ERROR.
- KET LUAN: moi drift HE THONG fixed; residual = 4 script DATA off-by-one (tSkillData<MaxLevel), real
  tolerate qua co che chua pin (harness limit). Framework logic ta = real.

### Q5. DONG DUT DIEM — data-fix 4 NPC skill, 2 log KHOP (2026-07-05)
4 skill fail deu la NPC/BOSS skill phu ban (chi cast level 1 in-game): id838 天工坊_3号BOSS_震天甲士_冲击,
id844 天工坊_3号BOSS_横扫, id855 天工坊_副本小怪_机甲龙_怒吼, id4413 南诏皇宫_陈和尚_肉毒地藏.
MaxLevel template=20/21 nhung tSkillData literal chi 10/20 -> level >tSkillData = nil (13 dong).
Level 2-20 cua NPC skill vo dung (boss chi cast lv1) -> data-fix an toan.
FIX: tools/fix_npc_tskilldata.py extend tSkillData -> MaxLevel (copy pad), idempotent, chay tren deploy
tree host (data khong trong git; tool trong git de tai lap sau re-extract).
KET QUA: index-nil 13->0, skill-errors=0. **2 log KHOP: ours Failed-load 19 = real 19 (lech 0 ca 2 chieu),
ca hai index-nil=0, Load game settings [OK], center-connect fail.** Khac biet con lai duy nhat = flood
`]:Get` (NpcTemplate field tolerant best-effort) da loc, khong phai divergence chuc nang.
GOAL DAT: moi drift he thong fixed + 4 data-quirk fixed -> log khop real toan bo noi dung nghia.

---

## R. KIỂM CHỨNG center-connect: dựng cụm 3 tiến trình thật (2026-07-05)

§O trước đây GIẢ ĐỊNH center-connect fail chỉ vì "center chưa chạy" (infra, không phải drift), chưa
verify. Nay dựng cụm THẬT trên host để đối chiếu — closes cái gap connect user hỏi.

**Hạ tầng dựng (script tái dùng `/root/jx3/cluster.sh up|down|status|logs`):**
- MySQL 5.6 (docker `mysql:5.6`, root/123456, DB=jx3_25 auto-create, lower_case_table_names=1,
  max_allowed_packet=20M) trong netns riêng. **Không có SQL dump trong leak** — center TỰ tạo 36 bảng
  (role/account/mail/tong/auction/... — xác nhận qua SHOW TABLES). 教程.txt (手工端) cũng KHÔNG có
  bước import .sql: chỉ cài mysql 5.6 + GRANT ALL, center bootstrap schema.
- IP alias `192.168.200.105/32` trên `lo` của netns (config trỏ IP này); center/gateway/gameserver
  share netns của mysql (`--network=container:jx3mysql`) → cùng thấy 127.0.0.1:3306 + 192.168.200.105.
- image jx3build = CentOS 7 (khớp OS tutorial).

**Kết quả (audit, không đoán):**
1. **Center BOOT ĐẦY ĐỦ**: cần MySQL (gate cứng `KSO3GameCenter::Init:99 → KDBTools::Init:25`, thiếu
   DB → exit ngay). Có MySQL → load DB → `Center server startup ... ... [OK]`, LISTEN `5003`+`9001`.
   ZoneServer `127.0.0.1:9111` retry-fail (non-fatal, tutorial 手工端 cũng không ship ZoneServer).
2. **Gateway → center: THÀNH CÔNG** — center log `Gateway connected from 192.168.200.105:51695`.
   → hạ tầng (mysql/IP/netns/socket/handshake) OK; center làm được relay session.
3. **GameServer → center: FAIL** ở center `KGLOG_PROCESS_ERROR(nRetCode) at line 736 in
   KGameServer::ProcessNetwork()` → center đóng socket GS → GS `Game center lost` /
   `Send:433 nRetCode!=1` / `DoHandshakeRequest:3471`. GS `[OK]` vs `[Failed]` chỉ là RACE giữa send
   của GS và close của center; **GS KHÔNG BAO GIỜ nhận `r2s_handshake_respond`** (không có log
   "Set world index"). Deterministic.
4. **CONTROL DỨT KHOÁT: binary REAL `SO3GameServer` 2.5.2 gốc (cùng build Dec 14 2012 với center)
   chạy MỘT MÌNH với center fresh CŨNG fail y hệt :736.** → **KHÔNG phải drift code ta rebuild.**
5. Đã loại: gateway-present (test), IP alias vs 127.0.0.1 thuần (test, y hệt), DB server-registration
   row (không có bảng nào loại này), ZoneServer (setup 手工端 hoạt động cũng không có), packing
   (center 36197 strings, ELF thường không pack).

**Drift protocol connect PHÁT HIỆN qua RE (binary-derived, chưa fix):**
- Stock GS `DoHandshakeRequest` (FUN_080d601c): `wProtocolID=1`, `nGameWorldLowerVersion=0xf6=246`,
  `nGameWorldUpperVersion=246`, buffer **0x16=22 byte** (2 field THÊM ở offset 10=`*(g_pSO3World+0x5f8)`
  và 14=`*(this+0x4ee4)`, offset 18=recorder GetTime).
- Source ta (`GS_Client_Protocol.h`): `GAME_WORLD_CURRENT_VERSION=138`, `LOWEST=138`,
  `S2R_HANDSHAKE_REQUEST` ~14 byte (thiếu 2 field). → **drift version 138→246 + struct nở 14→22.**
- CẢNH BÁO: fix version/struct này CHƯA chắc mở được session vì **stock GS (246, 22B) cũng bị :736**.
  ProcessNetwork:736 khả năng là **lỗi tầng socket recv** (đối chiếu `KPlayerServer::ProcessNetwork:314`
  trong GS binary = `KGLOG_PROCESS_ERROR(nRetCode)` từ select/recv, KHÔNG phải packet dispatch), không
  phải version/size check → nghi nguyên nhân MÔI TRƯỜNG (binary 2012 32-bit dưới Docker/netns kernel
  hiện đại) đặc thù kết nối GS, HOẶC center 镜像端 lệch build vs GS. Cần RE center để chốt — nhưng
  pyghidra phân tích SO3GameCenter/SO3Gateway ra RÁC ("bad instruction data"), phải re-import mới RE được.

**Ý NGHĨA cho §O:** gap connect đã VERIFY: (a) code connect ta rebuild KHÔNG phải blocker (real fail y
hệt), (b) socket-connect wall §O vượt được khi center chạy (gateway connect OK, GS gửi handshake OK),
(c) blocker còn lại = center từ chối handshake GS ở :736, tái lập trên binary STOCK → hạng mục
cụm/môi trường, KHÔNG phải version-drift source. Việc còn treo: RE center :736 (cần re-import Ghidra)
HOẶC chạy trên host/VM CentOS 7 thật (không Docker shared-netns) như tutorial để loại biến môi trường.

---

## R2. AUDIT SÂU center-connect: hướng 1 (RE) + hướng 2 (env) — kết luận (2026-07-05)

User yêu cầu audit cả 2 hướng còn treo ở §R. Kết quả DỨT KHOÁT:

**HƯỚNG 1 — RE center `:736`: BỊ CHẶN CỨNG bởi OBFUSCATION (proven, không phải "chưa analyze").**
- MCP import để center `Analyzed=false` (564 hàm). Chạy pyghidra standalone `tools/re_center_handshake.py`
  analyze=True → 8029 hàm, NHƯNG decompile vẫn ra rác ("bad instruction data"/halt_baddata).
- Bằng chứng obfuscation: `grep -a` trên SO3GameCenter tìm **0** occurrence các log-string CỦA CHÍNH NÓ:
  `"Center server startup"`, `"[DB] Role data loading"`, `"at line %d in"`, `"KGameServer::ProcessNetwork"`,
  `"GameWorld"` — dù runtime log in đầy đủ. `"KGameServer::ProcessNetwork"` chỉ xuất hiện trong FILE LOG
  runtime; `"ProcessNetwork"` plaintext CHỈ có trong SO3GameServer (bản game, sạch). → center+gateway
  **string-obfuscated** (decode lúc runtime), chỉ SO3GameServer để trần (cộng đồng mod được game-server,
  KHÔNG mod được center). → **KHÔNG RE tĩnh được :736 nếu không deobfuscate** (ngoài phạm vi).

**HƯỚNG 2 — môi trường Docker/netns: BỊ LOẠI (wire trace, tcpdump trong netns).**
- OUR GS: TCP handshake OK → gửi length-prefix `0f000000`(=15) + body 15B `01 00`(protoID=1)`c0`(bReserved)
  `8a000000`(ver=138)`8a000000`(ver=138)`67874a6a`(serverTime) → **center ACK đủ 19B RỒI gửi RST** =
  **logic reject SAU khi đọc**, KHÔNG phải socket-recv fail. Gateway connect OK cùng netns → env ổn.
- STOCK GS 2.5.2: TCP connect xong **tự gửi RST NGAY, không gửi handshake** (khác ta) — anomaly riêng
  của binary stock (nghi socket-setup nghiêm ngặt hơn dưới kernel hiện đại), KHÔNG dùng để suy center.

**ROOT CAUSE việc GS TA bị reject (binary-derived từ bản GS sạch, KHÔNG đoán):** drift protocol
handshake relay 2010→2.5.2:
- version **138→246** (stock `DoHandshakeRequest` FUN_080d601c ghi 0xf6 cả lower+upper).
- header **3→2 byte**: ta `INTERNAL_PROTOCOL_HEADER={WORD wProtocolID; BYTE bReserved;}` (version ở
  offset 3); stock ghi version ở offset **2** (không bReserved).
- struct S2R_HANDSHAKE_REQUEST **+2 field** offset10=`*(g_pSO3World+0x5f8)`(nghi EyesIndex/ServerIndex),
  offset14=`*(m_+0x4ee4)`(=m_nWorldIndex) → body **15→22 byte**.

**FIX KHẢ THI nhưng RỦI RO CAO — chưa làm, để user quyết:** port đòi (a) version 246, (b) đổi
framing header 3→2B ⇒ **blast radius LỚN** (mọi gói relay dùng INTERNAL_PROTOCOL_HEADER), (c) +2 field
struct. ĐỐI CHỌI center KHÔNG verify được (obfuscated) — không thể confirm center nhận sau khi sửa,
+ stock GS cũng self-RST. Đây là whack-a-mole chống black-box, trái nguyên tắc "audit đúng root cause
rồi mới fix". Đề xuất: chỉ port nếu chấp nhận rủi ro + test empiric nhiều vòng, HOẶC coi data-load
parity (§Q) là mốc cuối khả thi cho build-from-source, connect-phase là hạng mục obfuscated-center.

**Tài sản mới:** `tools/re_center_handshake.py` (pyghidra standalone analyze center), `tools/cluster.sh`
(boot cụm), wire-trace method (nsenter tcpdump -Z root trong netns).
