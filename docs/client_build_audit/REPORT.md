# REPORT — Audit khả-build client JX3 (frontend)

Ngày: 2026-07-08 · Thời gian audit: ~12 phút · 7 lệnh khảo sát (không giải nén, không xoá).
Chi tiết lib/symbol: xem `AUDIT.md`.

## Kết luận 1 dòng

Client build **1834 (1.5.x, pre-exterior)** về cơ bản **build được ở Debug**. Sau khi gom lib rải rác, **gap thật chỉ còn 1**: `RainbowD.lib` — và gap này **KHÔNG còn phải RE**, đã có **full source SwordOnline** tái dựng. Vài lib còn lại chỉ là **tên/version lệch** (đổi tên hoặc build từ project có sẵn trong sln). Chưa link thử được vì **host này không có toolchain Windows**.

## 1. Buildability

Chuỗi project client (`SO3Client → SO3UI/SO3Interaction/CheckGameCheatLib/JX3Launcher/KG3DEngine/KGUI/SO3Represent/SO3World/...`) **có đủ source C++**: KG3DEngine 339 .cpp, SO3World 249, KGUI 79, SO3Represent 68, SO3Client 27. Không có gap tầng code.

Link-line Debug thật của `SO3Client` (21 lib) chia 3 nhóm:
- **System (DXSDK+WinSDK)**: 12 lib — có sẵn nếu cài DXSDK; hoặc dùng cây Sword3-Full đã bundle in-tree.
- **CÓ trong repo** (chỉ sai đường dẫn): commonD, Engine_lua5D, SO3WorldClientD, Lua5D, AHClientInterface, KG3DSoundD, JpegLibD — tất cả nằm dưới `JX3-AIO/.../Jx3D-master/{source/Base/Base/lib,Jx3D/DevEnv/lib}` (chính là `Base/Lib`+`DevEnv/Lib` mà vcxproj tham chiếu; memory cũ báo "missing" là do chỉ tìm ở đường tương đối).
- **Gap**: xem mục 3.

Xác nhận gap từ memory: `RainbowD.lib` = ĐÚNG (còn thiếu). `Engine_lua5.lib` release = ĐÚNG thiếu (chỉ có Debug) nhưng **không chặn Debug**.

## 2. So 2 cây source

Cùng build 1834, cùng 34-project Universe.sln. Khác biệt:

| | jx3dev-master | Sword3-FullSource/Jx3Full |
|---|---|---|
| Universe.sln + client chain | ✓ | ✓ |
| Sub-lib prebuilt (Lib/Win32) | ✓ 28 lib | (dùng base/ source) |
| **DirectX SDK libs in-tree** | ✗ (phải cài DXSDK) | ✓ `DirectX/lib` (d3d9,d3dx9,dinput8,ddraw,dxguid…) |
| **PhysX headers in-tree** | ✗ | ✓ `PhysX/include` |
| **base/ SOURCE** (build common/Engine_lua5 từ source) | ✗ (chỉ prebuilt lib) | ✓ `base/base.vcproj` |
| Per-project .sln (SO3Client.sln…) | ✗ | ✓ |

**Khuyến nghị:** dùng **Sword3-FullSource** làm cây build — tự-chứa hơn (SDK + base source bundled), giảm việc gom SDK. Lấy `jx3dev-master/Lib/Win32/*` làm nguồn prebuilt-lib dự phòng. Cả hai đều thiếu Rainbow như nhau.

## 3. Danh sách thiếu (Debug) + đường vá

