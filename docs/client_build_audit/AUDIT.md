# AUDIT — danh sách lib phụ thuộc client JX3 + đường vá

Ngày: 2026-07-08. Nguồn build đích: **client build 1834 (1.5.x)** — KHÔNG có exterior.
Cây chính: `source/jx3dev-master/Source/Universe.sln` (34 project, `.vcxproj`).
Cây đối chiếu: `source/Sword3-FullSource-master/Jx3Full/Source/Source/Universe.sln` (cùng build 1834, tự-chứa hơn).

## A. Chuỗi project client trong Universe.sln

Client EXE = `SO3Client`. Phụ thuộc (build từ source, đều CÓ trong repo):

| Project | vai trò | .cpp (jx3dev) | source? |
|---|---|---|---|
| SO3Client | client exe | 27 | ✓ |
| SO3UI | UI layer → JX3UI.lib | 16 | ✓ |
| SO3Interaction | → JX3Interaction.lib | 4 | ✓ |
| CheckGameCheatLib | anti-cheat | 7 | ✓ |
| JX3Launcher | launcher | 6 | ✓ |
| KG3DEngine | graphics engine | 339 | ✓ FULL |
| KGUI | GUI toolkit | 79 | ✓ |
| SO3Represent | → JX3Represent.lib | 68 | ✓ |
| SO3World | game world | 249 | ✓ |
| KG3DSound / IPPVideo / JpegLib / KMp3LibClass / KG3DShaderLibrary / zlib / pnglib | engine sub-libs | — | ✓ (project trong sln) |

Toàn bộ code C++ client-chain có source. Không có gap ở tầng .cpp.

## B. Link-line THẬT (SO3Client, cấu hình Debug — trích `SO3Client.vcxproj`)

```
d3d9.lib;dinput8.lib;winmm.lib;uuid.lib;Rpcrt4.lib;atlsd.lib;commonD.lib;
Engine_lua5D.lib;RainbowD.lib;SO3WorldClientD.lib;dxguid.lib;d3dx9.lib;
gdiplus.lib;WS2_32.lib;Lua5D.lib;odbc32.lib;odbccp32.lib;Dxguid.lib;
Msimg32.lib;AHClientInterface.lib;lzoD.lib
```

LibDirs: `../../../../Base/Lib;../../../Lib/Win32;../../../../DevEnv/Lib/Debug/FreeType2;../../../../DevEnv/Lib/debug;../../../Lib/Win32/apex`

Các `Base/Lib` + `DevEnv/Lib` này KHÔNG có tại đường tương đối trong `jx3dev-master` (memory cũ ghi "missing"), NHƯNG tồn tại đầy đủ dưới:
`source/JX3-AIO/JX3_Download/Jx3D-master/source/Base/Base/lib/debug/` và `.../Jx3D/DevEnv/lib/debug/`.
→ gap là **đường dẫn/gom lib**, không phải thiếu file (trừ 1 lib thật, mục D).

## C. Map từng NON-SYSTEM lib (Debug) → vị trí thực

