# JX3 (剑网3) — Tài liệu kỹ thuật build server Linux từ source

> Tài liệu tham chiếu cho việc **build `SO3GameServer` (JX3 v2.5.2) từ source trên Linux**, dựng lại từ toàn bộ quá trình reverse + build của các phiên trước và phần khôi phục sau khi `/tmp` bị wipe.
> Phạm vi: engine đóng (link binary `.so`) + rebuild **logic game** từ source C++. Chỉ cho mục đích học tập/nghiên cứu.
> Vị trí: `/Volumes/ExData/game/jx3/linux-build/` (sibling của `source/`). Cập nhật: 2026-07-04.

---

## 0. TL;DR

- Mục tiêu dài hạn: một private server JX3 **sửa được** (moddable) chạy trên Linux, build từ source.
- Engine nền (`Engine_Lua5`) là **binary đóng** — không build lại, mà **link lại 3 file `.so`** có sẵn.
- Logic game (`SO3World`, 205 file `.cpp`) **có source** → compile lại thành object rồi link tĩnh vào `SO3GameServer`.
- Trạng thái hiện tại (đã verify, khớp chính xác lần build cuối của phiên gốc — cmd 306):
  **189/197 object compile · link còn 32 undefined.**
- Còn lại: **8 file compile (endgame compile)** + **32 undefined (endgame link)** — tất cả là lỗi mức header/khai báo, không phải kiến trúc.
- Build chạy trong Docker `centos:7` + GCC 4.8.5 + `-m32` (khớp ABI libstdc++ 2012 của `.so`).

---

## 1. Kiến trúc build & vì sao chọn SO3World

Đây là phần cốt lõi — hiểu nó thì không phải research lại. Mọi kết luận dưới đây rút từ chính các file `.vcproj` gốc trong `jx3dev-master/Source/`.

### 1.1 Server gồm những gì

JX3 server là **3 exe** (`ConfigurationType="1"` = application):

| Exe | Vai trò | Có logic game? |
|---|---|---|
| **`SO3GameServer`** | Tiến trình game world thật — nơi nhân vật/quái/skill/item/quest chạy | **CÓ** (toàn bộ) |
| `SO3Gateway` | Cổng kết nối — route/định tuyến client ↔ game server | Không |
| `SO3GameCenter` | Điều phối liên server (chuyển vùng, cross-server) | Không |

Chỉ **`SO3GameServer`** mang logic game. Muốn mod hành vi game (chỉ số, cơ chế, cap, hệ thống) → chỉ cần rebuild exe này. `Gateway`/`GameCenter` để nguyên binary shipped là chạy được.

### 1.2 Vì sao rebuild SO3World (và chỉ SO3World)

Điểm mấu chốt: **`SO3GameServer.exe` gần như rỗng logic** — nó chỉ là glue ~4 file (`Main.cpp`, `KSO3GameServer.cpp`, `KGameServerEyes.cpp`, `stdafx.cpp`). Toàn bộ hành vi game nằm trong **`SO3World`**, một project **`ConfigurationType="4"` = static library** sinh ra **`SO3WorldServer.lib`**, rồi được **link tĩnh thẳng vào exe**.

`SO3World` = 205 file `.cpp`: `KCharacter`, `KPlayer`, `KNpc`, `KItem`, `KBuff`, `KSkill`, `KScene`, quest, threat/AI, kinh tế, tổ đội... — **đây là "trái tim" game, và có FULL source**.

Ba lý do chọn đúng SO3World:
1. **Bắt buộc**: `SO3GameServer` không link nổi nếu thiếu `SO3WorldServer.lib`. Không có cách "bỏ qua".
2. **Đủ**: mọi logic game cần sửa đều ở đây. Các tầng khác hoặc là đóng (engine) hoặc không mang logic (gateway).
3. **Static-in-exe, không hot-swap được**: vì `SO3World` link **tĩnh** vào exe (không phải `.so` rời), **mọi thay đổi C++ = recompile SO3World + relink exe**. Không có đường "thả `.so` game-logic vào" như với engine. Đó là lý do phải dựng được toàn bộ pipeline compile+link này.