| # | Lib | Loại | Vá |
|---|---|---|---|
| 1 | **RainbowD.lib** | THIẾU THẬT | Build từ source SwordOnline (mục 4) |
| 2 | lzoD.lib | tên lệch | point/rename `lzo2.lib` (DevEnv) |
| 3 | freetype235_D.lib | version lệch | thử `Freetype-2.3.9_d.lib`; lỗi symbol → build FT 2.3.5 |
| 4 | pnglibd.lib | tên lệch | build `pnglib.vcxproj` (trong sln) / point `libpngd.lib` |
| 5 | zlibd.lib | tên lệch | build `zlib.vcxproj` (trong sln) / point `zlib.lib` |
| 6 | atlsd.lib | toolchain | cài VS2008 có ATL (Professional) |
| — | Engine_lua5.lib (Release) | release-only | không cần cho Debug |

Danh sách symbol thiếu THẬT chỉ có sau khi link thử (mục 5) — giống server 1332→32.

## 4. RainbowD.lib — đánh giá 3 đường

Kết quả khảo sát **đảo ngược** giả định "RE bắt buộc" trong memory:

- **Đường 2 (forward-port SwordOnline) — KHUYẾN NGHỊ.** Có **full source hoàn chỉnh** tại
  `JX3-AIO/JX3_Download/剑网3Online+客戶/Sources/Network/Rainbow/`: `IClient.cpp, ClientStage.cpp, Rainbow.cpp, Rainbow.def`, tầng TCP `ESClient/` (14 file), `Interface/{IClient.h,RainbowInterface.h}`, `Protocol/protocol.h`, kèm **`Rainbow.dsp`** (VC6 project — port lên VS2008 dễ). Code 2007 reused unchanged → divergence thấp. Compile khớp interface JX3-era `Jx3D-master/source/Base/include/net/win32/iclient.h` + `rainbow_linklib.h`.
- **Đường 1 (dựng lại từ iclient.h):** không cần nữa — API nhỏ nhưng đường 2 đã cho sẵn implementation, dựng tay là thừa.
- **Đường 3 (RE JX3Client.exe):** chỉ dùng để **cross-check framing/crypto** nếu đường 2 link nhưng handshake sai với server. Không phải đường chính.

**Đã VERIFY (xem AUDIT §E-bis):** interface `IClient` SwordOnline ≡ `iclient.h` JX3 **byte-for-byte** (7 method cùng chữ ký + GUID `IID_IESClient` trùng khít) → lib dựng lại ABI-compatible. Rủi ro còn lại DUY NHẤT = **tên hàm export** (`.def` SwordOnline = `QueryClientInterface`; header JX3 = `CreateInterface`/`IClientFactory::CreateClientInterface`) — không source/prebuilt-lib ship nào gọi theo tên nên chỉ lộ lúc link (LNK2019); vá bằng alias trong `.def`.

**Loop-closer:** server `kg_socket.cpp` hiện guessed framing/crypto NONE. Rainbow = đầu kia của cùng protocol. Tái dựng Rainbow từ `Protocol/protocol.h` (SwordOnline) buộc chọn framing → chính là cách chốt protocol còn đoán của server.

## 5. Việc kế tiếp (theo thứ tự)

1. Dựng host **Windows + VS2008 (có ATL) + DXSDK June2010** (mirror của Linode host cho server). Không thể link trên Mac arm64 này.
2. Cây Sword3-Full → mở `SO3Client.sln`/`Universe.sln`, cấu hình Debug, trỏ LibDirs vào `Base/lib/debug` + `DevEnv/lib/debug` + `Lib/Win32` + `DirectX/lib`.
3. Port `Rainbow.dsp` → project VS2008, build `RainbowD.lib` (đường 4).
4. Đổi tên/point lzoD/freetype/png/zlib (mục 3).
5. Build Debug → để **linker liệt kê unresolved symbol** → ghi vào `AUDIT.md` §symbols, lặp cho tới link sạch (giống server 1332→32).

## 6. Nhắc phạm vi

Source = **1834/1.5.x, KHÔNG có exterior**. Build này ra client 1.5.x, **không render ngoại trang**. Client-có-exterior là 2.5.2/4550 (chỉ có ở binary). Audit này chỉ mở đường build client 1834 để làm nền; exterior là việc RE riêng ([[kexterior-port-re]]).