| Lib (Debug) | Trạng thái | Nguồn |
|---|---|---|
| commonD.lib | ✓ CÓ | `Jx3D-master/source/Base/Base/lib/debug/commond.lib`; cũng `jx3-server-build-libs/lib_from_Base7z/debug/` |
| Engine_lua5D.lib | ✓ CÓ | cùng chỗ trên |
| SO3WorldClientD.lib | ✓ CÓ | `jx3dev-master/Lib/Win32/SO3WorldClientD.lib` |
| Lua5D.lib | ✓ CÓ | `Jx3D-master/Jx3D/DevEnv/lib/debug/lua5d.lib` |
| AHClientInterface.lib | ✓ CÓ | `jx3dev-master/Lib/Win32/apex/` (và AIO) |
| KG3DSoundD.lib | ✓ CÓ | `jx3dev-master/Lib/Win32/` |
| JpegLibD.lib | ✓ CÓ | `jx3dev-master/Lib/Win32/` (hoặc build từ JpegLib.vcxproj) |
| CheckGameCheatLibD.lib | ✓ build từ source | project trong sln |
| **RainbowD.lib** | ✗ **THIẾU THẬT** | không có .lib/.dll ở đâu — mục D |
| **lzoD.lib** | ✗ tên lệch | chỉ có `DevEnv/lib/debug/lzo2.lib` (LZO2) — đổi tên/point |
| **freetype235_D.lib** | ✗ version lệch | có `Freetype-2.3.9_d.lib` (2.3.9) + `DevEnv/lib/debug/FreeType2/freetype220_D.lib` (2.2.0); KHÔNG có bản 2.3.5 |
| **pnglibd.lib** | ✗ tên lệch | có `Base/lib/debug/libpngd.lib`; hoặc build từ `pnglib.vcxproj` (source trong sln) |
| **zlibd.lib** | ✗ tên lệch | có `DevEnv/lib/debug/zlib.lib`; hoặc build từ `zlib.vcxproj` (source trong sln) |
| atlsd.lib | ⚠ toolchain | ATL static — đi kèm VS2008 (Pro/có ATL), KHÔNG có trong leak |

### Lib theo project khác trong chuỗi (Debug)
- KG3DEngine: `engine_Lua5D, commonD, freetype235_D, pnglibd, zlibd, PhysXLoader, JpegLibD, ddraw, d3d9/x9` → gap giống trên (freetype235_D, pnglibd, zlibd) + PhysXLoader ✓ (`DevEnv/lib/debug/PhysXLoader.lib`).
- SO3Represent: `atlsd, d3dx9, Engine_lua5D, lzoD` → lzoD gap.
- SO3UI: `atlsd, commonD, Engine_lua5D, Lua5D, KG3DSoundD, CheckGameCheatLibD, imm32` → hết ✓.
- KG3DSound: `Engine_Lua5D, KMp3LibClassD, dsound, dxguid` → ✓.
- SO3Interaction: `Engine_lua5D, Wininet` → ✓.

## D. System libs (DXSDK + Windows SDK — không phải gap repo)

`d3d9 d3dx9 dinput8 dsound ddraw dxguid winmm imm32 uuid Rpcrt4 WS2_32 Wininet gdiplus odbc32 odbccp32 Msimg32 kernel32` — từ **DirectX SDK (June 2010)** + **Windows/Platform SDK**.

Quan trọng: cây **Sword3-FullSource** đã **bundle sẵn DirectX SDK libs in-tree** (`Jx3Full/DirectX/lib/`: d3d9, d3dx9, d3dx9d, dinput8, ddraw, dxguid, dxof…) + `Jx3Full/PhysX/include`. → nếu build từ cây này, khỏi tự cài DXSDK.

## E. Gap THẬT sau khi gom (chỉ Debug)

Ranked:

1. **RainbowD.lib** — HARD, nhưng **KHÔNG còn phải RE**. Có **full source SwordOnline** tái dựng được:
   `source/JX3-AIO/JX3_Download/剑网3Online+客戶/Sources/Network/Rainbow/`
   — `IClient.cpp, ClientStage.cpp, Rainbow.cpp, Rainbow.def, StdAfx, ESClient/ (TCP layer 14 file), Interface/IClient.h+RainbowInterface.h, Protocol/protocol.h`, kèm **`Rainbow.dsp`** (project VC6). Interface JX3-era: `Jx3D-master/source/Base/include/net/win32/iclient.h` + `rainbow_linklib.h`.
   → compile source này thành `RainbowD.lib` khớp `iclient.h`. Đây cũng là **loop-closer** với server `kg_socket.cpp` (framing/crypto guessed) — Rainbow là đầu kia.

2. **lzoD.lib** — đổi tên/point `lzo2.lib` (LZO2 có sẵn). Rủi ro: nếu source gọi API LZO1x thì cần lib LZO đúng; kiểm khi linker báo unresolved.