> Hệ quả cho link: khi mới compile được một phần SO3World, số "undefined reference" phồng lên KHÔNG phải vì thiếu engine, mà vì các **symbol của SO3World định nghĩa trong chính các file CHƯA compile** (vd `KPlayerServer::Do*`, `g_PlayerServer`). Compile nốt các file đó → undefined tự sập. Đây đúng là quỹ đạo 1332 → 32 đã thấy.

### 1.3 Dependency graph (SO3GameServer link gì)

Từ `SO3GameServer.vcproj` (Release / Debug):

```
SO3GameServer.exe  (ConfigurationType=1, glue ~4 file)
        │  link tĩnh
        ├── SO3WorldServer(D).lib   ← SO3World (CT=4 static lib) ─ LOGIC GAME, có source ★
        ├── Engine_lua5(D).lib      ← import-lib của Engine_Lua5.dll/.so ─ ENGINE, ĐÓNG ☒
        ├── common(D).lib           ← net/buffer/util nền ─ 1 phần đóng (tái dựng, §6.4)
        ├── Lua5(D).lib             ← Lua 5.1 ─ mã nguồn mở
        ├── lzo(D).lib              ← miniLZO/UCL (NRV2B) ─ mã nguồn mở
        ├── libmysql.lib            ← MySQL C connector
        └── Ws2_32.lib              ← Windows socket (Linux: thay bằng epoll/socket hệ thống)
```

`AdditionalLibraryDirectories` trỏ `../../../../Base/Lib`, `../../../Lib/Win32`, `../../../../DevEnv/Lib/*` — trong đó **`Base/Lib` và `DevEnv/Lib` nằm NGOÀI repo, không kèm** → đây là gốc rễ mọi lib nền "bị thiếu".

Phân tầng: **đáy** = `Engine_Lua5` (đóng: memory/thread/file/pak/hash/Lua-binding/UCL). **giữa** = `common`/`Lua5`/`lzo`/`mysql`. **đỉnh** = `SO3World` (logic game, có source) + `SO3Represent` (`ConfigurationType="2"` = DLL client-facing; server chỉ dùng **header/enum** của nó, KHÔNG compile `.cpp`) + glue `SO3GameServer`.

### 1.4 Ánh xạ kiến trúc Windows → Linux build của ta

Trên Linux không có tầng static-lib trung gian; ta **compile thẳng `.cpp` → `.o`** rồi link một cục. Ánh xạ từng thành phần:

| Khái niệm Windows (.vcproj) | Trong build Linux của ta |
|---|---|
| `SO3WorldServer.lib` (từ SO3World CT=4) | **189 file `.o`** compile từ `src/SO3World/Src/*.cpp` (skip 14 file AI chết) |
| glue exe `SO3GameServer` | 4 `.o`: `Main/KSO3GameServer/KGameServerEyes/stdafx` |
| `Engine_lua5.lib` + `Engine_Lua5.dll` | **3 `.so`** `libEngine_Lua5D/SO3EnumConvertorD/SO3ItemHouseD` (link `-l...`) |
| `common.lib` (phần không leak) | **`common_recon/`** tự viết: `kg_buffer.cpp` + `kg_socket.cpp` (§6.4) |
| `Lua5.lib` | header Lua 5.1 (§5) — binding thật nằm trong engine `.so` |
| `lzo.lib` | `-llzo2` hệ thống (trong image) |
| `SO3Represent` (DLL) | chỉ header (`-Isrc/SO3Represent/Src`), không compile |
| `Ws2_32.lib` | epoll/socket POSIX trong `kg_socket.cpp` |

Kết quả link mong muốn: `g++ -m32 obj/*.o -lEngine_Lua5D -lSO3EnumConvertorD -lSO3ItemHouseD -llzo2 -ldl -lpthread`.

### 1.5 Hai bề mặt mod (vì sao vẫn cần build C++)

- **Lua scripts** (`手工端/root/scripts/`, ~3M dòng): sửa **runtime, không cần build**. Toàn bộ scene 单机 dùng cách này. Nhưng chỉ chạm được hành vi mà engine/SO3World đã *expose ra Lua*.
- **C++ `SO3World`**: sâu hơn — chạm cơ chế lõi, cap, hệ thống mà Lua không với tới. **Bắt buộc rebuild + relink** (vì static-in-exe, §1.2). Đây là thứ pipeline này mở khóa.

### 1.6 Vì sao build khó (tóm)

Engine nền (`Engine_Lua5`) chỉ ship **binary** (`.dll`/`.so`), **không source** cho đời JX3 → sửa được C++ tầng game, không đụng được engine. Và KingSoft build Linux native (gpr2mak → Makefile → gcc) nhưng **build system Linux không leak** (chỉ có `.vcproj` MSVC) → phải **tự dựng lại pipeline compile/link** = chính tài liệu này.

- **Windows**: build được (Debug ngay bằng import-lib có sẵn; Release phải tái tạo import-lib từ DLL). Dùng `.sln/.vcproj` VS2005.
- **Linux**: bế tắc build-from-source nếu đòi source engine; **mở khóa** bằng cách reuse 3 `.so` + rebuild SO3World (§3).

---

## 2. Version map (đừng lẫn client/server/source)

| Thành phần | Version | Chi tiết |
|---|---|---|
| **Server** | 2.5.2 build **4503** (14/12/2012) | Linux 32-bit i386, cap `MaxPlayerLevel=100`, dùng `libEngine_Lua5D.so`. Logic static-in-exe. |
| **Client** | 2.5.2 build **4550** (26/01/2013) | DX9, engine = **KG3DEngine** (KingSoft in-house, KHÔNG phải Ogre). Là client khớp server 2.5.2. |
| Client cũ | 1.5.2 build 2015 | Bản 盛大 (Shanda), cap 70 — gói lv90/GPH/polo. KHÔNG ghép với server 2.5.2. |
| Source | pap2 (viticm) ~10/2013 | Là source JX3 thật (SO3World + SO3Represent + KG3DEngine + KGUI). |

"v2.5.2" mặc định chỉ **server**. Client và server cùng đời 2.5.2 nhưng khác build number.

---

## 3. Chiến lược: reuse 3 `.so`, rebuild 1 exe

Không build engine. Thay vào đó **link lại `SO3GameServer` từ source** dựa trên 3 `.so` engine dựng sẵn (bản `D` = debug, không strip, có DWARF):

| `.so` | Kích thước | Vai trò |
|---|---|---|
| `libEngine_Lua5D.so` | 10.5 MB | Engine nền: memory/thread/file/pak/Lua-binding/UCL |
| `libSO3EnumConvertorD.so` | 1.0 MB | Chuyển đổi enum game |
| `libSO3ItemHouseD.so` | 2.5 MB | Hệ thống item |

Tất cả **ELF32 i386**. Đã kiểm chứng (phiên gốc, "check-common"): bề mặt symbol ngoài-hệ-thống của exe shipped = 74 symbol, **100% resolve** bởi 3 `.so` này, 0 unresolved. Tức chỉ cần compile source và link vào là đủ — không thiếu engine symbol nào ở tầng nguyên tắc.

Nguồn 3 `.so`: `/Volumes/ExData/game/jx3/镜像端/extracted/root/`.

---

## 4. Môi trường build

**Docker image `jx3build:latest`** (id `ca338d2601e3`, 553 MB). Dockerfile:

```dockerfile
FROM --platform=linux/amd64 centos:7
# CentOS 7 EOL -> repoint yum sang vault archive
RUN sed -i -e 's|^mirrorlist=|#mirrorlist=|g' \
           -e 's|^#\s*baseurl=http://mirror.centos.org|baseurl=http://vault.centos.org|g' \
           /etc/yum.repos.d/CentOS-*.repo \
 && yum -y install gcc gcc-c++ make file which binutils \
                   glibc-devel.i686 libstdc++-devel.i686 zlib-devel.i686 && yum clean all
RUN yum -y install lzo-devel lzo-devel.i686 && yum clean all
WORKDIR /work
```