3. **freetype235_D.lib** — version lệch. Có 2.3.9 và 2.2.0, KHÔNG có 2.3.5. Thử point 2.3.9 trước (ABI freetype ổn định trong 2.3.x); nếu lỗi symbol → build freetype 2.3.5 từ source.

4. **pnglibd.lib / zlibd.lib** — build từ `pnglib.vcxproj` + `zlib.vcxproj` (đều trong sln); hoặc point `libpngd.lib` + `DevEnv zlib.lib`.

5. **atlsd.lib** — cài VS2008 có ATL (Professional). Không tái dựng được từ leak.

6. **Engine_lua5.lib (Release)** — chỉ có bản Debug. **Không ảnh hưởng build Debug**; chỉ chặn Release (giống server).

## E-bis. VERIFY interface Rainbow (đối chiếu source ↔ header ↔ prebuilt lib)

Đã kiểm để xác nhận đường vá #1 (build RainbowD.lib từ source SwordOnline) khớp cái client cần:

1. **Interface `IClient` KHỚP BYTE-FOR-BYTE.** `iclient.h` JX3-era (`Jx3D-master/source/Base/include/net/win32/iclient.h`) và `Rainbow/Interface/IClient.h` (SwordOnline) có **cùng 7 method, cùng thứ tự, cùng chữ ký**: `Startup, Cleanup, ConnectTo(const char*const&, unsigned short), RegisterMsgFilter(LPVOID, CALLBACK_CLIENT_EVENT), SendPackToServer(const void*const&, const size_t&), GetPackFromServer(size_t&)→const void*, Shutdown`. GUID `IID_IESClient` **trùng khít** (`0xd38249a9,0x4565,0x4336,9f70...`). → lib dựng lại **ABI-compatible** với vtable client mong đợi.

2. **Entry-point NAME lệch — điểm PHẢI chốt khi link.** `Rainbow.def` (SwordOnline) chỉ export `QueryClientInterface @1`. Nhưng `iclient.h` JX3 khai báo free-func `STDAPI CreateInterface(...)` + `IClientFactory::CreateClientInterface(...)`. Tên hàm khởi tạo **khác nhau** giữa 2 thế hệ.

3. **KHÔNG source/prebuilt-lib nào ship gọi entry-point Rainbow theo tên.** Grep toàn 2 cây source (`*.cpp/*.h`): 0 include `iclient.h/rainbowinterface.h`, 0 gọi `CreateClientInterface/QueryClientInterface`. Consumer THẬT nằm compiled trong `SO3WorldClientD.lib` (strings lộ include `d:\code\game\base\include\net\win32\rainbowinterface.h` + tham chiếu `IClientFactory`) — NHƯNG dump `SO3WorldClientD.lib` + `commond.lib`: **0 hit** symbol `QueryClientInterface/CreateClientInterface` decorated. → tên export chính xác linker cần **chỉ biết được lúc link (LNK2019)** hoặc qua import-table `JX3Client.exe`.

**Hệ quả cho đường vá #1:** dựng lib từ source SwordOnline = OK về interface/vtable; rủi ro còn lại DUY NHẤT = **tên hàm export**. Nếu linker báo unresolved (vd cần `CreateInterface` thay vì `QueryClientInterface`), thêm alias trong `.def` cho khớp. Đây là lúc **đường #3 (RE JX3Client.exe import table)** có giá trị — không phải để lấy code, chỉ để đọc đúng tên symbol + framing.

## F. Chưa thực thi được (blocker môi trường)

Host hiện tại = **macOS arm64, KHÔNG có VS2008 / cl.exe / DXSDK / wine**. Client là Windows-only (DX9 + ATL + COM + `.vcxproj`) → **không link được ở đây**. Bước "build Debug cho linker liệt kê symbol thiếu" (giống server 1332→32) **cần host Windows + VS2008 + DXSDK June2010** — tương đương host Linux Linode đã dùng cho server. Đây là việc kế tiếp, không phải bế tắc thiết kế.