Vì sao **GCC 4.8.5** (mặc định CentOS 7): các `.so` build 2012 dùng libstdc++ ABI cũ (pre-C++11 `std::string`/`std::list` layout). GCC mới (ABI `cxx11`) sẽ mismatch layout → symbol/crash. 4.8.5 khớp.

**Cảnh báo môi trường:** trên máy Apple Silicon, Docker/OrbStack chạy x86 qua **emulation** — bất ổn dưới tải compile ~190 file ("unexpected EOF", VM exit → compile dở). Ngoài ra **virtiofs share volume macOS (case-insensitive) vào container Linux** gây race chập chờn (`ls include/Base/engine/` lúc thấy lúc không, vì `engine.h`/`Engine.h`/`engine/` đụng case). **Khuyến nghị: build+link bản hoàn chỉnh trên host x86_64 Linux thật** (ext4 case-sensitive, không emulation). Scaffold + script portable — copy `linux-build/` sang là chạy.

---

## 5. Cấu trúc scaffold & nguồn từng phần

`setup.sh` dựng lại toàn bộ cây build một cách idempotent. Neo mọi đường dẫn vào **một biến gốc** `JX3ROOT="$HERE/.."` (gốc `jx3/`), nên di chuyển cả bộ sưu tập không phải sửa script.

```
linux-build/
├── setup.sh              # dựng scaffold (chạy trên host)
├── build.sh             # compile + link (chạy trong container jx3build)
├── Dockerfile           # định nghĩa image (đã build sẵn)
├── mkshim.py            # sinh symlink #include case-compat
├── compat/              # shim Windows→Linux: windows.h tchar.h conio.h curses.h prelude.h
├── devenv/include/      # luaapi.h (TÁI DỰNG)
├── include/
│   ├── Include/         # ← source/jx3dev-master/Include        (132 .h)
│   └── Base/            # ← Jx3D-master/source/Base/include      (203 .h, +KTextFilter +lua5)
├── src/
│   ├── SO3World/        # ← jx3dev-master/Source/Common/SO3World (205 .cpp)
│   ├── SO3Represent/    # ← jx3dev-master/Source/Common/SO3Represent
│   ├── SO3GameServer/   # ← jx3dev-master/Source/Server/SO3GameServer (glue: Main/KSO3GameServer/KGameServerEyes/stdafx)
│   └── common_recon/    # kg_buffer.cpp kg_socket.cpp (TÁI DỰNG)
├── libs/                # 3 .so (← 镜像端/extracted/root/)
├── Source/Common/       # BRIDGE symlink -> src/ (giải thích §7)
├── shim/                # sinh bởi mkshim.py (~218 symlink)
└── obj/                 # object output (regenerable)
```

Nguồn cố định (đều dưới `$JX3ROOT`):

| Phần | Đường dẫn |
|---|---|
| src (SO3World/Represent/glue) | `source/jx3dev-master/Source/{Common,Server}/` |
| Base headers (194 gốc) | `source/JX3-AIO/JX3_Download/Jx3D-master/source/Base/include` |
| Lua 5.1 headers | `source/Sword3-FullSource-master/.../DevEnv/Include/Lua5` |
| KTextFilter.h | `source/Sword3-FullSource-master/.../Include/Engine` |
| 3 `.so` | `镜像端/extracted/root/` |

---

## 6. Các file TÁI DỰNG (phần không sinh lại được)

Đây là phần quan trọng nhất — những file **không có trong bất kỳ leak nào**, phải viết tay/tái dựng. Lưu tách ở `_recovered/` (12 file), `setup.sh` overlay chúng **sau cùng** để đè lên header thật.

### 6.1 Header engine stub — `include/Base/engine/{kmemory.h, KLuaScriptEx.h}`
Base thật CÓ `KMemory.h`/`KLuaScriptEx.h`, nhưng `KMemory` thật là **namespace với `kstring` = `basic_string` pooled trên `KMemory::KAllocator`**. Áp bản thật vào cây stub làm **regress build 178→53** (pooled-kstring vs `std::string` mà phần còn lại dùng → mismatch type hàng loạt). Nên dùng **stub `std::string`-compatible** (`kmemory.h` viết tay, class form, `KMemory::X` resolve như namespace form).

**Cơ chế đè:** volume ExData **case-insensitive** → ghi `kmemory.h` **đè lên** `KMemory.h` (cùng file). `setup.sh` copy Base thật trước, overlay stub sau → stub thắng. (Đây là "trick" tình cờ nhưng cần thiết.)

### 6.2 `devenv/include/luaapi.h` — TÁI DỰNG (mất thật)
`luaapi.h` **mất khỏi mọi leak** (jx3dev, JX3-AIO, songpo/Sword3-FullSource, 909160) — được reference khắp nơi, không chứa ở đâu. Tái dựng từ SwordOnline `LuaExtend.c` (API `Lua_*` của KingSoft = alias mỏng trên `lua_*` gốc). Include `lua.h`/`lualib.h`/`lauxlib.h`.

### 6.3 `compat/*.h` — shim nền tảng Windows→Linux
- `prelude.h`: force-include đầu mọi TU. Định nghĩa `__int64/__int32/...`, kéo `<string>/<vector>/<map>/...` + `using namespace std` (MSVC leak std vào global, gcc không). `typedef std::string kstring`.
- `windows.h`, `tchar.h`, `conio.h`, `curses.h`: stub các API Windows-only.

### 6.4 `src/common_recon/{kg_buffer.cpp, kg_socket.cpp}` — TẦNG NETWORK TÁI DỰNG
Tầng `common` (net/buffer) **source không leak** — chỉ có interface header (`kg_socket.h`/`kg_package.h`/`kg_memory.h`, có sẵn Linux epoll path). Viết lại từ interface:
- `kg_buffer.cpp`: COM `IKG_Buffer`/`IKG_Buffer_ReSize` impl + `KG_MemoryCreateBuffer/Init/UnInit` + GUID (lấy từ comment header) + `IID_IUnknown`.
- `kg_socket.cpp`: `IKG_SocketStream` trên fd với **framing 4-byte length little-endian**; `KG_SocketConnector` (TCP); `KG_SocketServerAcceptor` (epoll); `KG_Packer`.

**Caveat protocol quan trọng:** encode/decode hiện = **pass-through NONE**. Framing/crypto ở tầng socket là **phần ĐOÁN**, phải validate với **client 2.5.2 thật** sau. Nội dung packet (protocol tầng trên) thì có sẵn trong `SO3World` — chỉ tầng khung socket là tái dựng.

### 6.5 `mkshim.py` — shim #include case-compat
Windows/macOS resolve `#include "Engine/KMemory.h"` với file on-disk `engine/kmemory.h` vì FS case-insensitive; Linux thì không. `mkshim.py` quét mọi `#include "..."`, với target không khớp case chính xác thì tạo symlink `shim/<target-case-đúng>` → file thật (target dạng `/work/...` container-absolute). `-Ishim` trong INC làm fallback. Sinh ~218 symlink.

---

## 7. Quy trình dựng (`setup.sh`) & các bẫy

Các bước (idempotent, `rm -rf` rồi lay lại mỗi lần):

1. **Sanity check** nguồn tồn tại.
2. **Bulk source**: copy SO3World/Represent/glue vào `src/`.
3. **Header roots**: copy `Include` → `include/Include`, Base thật → `include/Base`. Sau đó **Lua 5.1 headers** lay 2 kiểu:
   - phẳng (`include/Base/lua5/lua.h`) cho `luaapi.h` dùng `#include "lua.h"`
   - subdir (`include/Base/lua5/lua5/lualib.h`) cho `KLuaScriptEx.h` dùng `#include <lua5/lualib.h>`
   > **Bẫy #1:** thiếu dir `lua5` → **mọi** TU fail `fatal error: lua.h`.
4. **3 `.so`** → `libs/`.
5. **Overlay `_recovered/` (SAU CÙNG)**: Dockerfile, mkshim.py, compat/, devenv/, engine stubs, **KTextFilter.h**, common_recon. Thứ tự sau cùng để stub đè header thật.
   > `KTextFilter.h`: Base tree Jx3D **thiếu** header này (`KPlayerServer`/`KBaseFuncList`/`KLuaMailClient` cần) → lấy từ FullSource engine headers.
6. **Bridge original-layout**: một số header trong `Include/` dùng include tương đối kiểu `#include "../../Source/Common/SO3World/Src/KPlayer.h"` — chỉ resolve khi `Include/` và `Source/` là anh em (layout gốc). Cây tách (`include/` vs `src/`) không có. Tạo symlink:
   ```
   Source/Common/SO3World     -> ../../src/SO3World
   Source/Common/SO3Represent -> ../../src/SO3Represent
   ```
   Resolve qua `-Iinclude/Include` → `include/Include/../../Source/Common/...` = `/work/Source/Common/...`.
   > **Bẫy #2 (lớn nhất):** thiếu bridge → **132/197 fail** (KCharacter/KItem/KBuff/KPlayer/KNpc...).
7. **mkshim.py** sinh shim.

> **Bẫy #3:** **KHÔNG** tạo `ln -sf engine include/Base/Engine` / `Common`. Lệnh build cuối của phiên gốc (cmd 306) không có chúng — include chữ hoa resolve qua `mkshim`. Trên mount case-insensitive, symlink này đụng với dir `engine/`/`common/` thật → OrbStack virtiofs flake.

---

## 8. Quy trình compile + link (`build.sh`)

Chạy trong container `jx3build`, mount `linux-build/` tại `/work`. Khớp **chính xác** cmd 306 của phiên gốc.

**Flags:**
```
M32   = -m32 -I/usr/include/c++/4.8.2/i686-redhat-linux
FLAGS = $M32 -include compat/prelude.h -c -w -fpermissive -std=gnu++98 \
        -D__linux -D_SERVER -D_STANDALONE $INC
```

**Include path (INC), thứ tự nguyên bản:**
```
-Isrc/SO3GameServer -Isrc/SO3World -Isrc/SO3World/Src -Ishim -Icompat
-Iinclude/Include -Iinclude/Include/SO3World -Iinclude/Base -Iinclude/Base/engine
-Iinclude/Base/common -Iinclude/Base/lua5 -Idevenv/include
-Isrc/SO3Represent/Src -Iinclude/Include/SO3Represent
```

**DEAD set — 14 file AI cũ, LOẠI khỏi compile:**
```
KAIBase KAIBaseTactics KAIControl KAIEvent KLuaAIEvent KAISkillSelect
KAI_Negative KAI_Critter KAI_Monk KAI_Positive KAI_Retardate KAI_Wolf KAI_WolfKing KAI_Wood
```
Vì sao loại: server 2022 thật dùng **AI dạng VM** (`KAIVM/KAIAction/KAILogic/KAIState/KAIManager/KAISearchTactics/KAISkillSelector`). Họ `KAIBase/KAIController/KAI_*` là **legacy chết**, server thật không compile. `KCharacter.h` wire AI qua `KAIVM m_AIVM;` (dòng 1377); `friend class KAIBase/KAIController` (1224-25) chỉ là khai báo vô hại không sinh symbol.

**Vòng compile:** `rm -rf obj; mkdir obj` → compile 205−14 file SO3World (skip DEAD) → 4 glue → 2 recon (`kg_buffer`, `kg_socket`).

**Link:**
```
g++ -m32 obj/*.o -L libs -lEngine_Lua5D -lSO3EnumConvertorD -lSO3ItemHouseD \
    -llzo2 -ldl -lpthread -o /tmp/SO3GameServer
```

---

## 9. Trạng thái hiện tại (đã verify)

**189/197 object compile · link 32 undefined.** Khớp **byte-cho-số** với lần build cuối phiên gốc (cmd 306, transcript event 3465): cùng 189 object, cùng 32 undefined, cùng danh sách symbol.

> Lưu ý: các số 18/14 thấy giữa phiên gốc (cmd 303/304) là thí nghiệm trên `obj/` **bẩn** (còn `.o` tích lũy) — KHÔNG phải số canonical. Bản `rm -rf obj` sạch = 189/32.

---

## 10. Endgame compile — 8 file còn lại

Tất cả lỗi mức header/khai báo. Sửa nên đặt thành **bước patch trong `setup.sh`** (source upstream giữ nguyên).

| File | Vai trò game | Lỗi | Hướng fix |
|---|---|---|---|
| **KDoodadFile** | I/O file doodad (vật thể tĩnh cảnh) | `_NAME_LEN` chưa khai báo (`IDoodadFile.h`) | thêm `#define _NAME_LEN 32` (có sẵn trong nhiều header khác, chỉ chưa include tới) |
| **KNpcFile** | I/O file NPC template | `_NAME_LEN` (`INpcFile.h`) | như trên |
| **KGMList** | Danh sách GM + quyền | `_NAME_LEN` | như trên |
| **KMissile** | Đạn/phi tiêu (projectile skill) | member `m_nExistingFrame` thiếu ở `KMissile.h` | thêm member vào header |
| **KNpcTeam** | Tổ đội NPC (bầy quái) | type `KThreatNode` chưa khai báo | include đúng bản `KThreatList.h` (bản server cũ có `KThreatNode`) |
| **KPathFinder** | Tìm đường NPC | `KRegion::GetProximalCellForObj(...)` lệch chữ ký | khớp lại overload trong `KRegion.h` |
| **KProtocolRecorder** | Ghi gói protocol (debug/replay) | `in_addr` không tên type (`KProtocolRecorderBase.h:134`) | thêm `#include <netinet/in.h>` |
| **KTrackList** | Danh sách track (theo dõi vị trí) | `KTRACK_INFO` thiếu member `dwMapID` | thêm member vào struct |

Nguồn `_NAME_LEN` đã xác nhận có ở 5 header khác nhau trong cây (`#define _NAME_LEN 32`). `KThreatNode` có trong nhiều file AI cũ — cần bản `KThreatList.h` phù hợp.

---

## 11. Endgame link — 32 undefined

Sau khi 8 file trên compile, số undefined sẽ thay đổi (KGMList/KDoodadFile/KNpcFile/KProtocolRecorder tự resolve khi có object của chúng). Nhóm undefined hiện tại (32 ref, 26 symbol distinct):

- **`CRC32`** — **THẬT RA có export trong `.so`** (`nm -D --defined-only libs/*.so | grep -c CRC32` = 1). Undefined chỉ vì **lệch mangling/khai báo C++**. Cần **khai báo extern đúng**, KHÔNG phải viết lại impl. (Đính chính so với ghi chú cũ.)
- **`KAIBase::*`** (18 hàm: `Activate/CheckAttackTarget/DoIdle/Escape/7×GetSkillExp_*/Init/InitSkillCD/OnClearThreat/OnFollow/OnPathResult/UnInit`) + `typeinfo/vtable KAIBase` — `KAIBase.cpp` nằm trong DEAD set nên không compile, nhưng **một file LIVE vẫn reference** các hàm này ("old-AI header ref cần pin"). Cần tìm file live include và cắt tham chiếu, hoặc thêm stub.
- **`KAIEvent::KAIEvent()` / `~KAIEvent()`** — tương tự, tham chiếu tới AI-cũ còn sót.
- **`CreateDoodadFileInterface()` / `CreateNpcFileInterface()`** — factory, resolve khi `KDoodadFile`/`KNpcFile` compile.
- **`KGMList::CheckPermission` / `Load`** — resolve khi `KGMList` compile.
- **`vtable for KProtocolRecorder`** — resolve khi `KProtocolRecorder` compile.

Nghĩa là ~10 undefined tự tan khi endgame compile xong; còn lại chủ yếu là **cụm KAIBase::* cần pin tham chiếu AI-cũ** + **CRC32 cần khai báo đúng**.

---

## 12. Sau khi link — chạy server (mốc riêng)

Link ra binary **chưa** phải là server chạy. Cần thêm:
1. **Cây data** `手工端/root/` (Lua scripts ~3M dòng + tab + config + 3 `.so`).
2. **MySQL** (schema server).
3. **Client 2.5.2 thật** để **validate framing/crypto socket** (phần tái dựng ở §6.4 đang đoán NONE).

Đây là 3 mốc tách biệt sau "link hoàn tất".

---

## 13. Khôi phục nếu mất lại

Thư mục build từng ở `/tmp` và **mất khi reset máy**. Đã dời ra `linux-build/` (persistent). Nếu mất nữa, ba nguồn khôi phục:
- **Docker image `jx3build`** — còn trong Docker; rebuild từ `Dockerfile`.
- **File-history của Claude Code** (`~/.claude/file-history/<session>/`) — chụp mọi Write/Edit, đặt tên `sha256(path)[:16]@vN`. Map ngược path → lấy file tái dựng.
- **Transcript chat** (`~/.claude/projects/<proj>/<session>.jsonl`) — lưu nguyên văn mọi command → dựng lại `setup.sh`/`build.sh`.

12 file trong `_recovered/` là phần **không sinh lại được** — sao lưu riêng.

---

## 14. Reference — vị trí file & sự thật RE đã verify

**Đường dẫn chính:**
- Build workspace: `/Volumes/ExData/game/jx3/linux-build/`
- Source jx3dev: `source/jx3dev-master/`
- Base headers: `source/JX3-AIO/JX3_Download/Jx3D-master/source/Base/include`
- FullSource (Lua5 + KTextFilter): `source/Sword3-FullSource-master/Jx3Full/Source/Source/base/base/`
- 3 `.so`: `镜像端/extracted/root/`
- Binary server Linux tham chiếu: `镜像端/extracted/root/`, `手工端`
- Tài liệu tổng quan Windows/Linux: `source/JX3-BUILD-GUIDE.md`
- Công cụ pak (đã cứu): `_packPAK_recovered/jx3-{unpack,pack}-v1.py`, `jx3-pak-format-research.md`

**Sự thật reverse đã verify trên binary:**
- `g_FileNameHash` (disasm `libEngine_Lua5.so`) khớp **byte-exact** với `v1_hash` trong `jx3-unpack-v1.py`: hằng `0x8000000b`, `0xffffffef`, `^0x12345678`, chuẩn hóa `/`→`\`.
- `CD_LCU_C` → gọi `ucl_nrv2b_99_compress` → nén = **UCL/NRV2B**. `lzo1x_decompress` cũng có mặt (PakV3 dùng LZO1X).
- Bề mặt symbol ngoài của exe = 74, resolve 100% bởi 3 `.so`.

**Toolchain gốc (Windows):** VS2005 (`.vcproj Version="8.00"`), `CharacterSet=MultiByte`, DirectX SDK cũ, PhysX cũ. Trần version engine ≈ 2012 (v2.5.2) — đóng băng.

---

## 15. Lệnh nhanh

```bash
cd /Volumes/ExData/game/jx3/linux-build
./setup.sh          # dựng scaffold (idempotent)
./build.sh          # compile + link trong container jx3build
grep -E 'COMPILE:|link exit' _build.log   # xem kết quả

# rebuild image nếu mất:
docker build --platform linux/amd64 -t jx3build .
```
